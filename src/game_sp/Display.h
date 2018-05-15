// ----------------------------------------------------------------------------
// Display.h
// ----------------------------------------------------------------------------

#ifndef DISPLAY_H
#define DISPLAY_H

#include "global.h"


extern int ScreenScrollXPos, ScreenScrollYPos;

extern int ExplosionShake, ExplosionShakeMurphy;
extern boolean NoDisplayFlag;

extern int DisplayMinX, DisplayMaxX;
extern int DisplayMinY, DisplayMaxY;

extern void subDisplayLevel();
extern void ScrollTo(int, int);
extern void ScrollTowards(int, int);
extern void SoftScrollTo(int, int, int, int);

#endif /* DISPLAY_H */
