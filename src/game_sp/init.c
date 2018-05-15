
#include "main_sp.h"
#include "global.h"


char *preceding_playfield_memory[] =
{
  "95 89 95 89 95 89 3b 8a  3b 8a 3b 8a 3b 8a 3b 8a",	// |......;.;.;.;.;.|
  "3b 8a 3b 8a 3b 8a e8 8a  e8 8a e8 8a e8 8a e8 8a",	// |;.;.;...........|
  "e8 8a e8 8a e8 8a b1 8b  b1 8b b1 8b b1 8b b1 8b",	// |................|
  "b1 8b b1 8b b1 8b 85 8c  85 8c 85 8c 85 8c 85 8c",	// |................|
  "85 8c 85 8c 85 8c 5b 8d  5b 8d 5b 8d 5b 8d 5b 8d",	// |......[.[.[.[.[.|
  "5b 8d 5b 8d 5b 8d 06 8e  06 8e 06 8e 06 8e 06 8e",	// |[.[.[...........|
  "06 8e 06 8e 06 8e ac 8e  ac 8e ac 8e ac 8e ac 8e",	// |................|
  "ac 8e ac 8e ac 8e 59 8f  59 8f 59 8f 59 8f 59 8f",	// |......Y.Y.Y.Y.Y.|
  "59 8f 59 8f 59 8f 00 00  70 13 00 00 00 00 e8 17",	// |Y.Y.Y...p.......|
  "00 00 00 00 00 00 69 38  00 00 00 00 00 00 00 00",	// |......i8........|
  "00 00 00 00 00 00 00 00  d0 86 00 00 b2 34 00 00",	// |.............4..|
  "00 00 00 00 00 00 8f 8b  1d 34 00 00 00 00 00 00",	// |.........4......|
  "00 00 00 00 23 39 09 09  00 0c 00 08 00 58 00 00",	// |....#9.......X..|
  "00 00 00 25 77 06 7f 00  00 00 01 00 00 00 00 00",	// |...%w...........|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 ec 06 26 05 00 00 00",	// |...........&....|
  "00 00 00 01 00 00 00 00  31 32 33 34 35 36 37 38",	// |........12345678|
  "39 30 2d 00 08 00 51 57  45 52 54 59 55 49 4f 50",	// |90-...QWERTYUIOP|
  "00 00 0a 00 41 53 44 46  47 48 4a 4b 4c 00 00 00",	// |....ASDFGHJKL...|
  "00 00 5a 58 43 56 42 4e  4d 00 00 00 00 00 00 20",	// |..ZXCVBNM...... |
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 2e 00  1e 00 31 00 14 00 39 00",	// |..........1...9.|
  "1f 00 14 00 18 00 ff ff  01 00 01 4c 45 56 45 4c",	// |...........LEVEL|
  "53 2e 44 41 54 00 00 00  00 00 00 00 00 00 00 00",	// |S.DAT...........|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|
  "00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00",	// |................|

  NULL
};


Bitmap *bitmap_db_field_sp;

struct EngineSnapshotInfo_SP engine_snapshot_sp;

void sp_open_all()
{
  Form_Load();
}

void sp_close_all()
{
}

void InitPrecedingPlayfieldMemory()
{
  int preceding_buffer_size = 0;
  int i;

  for (i = 0; preceding_playfield_memory[i] != NULL; i++)
    preceding_buffer_size += 8;		/* eight 16-bit integer values */

  game_sp.preceding_buffer = preceding_playfield_memory;
  game_sp.preceding_buffer_size = preceding_buffer_size;
}

void InitGfxBuffers_SP()
{
  ReCreateBitmap(&bitmap_db_field_sp, FXSIZE, FYSIZE);
}

unsigned int InitEngineRandom_SP(int seed)
{
  if (seed == NEW_RANDOMIZE)
  {
    subRandomize();

    seed = (int)RandomSeed;
  }

  RandomSeed = (short)seed;

  return (unsigned int) seed;
}


/* ------------------------------------------------------------------------- */
/* Supaplex game engine snapshot handling functions                          */
/* ------------------------------------------------------------------------- */

void SaveEngineSnapshotValues_SP(ListNode **buffers)
{
  int i;

  engine_snapshot_sp.game_sp = game_sp;

  /* these arrays have playfield-size dependent variable size */

  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.PlayField16[i] = PlayField16[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.PlayField8[i] = PlayField8[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.DisPlayField[i] = DisPlayField[i];

  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    engine_snapshot_sp.AnimationPosTable[i] = AnimationPosTable[i];
  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    engine_snapshot_sp.AnimationSubTable[i] = AnimationSubTable[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    engine_snapshot_sp.TerminalState[i] = TerminalState[i];

  /* store special data into engine snapshot buffers */

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(FieldWidth));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(FieldHeight));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(FieldMax));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(LevelMax));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(TimerVar));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(RandomSeed));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(FreezeZonks));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(TerminalMaxCycles));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(mScrollX));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(mScrollY));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(mScrollX_last));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(mScrollY_last));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScreenScrollXPos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScreenScrollYPos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(DisplayMinX));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(DisplayMinY));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(DisplayMaxX));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(DisplayMaxY));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(InfotronsNeeded));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(KillMurphyFlag));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyMoveCounter));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyExplodePos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(SplitMoveFlag));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(RedDiskReleaseMurphyPos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyPosIndex));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyXPos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyYPos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyScreenXPos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyScreenYPos));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MurphyVarFaceLeft));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(RedDiskCount));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(RedDiskReleaseFlag));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(MovingPictureSequencePhase));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(RedDiskReleasePhase));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScratchGravity));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(GravityFlag));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(SnikSnaksElectronsFrozen));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(YellowDisksExploded));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(YawnSleepCounter));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(LeadOutCounter));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(GfxElementLast));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(GfxGraphicLast));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(GfxGraphic));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(GfxFrame));

  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScrollMinX));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScrollMinY));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScrollMaxX));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScrollMaxY));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScrollX));
  SaveSnapshotBuffer(buffers, ARGS_ADDRESS_AND_SIZEOF(ScrollY));

  SaveSnapshotBuffer(buffers, &PlayField16[-game_sp.preceding_buffer_size],
		     game_sp.preceding_buffer_size * sizeof(int));
}

void LoadEngineSnapshotValues_SP()
{
  int i;

  /* stored engine snapshot buffers already restored at this point */

  game_sp = engine_snapshot_sp.game_sp;

  /* these arrays have playfield-size dependent variable size */

  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    PlayField16[i] = engine_snapshot_sp.PlayField16[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    PlayField8[i] = engine_snapshot_sp.PlayField8[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    DisPlayField[i] = engine_snapshot_sp.DisPlayField[i];

  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    AnimationPosTable[i] = engine_snapshot_sp.AnimationPosTable[i];
  for (i = 0; i < FieldWidth * (FieldHeight - 2); i++)
    AnimationSubTable[i] = engine_snapshot_sp.AnimationSubTable[i];
  for (i = 0; i < FieldWidth * FieldHeight + HeaderSize; i++)
    TerminalState[i] = engine_snapshot_sp.TerminalState[i];

  RedrawPlayfield_SP(TRUE);
}
