// ----------------------------------------------------------------------------
// BugsTerminals.c
// ----------------------------------------------------------------------------

#include "BugsTerminals.h"


byte TerminalState[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];

int TerminalMaxCycles;


// ==========================================================================
//                              SUBROUTINE
// Animate bugs
// ==========================================================================

void subAnimateBugs(int si)
{
  int bl;

  if (fiBug != LowByte(PlayField16[si]))
    return;

  bl = SgnHighByte(PlayField16[si]);	// get and increment sequence number

  if ((TimerVar & 3) == 0)
  {
    bl = bl + 1;
    if (bl >= 14)			// bugs animation has 14 frames
    {
      bl = subGetRandomNumber();	// generate new random number
      bl = -((bl & 0x3F) + 0x20);
    }

    MovHighByte(&PlayField16[si], bl);	// save sequence number
  }

  if (bl < 0)				// bug sleeps / is inactive
    return;

  // now the bug is active! Beware Murphy!
  if ((TimerVar & 3) == 0 &&
      (LowByte(PlayField16[si - FieldWidth - 1]) == fiMurphy ||
       LowByte(PlayField16[si - FieldWidth])     == fiMurphy ||
       LowByte(PlayField16[si - FieldWidth + 1]) == fiMurphy ||
       LowByte(PlayField16[si - 1])              == fiMurphy ||
       LowByte(PlayField16[si + 1])              == fiMurphy ||
       LowByte(PlayField16[si + FieldWidth - 1]) == fiMurphy ||
       LowByte(PlayField16[si + FieldWidth])     == fiMurphy ||
       LowByte(PlayField16[si + FieldWidth + 1]) == fiMurphy))
    subSoundFX(si, fiBug, actActive);		// play dangerous sound

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GfxGraphic[GetX(si)][GetY(si)] = (bl == 0  ? aniBugActivating :
				    bl == 12 ? aniBugDeactivating :
				    bl == 13 ? aniBug : aniBugActive);
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}


// ==========================================================================
//                              SUBROUTINE
// Animate terminals
// ==========================================================================

void subAnimateTerminals(int si)
{
  int bl;

  int lx = GetX(si);
  int ly = GetY(si);
  int graphic;

  if (LowByte(PlayField16[si]) != fiTerminal)
    return;

  /* use native frame handling (undo frame incrementation in main loop) */
  if (game.use_native_sp_graphics_engine)
    GfxFrame[lx][ly]--;

  /* get last random animation delay */
  bl = SgnHighByte(PlayField16[si]);

  bl = bl + 1;
  if (bl <= 0)		/* return if random animation delay not yet reached */
  {
    MovHighByte(&PlayField16[si], bl);

    return;
  }

  /* calculate new random animation delay */
  bl = -(subGetRandomNumber() & TerminalMaxCycles); // generate new random number
  MovHighByte(&PlayField16[si], bl); // save new sequence number

  /* check terminal state (active or inactive) */
  bl = TerminalState[si] + 1;
  if (bl == 8)
    bl = 0;
  else if (15 < bl)
    bl = 8;

  TerminalState[si] = bl;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

  graphic = (bl < 8 ? aniTerminal : aniTerminalActive);

  if (game.use_native_sp_graphics_engine)
    GfxFrame[lx][ly] += getGraphicInfo_Delay(graphic);

  GfxGraphic[lx][ly] = (bl < 8 ? aniTerminal : aniTerminalActive);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}


// ==========================================================================
//                              SUBROUTINE
// Randomize random number generator
// ==========================================================================

void subRandomize()
{
  int Tick = MyGetTickCount();

  RandomSeed = (Tick ^ (Tick >> 16)) & 0xFFFF;
}


// ==========================================================================
//                              SUBROUTINE
// Generate new random number, first method (see also sub_g_8580)
// ==========================================================================

int subGetRandomNumber()
{
  RandomSeed = (RandomSeed * 0x5E5 + 0x31) & 0xFFFF;

  return (RandomSeed >> 1);

  //  Mov ax, randomseed
  //  Mov bx, &H5E5
  //  mul bx                          ' dx:ax = reg * ax
  //  Add ax, &H31
  //  Mov randomseed, ax
  //  shr ax,1
}
