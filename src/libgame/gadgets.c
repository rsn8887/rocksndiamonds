// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// gadgets.c
// ============================================================================

#include <stdarg.h>
#include <string.h>

#include "gadgets.h"
#include "text.h"
#include "misc.h"


/* values for DrawGadget() */
#define DG_UNPRESSED		0
#define DG_PRESSED		1

#define DG_BUFFERED		0
#define DG_DIRECT		1

#define OPTION_TEXT_SELECTABLE(g, t)					\
  (t[0] != g->selectbox.char_unselectable &&				\
   t[0] != '\0' &&							\
   !strEqual(t, " "))
#define CURRENT_OPTION_SELECTABLE(g)					\
  OPTION_TEXT_SELECTABLE(g, g->selectbox.options[g->selectbox.current_index].text)


static struct GadgetInfo *gadget_list_first_entry = NULL;
static struct GadgetInfo *gadget_list_last_entry = NULL;
static struct GadgetInfo *last_info_gi = NULL;
static int next_free_gadget_id = 1;
static boolean gadget_id_wrapped = FALSE;

static void (*PlayGadgetSoundActivating)(void) = NULL;
static void (*PlayGadgetSoundSelecting)(void) = NULL;


void InitGadgetsSoundCallback(void (*activating_function)(void),
			      void (*selecting_function)(void))
{
  PlayGadgetSoundActivating = activating_function;
  PlayGadgetSoundSelecting = selecting_function;
}

static struct GadgetInfo *getGadgetInfoFromGadgetID(int id)
{
  struct GadgetInfo *gi = gadget_list_first_entry;

  while (gi && gi->id != id)
    gi = gi->next;

  return gi;
}

static int getNewGadgetID()
{
  int id = next_free_gadget_id++;

  if (next_free_gadget_id <= 0)		/* counter overrun */
  {
    gadget_id_wrapped = TRUE;		/* now we must check each ID */
    next_free_gadget_id = 0;
  }

  if (gadget_id_wrapped)
  {
    next_free_gadget_id++;
    while (getGadgetInfoFromGadgetID(next_free_gadget_id) != NULL)
      next_free_gadget_id++;
  }

  if (next_free_gadget_id <= 0)		/* cannot get new gadget id */
    Error(ERR_EXIT, "too much gadgets -- this should not happen");

  return id;
}

static struct GadgetInfo *getGadgetInfoFromMousePosition(int mx, int my,
							 int button)
{
  struct GadgetInfo *gi;

  /* first check for scrollbars in case of mouse scroll wheel button events */
  if (IS_WHEEL_BUTTON(button))
  {
    /* real horizontal wheel or vertical wheel with modifier key pressed */
    boolean check_horizontal = (IS_WHEEL_BUTTON_HORIZONTAL(button) ||
				GetKeyModState() & KMOD_Shift);

    /* check for the first active scrollbar directly under the mouse pointer */
    for (gi = gadget_list_first_entry; gi != NULL; gi = gi->next)
    {
      if (gi->mapped && gi->active &&
	  (gi->type & GD_TYPE_SCROLLBAR) &&
	  mx >= gi->x && mx < gi->x + gi->width &&
	  my >= gi->y && my < gi->y + gi->height)
	return gi;
    }

    /* check for the first active scrollbar with matching mouse wheel area */
    for (gi = gadget_list_first_entry; gi != NULL; gi = gi->next)
    {
      if (gi->mapped && gi->active &&
	  ((gi->type & GD_TYPE_SCROLLBAR_HORIZONTAL && check_horizontal) ||
	   (gi->type & GD_TYPE_SCROLLBAR_VERTICAL && !check_horizontal)) &&
	  mx >= gi->wheelarea.x && mx < gi->wheelarea.x + gi->wheelarea.width &&
	  my >= gi->wheelarea.y && my < gi->wheelarea.y + gi->wheelarea.height)
	return gi;
    }

    /* no active scrollbar found -- ignore this scroll wheel button event */
    return NULL;
  }

  /* open selectboxes may overlap other active gadgets, so check them first */
  for (gi = gadget_list_first_entry; gi != NULL; gi = gi->next)
  {
    if (gi->mapped && gi->active &&
	gi->type & GD_TYPE_SELECTBOX && gi->selectbox.open &&
	mx >= gi->selectbox.x && mx < gi->selectbox.x + gi->selectbox.width &&
	my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height)
      return gi;
  }

  /* check all other gadgets */
  for (gi = gadget_list_first_entry; gi != NULL; gi = gi->next)
  {
    if (gi->mapped && gi->active &&
	mx >= gi->x && mx < gi->x + gi->width &&
	my >= gi->y && my < gi->y + gi->height)
      return gi;
  }

  return NULL;
}

static void setTextAreaCursorExt(struct GadgetInfo *gi, boolean set_cursor_pos)
{
  char *text = gi->textarea.value;
  int area_xsize = gi->textarea.xsize;
  int area_ysize = gi->textarea.ysize;
  int cursor_position = gi->textarea.cursor_position;
  int cursor_x = gi->textarea.cursor_x;
  int cursor_y = gi->textarea.cursor_y;
  int pos = 0;
  int x = 0;
  int y = 0;

  while (*text)
  {
    if (set_cursor_pos)		/* x/y => position */
    {
      if (y == cursor_y && (x == cursor_x || (x < cursor_x && *text == '\n')))
	break;
    }
    else			/* position => x/y */
    {
      if (pos == cursor_position)
	break;
    }

    if (x + 1 >= area_xsize || *text == '\n')
    {
      if (y + 1 >= area_ysize)
	break;

      x = 0;
      y++;
    }
    else
      x++;

    text++;
    pos++;
  }

  gi->textarea.cursor_x = x;
  gi->textarea.cursor_y = y;
  gi->textarea.cursor_x_preferred = x;
  gi->textarea.cursor_position = pos;
}

static void setTextAreaCursorXY(struct GadgetInfo *gi, int x, int y)
{
  gi->textarea.cursor_x = x;
  gi->textarea.cursor_y = y;

  setTextAreaCursorExt(gi, TRUE);
}

static void setTextAreaCursorPosition(struct GadgetInfo *gi, int pos)
{
  gi->textarea.cursor_position = pos;

  setTextAreaCursorExt(gi, FALSE);
}

static void default_callback_info(void *ptr)
{
  return;
}

static void default_callback_action(void *ptr)
{
  return;
}

