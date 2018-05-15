#include "psp2_input.h"
#include <math.h>

int lastmx = 0;
int lastmy = 0;
int hiresDX = 0;
int hiresDY = 0;
int insideMenu = 1;
int holding_R = 0;

extern SDL_Joystick *vitaJoy0;

int PSP2_PollEvent(SDL_Event *event) {
	
	int ret = SDL_PollEvent(event);

	if(event != NULL) {

		switch (event->type) {

			case SDL_MOUSEMOTION:
				lastmx = event->motion.x;
				lastmy = event->motion.y;
				break;
			case SDL_JOYBUTTONDOWN:
				if (event->jbutton.which==0) // Only Joystick 0 controls the game
				{
					switch (event->jbutton.button) {
						case PAD_CIRCLE:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_ESCAPE;
							event->key.keysym.mod = 0;
							break;
						case PAD_CROSS:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_RETURN;
							event->key.keysym.mod = 0;
							break;
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
						case PAD_START:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_F1;
							event->key.keysym.mod = 0;
							break;
						case PAD_SELECT:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_F3;
							if (holding_R) {
								event->key.keysym.mod = KMOD_LSHIFT;
							}
							else {
								event->key.keysym.mod = 0;
							}
							break;
						case PAD_LEFT:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_LEFT;
							event->key.keysym.mod = 0;
							break;
						case PAD_RIGHT:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_RIGHT;
							event->key.keysym.mod = 0;
							break;
						case PAD_UP:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_UP;
							event->key.keysym.mod = 0;
							break;
						case PAD_DOWN:
							event->type = SDL_KEYDOWN;
							event->key.keysym.sym = SDLK_DOWN;
							event->key.keysym.mod = 0;
							break;
						default:
							break;
					}
				}
				break;

			case SDL_JOYBUTTONUP:
				if (event->jbutton.which==0) // Only Joystick 0 controls the game
				{
					switch (event->jbutton.button) {
						case PAD_CIRCLE:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_ESCAPE;
							event->key.keysym.mod = 0;
							break;
						case PAD_CROSS:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_RETURN;
							event->key.keysym.mod = 0;
							break;
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
						case PAD_START:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_F1;
							event->key.keysym.mod = 0;
							break;
						case PAD_SELECT:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_F3;
							if (holding_R) {
								event->key.keysym.mod = KMOD_LSHIFT;
							}
							else {
								event->key.keysym.mod = 0;
							}
							break;
						case PAD_LEFT:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_LEFT;
							event->key.keysym.mod = 0;
							break;
						case PAD_RIGHT:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_RIGHT;
							event->key.keysym.mod = 0;
							break;
						case PAD_UP:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_UP;
							event->key.keysym.mod = 0;
							break;
						case PAD_DOWN:
							event->type = SDL_KEYUP;
							event->key.keysym.sym = SDLK_DOWN;
							event->key.keysym.mod = 0;
							break;
						default:
							break;
					}
				}
				break;

			default:
				break;
		}
	}

	return ret;
}

void PSP2_HandleJoysticks() {
	SDL_JoystickUpdate();
	int analogX = SDL_JoystickGetAxis(vitaJoy0, 0);
	int analogY = SDL_JoystickGetAxis(vitaJoy0, 1);

	rescaleAnalog( &analogX, &analogY, 3000);
	hiresDX += analogX;
	hiresDY += analogY;

	const int slowdown = 4096;

	if (hiresDX != 0 || hiresDY != 0) {
		int xrel = hiresDX / slowdown;
		int yrel = hiresDY / slowdown;

		if (insideMenu) {
			SDL_WarpMouse(lastmx + xrel, lastmy + yrel);
		}
		else
		{
			SDL_Event event;
			event.type = SDL_MOUSEMOTION;
			event.motion.x = lastmx + xrel;
			event.motion.y = lastmy + yrel;
			event.motion.xrel = xrel;
			event.motion.yrel = yrel;
			SDL_PushEvent(&event);
		}

		hiresDX %= slowdown;
		hiresDY %= slowdown;
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