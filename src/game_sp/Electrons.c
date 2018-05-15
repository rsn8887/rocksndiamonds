// ----------------------------------------------------------------------------
// Electrons.c
// ----------------------------------------------------------------------------

#include "Electrons.h"


// ==========================================================================
//                              SUBROUTINE
// Animate/move Electrons
// ==========================================================================

void subAnimateElectrons(int si)
{
  int bx, Tmp;

  if (SnikSnaksElectronsFrozen == 1)
    return;

  if (LowByte(PlayField16[si]) != fiElectron)
    return;

  bx = HighByte(PlayField16[si]);

  Tmp = bx / 8;
  switch (Tmp)
  {
    case 0:
      subElectronTurnLeft(si, bx); // turning, bx=0 -> point N, bx = 1 -> point NW etc.
      break;

    case 1:
      subElectronTurnRight(si, bx); // turn right
      break;

    case 2:
      subElectronFromBelow(si, bx); // access si from below
      break;

    case 3:
      subElectronFromRight(si, bx); // access si from right
      break;

    case 4:
      subElectronFromAbove(si, bx); // access si from above
      break;

    case 5:
      subElectronFromLeft(si, bx); // access si from left
      break;
  }
}

void subDrawAnimatedElectrons(int si)
{
  int bx, Tmp;

  // If SnikSnaksElectronsFrozen = 1 Then Exit Function
  if (LowByte(PlayField16[si]) != fiElectron)
    return;

  bx = HighByte(PlayField16[si]);

  Tmp = bx / 8;
  switch (Tmp)
  {
    case 0:
      subDrawElectronTurnLeft(si, bx); // turning, bx=0 -> point N, bx = 1 -> point NW etc.
      break;

    case 1:
      subDrawElectronTurnRight(si, bx); // turn right
      break;

    case 2:
      subDrawElectronFromBelow(si, bx); // access si from below
      break;

    case 3:
      subDrawElectronFromRight(si, bx); // access si from right
      break;

    case 4:
      subDrawElectronFromAbove(si, bx); // access si from above
      break;

    case 5:
      subDrawElectronFromLeft(si, bx); // access si from left
      break;
  }
}

