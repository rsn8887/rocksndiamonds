//
// Created by rsn8887 on 02/05/18.
#ifndef PSP2_TOUCH_H
#define PSP2_TOUCH_H

#include <SDL2/SDL.h>
#if defined(__vita__)
#include <psp2/touch.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void PSP2_HandleTouch(SDL_Event *event);
void PSP2_FinishSimulatedMouseClicks(void);

#ifdef __cplusplus
}
#endif
#endif /* PSP2_TOUCH_H */
