// ----------------------------------------------------------------------------
// InitGameConditions.c
// ----------------------------------------------------------------------------

#include "InitGameConditions.h"


// ==========================================================================
//                              SUBROUTINE
// Init game conditions (variables)
// ==========================================================================

void subInitGameConditions()
{
  MurphyVarFaceLeft = 0;
  KillMurphyFlag = 0;			// no "kill Murphy"
  ExitToMenuFlag = 0;
  LeadOutCounter = 0;			// quit flag: lead-out counter
  RedDiskCount = 0;			// Red disk counter

  YawnSleepCounter = 0;			// Wake up sleeping Murphy

  ExplosionShake = 0;			// Force explosion flag off
  ExplosionShakeMurphy = 0;		// Force explosion flag off

  TerminalMaxCycles = 0x7F;
  YellowDisksExploded = 0;

  TimerVar = 0;

  SnikSnaksElectronsFrozen = 0;		// Snik-Snaks and Electrons move!

  SplitMoveFlag = 0;			// Reset Split-through-ports
  RedDiskReleasePhase = 0;		// (re-)enable red disk release
  RedDiskReleaseMurphyPos = 0;		// Red disk was released here
}


// ==========================================================================
//                              SUBROUTINE
// Locate Murphy and init location.
// ==========================================================================

void InitMurphyPos()
{
  int si;

  for (si = 0; si < LevelMax; si++)
    if (PlayField16[si] == fiMurphy)
      break;

  InitMurphyPosB(si);

  MurphyPosIndex = si;
}

void InitMurphyPosB(int si)
{
  MurphyScreenXPos = MurphyXPos = GetStretchX(si); // Murphy's screen x-position
  MurphyScreenYPos = MurphyYPos = GetStretchY(si); // Murphy's screen y-position

  // To Do: draw Murphy in location ax
  DDSpriteBuffer_BltImg(MurphyScreenXPos, MurphyScreenYPos, aniMurphy, 0);

  subCalculateScreenScrollPos();           // calculate screen start addrs

  if (AutoScrollFlag)
  {
    if (bPlaying)
      SoftScrollTo(ScreenScrollXPos, ScreenScrollYPos, 1000, 25);
    else
      ScrollTo(ScreenScrollXPos, ScreenScrollYPos);
  }
}


// ==========================================================================
//                              SUBROUTINE
// Convert to easy symbols and reset Infotron count If not ThenVer62
// ==========================================================================

