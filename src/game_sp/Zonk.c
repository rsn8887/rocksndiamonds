// ----------------------------------------------------------------------------
// Zonk.c
// ----------------------------------------------------------------------------

#include "Zonk.h"


void subCleanUpForZonksAbove(int si);


// static char *VB_Name = "modZonk";
// --- Option Explicit

// ==========================================================================
//                              SUBROUTINE
// Animate Zonks (falling)
// ==========================================================================

void subAnimateZonks(int si)
{
  int tFld;

  // PseudoRegisters:
  // int ax, bx, cx, dx, di, X, Y;
  int ax, bx, dx, X, Y;
  // int ah, bh, ch, dh, al, bl, cl, dl;
  int al, bl;

  tFld = PlayField16[si];
  if ((tFld & 0xFF) != fiZonk)
    return;

  if (tFld == fiZonk)
  {
    if (FreezeZonks == 2) // Do Zonks fall? (debug)
      return;

    ax = PlayField16[si + FieldWidth]; // select case playfield16(si+60)
    if (ax == 0)
      goto loc_g_0D64;

    if (ax == fiZonk)
      goto loc_g_0D35;

    if (ax == fiInfotron)
      goto loc_g_0D35;

    if (ax == fiRAM)
      goto loc_g_0D35;

    return;

loc_g_0D35: //        Case fiZonk, fiInfotron, fiRAM
    ax = PlayField16[si + FieldWidth - 1];
    if (ax == 0 || ax == 0x8888 || ax == 0xAAAA)
      goto loc_g_0D6B;

loc_g_0D4C:
    ax = PlayField16[si + FieldWidth + 1];
    if (ax == 0 || ax == 0x8888 || ax == 0xAAAA)
      goto loc_g_0D81;

    return;

loc_g_0D64: //       Case fiSpace
    MovHighByte(&PlayField16[si], 0x40);
    goto loc_g_0DA5;

loc_g_0D6B: // roll left?
    if (PlayField16[si - 1] == 0)
      goto loc_g_0D74;

    goto loc_g_0D4C;

loc_g_0D74:
    MovHighByte(&PlayField16[si], 0x50);
    PlayField16[si - 1] = 0x8888;
    goto loc_g_0DA5;

loc_g_0D81: // roll right?
    if (PlayField16[si + 1] == 0)
      goto loc_g_0D98;

    if (PlayField16[si + 1] != 0x9999) // wow right is different from left!
      return;

    if (LowByte(PlayField16[si - FieldWidth + 1]) != 1)
      return;

loc_g_0D98:
    MovHighByte(&PlayField16[si], 0x60);
    PlayField16[si + 1] = 0x8888;
  } // tFld = fiZonk

loc_g_0DA5:
  // from now on the zonk is definitely moving,
  // maybe the sequence is in an advanced frame
  // or just beeing initialized due to the code above
  bl = HighByte(PlayField16[si]);
  bx = 0;
  MovLowByte(&bx, bl);
  al = bl & 0xF0;
  if (al == 0x10) // zonk comes falling from above
    goto loc_g_0DE8;

  if (al == 0x20) // zonk comes rolling from right to left
    goto loc_g_0F83;

  if (al == 0x30) // zonk comes rolling from left to right
    goto loc_g_0FE8;

  if (FreezeZonks == 2)
    return;

  if (al == 0x40) // zonk falls straight down
    goto loc_g_104D;

  if (al == 0x50) // zonk rolls left
    goto loc_g_107B;

  if (al == 0x60) // zonk rolls right
    goto loc_g_10E9;

  if (al == 0x70) // intermediate state
    goto loc_g_1157;

  return;

loc_g_0DE8: // zonk comes falling from above
  //      To Do: draw zonk falling from above
  //      according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si - FieldWidth);
  dx = bl & 0x7;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X, Y + TwoPixels * (dx + 1), aniZonk, dx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1;
  if (bl == 0x16)
  {
    MovHighByte(&PlayField16[si], bl);
    subCleanUpForZonksAbove(si - FieldWidth);
    return;
  } // loc_g_0E2B:

  if (bl < 0x18)
  {
    MovHighByte(&PlayField16[si], bl);
    return;
  } // loc_g_0E35:

  MovHighByte(&PlayField16[si], 0); // zonk arrived at the field
  if ((FreezeZonks & 0xFF) == 2)
    return;

  // loc_g_0E42:     // now check if the zonk may go on falling somehow
  ax = PlayField16[si + FieldWidth];
  if (ax == 0) // below is empty!-> go on falling 'loc_g_0E4C:
    goto loc_g_0EDD;

  if (ax == 0x9999) // below is only temporarily used ' loc_g_0E57:
    goto loc_g_0EDD;

  if ((ax & 0xFF) == fiMurphy) // Murphy dies 'loc_g_0E61:
    goto loc_g_0F14;

  if ((ax & 0xFF) == fiSnikSnak) // SnikSnak dies 'loc_g_0E6B:
    goto loc_g_0F6E;

  if (ax == 0x2BB) // loc_g_0E76:
    goto loc_g_0F36;

  if (ax == 0x4BB) // loc_g_0E81:
    goto loc_g_0F52;

  if ((ax & 0xFF) == fiElectron) // Electron cracked! 'loc_g_0E8B:
    goto loc_g_0F6E;

  if (ax == fiOrangeDisk) // OrangeDisk explodes 'loc_g_0E95:
    goto loc_g_0F75;

  // play the zonk sound, 'cause zonk hits something "hard"
  subSoundFX(si, fiZonk, actImpact);

  if (! (ax == fiZonk || ax == fiInfotron || ax == fiRAM))
    return;

  // loc_g_0EAE: ' Zonk rolls somewhere
  ax = PlayField16[si + FieldWidth - 1];
  if (ax == 0 || ax == 0x8888 || ax == 0xAAAA) // may roll left
    goto loc_g_0EEA;

  ax = PlayField16[si + FieldWidth + 1];
  if (ax == 0 || ax == 0x8888 || ax == 0xAAAA) // may roll right
    goto loc_g_0F00;

  return;

loc_g_0EDD:     // go on falling down?
  PlayField16[si] = 0x7001; // go into intermediate waitstate
  PlayField16[si + FieldWidth] = 0x9999; // mark as "zonk waiting to access"
  return;

loc_g_0EEA:     // test if zonk may roll left
  // This if(if true) jumps up far above
  // to the according rountine for fixed zonks!
  if (PlayField16[si - 1] != 0) // Remarkable!!! ' loc_g_0EF4:
    goto loc_g_0D4C;

  MovHighByte(&PlayField16[si], 0x50); // zonk rolls left
  PlayField16[si - 1] = 0x8888; // mark as zonk accessing?
  return;

loc_g_0F00:     // test if zonk may roll right
  if (PlayField16[si + 1] != 0) // loc_g_0F08:
    return;

  MovHighByte(&PlayField16[si], 0x60); // zonk rolls right
  PlayField16[si + 1] = 0x8888; // mark as zonk accessing?
  return;

loc_g_0F14:     // Murphy dies, but not in any case
  bl = HighByte(PlayField16[si + FieldWidth]);
  if (bl == 0xE || bl == 0xF || bl == 0x28)
    return;

  if (bl == 0x29 || bl == 0x25 || bl == 0x26)
    return;

loc_g_0F36:     // ??
  ax = LowByte(PlayField16[si + FieldWidth - 1]);
  if (ax == fiElectron) // loc_g_0F43:
    PlayField16[si + FieldWidth] = fiElectron;

  if (ax != 0x1F)
    PlayField16[si + FieldWidth - 1] = 0;

  goto loc_g_0F6E;

loc_g_0F52:     // ??
  ax = LowByte(PlayField16[si + FieldWidth + 1]);
  if (ax == fiElectron) // loc_g_0F5F:
    PlayField16[si + FieldWidth] = fiElectron;

  if (ax != 0x1F)
    PlayField16[si + FieldWidth + 1] = 0;

  goto loc_g_0F6E;

loc_g_0F6E:     // someone dies/explodes
  si = si + FieldWidth;                 // 1 field down
  ExplodeFieldSP(si);               // Explode
  return;

loc_g_0F75:     // OrangeDisk explodes next cycle
  si = si + FieldWidth;                 // 1 field down
  PlayField8[si] = fiHardWare;
  return;

loc_g_0F83: // zonk comes rolling from right to left
  //  To Do: draw zonk rolling from right
  //  according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si + 1);
  Y = GetStretchY(si);
  dx = (bl & 0x7) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X - (TwoPixels * dx), Y, aniZonkRollLeft, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1; // get and increment sequence#
  if (bl == 0x24)
    PlayField16[si + 1] = 0xAAAA;

  if (bl == 0x26)
  {
    MovHighByte(&PlayField16[si], bl);
    subCleanUpForZonksAbove(si + 1);
  }
  else if (bl < 0x28)
  {
    MovHighByte(&PlayField16[si], bl);
  }
  else
  {
    PlayField16[si] = 0xFFFF;
    si = si + FieldWidth;                 // 1 field down
    PlayField16[si] = 0x1001; // convert rolling zonk to a falling zonk
  }

  return;

loc_g_0FE8: // zonk comes rolling from left to right
  //  To Do: draw zonk rolling from left
  //  according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si - 1);
  Y = GetStretchY(si);
  dx = (bl & 0x7) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X + (TwoPixels * dx), Y, aniZonkRollRight, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1;
  if (bl == 0x34)
    PlayField16[si - 1] = 0xAAAA;

  if (bl == 0x36)
  {
    MovHighByte(&PlayField16[si], bl);
    subCleanUpForZonksAbove(si - 1);
  }
  else if (bl < 0x38)
  {
    MovHighByte(&PlayField16[si], bl);
  }
  else
  {
    PlayField16[si] = 0xFFFF;
    si = si + FieldWidth;                   // 1 field down
    PlayField16[si] = 0x1001; // convert rolling zonk to a falling zonk
  }

  return;

loc_g_104D: // zonk falls straight down
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
    PlayField16[si] = 0xFFFF; // mark as "zonk leaving"
    si = si + FieldWidth;                 // 1 field down
    PlayField16[si] = 0x1001; // go falling
  }

  return;