void subElectronTurnLeft(int si, int bx)
{
  int ax, bl;

  ax = (TimerVar & 3);
  if (ax != 0)
  {
    if (ax == 3)
      goto loc_g_7ACD;

    return;
  } // loc_g_7A9F:

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  subDrawElectronTurnLeft(si, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bx = (bx + 1) & 0x7;
  MovHighByte(&PlayField16[si], bx);

  return;

loc_g_7ACD:
  bl = HighByte(PlayField16[si]);
  if (bl == 0)
    goto loc_g_7AE6;

  if (bl == 2)
    goto loc_g_7B05;

  if (bl == 4)
    goto loc_g_7B24;

  if (bl == 6)
    goto loc_g_7B43;

  return;

loc_g_7AE6: // pointing up
  ax = PlayField16[si - FieldWidth];
  if (ax == 0) // above is empty -> go up
    goto loc_g_7AF5;

  if (LowByte(ax) == fiMurphy) // above is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7AF5: // above is empty -> go up
  PlayField16[si] = 0x1BB;
  si = si - FieldWidth; // 1 field up
  PlayField16[si] = 0x1018;

  return;

loc_g_7B05: // pointing left
  ax = PlayField16[si - 1];
  if (ax == 0) // left is empty -> go there
    goto loc_g_7B14;

  if (LowByte(ax) == fiMurphy) // left is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7B14: // left is empty -> go there
  PlayField16[si] = 0x2BB;
  si = si - 1; // 1 field left
  PlayField16[si] = 0x1818;

  return;

loc_g_7B24: // pointing down
  ax = PlayField16[si + FieldWidth];
  if (ax == 0) // below is empty -> go down
    goto loc_g_7B33;

  if (LowByte(ax) == fiMurphy) // below is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7B33: // below is empty -> go down
  PlayField16[si] = 0x3BB;
  si = si + FieldWidth; // 1 field down
  PlayField16[si] = 0x2018;

  return;

loc_g_7B43: // pointing Right
  ax = PlayField16[si + 1];
  if (ax == 0) // right is empty -> go there
    goto loc_g_7B55;

  if (LowByte(ax) == fiMurphy) // right is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7B55: // right is empty -> go there
  PlayField16[si] = 0x4BB;
  si = si + 1; // 1 field right
  PlayField16[si] = 0x2818;
}

void subElectronTurnRight(int si, int bx)
{
  int ax, bl;

  ax = (TimerVar & 3);
  if (ax != 0)
  {
    if (ax == 3)
      goto loc_g_7BA3;

    return;
  } // loc_g_7B73:

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  subDrawElectronTurnRight(si, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++

  bx = ((bx + 1) & 0x7) | 8;
  MovHighByte(&PlayField16[si], bx);

  return;

loc_g_7BA3:
  bl = HighByte(PlayField16[si]);
  if (bl == 0x8)
    goto loc_g_7BBC;

  if (bl == 0xA)
    goto loc_g_7C19;

  if (bl == 0xC)
    goto loc_g_7BFA;

  if (bl == 0xE)
    goto loc_g_7BDB;

  return;

loc_g_7BBC: // pointing up
  ax = PlayField16[si - FieldWidth];
  if (ax == 0) // above is empty -> go up
    goto loc_g_7BCB;

  if (LowByte(ax) == fiMurphy) // above is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7BCB: // above is empty -> go up
  PlayField16[si] = 0x1BB;
  si = si - FieldWidth; // 1 field up
  PlayField16[si] = 0x1018;

  return;

loc_g_7BDB: // pointing left
  ax = PlayField16[si - 1];
  if (ax == 0) // left is empty -> go there
    goto loc_g_7BEA;

  if (LowByte(ax) == fiMurphy) // left is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7BEA: // left is empty -> go there
  PlayField16[si] = 0x2BB;
  si = si - 1; // 1 field left
  PlayField16[si] = 0x1818;

  return;

loc_g_7BFA: // pointing down
  ax = PlayField16[si + FieldWidth];
  if (ax == 0) // below is empty -> go down
    goto loc_g_7C09;

  if (LowByte(ax) == fiMurphy) // below is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7C09: // below is empty -> go down
  PlayField16[si] = 0x3BB;
  si = si + FieldWidth; // 1 field down
  PlayField16[si] = 0x2018;

  return;

loc_g_7C19: // pointing Right
  ax = PlayField16[si + 1];
  if (ax == 0) // right is empty -> go there
    goto loc_g_7C2B;

  if (LowByte(ax) == fiMurphy) // right is murphy -> explode
    ExplodeFieldSP(si);

  return;

loc_g_7C2B: // right is empty -> go there
  PlayField16[si] = 0x4BB;
  si = si + 1; // 1 field right
  PlayField16[si] = 0x2818;
}

void subElectronFromBelow(int si, int bx)
{
  int ax, bl;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  subDrawElectronFromBelow(si, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bx = bx - 0xF;  // get and increment sequence#

  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si + FieldWidth]) != fiExplosion)
  {
    PlayField16[si + FieldWidth] = 0; // electron left that field
  }

  if (bl < 8) // electron still goes up
  {
    bl = bl + 0x10;
    MovHighByte(&PlayField16[si], bl);

    return;
  } // loc_g_7C84

  PlayField16[si] = 0x18; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si - 1]; // check left field
  if (ax == 0 || LowByte(ax) == fiMurphy) // check for empty or murphy
  {
    MovHighByte(&PlayField16[si], 1); // start to turn left

    return;
  } // loc_g_7CA4:

  ax = PlayField16[si - FieldWidth]; // cannot turn left -> check above
  if (ax == 0) // check if empty
  {
    PlayField16[si] = 0x1BB; // mark as "electron leaving"
    si = si - FieldWidth; // go up!
    PlayField16[si] = 0x1018;

    return;
  }

  if (LowByte(ax) == fiMurphy) // check for murphy above
  {
    ExplodeFieldSP(si); // Explode

    return;
  } // loc_g_7CC6:

  ax = PlayField16[si + 1]; // check right field
  if (ax == 0 || LowByte(ax) == fiMurphy) // check for empty or murphy
  {
    MovHighByte(&PlayField16[si], 9); // start to turn right

    return;
  } // loc_g_7CE0:

  // else: no way to go, start turning around
  MovHighByte(&PlayField16[si], 1);
}

void subElectronFromRight(int si, int bx)
{
  int ax, bl;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  subDrawElectronFromRight(si, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bx = bx - 0x17;  // get and increment sequence#

  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si + 1]) != fiExplosion)
  {
    PlayField16[si + 1] = 0; // electron left that field
  } // loc_g_7D1D:

  if (bl < 8) // sniksnak still goes left
  {
    bl = bl + 0x18;
    MovHighByte(&PlayField16[si], bl);

    return;
  } // loc_g_7D2A:

  PlayField16[si] = 0x18; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si + FieldWidth]; // check below
  if (ax == 0 || LowByte(ax) == fiMurphy) // empty or murphy?
  {
    MovHighByte(&PlayField16[si], 3); // yes -> turn left down

    return;
  } // loc_g_7D4A:

  ax = PlayField16[si - 1]; // check left, etc ... see the comments on subElectronFromBelow()
  if (ax == 0)
  {
    PlayField16[si] = 0x2BB;
    si = si - 1;                // 1 field left
    PlayField16[si] = 0x1818;

    return;
  } // loc_g_7D61:

  if (LowByte(ax) == fiMurphy)
  {
    ExplodeFieldSP(si);      // Explode

    return;
  } // loc_g_7D6C:

  ax = PlayField16[si - FieldWidth]; // check above
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 0xF);

    return;
  } // loc_g_7D86:

  MovHighByte(&PlayField16[si], 3);
}