static void DrawGadget(struct GadgetInfo *gi, boolean pressed, boolean direct)
{
  struct GadgetDesign *gd;
  int state = (pressed ? GD_BUTTON_PRESSED : GD_BUTTON_UNPRESSED);
  boolean redraw_selectbox = FALSE;

  if (gi == NULL || gi->deactivated)
    return;

  gd = (!gi->active ? &gi->alt_design[state] :
 	gi->checked ? &gi->alt_design[state] : &gi->design[state]);

  switch (gi->type)
  {
    case GD_TYPE_NORMAL_BUTTON:
    case GD_TYPE_CHECK_BUTTON:
    case GD_TYPE_RADIO_BUTTON:

      BlitBitmapOnBackground(gd->bitmap, drawto,
			     gd->x, gd->y, gi->width, gi->height,
			     gi->x, gi->y);

      if (gi->deco.design.bitmap)
      {
	// make sure that decoration does not overlap gadget border
	int deco_x = gi->deco.x + (pressed ? gi->deco.xshift : 0);
	int deco_y = gi->deco.y + (pressed ? gi->deco.yshift : 0);
	int deco_width  = MIN(gi->deco.width,  gi->width  - deco_x);
	int deco_height = MIN(gi->deco.height, gi->height - deco_y);

	if (gi->deco.masked)
	  BlitBitmapMasked(gi->deco.design.bitmap, drawto,
			   gi->deco.design.x, gi->deco.design.y,
			   deco_width, deco_height,
			   gi->x + deco_x, gi->y + deco_y);
	else
	  BlitBitmap(gi->deco.design.bitmap, drawto,
		     gi->deco.design.x, gi->deco.design.y,
		     deco_width, deco_height,
		     gi->x + deco_x, gi->y + deco_y);
      }

      break;

    case GD_TYPE_TEXT_BUTTON:
      {
	int i;
	int font_nr = (gi->active ? gi->font_active : gi->font);
	int font_width = getFontWidth(font_nr);
	int border_x = gi->border.xsize;
	int border_y = gi->border.ysize;
	int text_size = strlen(gi->textbutton.value);
	int text_start = (gi->width - text_size * font_width) / 2;

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
			       border_x, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i < gi->textbutton.size; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				 font_width, gi->height,
				 gi->x + border_x + i * font_width, gi->y);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x, gd->y,
			       border_x, gi->height,
			       gi->x + gi->width - border_x, gi->y);

	/* gadget text value */
	DrawTextExt(drawto,
		    gi->x + text_start + (pressed ? gi->deco.xshift : 0),
		    gi->y + border_y   + (pressed ? gi->deco.yshift : 0),
		    gi->textbutton.value, font_nr, BLIT_MASKED);
      }
      break;

    case GD_TYPE_TEXT_INPUT_ALPHANUMERIC:
    case GD_TYPE_TEXT_INPUT_NUMERIC:
      {
	int i;
	char cursor_letter;
	char cursor_string[2];
	char text[MAX_GADGET_TEXTSIZE + 1];
	int font_nr = (pressed ? gi->font_active : gi->font);
	int font_width = getFontWidth(font_nr);
	int border_x = gi->border.xsize;
	int border_y = gi->border.ysize;

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
			       border_x, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i < gi->textinput.size + 1; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				 font_width, gi->height,
				 gi->x + border_x + i * font_width, gi->y);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x, gd->y,
			       border_x, gi->height,
			       gi->x + gi->width - border_x, gi->y);

	/* set text value */
	strcpy(text, gi->textinput.value);
	strcat(text, " ");

	/* gadget text value */
	DrawTextExt(drawto,
		    gi->x + border_x, gi->y + border_y, text,
		    font_nr, BLIT_MASKED);

	cursor_letter = gi->textinput.value[gi->textinput.cursor_position];
	cursor_string[0] = (cursor_letter != '\0' ? cursor_letter : ' ');
	cursor_string[1] = '\0';

	/* draw cursor, if active */
	if (pressed)
	  DrawTextExt(drawto,
		      gi->x + border_x +
		      gi->textinput.cursor_position * font_width,
		      gi->y + border_y, cursor_string,
		      font_nr, BLIT_INVERSE);
      }
      break;

    case GD_TYPE_TEXT_AREA:
      {
	int i;
	char cursor_letter;
	char cursor_string[2];
	int font_nr = (pressed ? gi->font_active : gi->font);
	int font_width = getFontWidth(font_nr);
	int font_height = getFontHeight(font_nr);
	int border_x = gi->border.xsize;
	int border_y = gi->border.ysize;
	int gd_height = 2 * border_y + font_height;

	/* top left part of gadget border */
	BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
			       border_x, border_y, gi->x, gi->y);

	/* top middle part of gadget border */
	for (i=0; i < gi->textarea.xsize; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				 font_width, border_y,
				 gi->x + border_x + i * font_width, gi->y);

	/* top right part of gadget border */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x, gd->y,
			       border_x, border_y,
			       gi->x + gi->width - border_x, gi->y);

	/* left and right part of gadget border for each row */
	for (i=0; i < gi->textarea.ysize; i++)
	{
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y + border_y,
				 border_x, font_height,
				 gi->x, gi->y + border_y + i * font_height);
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.width - border_x,
				 gd->y + border_y,
				 border_x, font_height,
				 gi->x + gi->width - border_x,
				 gi->y + border_y + i * font_height);
	}

	/* bottom left part of gadget border */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x, gd->y + gd_height - border_y,
			       border_x, border_y,
			       gi->x, gi->y + gi->height - border_y);

	/* bottom middle part of gadget border */
	for (i=0; i < gi->textarea.xsize; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + border_x,
				 gd->y + gd_height - border_y,
				 font_width, border_y,
				 gi->x + border_x + i * font_width,
				 gi->y + gi->height - border_y);

	/* bottom right part of gadget border */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x,
			       gd->y + gd_height - border_y,
			       border_x, border_y,
			       gi->x + gi->width - border_x,
			       gi->y + gi->height - border_y);

	ClearRectangleOnBackground(drawto,
				   gi->x + border_x,
				   gi->y + border_y,
				   gi->width - 2 * border_x,
				   gi->height - 2 * border_y);

	/* gadget text value */
	DrawTextBuffer(gi->x + border_x, gi->y + border_y, gi->textarea.value,
		       font_nr, gi->textarea.xsize, -1, gi->textarea.ysize, 0,
		       BLIT_ON_BACKGROUND, FALSE, FALSE, FALSE);

	cursor_letter = gi->textarea.value[gi->textarea.cursor_position];
	cursor_string[0] = (cursor_letter != '\0' ? cursor_letter : ' ');
	cursor_string[1] = '\0';

	/* draw cursor, if active */
	if (pressed)
	  DrawTextExt(drawto,
		      gi->x + border_x + gi->textarea.cursor_x * font_width,
		      gi->y + border_y + gi->textarea.cursor_y * font_height,
		      cursor_string,
		      font_nr, BLIT_INVERSE);
      }
      break;

    case GD_TYPE_SELECTBOX:
      {
	int i;
	char text[MAX_GADGET_TEXTSIZE + 1];
	int font_nr_default = (pressed ? gi->font_active : gi->font);
	int font_nr = font_nr_default;
	int font_width = getFontWidth(font_nr);
	int font_height = getFontHeight(font_nr);
	int border_x = gi->border.xsize;
	int border_y = gi->border.ysize;
	int button = gi->border.xsize_selectbutton;
	int width_inner = gi->border.width - button - 2 * border_x;
	int box_width = gi->selectbox.width;
	int box_height = gi->selectbox.height;

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
			       border_x, gi->height, gi->x, gi->y);

	/* middle part of gadget */
	for (i=0; i < gi->selectbox.size; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				 font_width, gi->height,
				 gi->x + border_x + i * font_width, gi->y);

	/* button part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + border_x + width_inner, gd->y,
			       button, gi->height,
			       gi->x + gi->width - border_x - button, gi->y);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + gi->border.width - border_x, gd->y,
			       border_x, gi->height,
			       gi->x + gi->width - border_x, gi->y);

	/* set text value */
	strncpy(text, gi->selectbox.options[gi->selectbox.index].text,
		gi->selectbox.size);
	text[gi->selectbox.size] = '\0';

	/* set font value */
	font_nr = (OPTION_TEXT_SELECTABLE(gi, text) ? font_nr_default :
		   gi->font_unselectable);

	/* gadget text value */
	DrawTextExt(drawto, gi->x + border_x, gi->y + border_y, text,
		    font_nr, BLIT_MASKED);

	if (pressed)
	{
	  if (!gi->selectbox.open)
	  {
	    gi->selectbox.open = TRUE;
	    gi->selectbox.stay_open = FALSE;
	    gi->selectbox.current_index = gi->selectbox.index;

	    /* save background under selectbox */
	    BlitBitmap(drawto, gfx.field_save_buffer,
		       gi->selectbox.x,     gi->selectbox.y,
		       gi->selectbox.width, gi->selectbox.height,
		       gi->selectbox.x,     gi->selectbox.y);
	  }

	  /* draw open selectbox */

	  /* top left part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y,
				 border_x, border_y,
				 gi->selectbox.x, gi->selectbox.y);

	  /* top middle part of gadget border */
	  for (i=0; i < gi->selectbox.size; i++)
	    BlitBitmapOnBackground(gd->bitmap, drawto, gd->x + border_x, gd->y,
				   font_width, border_y,
				   gi->selectbox.x + border_x + i * font_width,
				   gi->selectbox.y);

	  /* top button part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + border_x + width_inner, gd->y,
				 button, border_y,
				 gi->selectbox.x + box_width -border_x -button,
				 gi->selectbox.y);

	  /* top right part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.width - border_x, gd->y,
				 border_x, border_y,
				 gi->selectbox.x + box_width - border_x,
				 gi->selectbox.y);

	  /* left and right part of gadget border for each row */
	  for (i=0; i < gi->selectbox.num_values; i++)
	  {
	    BlitBitmapOnBackground(gd->bitmap, drawto, gd->x, gd->y + border_y,
				   border_x, font_height,
				   gi->selectbox.x,
				   gi->selectbox.y + border_y + i*font_height);
	    BlitBitmapOnBackground(gd->bitmap, drawto,
				   gd->x + gi->border.width - border_x,
				   gd->y + border_y,
				   border_x, font_height,
				   gi->selectbox.x + box_width - border_x,
				   gi->selectbox.y + border_y + i*font_height);
	  }

	  /* bottom left part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x, gd->y + gi->height - border_y,
				 border_x, border_y,
				 gi->selectbox.x,
				 gi->selectbox.y + box_height - border_y);

	  /* bottom middle part of gadget border */
	  for (i=0; i < gi->selectbox.size; i++)
	    BlitBitmapOnBackground(gd->bitmap, drawto,
				   gd->x + border_x,
				   gd->y + gi->height - border_y,
				   font_width, border_y,
				   gi->selectbox.x + border_x + i * font_width,
				   gi->selectbox.y + box_height - border_y);

	  /* bottom button part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + border_x + width_inner,
				 gd->y + gi->height - border_y,
				 button, border_y,
				 gi->selectbox.x + box_width -border_x -button,
				 gi->selectbox.y + box_height - border_y);

	  /* bottom right part of gadget border */
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.width - border_x,
				 gd->y + gi->height - border_y,
				 border_x, border_y,
				 gi->selectbox.x + box_width - border_x,
				 gi->selectbox.y + box_height - border_y);

	  ClearRectangleOnBackground(drawto,
				     gi->selectbox.x + border_x,
				     gi->selectbox.y + border_y,
				     gi->selectbox.width - 2 * border_x,
				     gi->selectbox.height - 2 * border_y);

	  /* selectbox text values */
	  for (i=0; i < gi->selectbox.num_values; i++)
	  {
	    int mask_mode = BLIT_MASKED;

	    strncpy(text, gi->selectbox.options[i].text, gi->selectbox.size);
	    text[gi->selectbox.size] = '\0';

	    font_nr = (OPTION_TEXT_SELECTABLE(gi, text) ? font_nr_default :
		       gi->font_unselectable);

	    if (i == gi->selectbox.current_index &&
		OPTION_TEXT_SELECTABLE(gi, text))
	    {
	      FillRectangle(drawto,
			    gi->selectbox.x + border_x,
			    gi->selectbox.y + border_y + i * font_height,
			    gi->selectbox.width - 2 * border_x, font_height,
			    gi->selectbox.inverse_color);

	      /* prevent use of cursor graphic by drawing at least two chars */
	      strcat(text, "  ");
	      text[gi->selectbox.size] = '\0';

	      mask_mode = BLIT_INVERSE;
	    }

	    DrawTextExt(drawto,
			gi->selectbox.x + border_x,
			gi->selectbox.y + border_y + i * font_height, text,
			font_nr, mask_mode);
	  }

	  redraw_selectbox = TRUE;
	}
	else if (gi->selectbox.open)
	{
	  gi->selectbox.open = FALSE;

	  /* restore background under selectbox */
	  BlitBitmap(gfx.field_save_buffer, drawto,
		     gi->selectbox.x,     gi->selectbox.y,
		     gi->selectbox.width, gi->selectbox.height,
		     gi->selectbox.x,     gi->selectbox.y);

	  /* redraw closed selectbox */
	  DrawGadget(gi, FALSE, FALSE);

	  redraw_selectbox = TRUE;
	}
      }
      break;

    case GD_TYPE_SCROLLBAR_VERTICAL:
      {
	int i;
	int xpos = gi->x;
	int ypos = gi->y + gi->scrollbar.position;
	int design_full = gi->width;
	int design_body = design_full - 2 * gi->border.ysize;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->border.ysize;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	ClearRectangleOnBackground(backbuffer, gi->x, gi->y,
				   gi->width, gi->height);

	/* upper part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x, gd->y,
			       gi->width, gi->border.ysize,
			       xpos, ypos);

	/* middle part of gadget */
	for (i=0; i < num_steps; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x, gd->y + gi->border.ysize,
				 gi->width, design_body,
				 xpos,
				 ypos + gi->border.ysize + i * design_body);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x,  gd->y + gi->border.ysize,
				 gi->width, step_size_remain,
				 xpos,
				 ypos + gi->border.ysize
				 + num_steps * design_body);

	/* lower part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x, gd->y + design_full - gi->border.ysize,
			       gi->width, gi->border.ysize,
			       xpos, ypos + size_full - gi->border.ysize);
      }
      break;

    case GD_TYPE_SCROLLBAR_HORIZONTAL:
      {
	int i;
	int xpos = gi->x + gi->scrollbar.position;
	int ypos = gi->y;
	int design_full = gi->height;
	int design_body = design_full - 2 * gi->border.xsize;
	int size_full = gi->scrollbar.size;
	int size_body = size_full - 2 * gi->border.xsize;
	int num_steps = size_body / design_body;
	int step_size_remain = size_body - num_steps * design_body;

	/* clear scrollbar area */
	ClearRectangleOnBackground(backbuffer, gi->x, gi->y,
				   gi->width, gi->height);

	/* left part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x, gd->y,
			       gi->border.xsize, gi->height,
			       xpos, ypos);

	/* middle part of gadget */
	for (i=0; i < num_steps; i++)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.xsize, gd->y,
				 design_body, gi->height,
				 xpos + gi->border.xsize + i * design_body,
				 ypos);

	/* remaining middle part of gadget */
	if (step_size_remain > 0)
	  BlitBitmapOnBackground(gd->bitmap, drawto,
				 gd->x + gi->border.xsize, gd->y,
				 step_size_remain, gi->height,
				 xpos + gi->border.xsize
				 + num_steps * design_body,
				 ypos);

	/* right part of gadget */
	BlitBitmapOnBackground(gd->bitmap, drawto,
			       gd->x + design_full - gi->border.xsize, gd->y,
			       gi->border.xsize, gi->height,
			       xpos + size_full - gi->border.xsize, ypos);
      }
      break;

    default:
      return;
  }

  // do not use direct gadget drawing anymore; this worked as a speed-up once,
  // but would slow things down a lot now the screen is always fully redrawn
  direct = FALSE;

  if (direct)
  {
    BlitBitmap(drawto, window,
	       gi->x, gi->y, gi->width, gi->height, gi->x, gi->y);

    if (gi->type == GD_TYPE_SELECTBOX && redraw_selectbox)
      BlitBitmap(drawto, window,
		 gi->selectbox.x,     gi->selectbox.y,
		 gi->selectbox.width, gi->selectbox.height,
		 gi->selectbox.x,     gi->selectbox.y);
  }
  else
  {
    int x = gi->x;
    int y = gi->y;

    redraw_mask |= (IN_GFX_FIELD_FULL(x, y) ? REDRAW_FIELD :
		    IN_GFX_DOOR_1(x, y) ? REDRAW_DOOR_1 :
		    IN_GFX_DOOR_2(x, y) ? REDRAW_DOOR_2 :
		    IN_GFX_DOOR_3(x, y) ? REDRAW_DOOR_3 : REDRAW_ALL);
  }
}

