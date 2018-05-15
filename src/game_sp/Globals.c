// ----------------------------------------------------------------------------
// Globals.c
// ----------------------------------------------------------------------------

#include "Globals.h"


boolean LevelLoaded;

boolean DemoAvailable;
boolean menBorder;

int FieldWidth;		// standard size = 60
int FieldHeight;	// standard size = 24
int HeaderSize;		// standard size = 96
int FieldMax, LevelMax;

int *PlayField16;
byte *PlayField8;
byte DisPlayField[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];

int TimerVar;
short RandomSeed;

int FreezeZonks;

LevelInfoType LInfo;

int ScrollMinX, ScrollMaxX, ScrollMinY, ScrollMaxY;
int ScrollX, ScrollY;

int MurphyPosIndex, MurphyXPos, MurphyYPos;
int MurphyScreenXPos, MurphyScreenYPos;
int MurphyExplodePos, SplitMoveFlag, RedDiskReleaseMurphyPos;
int KillMurphyFlag, MurphyMoveCounter;
int YawnSleepCounter;
int MurphyVarFaceLeft;
int ScratchGravity, GravityFlag;
int RedDiskReleaseFlag, MovingPictureSequencePhase;

int YellowDisksExploded;
int AllowRedDiskCheat, AllowEatRightRedDiskBug;

int GameBusyFlag;
int InfotronsNeeded, TotalInfotronsNeeded;
int RedDiskCount;
int SnikSnaksElectronsFrozen;

int DemoKeyCode;

int RedDiskReleasePhase;

int fiGraphic[] =
{
  aniSpace,
  aniZonk,
  aniBase,
  aniMurphy,
  aniInfotron,
  aniRAM,
  aniHardWare,
  aniExit,
  aniOrangeDisk,
  aniPortRight,
  aniPortDown,
  aniPortLeft,
  aniPortUp,
  aniSpPortRight,
  aniSpPortDown,
  aniSpPortLeft,
  aniSpPortUp,
  aniSnikSnak,
  aniYellowDisk,
  aniTerminal,
  aniRedDisk,
  aniPortUpAndDown,
  aniPortLeftAndRight,
  aniPortAllDirections,
  aniElectron,
  aniBug,
  aniRAMLeft,
  aniRAMRight,
  aniHW0,
  aniHW1,
  aniHW2,
  aniHW3,
  aniHW4,
  aniHW5,
  aniHW6,
  aniHW7,
  aniHW8,
  aniHW9,
  aniRAMTop,
  aniRAMBottom,
  aniWallSpace
};

int aniSnikSnakTurningLeft[] =
{
  aniSnikSnakTurnUpToLeft,
  aniSnikSnakTurnLeftToDown,
  aniSnikSnakTurnDownToRight,
  aniSnikSnakTurnRightToUp
};

int aniSnikSnakTurningRight[] =
{
  aniSnikSnakTurnUpToRight,
  aniSnikSnakTurnRightToDown,
  aniSnikSnakTurnDownToLeft,
  aniSnikSnakTurnLeftToUp
};


int getSequenceLength(int sequence)
{
  switch (sequence)
  {
    case aniBug:
      return 14;

    case aniElectron:
    case aniExplosion:
      return 9;

    case aniTouchInfotron:
      return 7;

    case aniMurphyExit:
      return 40;

    case aniRedDisk:
      return 1;

    default:
      return 8;
  }
}

boolean isSnappingSequence(int sequence)
{
  switch (sequence)
  {
    case aniTouchBase:
    case aniTouchInfotron:
    case aniTouchRedDisk:
      return TRUE;

    default:
      return FALSE;
  }
}

void InitGlobals()
{
  InitPrecedingPlayfieldMemory();

  AutoScrollFlag = True;
  FreezeZonks = 0;
  LevelLoaded = False;
  FieldWidth = 60;
  FieldHeight = 24;
  HeaderSize = 96;
  FieldMax = (FieldWidth * FieldHeight) + HeaderSize - 1;
  LevelMax = (FieldWidth * FieldHeight) - 1;
  bPlaying = False;
  menBorder = False;

  /* add preceding playfield buffer (as large as preceding memory area) */
  PlayField16 = checked_calloc((game_sp.preceding_buffer_size +
				SP_MAX_PLAYFIELD_SIZE +
				SP_HEADER_SIZE) * sizeof(int));
  PlayField16 = &PlayField16[game_sp.preceding_buffer_size];

  /* add preceding playfield buffer (as large as one playfield row) */
  PlayField8 = checked_calloc((SP_MAX_PLAYFIELD_WIDTH +
			       SP_MAX_PLAYFIELD_SIZE +
			       SP_HEADER_SIZE) * sizeof(byte));
  PlayField8 = &PlayField8[SP_MAX_PLAYFIELD_WIDTH];
}

int GetSI(int X, int Y)
{
  return Y * FieldWidth + X;
}

int GetX(int si)
{
  return si % FieldWidth;
}

int GetY(int si)
{
  return si / FieldWidth;
}

int GetStretchX(int si)
{
  return StretchWidth * (si % FieldWidth);
}

int GetStretchY(int si)
{
  return StretchWidth * (si / FieldWidth);
}

void PrepareLevel()
{
  copyInternalEngineVars_SP();

  SetDisplayRegion();
  SetScrollEdges();

  LevelLoaded = True;
}

int Min(int A, int B)
{
  return (A < B ? A : B);
}

int Max(int A, int B)
{
  return (A < B ? B : A);
}
