// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// joystick.c
// ============================================================================

#if defined(PLATFORM_FREEBSD)
#include <machine/joystick.h>
#endif

#include "joystick.h"
#include "misc.h"


/* ========================================================================= */
/* platform independent joystick functions                                   */
/* ========================================================================= */

#define TRANSLATE_JOYSYMBOL_TO_JOYNAME	0
#define TRANSLATE_JOYNAME_TO_JOYSYMBOL	1

void translate_joyname(int *joysymbol, char **name, int mode)
{
  static struct
  {
    int joysymbol;
    char *name;
  } translate_joy[] =
  {
    { JOY_LEFT,		"joystick_left" },
    { JOY_RIGHT,	"joystick_right" },
    { JOY_UP,		"joystick_up" },
    { JOY_DOWN,		"joystick_down" },
    { JOY_BUTTON_1,	"joystick_button_1" },
    { JOY_BUTTON_2,	"joystick_button_2" },
  };

  int i;

  if (mode == TRANSLATE_JOYSYMBOL_TO_JOYNAME)
  {
    *name = "[undefined]";

    for (i = 0; i < 6; i++)
    {
      if (*joysymbol == translate_joy[i].joysymbol)
      {
	*name = translate_joy[i].name;
	break;
      }
    }
  }
  else if (mode == TRANSLATE_JOYNAME_TO_JOYSYMBOL)
  {
    *joysymbol = 0;

    for (i = 0; i < 6; i++)
    {
      if (strEqual(*name, translate_joy[i].name))
      {
	*joysymbol = translate_joy[i].joysymbol;
	break;
      }
    }
  }
}

char *getJoyNameFromJoySymbol(int joysymbol)
{
  char *name;

  translate_joyname(&joysymbol, &name, TRANSLATE_JOYSYMBOL_TO_JOYNAME);
  return name;
}

int getJoySymbolFromJoyName(char *name)
{
  int joysymbol;

  translate_joyname(&joysymbol, &name, TRANSLATE_JOYNAME_TO_JOYSYMBOL);
  return joysymbol;
}

int getJoystickNrFromDeviceName(char *device_name)
{
  char c;
  int joystick_nr = 0;

  if (device_name == NULL || device_name[0] == '\0')
    return 0;

  c = device_name[strlen(device_name) - 1];

  if (c >= '0' && c <= '9')
    joystick_nr = (int)(c - '0');

  if (joystick_nr < 0 || joystick_nr >= MAX_PLAYERS)
    joystick_nr = 0;

  return joystick_nr;
}

char *getDeviceNameFromJoystickNr(int joystick_nr)
{
  static char *joystick_device_name[MAX_PLAYERS] =
  {
    DEV_JOYSTICK_0,
    DEV_JOYSTICK_1,
    DEV_JOYSTICK_2,
    DEV_JOYSTICK_3
  };

  return (joystick_nr >= 0 && joystick_nr <= 3 ?
	  joystick_device_name[joystick_nr] : "");
}

char *getFormattedJoystickName(const char *name_raw)
{
  static char name[MAX_JOYSTICK_NAME_LEN + 1];
  boolean name_skip_space = TRUE;
  int i, j;

  if (name_raw == NULL)
    name_raw = "(unknown joystick)";

  // copy joystick name, cutting leading and multiple spaces
  for (i = 0, j = 0; i < strlen(name_raw) && i < MAX_JOYSTICK_NAME_LEN; i++)
  {
    if (name_raw[i] != ' ')
    {
      name[j++] = name_raw[i];
      name_skip_space = FALSE;
    }
    else if (!name_skip_space)
    {
      name[j++] = name_raw[i];
      name_skip_space = TRUE;
    }
  }

  // cut trailing space
  if (j > 0 && name[j - 1] == ' ')
    j--;

  name[j] = '\0';

  return name;
}

static int JoystickPositionPercent(int center, int border, int actual)
{
  int range, position;
  int percent;

  if (border < center && actual > center)
    return 0;
  if (border > center && actual < center)
    return 0;

  range = ABS(border - center);
  position = ABS(actual - center);

  percent = (int)(position * 100 / range);

  if (percent > 100)
    percent = 100;

  return percent;
}