static int get_minimal_size_for_numeric_input(int minmax_value)
{
  int min_size = 1;	/* value needs at least one digit */
  int i;

  /* add number of digits needed for absolute value */
  for (i = 10; i <= ABS(minmax_value); i *= 10)
    min_size++;

  /* if min/max value is negative, add one digit for minus sign */
  if (minmax_value < 0)
    min_size++;

  return min_size;
}

static void HandleGadgetTags(struct GadgetInfo *gi, int first_tag, va_list ap)
{
  int tag = first_tag;

  if (gi == NULL || gi->deactivated)
    return;

  while (tag != GDI_END)
  {
    switch(tag)
    {
      case GDI_CUSTOM_ID:
	gi->custom_id = va_arg(ap, int);
	break;

      case GDI_CUSTOM_TYPE_ID:
	gi->custom_type_id = va_arg(ap, int);
	break;

      case GDI_INFO_TEXT:
	{
	  int max_textsize = MAX_INFO_TEXTSIZE;
	  char *text = va_arg(ap, char *);

	  if (text != NULL)
	    strncpy(gi->info_text, text, max_textsize);
	  else
	    max_textsize = 0;

	  gi->info_text[max_textsize] = '\0';
	}
	break;

      case GDI_X:
	gi->x = va_arg(ap, int);
	break;

      case GDI_Y:
	gi->y = va_arg(ap, int);
	break;

      case GDI_WIDTH:
	gi->width = va_arg(ap, int);
	break;

      case GDI_HEIGHT:
	gi->height = va_arg(ap, int);
	break;

      case GDI_TYPE:
	gi->type = va_arg(ap, unsigned int);
	break;

      case GDI_STATE:
	gi->state = va_arg(ap, unsigned int);
	break;

      case GDI_ACTIVE:
	gi->active = (boolean)va_arg(ap, int);
	break;

      case GDI_DIRECT_DRAW:
	gi->direct_draw = (boolean)va_arg(ap, int);
	break;

      case GDI_CHECKED:
	gi->checked = (boolean)va_arg(ap, int);
	break;

      case GDI_RADIO_NR:
	gi->radio_nr = va_arg(ap, unsigned int);
	break;

      case GDI_NUMBER_VALUE:
	gi->textinput.number_value = va_arg(ap, int);
	sprintf(gi->textinput.value, "%d", gi->textinput.number_value);
	strcpy(gi->textinput.last_value, gi->textinput.value);
	gi->textinput.cursor_position = strlen(gi->textinput.value);
	break;

      case GDI_NUMBER_MIN:
	gi->textinput.number_min = va_arg(ap, int);
	if (gi->textinput.number_value < gi->textinput.number_min)
	{
	  gi->textinput.number_value = gi->textinput.number_min;
	  sprintf(gi->textinput.value, "%d", gi->textinput.number_value);
	  strcpy(gi->textinput.last_value, gi->textinput.value);
	}
	break;

      case GDI_NUMBER_MAX:
	gi->textinput.number_max = va_arg(ap, int);
	if (gi->textinput.number_value > gi->textinput.number_max)
	{
	  gi->textinput.number_value = gi->textinput.number_max;
	  sprintf(gi->textinput.value, "%d", gi->textinput.number_value);
	  strcpy(gi->textinput.last_value, gi->textinput.value);
	}
	break;

      case GDI_TEXT_VALUE:
	{
	  int max_textsize = MAX_GADGET_TEXTSIZE;

	  if (gi->textinput.size)
	    max_textsize = MIN(gi->textinput.size, MAX_GADGET_TEXTSIZE);

	  strncpy(gi->textinput.value, va_arg(ap, char *), max_textsize);
	  strcpy(gi->textinput.last_value, gi->textinput.value);

	  gi->textinput.value[max_textsize] = '\0';
	  gi->textinput.cursor_position = strlen(gi->textinput.value);

	  /* same tag also used for other gadget definitions */
	  strcpy(gi->textbutton.value, gi->textinput.value);
	  strcpy(gi->textarea.value, gi->textinput.value);
	  strcpy(gi->textarea.last_value, gi->textinput.value);
	}
	break;

      case GDI_TEXT_SIZE:
	{
	  int tag_value = va_arg(ap, int);
	  int max_textsize = MIN(tag_value, MAX_GADGET_TEXTSIZE);

	  gi->textinput.size = max_textsize;
	  gi->textinput.value[max_textsize] = '\0';
	  strcpy(gi->textinput.last_value, gi->textinput.value);

	  /* same tag also used for other gadget definitions */

	  gi->textarea.size = max_textsize;
	  gi->textarea.value[max_textsize] = '\0';
	  strcpy(gi->textarea.last_value, gi->textinput.value);

	  gi->textbutton.size = max_textsize;
	  gi->textbutton.value[max_textsize] = '\0';

	  gi->selectbox.size = gi->textinput.size;
	}
	break;

      case GDI_TEXT_FONT:
	gi->font = va_arg(ap, int);
	if (gi->font_active == 0)
	  gi->font_active = gi->font;
	if (gi->font_unselectable == 0)
	  gi->font_unselectable = gi->font;
	break;

      case GDI_TEXT_FONT_ACTIVE:
	gi->font_active = va_arg(ap, int);
	break;

      case GDI_TEXT_FONT_UNSELECTABLE:
	gi->font_unselectable = va_arg(ap, int);
	break;

      case GDI_SELECTBOX_OPTIONS:
	gi->selectbox.options = va_arg(ap, struct ValueTextInfo *);
	break;

      case GDI_SELECTBOX_INDEX:
	gi->selectbox.index = va_arg(ap, int);
	break;

      case GDI_SELECTBOX_CHAR_UNSELECTABLE:
	gi->selectbox.char_unselectable = (char)va_arg(ap, int);
	break;

      case GDI_DESIGN_UNPRESSED:
	gi->design[GD_BUTTON_UNPRESSED].bitmap = va_arg(ap, Bitmap *);
	gi->design[GD_BUTTON_UNPRESSED].x = va_arg(ap, int);
	gi->design[GD_BUTTON_UNPRESSED].y = va_arg(ap, int);
	break;

      case GDI_DESIGN_PRESSED:
	gi->design[GD_BUTTON_PRESSED].bitmap = va_arg(ap, Bitmap *);
	gi->design[GD_BUTTON_PRESSED].x = va_arg(ap, int);
	gi->design[GD_BUTTON_PRESSED].y = va_arg(ap, int);
	break;

      case GDI_ALT_DESIGN_UNPRESSED:
	gi->alt_design[GD_BUTTON_UNPRESSED].bitmap= va_arg(ap, Bitmap *);
	gi->alt_design[GD_BUTTON_UNPRESSED].x = va_arg(ap, int);
	gi->alt_design[GD_BUTTON_UNPRESSED].y = va_arg(ap, int);
	break;

      case GDI_ALT_DESIGN_PRESSED:
	gi->alt_design[GD_BUTTON_PRESSED].bitmap = va_arg(ap, Bitmap *);
	gi->alt_design[GD_BUTTON_PRESSED].x = va_arg(ap, int);
	gi->alt_design[GD_BUTTON_PRESSED].y = va_arg(ap, int);
	break;

      case GDI_BORDER_SIZE:
	gi->border.xsize = va_arg(ap, int);
	gi->border.ysize = va_arg(ap, int);
	break;

      case GDI_BORDER_SIZE_SELECTBUTTON:
	gi->border.xsize_selectbutton = va_arg(ap, int);
	break;

      case GDI_DESIGN_WIDTH:
	gi->border.width = va_arg(ap, int);
	break;

      case GDI_DECORATION_DESIGN:
	gi->deco.design.bitmap = va_arg(ap, Bitmap *);
	gi->deco.design.x = va_arg(ap, int);
	gi->deco.design.y = va_arg(ap, int);
	break;

      case GDI_DECORATION_POSITION:
	gi->deco.x = va_arg(ap, int);
	gi->deco.y = va_arg(ap, int);
	break;

      case GDI_DECORATION_SIZE:
	gi->deco.width = va_arg(ap, int);
	gi->deco.height = va_arg(ap, int);
	break;

      case GDI_DECORATION_SHIFTING:
	gi->deco.xshift = va_arg(ap, int);
	gi->deco.yshift = va_arg(ap, int);
	break;

      case GDI_DECORATION_MASKED:
	gi->deco.masked = (boolean)va_arg(ap, int);
	break;

      case GDI_EVENT_MASK:
	gi->event_mask = va_arg(ap, unsigned int);
	break;

      case GDI_AREA_SIZE:
	gi->drawing.area_xsize = va_arg(ap, int);
	gi->drawing.area_ysize = va_arg(ap, int);

	/* determine dependent values for drawing area gadget, if needed */
	if (gi->drawing.item_xsize != 0 && gi->drawing.item_ysize != 0)
	{
	  gi->width = gi->drawing.area_xsize * gi->drawing.item_xsize;
	  gi->height = gi->drawing.area_ysize * gi->drawing.item_ysize;
	}
	else if (gi->width != 0 && gi->height != 0)
	{
	  gi->drawing.item_xsize = gi->width / gi->drawing.area_xsize;
	  gi->drawing.item_ysize = gi->height / gi->drawing.area_ysize;
	}

	/* same tag also used for other gadget definitions */
	gi->textarea.xsize = gi->drawing.area_xsize;
	gi->textarea.ysize = gi->drawing.area_ysize;

	if (gi->type & GD_TYPE_TEXT_AREA)	/* force recalculation */
	{
	  gi->width = 0;
	  gi->height = 0;
	}

	break;

      case GDI_ITEM_SIZE:
	gi->drawing.item_xsize = va_arg(ap, int);
	gi->drawing.item_ysize = va_arg(ap, int);

	/* determine dependent values for drawing area gadget, if needed */
	if (gi->drawing.area_xsize != 0 && gi->drawing.area_ysize != 0)
	{
	  gi->width = gi->drawing.area_xsize * gi->drawing.item_xsize;
	  gi->height = gi->drawing.area_ysize * gi->drawing.item_ysize;
	}
	else if (gi->width != 0 && gi->height != 0)
	{
	  gi->drawing.area_xsize = gi->width / gi->drawing.item_xsize;
	  gi->drawing.area_ysize = gi->height / gi->drawing.item_ysize;
	}
	break;

      case GDI_SCROLLBAR_ITEMS_MAX:
	gi->scrollbar.items_max = va_arg(ap, int);
	break;

      case GDI_SCROLLBAR_ITEMS_VISIBLE:
	gi->scrollbar.items_visible = va_arg(ap, int);
	break;

      case GDI_SCROLLBAR_ITEM_POSITION:
	gi->scrollbar.item_position = va_arg(ap, int);
	break;

      case GDI_WHEEL_AREA_X:
	gi->wheelarea.x = va_arg(ap, int);
	break;

      case GDI_WHEEL_AREA_Y:
	gi->wheelarea.y = va_arg(ap, int);
	break;

      case GDI_WHEEL_AREA_WIDTH:
	gi->wheelarea.width = va_arg(ap, int);
	break;

      case GDI_WHEEL_AREA_HEIGHT:
	gi->wheelarea.height = va_arg(ap, int);
	break;

      case GDI_CALLBACK_INFO:
	gi->callback_info = va_arg(ap, gadget_function);
	break;

      case GDI_CALLBACK_ACTION:
	gi->callback_action = va_arg(ap, gadget_function);
	break;

      default:
	Error(ERR_EXIT, "HandleGadgetTags(): unknown tag %d", tag);
    }

    tag = va_arg(ap, int);	/* read next tag */
  }

  gi->deactivated = FALSE;

  /* check if gadget has undefined bitmaps */
  if (gi->type != GD_TYPE_DRAWING_AREA &&
      (gi->design[GD_BUTTON_UNPRESSED].bitmap == NULL ||
       gi->design[GD_BUTTON_PRESSED].bitmap == NULL))
    gi->deactivated = TRUE;

  /* check if gadget is placed off-screen */
  if (gi->x < 0 || gi->y < 0)
    gi->deactivated = TRUE;

  /* adjust gadget values in relation to other gadget values */

  if (gi->type & GD_TYPE_TEXT_INPUT)
  {
    int font_nr = gi->font_active;
    int font_width = getFontWidth(font_nr);
    int font_height = getFontHeight(font_nr);
    int border_xsize = gi->border.xsize;
    int border_ysize = gi->border.ysize;

    if (gi->type == GD_TYPE_TEXT_INPUT_NUMERIC)
    {
      int number_min = gi->textinput.number_min;
      int number_max = gi->textinput.number_max;
      int min_size_min = get_minimal_size_for_numeric_input(number_min);
      int min_size_max = get_minimal_size_for_numeric_input(number_max);
      int min_size = MAX(min_size_min, min_size_max);

      /* expand gadget text input size, if maximal value is too large */
      if (gi->textinput.size < min_size)
	gi->textinput.size = min_size;
    }

    gi->width  = 2 * border_xsize + (gi->textinput.size + 1) * font_width;
    gi->height = 2 * border_ysize + font_height;
  }

  if (gi->type & GD_TYPE_SELECTBOX)
  {
    int font_nr = gi->font_active;
    int font_width = getFontWidth(font_nr);
    int font_height = getFontHeight(font_nr);
    int border_xsize = gi->border.xsize;
    int border_ysize = gi->border.ysize;
    int button_size = gi->border.xsize_selectbutton;
    int bottom_screen_border = gfx.sy + gfx.sysize - font_height;
    Bitmap *src_bitmap;
    int src_x, src_y;

    gi->width  = 2 * border_xsize + gi->textinput.size*font_width +button_size;
    gi->height = 2 * border_ysize + font_height;

    if (gi->selectbox.options == NULL)
      Error(ERR_EXIT, "selectbox gadget incomplete (missing options array)");

    gi->selectbox.num_values = 0;
    while (gi->selectbox.options[gi->selectbox.num_values].text != NULL)
      gi->selectbox.num_values++;

    /* calculate values for open selectbox */
    gi->selectbox.width = gi->width;
    gi->selectbox.height =
      2 * border_ysize + gi->selectbox.num_values * font_height;

    gi->selectbox.x = gi->x;
    gi->selectbox.y = gi->y + gi->height;
    if (gi->selectbox.y + gi->selectbox.height > bottom_screen_border)
      gi->selectbox.y = gi->y - gi->selectbox.height;
    if (gi->selectbox.y < 0)
      gi->selectbox.y = bottom_screen_border - gi->selectbox.height;

    getFontCharSource(font_nr, FONT_ASCII_CURSOR, &src_bitmap, &src_x, &src_y);
    src_x += font_width / 2;
    src_y += font_height / 2;

    /* there may be esoteric cases with missing or too small font bitmap */
    if (src_bitmap != NULL &&
	src_x < src_bitmap->width && src_y < src_bitmap->height)
      gi->selectbox.inverse_color = GetPixel(src_bitmap, src_x, src_y);

    /* always start with closed selectbox */
    gi->selectbox.open = FALSE;
  }

  if (gi->type & GD_TYPE_TEXT_INPUT_NUMERIC)
  {
    struct GadgetTextInput *text = &gi->textinput;
    int value = text->number_value;

    text->number_value = (value < text->number_min ? text->number_min :
			  value > text->number_max ? text->number_max :
			  value);

    sprintf(text->value, "%d", text->number_value);
  }

  if (gi->type & GD_TYPE_TEXT_BUTTON)
  {
    int font_nr = gi->font_active;
    int font_width = getFontWidth(font_nr);
    int font_height = getFontHeight(font_nr);
    int border_xsize = gi->border.xsize;
    int border_ysize = gi->border.ysize;

    gi->width  = 2 * border_xsize + gi->textbutton.size * font_width;
    gi->height = 2 * border_ysize + font_height;
  }

  if (gi->type & GD_TYPE_SCROLLBAR)
  {
    struct GadgetScrollbar *gs = &gi->scrollbar;
    int scrollbar_size_cmp;

    if (gi->width == 0 || gi->height == 0 ||
	gs->items_max == 0 || gs->items_visible == 0)
      Error(ERR_EXIT, "scrollbar gadget incomplete (missing tags)");

    /* calculate internal scrollbar values */
    gs->size_min = (gi->type == GD_TYPE_SCROLLBAR_VERTICAL ?
		    gi->width : gi->height);
    gs->size_max = (gi->type == GD_TYPE_SCROLLBAR_VERTICAL ?
		    gi->height : gi->width);

    scrollbar_size_cmp = gs->size_max * gs->items_visible / gs->items_max;
    gs->size = MAX(scrollbar_size_cmp, gs->size_min);
    gs->size_max_cmp = (gs->size_max - (gs->size - scrollbar_size_cmp));

    gs->position = gs->size_max_cmp * gs->item_position / gs->items_max;
    gs->position_max = gs->size_max - gs->size;
    gs->correction = gs->size_max / gs->items_max / 2;

    /* finetuning for maximal right/bottom position */
    if (gs->item_position == gs->items_max - gs->items_visible)
      gs->position = gs->position_max;
  }

  if (gi->type & GD_TYPE_TEXT_AREA)
  {
    int font_nr = gi->font_active;
    int font_width = getFontWidth(font_nr);
    int font_height = getFontHeight(font_nr);
    int border_xsize = gi->border.xsize;
    int border_ysize = gi->border.ysize;

    if (gi->width == 0 || gi->height == 0)
    {
      gi->width  = 2 * border_xsize + gi->textarea.xsize * font_width;
      gi->height = 2 * border_ysize + gi->textarea.ysize * font_height;
    }
    else
    {
      gi->textarea.xsize = (gi->width  - 2 * border_xsize) / font_width;
      gi->textarea.ysize = (gi->height - 2 * border_ysize) / font_height;
    }
  }
}

