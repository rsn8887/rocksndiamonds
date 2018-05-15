// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// joystick.h
// ============================================================================

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "system.h"

#define JOYSTICK_NOT_AVAILABLE	0
#define	JOYSTICK_AVAILABLE	(1 << 0)
#define	JOYSTICK_ACTIVE		(1 << 1)
#define JOYSTICK_CONFIGURED	(1 << 2)
#define JOYSTICK_NOT_CONFIGURED	(1 << 3)
#define JOYSTICK_ACTIVATED	(JOYSTICK_AVAILABLE | JOYSTICK_ACTIVE)

#define MAX_JOYSTICK_NAME_LEN	40

#if defined(PLATFORM_FREEBSD)
#define DEV_JOYSTICK_0		"/dev/joy0"
#define DEV_JOYSTICK_1		"/dev/joy1"
#define DEV_JOYSTICK_2		"/dev/joy2"
#define DEV_JOYSTICK_3		"/dev/joy3"
#else
#define DEV_JOYSTICK_0		"/dev/js0"
#define DEV_JOYSTICK_1		"/dev/js1"
#define DEV_JOYSTICK_2		"/dev/js2"
#define DEV_JOYSTICK_3		"/dev/js3"
#endif

/* get these values from the program 'js' from the joystick package, */
/* set JOYSTICK_PERCENT to a threshold appropriate for your joystick */

#define JOYSTICK_MAX_AXIS_POS	32767

#define JOYSTICK_XLEFT		-JOYSTICK_MAX_AXIS_POS
#define JOYSTICK_XMIDDLE	0
#define JOYSTICK_XRIGHT		+JOYSTICK_MAX_AXIS_POS
#define JOYSTICK_YUPPER		-JOYSTICK_MAX_AXIS_POS
#define JOYSTICK_YMIDDLE	0
#define JOYSTICK_YLOWER		+JOYSTICK_MAX_AXIS_POS

#define JOYSTICK_PERCENT	25

#define JOY_NO_ACTION		0
#define JOY_LEFT		MV_LEFT
#define JOY_RIGHT		MV_RIGHT
#define JOY_UP			MV_UP
#define JOY_DOWN	       	MV_DOWN
#define JOY_BUTTON_1		KEY_BUTTON_1
#define JOY_BUTTON_2		KEY_BUTTON_2
#define JOY_BUTTON_SNAP		KEY_BUTTON_SNAP
#define JOY_BUTTON_DROP		KEY_BUTTON_DROP
#define JOY_MOTION		KEY_MOTION
#define JOY_BUTTON		KEY_BUTTON
#define JOY_ACTION		KEY_ACTION

#define JOY_BUTTON_NOT_PRESSED	0
#define JOY_BUTTON_PRESSED	1
#define JOY_BUTTON_NEW_PRESSED	2
#define JOY_BUTTON_NEW_RELEASED	3

char *getJoyNameFromJoySymbol(int);
int getJoySymbolFromJoyName(char *);
int getJoystickNrFromDeviceName(char *);
char *getDeviceNameFromJoystickNr(int);
char *getFormattedJoystickName(const char *);

void CheckJoystickData(void);
int Joystick(int);
int JoystickExt(int, boolean);
int JoystickButton(int);
int AnyJoystick(void);
int AnyJoystickButton(void);

void DeactivateJoystick();
void ActivateJoystick();

#endif	/* JOYSTICK_H */
