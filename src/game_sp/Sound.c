// ----------------------------------------------------------------------------
// Sound.c
// ----------------------------------------------------------------------------

#include "Sound.h"


void subSoundFX(int si, int element, int action)
{
  int x = GetX(si);
  int y = GetY(si);

  PlayLevelSound_SP(x, y, element, action);
}