void ModifyGadget(struct GadgetInfo *gi, int first_tag, ...)
{
  va_list ap;

  va_start(ap, first_tag);
  HandleGadgetTags(gi, first_tag, ap);
  va_end(ap);

  RedrawGadget(gi);
}

void RedrawGadget(struct GadgetInfo *gi)
{
  if (gi == NULL || gi->deactivated)
    return;

  if (gi->mapped)
    DrawGadget(gi, gi->state, gi->direct_draw);
}

struct GadgetInfo *CreateGadget(int first_tag, ...)
{
  struct GadgetInfo *new_gadget = checked_calloc(sizeof(struct GadgetInfo));
  va_list ap;

  /* always start with reliable default values */
  new_gadget->id = getNewGadgetID();
  new_gadget->callback_info = default_callback_info;
  new_gadget->callback_action = default_callback_action;
  new_gadget->active = TRUE;
  new_gadget->direct_draw = TRUE;

  new_gadget->next = NULL;

  va_start(ap, first_tag);
  HandleGadgetTags(new_gadget, first_tag, ap);
  va_end(ap);

  /* insert new gadget into global gadget list */
  if (gadget_list_last_entry)
  {
    gadget_list_last_entry->next = new_gadget;
    gadget_list_last_entry = gadget_list_last_entry->next;
  }
  else
    gadget_list_first_entry = gadget_list_last_entry = new_gadget;