void subElectronFromAbove(int si, int bx)
{
  int ax, bl;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  subDrawElectronFromAbove(si, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bx = bx - 0x1F;  // get and increment sequence#

  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si - FieldWidth]) != fiExplosion)
  {
    PlayField16[si - FieldWidth] = 0; // electron left that field
  }

  if (bl < 8) // electron still goes down
  {
    bl = bl + 0x20;
    MovHighByte(&PlayField16[si], bl);

    return;
  } // loc_g_7DD7

  PlayField16[si] = 0x18; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si + 1]; // check right
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 5);

    return;
  } // loc_g_7DF7:

  ax = PlayField16[si + FieldWidth]; // check below
  if (ax == 0)
  {
    PlayField16[si] = 0x3BB;
    si = si + FieldWidth;                 // 1 field down
    PlayField16[si] = 0x2018;

    return;
  } // loc_g_7E0E:

  if (LowByte(ax) == fiMurphy)
  {
    ExplodeFieldSP(si);        // Explode

    return;
  } // loc_g_7E19:

  ax = PlayField16[si - 1]; // check left
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 0xD);

    return;
  } // loc_g_7E33:

  MovHighByte(&PlayField16[si], 5);
}

void subElectronFromLeft(int si, int bx)
{
  int ax, bl;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  subDrawElectronFromLeft(si, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  bx = bx - 0x27;  // get and increment sequence#

  bl = LowByte(bx);
  if (bl == 7 && LowByte(PlayField16[si - 1]) != fiExplosion)
  {
    PlayField16[si - 1] = 0; // electron left that field
  }

  if (bl < 8) // electron still goes right
  {
    bl = bl + 0x28;
    MovHighByte(&PlayField16[si], bl);

    return;
  } // loc_g_7E7E:

  PlayField16[si] = 0x18; // sequence#=8 -> arrived at the new field
  ax = PlayField16[si - FieldWidth]; // check above
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 7);

    return;
  } // loc_g_7E9E:

  ax = PlayField16[si + 1]; // check right(straight on)
  if (ax == 0)
  {
    PlayField16[si] = 0x4BB;
    si = si + 1;                   // 1 field right
    PlayField16[si] = 0x2818;

    return;
  } // loc_g_7EB5:

  if (LowByte(ax) == fiMurphy)
  {
    ExplodeFieldSP(si);    // Explode

    return;
  } // loc_g_7EC0:

  ax = PlayField16[si + FieldWidth]; // check below
  if (ax == 0 || LowByte(ax) == fiMurphy)
  {
    MovHighByte(&PlayField16[si], 0xB);

    return;
  } // loc_g_7A69:

  MovHighByte(&PlayField16[si], 7);
}

void subDrawElectronTurnLeft(int si, int bx)
{
  int X, Y;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  DDSpriteBuffer_BltImg(X, Y, aniElectron, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void subDrawElectronTurnRight(int si, int bx)
{
  int X, Y;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  DDSpriteBuffer_BltImg(X, Y, aniElectron, 0x10 - bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void subDrawElectronFromBelow(int si, int bx)
{
  int X, Y;

  bx = bx - 0xF;  // get and increment sequence#

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si + FieldWidth);
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X, Y - bx * TwoPixels, aniElectron, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void subDrawElectronFromRight(int si, int bx)
{
  int X, Y;

  bx = bx - 0x17;  // get and increment sequence#

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si + 1);
  Y = GetStretchY(si);
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X - bx * TwoPixels, Y, aniElectron, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void subDrawElectronFromAbove(int si, int bx)
{
  int X, Y;

  bx = bx - 0x1F;  // get and increment sequence#

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si - FieldWidth);
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X, Y + bx * TwoPixels, aniElectron, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void subDrawElectronFromLeft(int si, int bx)
{
  int X, Y;

  bx = bx - 0x27;  // get and increment sequence#

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si - 1);
  Y = GetStretchY(si);
  DDSpriteBuffer_BltImg(X, Y, aniSpace, 0);
  DDSpriteBuffer_BltImg(X + bx * TwoPixels, Y, aniElectron, bx);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
}