loc_g_107B: // zonk rolls left
  //  To Do: draw zonk rolling to left
  //  according to position in (bl And &H0F)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  dx = (bl & 0xF) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X - (TwoPixels * dx), Y, aniZonkRollLeft, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1; // retrieve and increment sequence#
  if (bl < 0x52)
  {
    MovHighByte(&PlayField16[si], bl);
    return;
  }

  if (PlayField16[si + FieldWidth - 1] != 0)
    goto loc_g_10E2;

  if (PlayField16[si - 1] != 0)
  {
    if (PlayField16[si - 1] != 0x8888)
      goto loc_g_10E2;
  } // loc_g_10C8:

  PlayField16[si] = 0xFFFF;
  si = si - 1;                   // 1 field left
  PlayField16[si] = 0x2201;
  PlayField16[si + FieldWidth] = 0xFFFF;
  return;

loc_g_10E2: // stay waiting
  bl = bl - 1;
  MovHighByte(&PlayField16[si], bl);
  return;

loc_g_10E9: // zonk rolls right
  //  To Do: draw zonk rolling to right
  //  according to position in (bl And &H07)
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  dx = (bl & 0x7) + 1;
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X + (TwoPixels * dx), Y, aniZonkRollRight, dx - 1);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bl = HighByte(PlayField16[si]) + 1;
  if (bl < 0x62)
  {
    MovHighByte(&PlayField16[si], bl);
    return;
  }

  if (PlayField16[si + FieldWidth + 1] != 0)
    goto loc_g_1150;

  if (PlayField16[si + 1] != 0)
  {
    if (PlayField16[si + 1] != 0x8888)
      goto loc_g_1150;
  }

  PlayField16[si] = 0xFFFF;
  si = si + 1;
  PlayField16[si] = 0x3201;
  PlayField16[si + FieldWidth] = 0xFFFF;
  return;