  return new_gadget;
}

void FreeGadget(struct GadgetInfo *gi)
{
  struct GadgetInfo *gi_previous = gadget_list_first_entry;

  if (gi == NULL)
    return;

  /* prevent "last_info_gi" from pointing to memory that will be freed */
  if (last_info_gi == gi)
    last_info_gi = NULL;

  while (gi_previous != NULL && gi_previous->next != gi)
    gi_previous = gi_previous->next;

  if (gi == gadget_list_first_entry)
    gadget_list_first_entry = gi->next;

  if (gi == gadget_list_last_entry)
    gadget_list_last_entry = gi_previous;

  if (gi_previous != NULL)
    gi_previous->next = gi->next;

  free(gi);
}

static void CheckRangeOfNumericInputGadget(struct GadgetInfo *gi)
{
  if (gi->type != GD_TYPE_TEXT_INPUT_NUMERIC)
    return;

  gi->textinput.number_value = atoi(gi->textinput.value);

  if (gi->textinput.number_value < gi->textinput.number_min)
    gi->textinput.number_value = gi->textinput.number_min;
  if (gi->textinput.number_value > gi->textinput.number_max)
    gi->textinput.number_value = gi->textinput.number_max;

  sprintf(gi->textinput.value, "%d", gi->textinput.number_value);

  if (gi->textinput.cursor_position < 0)
    gi->textinput.cursor_position = 0;
  else if (gi->textinput.cursor_position > strlen(gi->textinput.value))
    gi->textinput.cursor_position = strlen(gi->textinput.value);
}

/* global pointer to gadget actually in use (when mouse button pressed) */
static struct GadgetInfo *last_gi = NULL;

static void MapGadgetExt(struct GadgetInfo *gi, boolean redraw)
{
  if (gi == NULL || gi->deactivated || gi->mapped)
    return;

  gi->mapped = TRUE;

  if (redraw)
    DrawGadget(gi, DG_UNPRESSED, DG_BUFFERED);
}

void MapGadget(struct GadgetInfo *gi)
{
  MapGadgetExt(gi, TRUE);
}

void UnmapGadget(struct GadgetInfo *gi)
{
  if (gi == NULL || gi->deactivated || !gi->mapped)
    return;

  gi->mapped = FALSE;

  if (gi == last_gi)
    last_gi = NULL;
}

#define MAX_NUM_GADGETS		1024
#define MULTIMAP_UNMAP		(1 << 0)
#define MULTIMAP_REMAP		(1 << 1)
#define MULTIMAP_REDRAW		(1 << 2)
#define MULTIMAP_PLAYFIELD	(1 << 3)
#define MULTIMAP_DOOR_1		(1 << 4)
#define MULTIMAP_DOOR_2		(1 << 5)
#define MULTIMAP_DOOR_3		(1 << 6)
#define MULTIMAP_ALL		(MULTIMAP_PLAYFIELD | \
				 MULTIMAP_DOOR_1    | \
				 MULTIMAP_DOOR_2    | \
				 MULTIMAP_DOOR_3)

static void MultiMapGadgets(int mode)
{
  struct GadgetInfo *gi = gadget_list_first_entry;
  static boolean map_state[MAX_NUM_GADGETS];
  int map_count = 0;

  while (gi != NULL)
  {
    int x = gi->x;
    int y = gi->y;

    if ((mode & MULTIMAP_PLAYFIELD && IN_GFX_FIELD_FULL(x, y)) ||
	(mode & MULTIMAP_DOOR_1 && IN_GFX_DOOR_1(x, y)) ||
	(mode & MULTIMAP_DOOR_2 && IN_GFX_DOOR_2(x, y)) ||
	(mode & MULTIMAP_DOOR_3 && IN_GFX_DOOR_3(x, y)) ||
	(mode & MULTIMAP_ALL) == MULTIMAP_ALL)
    {
      if (mode & MULTIMAP_UNMAP)
      {
	map_state[map_count++ % MAX_NUM_GADGETS] = gi->mapped;
	UnmapGadget(gi);
      }
      else
      {
	if (map_state[map_count++ % MAX_NUM_GADGETS])
	  MapGadgetExt(gi, (mode & MULTIMAP_REDRAW));
      }
    }

    gi = gi->next;
  }
}

void UnmapAllGadgets()
{
  MultiMapGadgets(MULTIMAP_ALL | MULTIMAP_UNMAP);
}

void RemapAllGadgets()
{
  MultiMapGadgets(MULTIMAP_ALL | MULTIMAP_REMAP);
}

boolean anyTextInputGadgetActive()
{
  return (last_gi && (last_gi->type & GD_TYPE_TEXT_INPUT) && last_gi->mapped);
}

