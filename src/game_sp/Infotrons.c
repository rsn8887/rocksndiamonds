// ----------------------------------------------------------------------------
// Infotrons.c
// ----------------------------------------------------------------------------

#include "Infotrons.h"


// ==========================================================================
//                              SUBROUTINE
// Animate Infotrons (falling)
// ==========================================================================

void subAnimateInfotrons(int si)
{
  int tFld;

  int ax, bx, dx, X, Y;
  int al, bl;

  tFld = PlayField16[si];
  if ((tFld & 0xFF) != fiInfotron)
    return;

  if (tFld == fiInfotron)
  {
    ax = PlayField16[si + FieldWidth]; // select case playfield16(si+60)
    if (ax == 0)
      goto loc_g_11D5;

    if (ax == fiZonk)
      goto loc_g_11A6;

    if (ax == fiInfotron)
      goto loc_g_11A6;

    if (ax == fiRAM)
      goto loc_g_11A6;

    return;

loc_g_11A6: //        Case fiZonk, fiInfotron, fiRAM
    ax = PlayField16[si + FieldWidth - 1];
    if (ax == 0 || ax == 0x8888 || ax == 0xAAAA)
      goto loc_g_11DC;

loc_g_11BD:
    ax = PlayField16[si + FieldWidth + 1];
    if (ax == 0 || ax == 0x8888 || ax == 0xAAAA)
      goto loc_g_11F2;

    return;

loc_g_11D5: //       Case fiSpace
    MovHighByte(&PlayField16[si], 0x40);
    goto loc_g_1207;

loc_g_11DC: // roll left?
    if (PlayField16[si - 1] == 0)
      goto loc_g_11E5;

    goto loc_g_11BD;

loc_g_11E5:
    MovHighByte(&PlayField16[si], 0x50);
    PlayField16[si - 1] = 0x8888;
    goto loc_g_1207;

loc_g_11F2: // roll right?
    if (PlayField16[si + 1] == 0)
      goto loc_g_11FA;

    return;

loc_g_11FA:
    MovHighByte(&PlayField16[si], 0x60);
    PlayField16[si + 1] = 0x8888;
  } // tFld = fiInfotron

loc_g_1207:
  // from now on the infotron is definitely moving,
  // maybe the sequence is in an advanced frame
  // or just beeing initialized due to the code above
  bl = HighByte(PlayField16[si]);
  bx = 0;
  MovLowByte(&bx, bl);

  al = bl & 0xF0;
  if (al == 0x10) // infotron comes falling from above
    goto loc_g_1242;

  if (al == 0x20) // infotron comes rolling from right to left
    goto loc_g_138D;

  if (al == 0x30) // infotron comes rolling from left to right
    goto loc_g_13E9;

  if (al == 0x40) // infotron falls straight down
    goto loc_g_1444;

  if (al == 0x50) // infotron rolls left
    goto loc_g_1472;

  if (al == 0x60) // infotron rolls right
    goto loc_g_14E0;

  if (al == 0x70) // intermediate state
    goto loc_g_154E;

  return;

loc_g_1242: // infotron comes falling from above
  //      To Do: draw infotron falling from above
  //      according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si - FieldWidth);
  dx = bl & 0x7;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X, Y + TwoPixels * (dx + 1), aniInfotron, dx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1;
  if (bl == 0x16)
  {
    MovHighByte(&PlayField16[si], bl);
    subCleanUpForInfotronsAbove(si - FieldWidth);

    return;
  } // loc_g_1285:

  if (bl < 0x18)
  {
    MovHighByte(&PlayField16[si], bl);

    return;
  } // loc_g_128F:

  MovHighByte(&PlayField16[si], 0); // infotron arrived at the field

  // now check if the zonk may go on falling somehow
  ax = PlayField16[si + FieldWidth];
  if (ax == 0) // below is empty!-> go on falling
    goto loc_g_132D;

  if (ax == 0x9999) // below is only temporarily used
    goto loc_g_132D;

  if ((ax & 0xFF) == fiMurphy) // Murphy dies
    goto loc_g_1364;

  if (ax == fiRedDisk) // red disk hit
    goto loc_g_1386;

  if ((ax & 0xFF) == fiSnikSnak) // SnikSnak dies
    goto loc_g_1386;


  if ((ax & 0xFF) == fiElectron) // Electron cracked!
    goto loc_g_1386;

  if (ax == fiYellowDisk) // yellow disk hit
    goto loc_g_1386;

  if (ax == fiOrangeDisk) // orange disk hit
    goto loc_g_1386;

  // play the infotron sound, 'cause infotron hits something "hard"
  subSoundFX(si, fiInfotron, actImpact);

  if (! (ax == fiZonk || ax == fiInfotron || ax == fiRAM))
    return;

  // infotron rolls somewhere
  ax = PlayField16[si + FieldWidth - 1];
  if (ax == 0 || ax == 0x8888 || ax == 0xAAAA) // may roll left
    goto loc_g_133A;

  ax = PlayField16[si + FieldWidth + 1];
  if (ax == 0 || ax == 0x8888 || ax == 0xAAAA) // may roll right
    goto loc_g_1350;

  return;

loc_g_132D:     // go on falling down?
  PlayField16[si] = 0x7004; // go into intermediate waitstate
  PlayField16[si + FieldWidth] = 0x9999; // mark as "zonk waiting to access"

  return;

loc_g_133A:     // test if infotron may roll left
  // This if(if true) jumps up far above
  // to the according rountine for fixed infotrons!
  if (PlayField16[si - 1] != 0) // Remarkable!!! ' loc_g_0EF4:
    goto loc_g_11BD;

  MovHighByte(&PlayField16[si], 0x50); // infotron rolls left
  PlayField16[si - 1] = 0x8888;

  return;

loc_g_1350:     // test if infotron may roll right
  if (PlayField16[si + 1] != 0)
    return;

  MovHighByte(&PlayField16[si], 0x60); // infotron rolls right
  PlayField16[si + 1] = 0x8888;

  return;

loc_g_1364:     // Murphy dies, but not in any case
  bl = HighByte(PlayField16[si + FieldWidth]);
  if (bl == 0xE || bl == 0xF || bl == 0x28)
    return;

  if (bl == 0x29 || bl == 0x25 || bl == 0x26)
    return;

loc_g_1386:     // someone dies/explodes immediately
  si = si + FieldWidth;                 // 1 field down
  ExplodeFieldSP(si);               // Explode

  return;

loc_g_138D: // infotron comes rolling from right to left
  //  To Do: draw infotron rolling from right
  //  according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si + 1);
  Y = GetStretchY(si);
  dx = (bl & 0x7) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X - (TwoPixels * dx), Y, aniInfotronRollLeft, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1; // get and increment sequence#
  if (bl == 0x24)
    PlayField16[si + 1] = 0xAAAA;

  if (bl == 0x26)
  {
    MovHighByte(&PlayField16[si], bl);
    subCleanUpForInfotronsAbove(si + 1);
  }
  else if (bl < 0x28)
  {
    MovHighByte(&PlayField16[si], bl);
  }
  else
  {
    PlayField16[si] = 0x7004; // go into intermediate state
  }

  return;

loc_g_13E9: // infotron comes rolling from left to right
  //  To Do: draw infotron rolling from left
  //  according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si - 1);
  Y = GetStretchY(si);
  dx = (bl & 0x7) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X + (TwoPixels * dx), Y, aniInfotronRollRight, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1;
  if (bl == 0x34)
    PlayField16[si - 1] = 0xAAAA;

  if (bl == 0x36)
  {
    MovHighByte(&PlayField16[si], bl);
    subCleanUpForInfotronsAbove(si - 1);
  }
  else if (bl < 0x38)
  {
    MovHighByte(&PlayField16[si], bl);
  }
  else
  {
    PlayField16[si] = 0x7004; // go into intermediate state
  }

  return;

loc_g_1444: // infotron falls straight down
  bl = bl + 1;
  if (bl < 0x42)
  {
    MovHighByte(&PlayField16[si], bl);
  }
  else if (PlayField16[si + FieldWidth] != 0)
  {
    bl = bl - 1; // stay waiting
    MovHighByte(&PlayField16[si], bl);
  }
  else
  {
    PlayField16[si] = 0xFFFF;
    si = si + FieldWidth;                 // 1 field down
    PlayField16[si] = 0x1004; // go falling
  }

  return;

loc_g_1472: // infotron rolls left
  //  To Do: draw infotron rolling to left
  //  according to position in (bl And &H0F)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  dx = (bl & 0xF) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X - (TwoPixels * dx), Y, aniInfotronRollLeft, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1; // retrieve and increment sequence#
  if (bl < 0x52)
  {
    MovHighByte(&PlayField16[si], bl);

    return;
  }

  if (PlayField16[si + FieldWidth - 1] != 0)
    goto loc_g_14D9;

  if (PlayField16[si - 1] != 0)
  {
    if (PlayField16[si - 1] != 0x8888)
      goto loc_g_14D9;
  }

  PlayField16[si] = 0xFFFF;
  si = si - 1;                   // 1 field left
  PlayField16[si] = 0x2204;
  PlayField16[si + FieldWidth] = 0x9999;

  return;

loc_g_14D9: // stay waiting
  bl = bl - 1;
  MovHighByte(&PlayField16[si], bl);

  return;

loc_g_14E0: // infotron rolls right
  //  To Do: draw infotron rolling to right
  //  according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  dx = (bl & 0x7) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X + (TwoPixels * dx), Y, aniInfotronRollRight, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1;
  if (bl < 0x62)
  {
    MovHighByte(&PlayField16[si], bl);

    return;
  }

  if (PlayField16[si + FieldWidth + 1] != 0)
    goto loc_g_1547;

  if (PlayField16[si + 1] != 0)
  {
    if (PlayField16[si + 1] != 0x8888)
      goto loc_g_1547;
  }

  PlayField16[si] = 0xFFFF;
  si = si + 1;
  PlayField16[si] = 0x3204;
  PlayField16[si + FieldWidth] = 0x9999;

  return;

loc_g_1547: // stay waiting
  bl = bl - 1;
  MovHighByte(&PlayField16[si], bl);

  return;

loc_g_154E: // intermediate state
  ax = PlayField16[si + FieldWidth];
  if (ax == 0 || ax == 0x9999)
  {
    PlayField16[si] = 0xFFFF;
    si = si + FieldWidth;                 // 1 field down
    PlayField16[si] = 0x1004; // start falling down
    goto loc_g_1242;
  }
}

void subCleanUpForInfotronsAbove(int si)
{
  int ax;

  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  if (PlayField16[si - FieldWidth] != 0)
  {
    if (PlayField16[si - FieldWidth] != 0x9999)
      return;

    if (LowByte(PlayField16[si - 2 * FieldWidth]) != fiZonk)
      return;
  }

  if (PlayField16[si - FieldWidth - 1] == fiInfotron)
    goto loc_g_16FE;

loc_g_16F6:
  if (PlayField16[si - FieldWidth + 1] == fiInfotron)
    goto loc_g_1722;

  return;

loc_g_16FE:
  ax = PlayField16[si - 1];
  if (ax == fiZonk || ax == fiInfotron || ax == fiRAM)
  {
    PlayField16[si - FieldWidth - 1] = 0x6004;
    PlayField16[si - FieldWidth] = 0x8888;

    return;
  }

  goto loc_g_16F6;

loc_g_1722:
  ax = PlayField16[si + 1];
  if (ax == fiZonk || ax == fiInfotron || ax == fiRAM)
  {
    PlayField16[si - FieldWidth + 1] = 0x5004;
    PlayField16[si - FieldWidth] = 0x8888;
  }
}
