#include "psp2_input.h"
#include "psp2_touch.h"
#if defined(__vita__)
#include "psp2_kbdvita.h"
#endif
#include <math.h>

int lastmx = 0;
int lastmy = 0;
int vita_mousepointer_visible = 1;
static int hiresDX = 0;
static int hiresDY = 0;
static int holding_R = 0;
static int can_use_IME_keyboard = 1;
static unsigned int focus_change_key = SDLK_F9;
static int can_change_focus = 1;

int PSP2_WaitEvent(SDL_Event *event) {

	int ret = SDL_WaitEvent(event);
	if(event != NULL) {
		PSP2_HandleTouch(event);
		switch (event->type) {
			case SDL_MOUSEMOTION:
				lastmx = event->motion.x;
				lastmy = event->motion.y;
				vita_mousepointer_visible = 1;
			break;
			case SDL_JOYBUTTONDOWN:
				if (event->jbutton.which==0) { // Only Joystick 0 controls the mouse
					switch (event->jbutton.button) {
						case PAD_L:
							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = SDL_BUTTON_LEFT;
							event->button.state = SDL_PRESSED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_R:
							holding_R = 1;
							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = SDL_BUTTON_RIGHT;
							event->button.state = SDL_PRESSED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_SQUARE:
							if (holding_R) {
								if (can_change_focus) {
									// set focus to previous player
									if (focus_change_key > SDLK_F5 && focus_change_key <= SDLK_F8)
										focus_change_key--;
									else if (focus_change_key == SDLK_F5)
										focus_change_key = SDLK_F9;
									event->type = SDL_KEYDOWN;
									event->key.keysym.sym = focus_change_key;
									event->key.keysym.mod = 0;
									event->key.repeat = 0;
									can_change_focus = 0;
								}
							} else {
								event->type = SDL_MOUSEBUTTONDOWN;
								event->button.button = SDL_BUTTON_LEFT;
								event->button.state = SDL_PRESSED;
								event->button.x = lastmx;
								event->button.y = lastmy;
							}
							break;
						case PAD_TRIANGLE:
							if (holding_R) {
								if (can_change_focus) {
									// R+Triangle: set focus to next player
									if (focus_change_key >= SDLK_F5 && focus_change_key < SDLK_F8)
										focus_change_key++;
									else if (focus_change_key == SDLK_F9)
										focus_change_key = SDLK_F5;
									event->type = SDL_KEYDOWN;
									event->key.keysym.sym = focus_change_key;
									event->key.keysym.mod = 0;
									event->key.repeat = 0;
									can_change_focus = 0;
								}
							} else {
								event->type = SDL_MOUSEBUTTONDOWN;
								event->button.button = SDL_BUTTON_RIGHT;
								event->button.state = SDL_PRESSED;
								event->button.x = lastmx;
								event->button.y = lastmy;
							}
							break;
						case PAD_UP: // intentional fall-through
						case PAD_DOWN:
						case PAD_LEFT:
						case PAD_RIGHT:
						case PAD_CROSS:
						case PAD_CIRCLE:
							vita_mousepointer_visible = 0;
							break;
						default:
							break;
					}
				}
				break;
			case SDL_JOYBUTTONUP:
				if (event->jbutton.which==0) {// Only Joystick 0 controls the mouse
					switch (event->jbutton.button) {
						case PAD_L:
							event->type = SDL_MOUSEBUTTONUP;
							event->button.button = SDL_BUTTON_LEFT;
							event->button.state = SDL_RELEASED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_R:
							holding_R = 0;
							event->type = SDL_MOUSEBUTTONUP;
							event->button.button = SDL_BUTTON_RIGHT;
							event->button.state = SDL_RELEASED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_SQUARE:
							if (holding_R) {
								if (!can_change_focus) {
									// release focus key
									event->type = SDL_KEYUP;
									event->key.keysym.sym = focus_change_key;
									event->key.keysym.mod = 0;
									event->key.repeat = 0;
									can_change_focus = 1;
								}
							} else {
								event->type = SDL_MOUSEBUTTONUP;
								event->button.button = SDL_BUTTON_LEFT;
								event->button.state = SDL_RELEASED;
								event->button.x = lastmx;
								event->button.y = lastmy;
							}
							break;
						case PAD_TRIANGLE:
							if (holding_R) {
								if (!can_change_focus) {
									// release focus key
									event->type = SDL_KEYUP;
									event->key.keysym.sym = focus_change_key;
									event->key.keysym.mod = 0;
									event->key.repeat = 0;
									can_change_focus = 1;
								}
							} else {
								event->type = SDL_MOUSEBUTTONUP;
								event->button.button = SDL_BUTTON_RIGHT;
								event->button.state = SDL_RELEASED;
								event->button.x = lastmx;
								event->button.y = lastmy;
							}
							break;
						default:
							break;
					}
				}
			default:
				break;
		}
	}
	return ret;
}

