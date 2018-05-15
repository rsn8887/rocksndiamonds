// ----------------------------------------------------------------------------
// Display.c
// ----------------------------------------------------------------------------

#include "Display.h"


int ScreenScrollXPos, ScreenScrollYPos;

int ExplosionShake, ExplosionShakeMurphy;
boolean NoDisplayFlag;

int DisplayMinX, DisplayMaxX;
int DisplayMinY, DisplayMaxY;


void subDisplayLevel()
{
  if (NoDisplayFlag || ! LevelLoaded)
    return;

  DisplayLevel();
}

void ScrollTo(int X, int Y)
{
  if (NoDisplayFlag)
    return;

  X = ScrollDelta * (X / ScrollDelta);
  X = Max(X, ScrollMinX);
  X = Min(X, ScrollMaxX);
  Y = ScrollDelta * (Y / ScrollDelta);
  Y = Max(Y, ScrollMinY);
  Y = Min(Y, ScrollMaxY);

  DDScrollBuffer_ScrollTo(X, Y);
}

void ScrollTowards(int X, int Y)
{
  if (NoDisplayFlag)
    return;

  X = ScrollDelta * (X / ScrollDelta);
  X = Max(X, ScrollMinX);
  X = Min(X, ScrollMaxX);
  Y = ScrollDelta * (Y / ScrollDelta);
  Y = Max(Y, ScrollMinY);
  Y = Min(Y, ScrollMaxY);

  DDScrollBuffer_ScrollTowards(X, Y, 2 * ZoomFactor);
}

void SoftScrollTo(int X, int Y, int TimeMS, int FPS)
{
  if (NoDisplayFlag)
    return;

  X = ScrollDelta * (X / ScrollDelta);
  X = Max(X, ScrollMinX);
  X = Min(X, ScrollMaxX);
  Y = ScrollDelta * (Y / ScrollDelta);
  Y = Max(Y, ScrollMinY);
  Y = Min(Y, ScrollMaxY);

  DDScrollBuffer_SoftScrollTo(X, Y, TimeMS, FPS);
}
