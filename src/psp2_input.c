#include "psp2_input.h"
#include "psp2_touch.h"
#include <math.h>

int lastmx = 0;
int lastmy = 0;
int hiresDX = 0;
int hiresDY = 0;
int holding_R = 0;
int vita_mousepointer_visible = 1;

int PSP2_WaitEvent(SDL_Event *event) {

	int ret = SDL_WaitEvent(event);
	if(event != NULL) {
		psp2HandleTouch(event);
		switch (event->type) {
			case SDL_MOUSEMOTION:
				lastmx = event->motion.x;
				lastmy = event->motion.y;
				vita_mousepointer_visible = 1;
			break;
			case SDL_JOYBUTTONDOWN:
				if (event->jbutton.which==0) { // Only Joystick 0 controls the mouse
					switch (event->jbutton.button) {
						case PAD_L: // intentional fall-through
						case PAD_SQUARE:
							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = SDL_BUTTON_LEFT;
							event->button.state = SDL_PRESSED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_R:
							holding_R = 1; // intentional fall-through
						case PAD_TRIANGLE:
							event->type = SDL_MOUSEBUTTONDOWN;
							event->button.button = SDL_BUTTON_RIGHT;
							event->button.state = SDL_PRESSED;
							event->button.x = lastmx;
							event->button.y = lastmy;
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
						case PAD_L: // intentional fall-through
						case PAD_SQUARE:
							event->type = SDL_MOUSEBUTTONUP;
							event->button.button = SDL_BUTTON_LEFT;
							event->button.state = SDL_RELEASED;
							event->button.x = lastmx;
							event->button.y = lastmy;
							break;
						case PAD_R:
							holding_R = 0; // intentional fall-through
						case PAD_TRIANGLE:
							event->type = SDL_MOUSEBUTTONUP;
							event->button.button = SDL_BUTTON_RIGHT;
							event->button.state = SDL_RELEASED;
							event->button.x = lastmx;
							event->button.y = lastmy;
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