void CheckJoystickData()
{
  int i;
  int distance = 100;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (setup.input[i].joy.xleft >= setup.input[i].joy.xmiddle)
      setup.input[i].joy.xleft = setup.input[i].joy.xmiddle - distance;
    if (setup.input[i].joy.xright <= setup.input[i].joy.xmiddle)
      setup.input[i].joy.xright = setup.input[i].joy.xmiddle + distance;

    if (setup.input[i].joy.yupper >= setup.input[i].joy.ymiddle)
      setup.input[i].joy.yupper = setup.input[i].joy.ymiddle - distance;
    if (setup.input[i].joy.ylower <= setup.input[i].joy.ymiddle)
      setup.input[i].joy.ylower = setup.input[i].joy.ymiddle + distance;
  }
}

int JoystickExt(int player_nr, boolean use_as_joystick_nr)
{
  int joystick_nr = joystick.nr[player_nr];
  int js_x, js_y;
  boolean js_b1, js_b2;
  int left, right, up, down;
  int result = JOY_NO_ACTION;

  if (use_as_joystick_nr)
    joystick_nr = player_nr;

  if (joystick.status != JOYSTICK_ACTIVATED)
    return JOY_NO_ACTION;

  if (joystick_nr < 0)
    return JOY_NO_ACTION;

  if (!ReadJoystick(joystick_nr, &js_x, &js_y, &js_b1, &js_b2))
  {
    Error(ERR_WARN, "cannot read joystick device '%s'",
	  setup.input[player_nr].joy.device_name);

    joystick.status = JOYSTICK_NOT_AVAILABLE;
    return JOY_NO_ACTION;
  }

  left  = JoystickPositionPercent(setup.input[player_nr].joy.xmiddle,
				  setup.input[player_nr].joy.xleft,  js_x);
  right = JoystickPositionPercent(setup.input[player_nr].joy.xmiddle,
				  setup.input[player_nr].joy.xright, js_x);
  up    = JoystickPositionPercent(setup.input[player_nr].joy.ymiddle,
				  setup.input[player_nr].joy.yupper, js_y);
  down  = JoystickPositionPercent(setup.input[player_nr].joy.ymiddle,
				  setup.input[player_nr].joy.ylower, js_y);

  if (left > JOYSTICK_PERCENT)
    result |= JOY_LEFT;
  else if (right > JOYSTICK_PERCENT)
    result |= JOY_RIGHT;
  if (up > JOYSTICK_PERCENT)
    result |= JOY_UP;
  else if (down > JOYSTICK_PERCENT)
    result |= JOY_DOWN;

  if (js_b1)
    result |= JOY_BUTTON_1;
  if (js_b2)
    result |= JOY_BUTTON_2;

  return result;
}

int Joystick(int player_nr)
{
  return JoystickExt(player_nr, FALSE);
}

int JoystickButtonExt(int player_nr, boolean use_as_joystick_nr)
{
  static int last_joy_button[MAX_PLAYERS] = { 0, 0, 0, 0 };
  int joy_button = (JoystickExt(player_nr, use_as_joystick_nr) & JOY_BUTTON);
  int result;

  if (joy_button)
  {
    if (last_joy_button[player_nr])
      result = JOY_BUTTON_PRESSED;
    else
      result = JOY_BUTTON_NEW_PRESSED;
  }
  else
  {
    if (last_joy_button[player_nr])
      result = JOY_BUTTON_NEW_RELEASED;
    else
      result = JOY_BUTTON_NOT_PRESSED;
  }

  last_joy_button[player_nr] = joy_button;
  return result;
}

int JoystickButton(int player_nr)
{
  return JoystickButtonExt(player_nr, FALSE);
}

int AnyJoystick()
{
  int i;
  int result = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
    result |= JoystickExt(i, TRUE);

  return result;
}

int AnyJoystickButton()
{
  int i;
  int result = JOY_BUTTON_NOT_PRESSED;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    result = JoystickButtonExt(i, TRUE);
    if (result != JOY_BUTTON_NOT_PRESSED)
      break;
  }

  return result;
}

void DeactivateJoystick()
{
  /* Temporarily deactivate joystick. This is needed for calibration
     screens, where the player has to select a joystick device that
     should be calibrated. If there is a totally uncalibrated joystick
     active, it may be impossible (due to messed up input from joystick)
     to select the joystick device to calibrate even when trying to use
     the mouse or keyboard to select the device. */

  if (joystick.status & JOYSTICK_AVAILABLE)
    joystick.status &= ~JOYSTICK_ACTIVE;
}

void ActivateJoystick()
{
  /* reactivate temporarily deactivated joystick */

  if (joystick.status & JOYSTICK_AVAILABLE)
    joystick.status |= JOYSTICK_ACTIVE;
}
