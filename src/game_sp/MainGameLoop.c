// ----------------------------------------------------------------------------
// MainGameLoop.c
// ----------------------------------------------------------------------------

#include "MainGameLoop.h"


boolean bPlaying;
int LeadOutCounter;
int ExitToMenuFlag;
boolean AutoScrollFlag;


// ==========================================================================
//                              SUBROUTINE
// Play a game/demo
// ==========================================================================

void subMainGameLoop_Init()
{
  // This was a bug in the original Supaplex: sometimes red disks could not
  // be released.  This happened if Murphy was killed DURING a red disk release
  // and the next try started.

  RedDiskReleasePhase = 0; // (re-)enable red disk release
}

void subMainGameLoop_Main(byte action, boolean warp_mode)
{
  // ---------------------------------------------------------------------------
  // --------------------- START OF GAME-BUSY LOOP -----------------------------
  // ---------------------------------------------------------------------------

  subProcessKeyboardInput(action);	// check keyboard, act on keys

  // ---------------------------------------------------------------------------
  //

  subDoGameStuff();			// do all game stuff

  //
  // ---------------------------------------------------------------------------

  subRedDiskReleaseExplosion();		// Red Disk release and explode
  subFollowUpExplosions();		// every explosion may cause up to 8 following explosions

  subCalculateScreenScrollPos();	// calculate screen start addrs

  if (AutoScrollFlag)
    ScrollTowards(ScreenScrollXPos, ScreenScrollYPos);

  TimerVar = TimerVar + 1;

  if (ExplosionShakeMurphy > 0)
    ExplosionShakeMurphy--;

  if (ExitToMenuFlag == 1)
  {
    // happens when demo ends or when Murphy enters exit (to be checked)
  }

  if (LeadOutCounter == 0) // no lead-out: game busy
    return;

  // ---------------------------------------------------------------------------
  // ---------------------- END OF GAME-BUSY LOOP ------------------------------
  // ---------------------------------------------------------------------------

  LeadOutCounter = LeadOutCounter - 1;		// do more lead-out after quit

  if (LeadOutCounter != 0)			// lead-out not ready: more
    return;

  // lead-out done: exit now
  // ---------------------- END OF GAME-BUSY LOOP (including lead-out) ---------

  /* if the game is not won when reaching this point, then it is lost */
  if (!game_sp.LevelSolved)
    game_sp.GameOver = TRUE;
}

void subCalculateScreenScrollPos()
{
  int jump_pos = TILEX / 2;

  /* handle wrap-around */
  if (MurphyScreenXPos < -jump_pos)
  {
    MurphyScreenXPos = FieldWidth * TILEX + MurphyScreenXPos;
    MurphyScreenYPos -= TILEY;
  }
  else if (MurphyScreenXPos >= FieldWidth * TILEX - jump_pos)
  {
    MurphyScreenXPos = MurphyScreenXPos - FieldWidth * TILEX;
    MurphyScreenYPos += TILEY;
  }

  if (ExplosionShake != 0)
  {
    subGetRandomNumber();

    // printf("::: ExplosionShake [%d]\n", FrameCounter);
  }

  ScreenScrollXPos = MurphyScreenXPos - (SCR_FIELDX / 2) * TILESIZE;
  ScreenScrollYPos = MurphyScreenYPos - (SCR_FIELDY / 2) * TILESIZE;
}