int subConvertToEasySymbols()
{
  int ax, bx, cx, dx, i;
  int al;

  bx = 0;
  dx = 0;
  cx = LevelMax + 1;
  i = 0;

loc_g_26C9:
  ax = PlayField16[i];
  al = LowByte(ax);
  if (al == 0xF1) // converted explosion?
  {
    MovLowByte(&PlayField16[i], 0x1F);      // restore explosions
    goto loc_g_2778;
  }

  if (LowByte(GameBusyFlag) != 1) // free screen write?
  {
    if (ax == fiInfotron) // Infotron? -> yes--count!
      goto loc_g_2704;

    if (ax == fiSnikSnak) // Snik Snak? -> yes--rearrange
      goto loc_g_2713;

    if (ax == fiElectron) // Electron? -> yes--rearrange
      goto loc_g_2741;
  }

  // test for fancy RAM Chips:
  if (ax == fiRAMLeft || ax == fiRAMRight)
    goto loc_g_2707;

  if (ax == fiRAMTop || ax == fiRAMBottom)
    goto loc_g_2707;

  if (ax < fiHWFirst) // All but deco hardware?
    goto loc_g_26F8;

  if (ax < fiRAMTop) // Decorative hardware?
    goto loc_g_270D;

loc_g_26F8:
  if (ax < fiSpPortRight) // Gravity change ports only?
    goto loc_g_2778;

  if (ax < fiSnikSnak) // Gravity change port! 'loc_g_2702:
    goto loc_g_276F;

  goto loc_g_2778;

loc_g_2704:                                     // INFOTRON
  dx = dx + 1;                      // Count Infotrons
  goto loc_g_2778;

loc_g_2707:                                     // DECO RAM CHIPS
  PlayField16[i] = fiRAM; // Convert to standard RAM chip
  goto loc_g_2778;

loc_g_270D:                                     // DECO HARDWARE
  PlayField16[i] = fiHardWare; // Convert to standard hardware
  goto loc_g_2778;

loc_g_2713:                                     // SNIK-SNAK
  if (PlayField16[i - 1] != 0) // 1 field left empty? -> no--try up
    goto loc_g_271F;

  MovHighByte(&PlayField16[i], 1); // turn left, step = NorthWest
  goto loc_g_2778;

loc_g_271F:
  if (PlayField16[i - FieldWidth] != 0) // 1 field up empty? -> no--try right
    goto loc_g_2730;

  PlayField16[i - FieldWidth] = 0x1011; // SnikSnak accessing from below, step = 0
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_2730:
  if (PlayField16[i + 1] != 0) // 1 field right empty? -> point up
    goto loc_g_2778;

  PlayField16[i + 1] = 0x2811; // SnikSnak accessing from left, step = 0
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_2741:                                     // ELECTRON
  if (PlayField16[i - 1] != 0) // 1 field left empty? -> no--try up
    goto loc_g_274D;

  MovHighByte(&PlayField16[i], 1);
  goto loc_g_2778;

loc_g_274D:
  if (PlayField16[i - FieldWidth] != 0) // 1 field up empty? -> no--try right
    goto loc_g_275E;

  PlayField16[i - FieldWidth] = 0x1018; // 1 field up
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_275E:
  if (PlayField16[i + 1] != 0) // 1 field right empty? -> no--point down
    goto loc_g_2778;

  PlayField16[i + 1] = 0x2818;
  PlayField16[i] = 0xFFFF;
  goto loc_g_2778;

loc_g_276F:                                     // GRAVITY CHANGING PORTS
  PlayField16[i] = (ax - 4) | 0x100;    // Convert to standard ports
  goto loc_g_2778;

loc_g_2778:
  i = i + 1;                   // Next field
  bx = bx + 1;
  cx = cx - 1;
  if (0 < cx) // Until all done 'loc_g_2782:
    goto loc_g_26C9;

  return dx;	// return InfotronCount
}


// ==========================================================================
//                              SUBROUTINE
// Reset Infotron count.  Call immediately after subConvertToEasySymbols
// ==========================================================================

void ResetInfotronsNeeded(int dx)
{
  if (LInfo.InfotronsNeeded != 0)		// Jump If equal (autodetect)
    dx = LInfo.InfotronsNeeded;

  InfotronsNeeded = LowByte(dx);		// Remaining Infotrons needed
  TotalInfotronsNeeded = InfotronsNeeded;	// Number of Infotrons needed
}


// ==========================================================================
//                              SUBROUTINE
// Fetch and initialize a level
// ==========================================================================

void subFetchAndInitLevelB()
{
  subFetchAndInitLevelA();
}

void subFetchAndInitLevelA()
{
  GameBusyFlag = 0;				// restore scissors too
  subFetchAndInitLevel();			// fetch and initialize a level
  GameBusyFlag = 1;				// no free screen write

  DemoKeyCode = 0;				// delete last demo key!
}

void subFetchAndInitLevel()
{
  int InfoCountInLevel;

  PrepareLevel();				// initialize level data

  GameBusyFlag = -GameBusyFlag;			// make != 1
  InfoCountInLevel = subConvertToEasySymbols();	// convert to easy symbols
  GameBusyFlag = -GameBusyFlag;			// restore

  subDisplayLevel();				// paint (init) game field

  ResetInfotronsNeeded(InfoCountInLevel);	// and reset Infotron count

  subInitGameConditions();			// init game conditions (vars)

  InitMurphyPos();				// locate Murphy + screen pos
}