void PSP2_HandleJoystickMouse(int analogX, int analogY) {
	rescaleAnalog( &analogX, &analogY, 3000);
	hiresDX += analogX;
	hiresDY += analogY;

	const int slowdown = 2048;

	if (hiresDX != 0 || hiresDY != 0) {
		int xrel = hiresDX / slowdown;
		int yrel = hiresDY / slowdown;
		hiresDX %= slowdown;
		hiresDY %= slowdown;
		if (xrel != 0 || yrel !=0) {
			SDL_Event event;
			event.type = SDL_MOUSEMOTION;
			event.motion.x = lastmx + xrel;
			event.motion.y = lastmy + yrel;
			event.motion.xrel = xrel;
			event.motion.yrel = yrel;
			SDL_PushEvent(&event);
		}
	}
}


void rescaleAnalog(int *x, int *y, int dead) {
	//radial and scaled deadzone
	//http://www.third-helix.com/2013/04/12/doing-thumbstick-dead-zones-right.html

	float analogX = (float) *x;
	float analogY = (float) *y;
	float deadZone = (float) dead;
	float maximum = 32768.0f;
	float magnitude = sqrt(analogX * analogX + analogY * analogY);
	if (magnitude >= deadZone)
	{
		float scalingFactor = maximum / magnitude * (magnitude - deadZone) / (maximum - deadZone);		
		*x = (int) (analogX * scalingFactor);
		*y = (int) (analogY * scalingFactor);
	} else {
		*x = 0;
		*y = 0;
	}
}

void PSP2_StartTextInput(char *initial_text, int multiline) {
#if !defined(__vita__)
	return;
#else
	if (!can_use_IME_keyboard)
	return;

	can_use_IME_keyboard = 0;
	char *text = kbdvita_get("Enter New Text:", initial_text, 600, multiline);
	if (text != NULL)
	{
		if (multiline) {
			// prevent mouse button state from locking the cursor while virtual keys are entered
			SDL_Event ev1;
			ev1.type = SDL_MOUSEBUTTONUP;
			ev1.button.button = SDL_BUTTON_RIGHT;
			ev1.button.state = SDL_RELEASED;
			ev1.button.x = lastmx;
			ev1.button.y = lastmy;
			SDL_PushEvent(&ev1);
			SDL_Event ev2;
			ev2.type = SDL_MOUSEBUTTONUP;
			ev2.button.button = SDL_BUTTON_LEFT;
			ev2.button.state = SDL_RELEASED;
			ev2.button.x = lastmx;
			ev2.button.y = lastmy;
			SDL_PushEvent(&ev2);
		}
		// the max textarea size is 30x20, which is 600 chars max
		for (int i = 0; i < 600; i++) {
			SDL_Event down_event;
			down_event.type = SDL_KEYDOWN;
			down_event.key.keysym.sym = SDLK_BACKSPACE;
			down_event.key.keysym.mod = 0;
			SDL_PushEvent(&down_event);
			SDL_Event up_event;
			up_event.type = SDL_KEYUP;
			up_event.key.keysym.sym = SDLK_BACKSPACE;
			up_event.key.keysym.mod = 0;
			SDL_PushEvent(&up_event);
		}
		for (int i = 0; i < 600; i++) {
			SDL_Event down_event;
			down_event.type = SDL_KEYDOWN;
			down_event.key.keysym.sym = SDLK_DELETE;
			down_event.key.keysym.mod = 0;
			SDL_PushEvent(&down_event);
			SDL_Event up_event;
			up_event.type = SDL_KEYUP;
			up_event.key.keysym.sym = SDLK_DELETE;
			up_event.key.keysym.mod = 0;
			SDL_PushEvent(&up_event);
		}
		int i=0;
		while (text[i]!=0 && i<600) {
			if (text[i]>='A' && text[i]<='Z')
				text[i]+=32;
			// convert lf to return
			if (text[i]==10)
				text[i]=SDLK_RETURN;
			SDL_Event down_event;
			down_event.type = SDL_KEYDOWN;
			down_event.key.keysym.sym = text[i];
			down_event.key.keysym.mod = 0;
			SDL_PushEvent(&down_event);
			SDL_Event up_event;
			up_event.type = SDL_KEYUP;
			up_event.key.keysym.sym = text[i];
			up_event.key.keysym.mod = 0;
			SDL_PushEvent(&up_event);
			i++;
		}
	}
	// append return to single-line text entry
	if (!multiline) {
		SDL_Event down_event;
		down_event.type = SDL_KEYDOWN;
		down_event.key.keysym.sym = SDLK_RETURN;
		down_event.key.keysym.mod = 0;
		SDL_PushEvent(&down_event);
		SDL_Event up_event;
		up_event.type = SDL_KEYUP;
		up_event.key.keysym.sym = SDLK_RETURN;
		up_event.key.keysym.mod = 0;
		SDL_PushEvent(&up_event);
	}
#endif
}

void PSP2_StopTextInput() {
	can_use_IME_keyboard = 1;
}