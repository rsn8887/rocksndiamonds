// ----------------------------------------------------------------------------
// DoGameStuff.c
// ----------------------------------------------------------------------------

#include "DoGameStuff.h"


static void CallAnimation(int si, byte bl);
static boolean IsToBeAnimated(int bl);

int AnimationPosTable[SP_MAX_PLAYFIELD_SIZE];
byte AnimationSubTable[SP_MAX_PLAYFIELD_SIZE];


// ==========================================================================
//                              SUBROUTINE
// Do game stuff
// ==========================================================================

void subDoGameStuff()
{
  int si, cx, dx, bl;
  int InfotronsNeeded_last = InfotronsNeeded;

  subAnimateMurphy(&MurphyPosIndex);       // move Murphy in any direction

  if (InfotronsNeeded != InfotronsNeeded_last)
    game.snapshot.collected_item = TRUE;

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Build a database of locations and subs-to-call of animatable fields only:
  // Make a snapshot from the field before the animation cycle starts.
  // first and last line are not animated.
  si = FieldWidth + 1;
  cx = LevelMax - 2 * FieldWidth - 1;
  dx = 0;
  do // locloop_g_2282:
  {
    bl = LowByte(PlayField16[si]);
    if (((bl & 0xD) != 0) && (bl < 0x20)) // all animatables have 1's in &H0D' above &H1F? (&H1F=explosion!)
    {
      if (IsToBeAnimated(bl))
      {
        AnimationPosTable[dx] = si;
        AnimationSubTable[dx] = bl;
        dx = dx + 1; // count database entries
      }
    }

    si = si + 1; // next field
    cx = cx - 1;
  }
  while (0 < cx); // locloop_g_2282' until all lines scanned(not top- and bottom edge)

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Now use the database to animate all animatables the fastest way.
  // All the other fields are not checked anymore: those have no database entry.
  // The field from before animation is frozen in the database in order not to
  // do follow-up animations in the same loop.
  if (dx != 0) // any database entries?
  {
    dx = dx - 1;
    for (cx = 0; cx <= dx; cx++)
    {
      CallAnimation(AnimationPosTable[cx], AnimationSubTable[cx]);
    } // loop    locloop_g_22B8          ' until all animatables done
  }

  // All animations are done now
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  if (KillMurphyFlag == 1 || MurphyMoveCounter == 0)
  {
    if (LeadOutCounter == 0 &&
	!game_sp.LevelSolved &&
	!game_sp.GameOver)
    {
      KillMurphyFlag = 0;			// no more "kill Murphy"
      ExplodeFieldSP(MurphyExplodePos);		// Explode
      LeadOutCounter = 0x40;			// quit: start lead-out

      /* give Murphy some more time (LeadOutCounter) to reach the exit */
    }
  } //  loc_g_22FB:
}

static boolean IsToBeAnimated(int bl)
{
  static boolean IsToBeAnimated;

  switch (bl)
  {
    case fiZonk:
    case fiInfotron:
    case fiOrangeDisk:
    case fiSnikSnak:
    case fiTerminal:
    case fiElectron:
    case fiBug:
    case fiExplosion:
      IsToBeAnimated = True;
      break;

    default:
      IsToBeAnimated = False;
      break;
  }

  return IsToBeAnimated;
}

static void CallAnimation(int si, byte bl)
{
  switch (bl)
  {
    case fiZonk:
      subAnimateZonks(si);
      break;

    case fiInfotron:
      subAnimateInfotrons(si);
      break;

    case fiOrangeDisk:
      subAnimateOrangeDisks(si);
      break;

    case fiSnikSnak:
      subAnimateSnikSnaks(si);
      break;

    case fiTerminal:
      subAnimateTerminals(si);
      break;

    case fiElectron:
      subAnimateElectrons(si);
      break;

    case fiBug:
      subAnimateBugs(si);
      break;

    case fiExplosion:
      subAnimateExplosion(si);
      break;

    default:
      // Debug.Assert(False);
      break;
  }
}

