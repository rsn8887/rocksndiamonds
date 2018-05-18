#ifndef PSP2_INPUT_H
#define PSP2_INPUT_H

#include <SDL2/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

int PSP2_WaitEvent(SDL_Event *event);

void PSP2_HandleJoystickMouse(int analogX, int analogY);

//SDLKey getKey(Uint8 button);
void rescaleAnalog(int *x, int *y, int dead);

#define PAD_UP 8
#define PAD_DOWN 6
#define PAD_LEFT 7
#define PAD_RIGHT 9
#define PAD_TRIANGLE 0
#define PAD_SQUARE 3
#define PAD_CROSS 2
#define PAD_CIRCLE 1
#define PAD_SELECT 10
#define PAD_START 11
#define PAD_L 4
#define PAD_R 5

#ifdef __cplusplus
}
#endif

#endif
