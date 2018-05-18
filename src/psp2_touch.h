//
// Created by rsn8887 on 02/05/18.
#ifndef PSP2_TOUCH_H
#define PSP2_TOUCH_H

#include <SDL2/SDL.h>
#include <psp2/touch.h>

#ifdef __cplusplus
extern "C" {
#endif

void psp2HandleTouch(SDL_Event *event);
static void preprocessEvents(SDL_Event *event);
enum {
	MAX_NUM_FINGERS = 3, // number of fingers to track per panel
	MAX_TAP_TIME = 250, // taps longer than this will not result in mouse click events
	MAX_TAP_MOTION_DISTANCE = 10, // max distance finger motion in Vita screen pixels to be considered a tap
	SIMULATED_CLICK_DURATION = 50, // time in ms how long simulated mouse clicks should be
}; // track three fingers per panel

typedef struct {
	int id; // -1: no touch
	Uint32 timeLastDown;
	int lastX; // last known screen coordinates
	int lastY; // last known screen coordinates
	float lastDownX; // SDL touch coordinates when last pressed down
	float lastDownY; // SDL touch coordinates when last pressed down
} Touch;

Touch _finger[SCE_TOUCH_PORT_MAX_NUM][MAX_NUM_FINGERS]; // keep track of finger status

typedef enum DraggingType {
	DRAG_NONE = 0,
	DRAG_TWO_FINGER,
	DRAG_THREE_FINGER,
} DraggingType;

DraggingType _multiFingerDragging[SCE_TOUCH_PORT_MAX_NUM]; // keep track whether we are currently drag-and-dropping

unsigned int _simulatedClickStartTime[SCE_TOUCH_PORT_MAX_NUM][2]; // initiation time of last simulated left or right click (zero if no click)

static void preprocessFingerDown(SDL_Event *event);
static void preprocessFingerUp(SDL_Event *event);
static void preprocessFingerMotion(SDL_Event *event);
static void finishSimulatedMouseClicks(void);

#ifdef __cplusplus
}
#endif
#endif /* PSP2_TOUCH_H */