loc_g_1150: // stay waiting
  bl = bl - 1;
  MovHighByte(&PlayField16[si], bl);
  return;

loc_g_1157: // intermediate state
  ax = PlayField16[si + FieldWidth];
  if (ax == 0 || ax == 0x9999)
  {
    PlayField16[si] = 0xFFFF;
    si = si + FieldWidth;                 // 1 field down
    PlayField16[si] = 0x1001; // start falling down
    goto loc_g_0DE8;
  }

  return;
} // subAnimateZonks endp

void subCleanUpForZonksAbove(int si)
{
  int ax;

  if (LowByte(PlayField16[si]) != fiExplosion)
    PlayField16[si] = 0;

  if (PlayField16[si - FieldWidth] != 0)
  {
    if (PlayField16[si - FieldWidth] != 0x9999)
      return;

    if (LowByte(PlayField16[si - 2 * FieldWidth]) != fiInfotron)
      return;
  } // loc_g_1674:

  if (PlayField16[si - FieldWidth - 1] != fiZonk)
  {
    if (PlayField16[si - FieldWidth + 1] != fiZonk)
      return;

    goto loc_g_16A7;
  }

  ax = PlayField16[si - 1];
  if (ax == fiZonk || ax == fiInfotron || ax == fiRAM)
  {
    PlayField16[si - FieldWidth - 1] = 0x6001;
    PlayField16[si - FieldWidth] = 0x8888;
    return;
  }

  if (PlayField16[si - FieldWidth + 1] != fiZonk)
    return;

loc_g_16A7:
  ax = PlayField16[si + 1];
  if (ax == fiZonk || ax == fiInfotron || ax == fiRAM)
  {
    PlayField16[si - FieldWidth + 1] = 0x5001;
    PlayField16[si - FieldWidth] = 0x8888;
  }

  return;
} // subCleanUpForZonksAbove