boolean anyTextAreaGadgetActive()
{
  return (last_gi && (last_gi->type & GD_TYPE_TEXT_AREA) && last_gi->mapped);
}

boolean anySelectboxGadgetActive()
{
  return (last_gi && (last_gi->type & GD_TYPE_SELECTBOX) && last_gi->mapped);
}

boolean anyScrollbarGadgetActive()
{
  return (last_gi && (last_gi->type & GD_TYPE_SCROLLBAR) && last_gi->mapped);
}

boolean anyTextGadgetActive()
{
  return (anyTextInputGadgetActive() ||
	  anyTextAreaGadgetActive() ||
	  anySelectboxGadgetActive());
}

static boolean insideSelectboxLine(struct GadgetInfo *gi, int mx, int my)
{
  return (gi != NULL &&
	  gi->type & GD_TYPE_SELECTBOX &&
	  mx >= gi->x && mx < gi->x + gi->width &&
	  my >= gi->y && my < gi->y + gi->height);
}

static boolean insideSelectboxArea(struct GadgetInfo *gi, int mx, int my)
{
  return (gi != NULL &&
	  gi->type & GD_TYPE_SELECTBOX &&
	  mx >= gi->selectbox.x && mx < gi->selectbox.x + gi->selectbox.width &&
	  my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height);
}

void ClickOnGadget(struct GadgetInfo *gi, int button)
{
  if (gi == NULL || gi->deactivated || !gi->mapped)
    return;

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  /* simulate pressing mouse button over specified gadget */
  HandleGadgets(gi->x, gi->y, button);

  /* simulate releasing mouse button over specified gadget */
  HandleGadgets(gi->x, gi->y, 0);
}

