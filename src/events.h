// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// events.h
// ============================================================================

#ifndef EVENTS_H
#define EVENTS_H

#include "main.h"

boolean NextValidEvent(Event *);

void EventLoop(void);
void HandleOtherEvents(Event *);
void ClearEventQueue(void);
void ClearPlayerAction(void);
void SleepWhileUnmapped(void);

void HandleExposeEvent(ExposeEvent *);
void HandleButtonEvent(ButtonEvent *);
void HandleMotionEvent(MotionEvent *);
#if defined(TARGET_SDL2)
void HandleWheelEvent(WheelEvent *);
void HandleWindowEvent(WindowEvent *);
void HandleFingerEvent(FingerEvent *);
void HandleTextEvent(TextEvent *);
void HandlePauseResumeEvent(PauseResumeEvent *);
#endif
void HandleKeysDebug(Key);
void HandleKeyEvent(KeyEvent *);
void HandleFocusEvent(FocusChangeEvent *);
void HandleClientMessageEvent(ClientMessageEvent *);
void HandleWindowManagerEvent(Event *);

void HandleToonAnimations(void);

void HandleButton(int, int, int, int);
void HandleKey(Key, int);
void HandleJoystick();
void HandleSpecialGameControllerButtons(Event *);
void HandleSpecialGameControllerKeys(Key, int);

#endif
