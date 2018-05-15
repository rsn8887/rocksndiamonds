// ----------------------------------------------------------------------------
// OrangeDisk.c
// ----------------------------------------------------------------------------

#include "OrangeDisk.h"


// ==========================================================================
//                              SUBROUTINE
// Animate/move orange disks (falling)
// ==========================================================================

void subAnimateOrangeDisks(int si)
{
  int ax, bl, dx, X, Y;

  ax = PlayField16[si];
  if (LowByte(ax) != fiOrangeDisk)
    return;

  if (ax >= 0x3008) // disk is falling
    goto loc_g_2804;

  if (ax >= 0x2008) // disk is in wait state before falling
    goto loc_g_27DA;

  if (PlayField16[si + FieldWidth] == 0)
    goto loc_g_27CF;

  return;

loc_g_27CF: // below is empty -> disk may start to fall
  MovHighByte(&PlayField16[si], 0x20);
  MovHighByte(&PlayField16[si + FieldWidth], fiOrangeDisk);

  return;

loc_g_27DA:
  if (PlayField16[si + FieldWidth] == 0)
  {
    PlayField16[si] = fiOrangeDisk;

    return;
  } // loc_g_27E8:

  bl = HighByte(PlayField16[si]) + 1;
  if (bl == 0x22) // wait phase is finished
    bl = 0x30;

  MovHighByte(&PlayField16[si], bl);

  return;

loc_g_2804: // disk is falling
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  dx = HighByte(PlayField16[si]) & 0x7;
  X = GetStretchX(si);
  Y = GetStretchY(si);
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X, Y + TwoPixels * (dx + 1), aniOrangeDisk, dx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1;
  if ((bl & 0x7) != 0)
  {
    MovHighByte(&PlayField16[si], bl);

    return;
  }

  PlayField16[si] = 0;
  PlayField16[si + FieldWidth] = fiOrangeDisk;
  si = si + FieldWidth;
  if (PlayField16[si + FieldWidth] == 0)
  {
    MovHighByte(&PlayField16[si], 0x30); // go on falling down
    MovHighByte(&PlayField16[si + FieldWidth], fiOrangeDisk);

    return;
  } // loc_g_2867:

  if (LowByte(PlayField16[si + FieldWidth]) == fiExplosion)
    return;

  ExplodeFieldSP(si);                 // Explode
}