boolean HandleGadgets(int mx, int my, int button)
{
  static unsigned int pressed_delay = 0;
  static unsigned int pressed_delay_value = GADGET_FRAME_DELAY;
  static int last_button = 0;
  static int last_mx = 0, last_my = 0;
  static int pressed_mx = 0, pressed_my = 0;
  static boolean keep_selectbox_open = FALSE;
  static boolean gadget_stopped = FALSE;
  int scrollbar_mouse_pos = 0;
  struct GadgetInfo *new_gi, *gi;
  boolean press_event;
  boolean release_event;
  boolean mouse_moving;
  boolean mouse_inside_select_line;
  boolean mouse_inside_select_area;
  boolean mouse_released_where_pressed;
  boolean gadget_pressed;
  boolean gadget_pressed_repeated;
  boolean gadget_pressed_off_borders;
  boolean gadget_pressed_inside_select_line;
  boolean gadget_pressed_delay_reached;
  boolean gadget_moving;
  boolean gadget_moving_inside;
  boolean gadget_moving_off_borders;
  boolean gadget_draggable;
  boolean gadget_dragging;
  boolean gadget_released;
  boolean gadget_released_inside;
  boolean gadget_released_inside_select_area;
  boolean gadget_released_off_borders;
  boolean changed_position = FALSE;

  /* check if there are any gadgets defined */
  if (gadget_list_first_entry == NULL)
    return FALSE;

  /* simulated release of mouse button over last gadget */
  if (mx == -1 && my == -1 && button == 0)
  {
    mx = last_mx;
    my = last_my;
  }

  /* check which gadget is under the mouse pointer */
  new_gi = getGadgetInfoFromMousePosition(mx, my, button);

  /* check if button state has changed since last invocation */
  press_event   = (button != 0 && last_button == 0);
  release_event = (button == 0 && last_button != 0);
  last_button = button;

  /* check if mouse has been moved since last invocation */
  mouse_moving = ((mx != last_mx || my != last_my) && motion_status);
  last_mx = mx;
  last_my = my;

  if (press_event && new_gi != last_gi)
  {
    pressed_mx = mx;
    pressed_my = my;
  }

  mouse_released_where_pressed =
    (release_event && mx == pressed_mx && my == pressed_my);

  mouse_inside_select_line = insideSelectboxLine(new_gi, mx, my);
  mouse_inside_select_area = insideSelectboxArea(new_gi, mx, my);

  gadget_pressed_off_borders = (press_event && new_gi != last_gi);

  gadget_pressed_inside_select_line =
    (press_event && new_gi != NULL &&
     new_gi->type & GD_TYPE_SELECTBOX && new_gi->selectbox.open &&
     insideSelectboxLine(new_gi, mx, my));

  /* if mouse button pressed outside text or selectbox gadget, deactivate it */
  if (anyTextGadgetActive() &&
      (gadget_pressed_off_borders ||
       (gadget_pressed_inside_select_line && !mouse_inside_select_area)))
  {
    struct GadgetInfo *gi = last_gi;
    boolean gadget_changed = ((gi->event_mask & GD_EVENT_TEXT_LEAVING) != 0);

    /* check if text gadget has changed its value */
    if (gi->type & GD_TYPE_TEXT_INPUT)
    {
      CheckRangeOfNumericInputGadget(gi);

      if (!strEqual(gi->textinput.last_value, gi->textinput.value))
	strcpy(gi->textinput.last_value, gi->textinput.value);
      else
	gadget_changed = FALSE;
    }

    /* selectbox does not change its value when closed by clicking outside */
    if (gi->type & GD_TYPE_SELECTBOX)
      gadget_changed = FALSE;

    DrawGadget(gi, DG_UNPRESSED, gi->direct_draw);

    gi->event.type = GD_EVENT_TEXT_LEAVING;

    if (gadget_changed && !(gi->type & GD_TYPE_SELECTBOX))
      gi->callback_action(gi);

    last_gi = NULL;

    if (gadget_pressed_inside_select_line)
      new_gi = NULL;

    StopTextInput();
  }

  gadget_pressed =
    (button != 0 && last_gi == NULL && new_gi != NULL && press_event);
  gadget_pressed_repeated =
    (button != 0 && last_gi != NULL && new_gi == last_gi);

  gadget_pressed_delay_reached =
    DelayReached(&pressed_delay, pressed_delay_value);

  gadget_released =		(release_event && last_gi != NULL);
  gadget_released_inside =	(gadget_released && new_gi == last_gi);
  gadget_released_off_borders =	(gadget_released && new_gi != last_gi);

  gadget_moving =	      (button != 0 && last_gi != NULL && mouse_moving);
  gadget_moving_inside =      (gadget_moving && new_gi == last_gi);
  gadget_moving_off_borders = (gadget_moving && new_gi != last_gi);

  /* when handling selectbox, set additional state values */
  if (gadget_released_inside && (last_gi->type & GD_TYPE_SELECTBOX))
    gadget_released_inside_select_area = insideSelectboxArea(last_gi, mx, my);
  else
    gadget_released_inside_select_area = FALSE;

  /* setting state for handling over-large selectbox */
  if (keep_selectbox_open && (press_event || !mouse_inside_select_line))
    keep_selectbox_open = FALSE;

  /* if new gadget pressed, store this gadget  */
  if (gadget_pressed)
    last_gi = new_gi;

  /* 'gi' is actually handled gadget */
  gi = last_gi;

  /* if gadget is scrollbar, choose mouse position value */
  if (gi && gi->type & GD_TYPE_SCROLLBAR)
    scrollbar_mouse_pos =
      (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL ? mx - gi->x : my - gi->y);

  /* if mouse button released, no gadget needs to be handled anymore */
  if (gadget_released)
  {
    if (gi->type & GD_TYPE_SELECTBOX &&
	(keep_selectbox_open ||
	 mouse_released_where_pressed ||
	 !gadget_released_inside_select_area ||
	 !CURRENT_OPTION_SELECTABLE(gi)))	    /* selectbox stays open */
    {
      gi->selectbox.stay_open = TRUE;
      pressed_mx = 0;
      pressed_my = 0;
    }
    else if (!(gi->type & GD_TYPE_TEXT_INPUT ||
	       gi->type & GD_TYPE_TEXT_AREA))	    /* text input stays open */
      last_gi = NULL;
  }

  /* modify event position values even if no gadget is pressed */
  if (button == 0 && !release_event)
    gi = new_gi;

  /* if new gadget or if no gadget was pressed, release stopped processing */
  if (gadget_pressed || new_gi == NULL)
    gadget_stopped = FALSE;

  /* if gadget was stopped while being handled, stop gadget processing here */
  if (gadget_stopped)
    return TRUE;

  if (gi != NULL)
  {
    int last_x = gi->event.x;
    int last_y = gi->event.y;
    int last_mx = gi->event.mx;
    int last_my = gi->event.my;

    gi->event.x = gi->event.mx = mx - gi->x;
    gi->event.y = gi->event.my = my - gi->y;

    if (gi->type == GD_TYPE_DRAWING_AREA)
    {
      gi->event.x /= gi->drawing.item_xsize;
      gi->event.y /= gi->drawing.item_ysize;

      if (last_x != gi->event.x || last_y != gi->event.y ||
	  ((last_mx != gi->event.mx || last_my != gi->event.my) &&
	   gi->event_mask & GD_EVENT_PIXEL_PRECISE))
	changed_position = TRUE;
    }
    else if (gi->type & GD_TYPE_TEXT_INPUT && button != 0 && !motion_status)
    {
      int old_cursor_position = gi->textinput.cursor_position;

      /* if mouse button pressed inside activated text gadget, set cursor */
      gi->textinput.cursor_position =
	(mx - gi->x - gi->border.xsize) / getFontWidth(gi->font);

      if (gi->textinput.cursor_position < 0)
	gi->textinput.cursor_position = 0;
      else if (gi->textinput.cursor_position > strlen(gi->textinput.value))
	gi->textinput.cursor_position = strlen(gi->textinput.value);

      if (gi->textinput.cursor_position != old_cursor_position)
	DrawGadget(gi, DG_PRESSED, gi->direct_draw);

      if (press_event)
	StartTextInput(gi->x, gi->y, gi->width, gi->height);
    }
    else if (gi->type & GD_TYPE_TEXT_AREA && button != 0 && !motion_status)
    {
      int old_cursor_position = gi->textarea.cursor_position;
      int x = (mx - gi->x - gi->border.xsize) / getFontWidth(gi->font);
      int y = (my - gi->y - gi->border.ysize) / getFontHeight(gi->font);

      x = (x < 0 ? 0 : x >= gi->textarea.xsize ? gi->textarea.xsize - 1 : x);
      y = (y < 0 ? 0 : y >= gi->textarea.ysize ? gi->textarea.ysize - 1 : y);

      setTextAreaCursorXY(gi, x, y);

      if (gi->textarea.cursor_position != old_cursor_position)
	DrawGadget(gi, DG_PRESSED, gi->direct_draw);

      if (press_event)
	StartTextInput(gi->x, gi->y, gi->width, gi->height);
    }
    else if (gi->type & GD_TYPE_SELECTBOX && gi->selectbox.open &&
	     !keep_selectbox_open)
    {
      int old_index = gi->selectbox.current_index;

      /* if mouse moving inside activated selectbox, select value */
      if (my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height)
	gi->selectbox.current_index =
	  (my - gi->selectbox.y - gi->border.ysize) / getFontHeight(gi->font);

      if (gi->selectbox.current_index < 0)
	gi->selectbox.current_index = 0;
      else if (gi->selectbox.current_index > gi->selectbox.num_values - 1)
	gi->selectbox.current_index = gi->selectbox.num_values - 1;

      if (gi->selectbox.current_index != old_index)
	DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
  }

  /* handle gadget popup info text */
  if (last_info_gi != new_gi ||
      (new_gi && new_gi->type == GD_TYPE_DRAWING_AREA && changed_position))
  {
    if (new_gi != NULL && (button == 0 || new_gi == last_gi))
    {
      new_gi->event.type = GD_EVENT_INFO_ENTERING;
      new_gi->callback_info(new_gi);
    }
    else if (last_info_gi != NULL && last_info_gi->mapped)
    {
      last_info_gi->event.type = GD_EVENT_INFO_LEAVING;
      last_info_gi->callback_info(last_info_gi);
    }

    last_info_gi = new_gi;
  }

  gadget_draggable = (gi && gi->type & GD_TYPE_SCROLLBAR);

  /* reset drag position for newly pressed scrollbar to "not dragging" */
  if (gadget_pressed && gadget_draggable)
    gi->scrollbar.drag_position = -1;

  gadget_dragging = (gadget_draggable && gi->scrollbar.drag_position != -1);

  /* clicking next to a scrollbar to move it is not considered "moving" */
  if (gadget_draggable && !gadget_dragging)
    gadget_moving = FALSE;

  /* when leaving scrollbar area when jump-scrolling, stop gadget processing */
  if (gadget_draggable && !gadget_dragging && gadget_moving_off_borders)
    gadget_stopped = TRUE;

  if ((gadget_pressed) ||
      (gadget_pressed_repeated && gadget_pressed_delay_reached))
  {
    if (gadget_pressed)		/* gadget pressed the first time */
    {
      /* initialize delay counter */
      DelayReached(&pressed_delay, 0);

      /* start gadget delay with longer delay after first click on gadget */
      pressed_delay_value = GADGET_FRAME_DELAY_FIRST;
    }
    else			/* gadget hold pressed for some time */
    {
      /* after first repeated gadget click, continue with shorter delay value */
      pressed_delay_value = GADGET_FRAME_DELAY;
    }

    if (gi->type & GD_TYPE_SCROLLBAR && !gadget_dragging)
    {
      int mpos = (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL ? mx    : my);
      int gpos = (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL ? gi->x : gi->y);
      int slider_start = gpos + gi->scrollbar.position;
      int slider_end   = gpos + gi->scrollbar.position + gi->scrollbar.size - 1;
      boolean inside_slider = (mpos >= slider_start && mpos <= slider_end);

      if (IS_WHEEL_BUTTON(button) || !inside_slider)
      {
	/* click scrollbar one scrollbar length up/left or down/right */

	struct GadgetScrollbar *gs = &gi->scrollbar;
	int old_item_position = gs->item_position;
	int item_steps = gs->items_visible - 1;
	int item_direction = (mpos < gpos + gi->scrollbar.position ? -1 : +1);

	if (IS_WHEEL_BUTTON(button))
	{
	  boolean scroll_single_step = ((GetKeyModState() & KMOD_Alt) != 0);

	  item_steps = (scroll_single_step ? 1 : wheel_steps);
	  item_direction = (button == MB_WHEEL_UP ||
			    button == MB_WHEEL_LEFT ? -1 : +1);
	}

	changed_position = FALSE;

	gs->item_position += item_steps * item_direction;

	if (gs->item_position < 0)
	  gs->item_position = 0;
	else if (gs->item_position > gs->items_max - gs->items_visible)
	  gs->item_position = gs->items_max - gs->items_visible;

	if (old_item_position != gs->item_position)
	{
	  gi->event.item_position = gs->item_position;
	  changed_position = TRUE;
	}

	ModifyGadget(gi, GDI_SCROLLBAR_ITEM_POSITION, gs->item_position,
		     GDI_END);

	gi->state = GD_BUTTON_UNPRESSED;
	gi->event.type = GD_EVENT_MOVING;
	gi->event.off_borders = FALSE;

	if (gi->event_mask & GD_EVENT_MOVING && changed_position)
	  gi->callback_action(gi);

	return TRUE;
      }
      else
      {
	/* don't handle this scrollbar anymore when mouse position reached */
	if (gadget_pressed_repeated)
	{
	  gadget_stopped = TRUE;

	  return TRUE;
	}
      }
    }
  }

  if (gadget_pressed)
  {
    PlayGadgetSoundActivating();

    if (gi->type == GD_TYPE_CHECK_BUTTON)
    {
      gi->checked = !gi->checked;
    }
    else if (gi->type == GD_TYPE_RADIO_BUTTON)
    {
      struct GadgetInfo *rgi = gadget_list_first_entry;

      while (rgi)
      {
	if (rgi->mapped &&
	    rgi->type == GD_TYPE_RADIO_BUTTON &&
	    rgi->radio_nr == gi->radio_nr &&
	    rgi != gi)
	{
	  rgi->checked = FALSE;
	  DrawGadget(rgi, DG_UNPRESSED, rgi->direct_draw);
	}

	rgi = rgi->next;
      }

      gi->checked = TRUE;
    }
    else if (gi->type & GD_TYPE_SCROLLBAR)
    {
      int mpos = (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL ? mx    : my);
      int gpos = (gi->type == GD_TYPE_SCROLLBAR_HORIZONTAL ? gi->x : gi->y);
      int slider_start = gpos + gi->scrollbar.position;
      int slider_end   = gpos + gi->scrollbar.position + gi->scrollbar.size - 1;
      boolean inside_slider = (mpos >= slider_start && mpos <= slider_end);

      if (!IS_WHEEL_BUTTON(button) && inside_slider)
      {
	/* start dragging scrollbar */
	gi->scrollbar.drag_position =
	  scrollbar_mouse_pos - gi->scrollbar.position;
      }
    }
    else if (gi->type & GD_TYPE_SELECTBOX)
    {
      /* keep selectbox open in case of over-large selectbox */
      keep_selectbox_open = (mouse_inside_select_line &&
			     mouse_inside_select_area);
    }

    DrawGadget(gi, DG_PRESSED, gi->direct_draw);

    gi->state = GD_BUTTON_PRESSED;
    gi->event.type = GD_EVENT_PRESSED;
    gi->event.button = button;
    gi->event.off_borders = FALSE;

    if (gi->event_mask & GD_EVENT_PRESSED)
      gi->callback_action(gi);
  }

  if (gadget_pressed_repeated)
  {
    gi->event.type = GD_EVENT_PRESSED;

    if (gi->event_mask & GD_EVENT_REPEATED && gadget_pressed_delay_reached)
      gi->callback_action(gi);
  }

  if (gadget_moving)
  {
    if (gi->type & GD_TYPE_BUTTON)
    {
      if (gadget_moving_inside && gi->state == GD_BUTTON_UNPRESSED)
	DrawGadget(gi, DG_PRESSED, gi->direct_draw);
      else if (gadget_moving_off_borders && gi->state == GD_BUTTON_PRESSED)
	DrawGadget(gi, DG_UNPRESSED, gi->direct_draw);
    }
    else if (gi->type & GD_TYPE_SELECTBOX && !keep_selectbox_open)
    {
      int old_index = gi->selectbox.current_index;

      /* if mouse moving inside activated selectbox, select value */
      if (my >= gi->selectbox.y && my < gi->selectbox.y + gi->selectbox.height)
	gi->selectbox.current_index =
	  (my - gi->selectbox.y - gi->border.ysize) / getFontHeight(gi->font);

      if (gi->selectbox.current_index < 0)
	gi->selectbox.current_index = 0;
      else if (gi->selectbox.current_index > gi->selectbox.num_values - 1)
	gi->selectbox.current_index = gi->selectbox.num_values - 1;

      if (gi->selectbox.current_index != old_index)
	DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (gi->type & GD_TYPE_SCROLLBAR)
    {
      struct GadgetScrollbar *gs = &gi->scrollbar;
      int old_item_position = gs->item_position;

      gs->position = scrollbar_mouse_pos - gs->drag_position;

      /* make sure to always precisely reach end positions when dragging */
      if (gs->position <= 0)
      {
	gs->position = 0;
	gs->item_position = 0;
      }
      else if (gs->position >= gs->position_max)
      {
	gs->position = gs->position_max;
	gs->item_position = gs->items_max - gs->items_visible;
      }
      else
      {
	gs->item_position =
	  gs->items_max * (gs->position + gs->correction) / gs->size_max_cmp;
      }

      if (gs->item_position < 0)
	gs->item_position = 0;
      if (gs->item_position > gs->items_max - 1)
	gs->item_position = gs->items_max - 1;

      if (old_item_position != gs->item_position)
      {
	gi->event.item_position = gs->item_position;
	changed_position = TRUE;
      }

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }

    gi->state = (gadget_moving_inside || gadget_draggable ?
		 GD_BUTTON_PRESSED : GD_BUTTON_UNPRESSED);
    gi->event.type = GD_EVENT_MOVING;
    gi->event.off_borders = gadget_moving_off_borders;

    if (gi->event_mask & GD_EVENT_MOVING && changed_position &&
	(gadget_moving_inside || gi->event_mask & GD_EVENT_OFF_BORDERS))
      gi->callback_action(gi);
  }

  if (gadget_released_inside)
  {
    boolean deactivate_gadget = TRUE;
    boolean gadget_changed = TRUE;

    if (gi->type & GD_TYPE_SELECTBOX)
    {
      if (keep_selectbox_open ||
	  mouse_released_where_pressed ||
	  !gadget_released_inside_select_area ||
	  !CURRENT_OPTION_SELECTABLE(gi))	    /* selectbox stays open */
      {
	deactivate_gadget = FALSE;
	gadget_changed = FALSE;
      }
      else if (gi->selectbox.index != gi->selectbox.current_index)
	gi->selectbox.index = gi->selectbox.current_index;
      else
	gadget_changed = FALSE;
    }

    if (deactivate_gadget &&
	!(gi->type & GD_TYPE_TEXT_INPUT ||
	  gi->type & GD_TYPE_TEXT_AREA))	    /* text input stays open */
      DrawGadget(gi, DG_UNPRESSED, gi->direct_draw);

    gi->state = GD_BUTTON_UNPRESSED;
    gi->event.type = GD_EVENT_RELEASED;

    if ((gi->event_mask & GD_EVENT_RELEASED) && gadget_changed)
    {
      gi->callback_action(gi);
    }
  }

  if (gadget_released_off_borders)
  {
    if (gi->type & GD_TYPE_SCROLLBAR)
      DrawGadget(gi, DG_UNPRESSED, gi->direct_draw);

    gi->state = GD_BUTTON_UNPRESSED;
    gi->event.type = GD_EVENT_RELEASED;

    if (gi->event_mask & GD_EVENT_RELEASED &&
	gi->event_mask & GD_EVENT_OFF_BORDERS)
      gi->callback_action(gi);
  }

  /* handle gadgets unmapped/mapped between pressing and releasing */
  if (release_event && !gadget_released && new_gi)
    new_gi->state = GD_BUTTON_UNPRESSED;

  return (gadget_pressed || gadget_pressed_repeated ||
	  gadget_released || gadget_moving);
}

static void insertCharIntoTextArea(struct GadgetInfo *gi, char c)
{
  char text[MAX_GADGET_TEXTSIZE + 1];
  int cursor_position = gi->textarea.cursor_position;

  if (strlen(gi->textarea.value) >= MAX_GADGET_TEXTSIZE) /* no space left */
    return;

  strcpy(text, gi->textarea.value);
  strcpy(&gi->textarea.value[cursor_position + 1], &text[cursor_position]);
  gi->textarea.value[cursor_position] = c;

  setTextAreaCursorPosition(gi, gi->textarea.cursor_position + 1);
}

boolean HandleGadgetsKeyInput(Key key)
{
  struct GadgetInfo *gi = last_gi;

  if (gi == NULL || gi->deactivated || !gi->mapped ||
      !(gi->type & GD_TYPE_TEXT_INPUT ||
	gi->type & GD_TYPE_TEXT_AREA ||
	gi->type & GD_TYPE_SELECTBOX))
    return FALSE;

  if (key == KSYM_Escape)
  {
    StopTextInput();
  }
  else if (key == KSYM_Return)	/* valid for both text input and selectbox */
  {
    boolean gadget_changed = ((gi->event_mask & GD_EVENT_TEXT_RETURN) != 0);

    if (gi->type & GD_TYPE_TEXT_INPUT)
    {
      CheckRangeOfNumericInputGadget(gi);

      if (!strEqual(gi->textinput.last_value, gi->textinput.value))
	strcpy(gi->textinput.last_value, gi->textinput.value);
      else
	gadget_changed = FALSE;

      StopTextInput();
    }
    else if (gi->type & GD_TYPE_SELECTBOX)
    {
      if (gi->selectbox.index != gi->selectbox.current_index)
	gi->selectbox.index = gi->selectbox.current_index;
      else
	gadget_changed = FALSE;
    }

    if (gi->type & GD_TYPE_TEXT_AREA)
    {
      insertCharIntoTextArea(gi, '\n');

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else
    {
      DrawGadget(gi, DG_UNPRESSED, gi->direct_draw);

      gi->event.type = GD_EVENT_TEXT_RETURN;

      last_gi = NULL;
    }

    if (gadget_changed)
      gi->callback_action(gi);
  }
  else if (gi->type & GD_TYPE_TEXT_INPUT)	/* only valid for text input */
  {
    char text[MAX_GADGET_TEXTSIZE + 1];
    int text_length = strlen(gi->textinput.value);
    int cursor_pos = gi->textinput.cursor_position;
    char letter = getCharFromKey(key);
    boolean legal_letter = (gi->type == GD_TYPE_TEXT_INPUT_NUMERIC ?
			    letter >= '0' && letter <= '9' :
			    letter != 0);

    if (legal_letter && text_length < gi->textinput.size)
    {
      strcpy(text, gi->textinput.value);
      strcpy(&gi->textinput.value[cursor_pos + 1], &text[cursor_pos]);
      gi->textinput.value[cursor_pos] = letter;
      gi->textinput.cursor_position++;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Left && cursor_pos > 0)
    {
      gi->textinput.cursor_position--;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Right && cursor_pos < text_length)
    {
      gi->textinput.cursor_position++;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_BackSpace && cursor_pos > 0)
    {
      strcpy(text, gi->textinput.value);
      strcpy(&gi->textinput.value[cursor_pos - 1], &text[cursor_pos]);
      gi->textinput.cursor_position--;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Delete && cursor_pos < text_length)
    {
      strcpy(text, gi->textinput.value);
      strcpy(&gi->textinput.value[cursor_pos], &text[cursor_pos + 1]);

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
  }
  else if (gi->type & GD_TYPE_TEXT_AREA)	/* only valid for text area */
  {
    char text[MAX_GADGET_TEXTSIZE + 1];
    int text_length = strlen(gi->textarea.value);
    int area_ysize = gi->textarea.ysize;
    int cursor_x_pref = gi->textarea.cursor_x_preferred;
    int cursor_y = gi->textarea.cursor_y;
    int cursor_pos = gi->textarea.cursor_position;
    char letter = getCharFromKey(key);
    boolean legal_letter = (letter != 0);

    if (legal_letter)
    {
      insertCharIntoTextArea(gi, letter);

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Left && cursor_pos > 0)
    {
      setTextAreaCursorPosition(gi, gi->textarea.cursor_position - 1);

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Right && cursor_pos < text_length)
    {
      setTextAreaCursorPosition(gi, gi->textarea.cursor_position + 1);

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Up && cursor_y > 0)
    {
      setTextAreaCursorXY(gi, cursor_x_pref, cursor_y - 1);
      gi->textarea.cursor_x_preferred = cursor_x_pref;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Down && cursor_y < area_ysize - 1)
    {
      setTextAreaCursorXY(gi, cursor_x_pref, cursor_y + 1);
      gi->textarea.cursor_x_preferred = cursor_x_pref;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_BackSpace && cursor_pos > 0)
    {
      strcpy(text, gi->textarea.value);
      strcpy(&gi->textarea.value[cursor_pos - 1], &text[cursor_pos]);

      setTextAreaCursorPosition(gi, gi->textarea.cursor_position - 1);

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Delete && cursor_pos < text_length)
    {
      strcpy(text, gi->textarea.value);
      strcpy(&gi->textarea.value[cursor_pos], &text[cursor_pos + 1]);

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
  }
  else if (gi->type & GD_TYPE_SELECTBOX)	/* only valid for selectbox */
  {
    int index = gi->selectbox.current_index;
    int num_values = gi->selectbox.num_values;

    if (key == KSYM_Up && index > 0)
    {
      gi->selectbox.current_index--;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
    else if (key == KSYM_Down && index < num_values - 1)
    {
      gi->selectbox.current_index++;

      DrawGadget(gi, DG_PRESSED, gi->direct_draw);
    }
  }

  return TRUE;
}
