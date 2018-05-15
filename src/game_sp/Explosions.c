// ----------------------------------------------------------------------------
// Explosions.c
// ----------------------------------------------------------------------------

#include "Explosions.h"


static void LetExplodeFieldSP(int tsi, int cx, int dh);
static void subExplodeInfotron(int tsi, int cx);
static void subExplodeZonk(int tsi, int cx);


// ==========================================================================
//                              SUBROUTINE
// Animate explosion
// ==========================================================================

void subAnimateExplosion(int si)
{
  int ax, bl;

  if (LowByte(PlayField16[si]) != fiExplosion)
    return;

  ax = (TimerVar & 3);
  if (ax != 0)
    return;

  bl = HighByte(PlayField16[si]);

  if ((bl & 0x80) != 0) // infotron explosion!
    goto loc_g_28D0;

  bl = bl + 1;
  MovHighByte(&PlayField16[si], bl);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GfxGraphic[GetX(si)][GetY(si)] = aniDefaultExplosion;
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  if (bl == 8)
  {
    PlayField16[si] = 0;
    ExplosionShake = 0;		// nothing explodes
    // ExplosionShakeMurphy = 0;	// nothing explodes

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    GfxGraphic[GetX(si)][GetY(si)] = aniSpace;
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  } // loc_ret_g_28CF:

  return;

loc_g_28D0: // explosion produces infotron
  bl = bl + 1;
  if (bl == 0x89)
  {
    PlayField16[si] = fiInfotron;
    MovLowByte(&ExplosionShake, 0);		// nothing explodes
    // MovLowByte(&ExplosionShakeMurphy, 0);	// nothing explodes

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    GfxGraphic[GetX(si)][GetY(si)] = aniInfotron;
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    return;
  } // loc_g_28E3:

  MovHighByte(&PlayField16[si], bl);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GfxGraphic[GetX(si)][GetY(si)] = aniElectronExplosion;
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

// ==========================================================================
//                              SUBROUTINE
// Explode
// ==========================================================================

void ExplodeFieldSP(int si)
{
  int ax, cx, dl;

  ax = LowByte(PlayField16[si]);
  if (ax == fiHardWare)
    return;

  ExplosionShake = 1;		// something explodes

  if (ax == fiMurphy)
  {
    KillMurphyFlag = 1;

    ExplosionShakeMurphy = 30;	// Murphy explodes
  }

  if (ax == fiElectron)
  {
    cx = 0x801F; // produce infotrons
    dl = 0xF3;
  }
  else // loc_g_2977:
  {
    cx = 0x1F; // normal explosion
    dl = 0xD;
  } // loc_g_297C:

  LetExplodeFieldSP(si - FieldWidth - 1, cx, dl);
  LetExplodeFieldSP(si - FieldWidth, cx, dl);
  LetExplodeFieldSP(si - FieldWidth + 1, cx, dl);
  LetExplodeFieldSP(si - 1, cx, dl);
  PlayField16[si] = cx;
  LetExplodeFieldSP(si + 1, cx, dl);
  LetExplodeFieldSP(si + FieldWidth - 1, cx, dl);
  LetExplodeFieldSP(si + FieldWidth, cx, dl);
  LetExplodeFieldSP(si + FieldWidth + 1, cx, dl);

  GfxGraphic[GetX(si)][GetY(si)] = -1;		// restart for chain-explosions

  // loc_g_2C3B:
  subSoundFX(si, ax, actExploding);
}

static void LetExplodeFieldSP(int tsi, int cx, int dh)
{
  int al;

  if (tsi < -FieldWidth)
    return;

  al = LowByte(PlayField16[tsi]);
  switch (al)
  {
    case fiHardWare:
      return;

      break;

    case fiOrangeDisk:
    case fiYellowDisk:
    case fiSnikSnak:
      PlayField8[tsi] = dh;
      PlayField16[tsi] = cx;
      break;

    case fiZonk:
      subExplodeZonk(tsi, cx);
      break;

    case fiInfotron:
      subExplodeInfotron(tsi, cx);
      break;

    case fiElectron:
      PlayField8[tsi] = (-dh) & 0xFF;
      PlayField16[tsi] = 0x801F;
      break;

    case fiMurphy:
      KillMurphyFlag = 1;
      PlayField8[tsi] = dh;
      PlayField16[tsi] = cx;
      break;

    default:
      PlayField16[tsi] = cx;
      break;
  }

  GfxGraphic[GetX(tsi)][GetY(tsi)] = -1;	// restart for chain-explosions
}

static void subExplodeZonk(int tsi, int cx)
{
  int ah;

  ah = HighByte(PlayField16[tsi]) & 0xF0;
  PlayField16[tsi] = cx;
  switch (ah)
  {
    case 0x10:
    case 0x70:
      subClearFieldDueToExplosion(tsi - FieldWidth);
      tsi = tsi + FieldWidth;
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x20:
      subClearFieldDueToExplosion(tsi + 1);
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;

    case 0x30:
      subClearFieldDueToExplosion(tsi - 1);
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;

    case 0x50:
      subClearFieldDueToExplosion(tsi - 1);
      break;

    case 0x60:
      subClearFieldDueToExplosion(tsi + 1);
      break;

    case 0xFF000070: // !!! 0x70; this will never be reached! ...??
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;
  }
}

static void subExplodeInfotron(int tsi, int cx)
{
  int ah;

  ah = HighByte(PlayField16[tsi]) & 0xF0;
  PlayField16[tsi] = cx;
  switch (ah)
  {
    case 0x10:
    case 0x70:
      subClearFieldDueToExplosion(tsi - FieldWidth);
      tsi = tsi + FieldWidth;
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x20:
      subClearFieldDueToExplosion(tsi + 1);
      tsi = tsi + FieldWidth; // differnt from zonk version
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x30:
      subClearFieldDueToExplosion(tsi - 1);
      tsi = tsi + FieldWidth; // differnt from zonk version
      if (PlayField16[tsi] == 0x9999)
        subClearFieldDueToExplosion(tsi);

      break;

    case 0x50:
      subClearFieldDueToExplosion(tsi - 1);
      break;

    case 0x60:
      subClearFieldDueToExplosion(tsi + 1);
      break;

    case 0xFF000070: // !!! 0x70; this will never be reached! ...??
      subClearFieldDueToExplosion(tsi + FieldWidth);
      break;
  }
}

void subClearFieldDueToExplosion(int si)
{
  if (LowByte(PlayField16[si]) == fiExplosion)
    return;

  PlayField16[si] = 0;
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GfxGraphic[GetX(si)][GetY(si)] = aniSpace;
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void subRedDiskReleaseExplosion()
{
  int al, X, Y, si;

  al = RedDiskReleasePhase;            // Red disk release phase
  if (al <= 1)
    return;

  si = RedDiskReleaseMurphyPos;
  if (PlayField16[si] == 0) // Release red disk
    PlayField16[si] = fiRedDisk;

  // +++++++++++++++++++++++++++++++++++++++++
  X = GetStretchX(si);
  Y = GetStretchY(si);
  DDSpriteBuffer_BltImg(X, Y, aniRedDisk, 0);
  // +++++++++++++++++++++++++++++++++++++++++

  RedDiskReleasePhase = RedDiskReleasePhase + 1;
  if (RedDiskReleasePhase >= 0x28)
  {
    // si = RedDiskReleaseMurphyPos           ' Red disk was released here
    ExplodeFieldSP(si);                 // Explode
    RedDiskReleasePhase = 0;
  }
}

void subFollowUpExplosions()
{
  int ax, si;

  // locloop_g_2919:
  for (si = 0; si <= LevelMax; si++)
  {
    ax = ByteToInt(PlayField8[si]);
    if (ax != 0)
    {
      if (ax < 0)
      {
        ax = ax + 1;
        PlayField8[si] = ax & 0xFF;
        if (ax == 0)
        {
          PlayField16[si] = 0xFF18;
          ExplodeFieldSP(si);                 // Explode
        }
      }
      else
      {
        ax = ax - 1;
        PlayField8[si] = ax;
        if (ax == 0)
          ExplodeFieldSP(si);
      }
    }
  }
}
