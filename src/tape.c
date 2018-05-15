// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// tape.c
// ============================================================================

#include "libgame/libgame.h"

#include "tape.h"
#include "init.h"
#include "game.h"
#include "tools.h"
#include "files.h"
#include "network.h"
#include "anim.h"

#define DEBUG_TAPE_WHEN_PLAYING			FALSE

/* tape button identifiers */
#define TAPE_CTRL_ID_EJECT			0
#define TAPE_CTRL_ID_EXTRA			1
#define TAPE_CTRL_ID_STOP			2
#define TAPE_CTRL_ID_PAUSE			3
#define TAPE_CTRL_ID_RECORD			4
#define TAPE_CTRL_ID_PLAY			5

#define NUM_TAPE_BUTTONS			6

/* values for tape handling */
#define TAPE_PAUSE_SECONDS_BEFORE_DEATH		5

/* forward declaration for internal use */
static void HandleTapeButtons(struct GadgetInfo *);
static void TapeStopWarpForward();
static float GetTapeLengthSecondsFloat();

static struct GadgetInfo *tape_gadget[NUM_TAPE_BUTTONS];


/* ========================================================================= */
/* video tape definitions                                                    */
/* ========================================================================= */

#define VIDEO_INFO_OFF			(VIDEO_STATE_DATE_OFF	|	\
					 VIDEO_STATE_TIME_OFF	|	\
					 VIDEO_STATE_FRAME_OFF)
#define VIDEO_STATE_OFF			(VIDEO_STATE_PLAY_OFF	|	\
					 VIDEO_STATE_REC_OFF	|	\
					 VIDEO_STATE_PAUSE_OFF	|	\
					 VIDEO_STATE_FFWD_OFF	|	\
					 VIDEO_STATE_WARP_OFF	|	\
					 VIDEO_STATE_WARP2_OFF	|	\
					 VIDEO_STATE_PBEND_OFF	|	\
					 VIDEO_STATE_1STEP_OFF)
#define VIDEO_PRESS_OFF			(VIDEO_PRESS_PLAY_OFF	|	\
					 VIDEO_PRESS_REC_OFF	|	\
					 VIDEO_PRESS_PAUSE_OFF	|	\
					 VIDEO_PRESS_STOP_OFF	|	\
					 VIDEO_PRESS_EJECT_OFF)
#define VIDEO_ALL_OFF			(VIDEO_INFO_OFF		|	\
	 				 VIDEO_STATE_OFF	|	\
	 				 VIDEO_PRESS_OFF)

#define VIDEO_INFO_ON			(VIDEO_STATE_DATE_ON	|	\
					 VIDEO_STATE_TIME_ON	|	\
					 VIDEO_STATE_FRAME_ON)
#define VIDEO_STATE_ON			(VIDEO_STATE_PLAY_ON	|	\
					 VIDEO_STATE_REC_ON	|	\
					 VIDEO_STATE_PAUSE_ON	|	\
					 VIDEO_STATE_FFWD_ON	|	\
					 VIDEO_STATE_WARP_ON	|	\
					 VIDEO_STATE_WARP2_ON	|	\
					 VIDEO_STATE_PBEND_ON	|	\
					 VIDEO_STATE_1STEP_ON)
#define VIDEO_PRESS_ON			(VIDEO_PRESS_PLAY_ON	|	\
					 VIDEO_PRESS_REC_ON	|	\
					 VIDEO_PRESS_PAUSE_ON	|	\
					 VIDEO_PRESS_STOP_ON	|	\
					 VIDEO_PRESS_EJECT_ON)
#define VIDEO_ALL_ON			(VIDEO_INFO_ON		|	\
	 				 VIDEO_STATE_ON		|	\
	 				 VIDEO_PRESS_ON)

#define VIDEO_INFO			(VIDEO_INFO_ON | VIDEO_INFO_OFF)
#define VIDEO_STATE			(VIDEO_STATE_ON | VIDEO_STATE_OFF)
#define VIDEO_PRESS			(VIDEO_PRESS_ON | VIDEO_PRESS_OFF)
#define VIDEO_ALL			(VIDEO_ALL_ON | VIDEO_ALL_OFF)

#define NUM_TAPE_FUNCTIONS		11
#define NUM_TAPE_FUNCTION_PARTS		2
#define NUM_TAPE_FUNCTION_STATES	2


/* ========================================================================= */
/* video display functions                                                   */
/* ========================================================================= */

static void DrawVideoDisplay_Graphics(unsigned int state, unsigned int value)
{
  int i, j, k;

  static struct
  {
    int graphic;
    struct XY *pos;
  }
  video_pos[NUM_TAPE_FUNCTIONS][NUM_TAPE_FUNCTION_PARTS] =
  {
    {
      { IMG_GFX_TAPE_LABEL_PLAY,		&tape.label.play	},
      { IMG_GFX_TAPE_SYMBOL_PLAY,		&tape.symbol.play	},
    },
    {
      { IMG_GFX_TAPE_LABEL_RECORD,		&tape.label.record	},
      { IMG_GFX_TAPE_SYMBOL_RECORD,		&tape.symbol.record	},
    },
    {
      { IMG_GFX_TAPE_LABEL_PAUSE,		&tape.label.pause	},
      { IMG_GFX_TAPE_SYMBOL_PAUSE,		&tape.symbol.pause	},
    },
    {
      { IMG_GFX_TAPE_LABEL_DATE,		&tape.label.date	},
      { -1,					NULL			},
    },
    {
      { IMG_GFX_TAPE_LABEL_TIME,		&tape.label.time	},
      { -1,					NULL			},
    },
    {
      /* (no label for displaying optional frame) */
      { -1,					NULL			},
      { -1,					NULL			},
    },
    {
      { IMG_GFX_TAPE_LABEL_FAST_FORWARD,	&tape.label.fast_forward  },
      { IMG_GFX_TAPE_SYMBOL_FAST_FORWARD,	&tape.symbol.fast_forward },
    },
    {
      { IMG_GFX_TAPE_LABEL_WARP_FORWARD,	&tape.label.warp_forward  },
      { IMG_GFX_TAPE_SYMBOL_WARP_FORWARD,	&tape.symbol.warp_forward },
    },
    {
      { IMG_GFX_TAPE_LABEL_WARP_FORWARD_BLIND,	&tape.label.warp_forward_blind},
      { IMG_GFX_TAPE_SYMBOL_WARP_FORWARD_BLIND, &tape.symbol.warp_forward_blind},
    },
    {
      { IMG_GFX_TAPE_LABEL_PAUSE_BEFORE_END,	&tape.label.pause_before_end  },
      { IMG_GFX_TAPE_SYMBOL_PAUSE_BEFORE_END,	&tape.symbol.pause_before_end },
    },
    {
      { IMG_GFX_TAPE_LABEL_SINGLE_STEP,		&tape.label.single_step	 },
      { IMG_GFX_TAPE_SYMBOL_SINGLE_STEP,	&tape.symbol.single_step },
    },
  };

  for (k = 0; k < NUM_TAPE_FUNCTION_STATES; k++)	/* on or off states */
  {
    for (i = 0; i < NUM_TAPE_FUNCTIONS; i++)		/* record, play, ... */
    {
      for (j = 0; j < NUM_TAPE_FUNCTION_PARTS; j++)	/* label or symbol */
      {
	int graphic = video_pos[i][j].graphic;
	struct XY *pos = video_pos[i][j].pos;

	if (graphic == -1 ||
	    pos->x == -1 ||
	    pos->y == -1)
	  continue;

	if (state & (1 << (i * 2 + k)))
	{
	  struct GraphicInfo *gfx_bg = &graphic_info[IMG_BACKGROUND_TAPE];
	  struct GraphicInfo *gfx = &graphic_info[graphic];
	  Bitmap *gd_bitmap;
	  int gd_x, gd_y;
	  int skip_value =
	    (j == 0 ? VIDEO_DISPLAY_SYMBOL_ONLY : VIDEO_DISPLAY_LABEL_ONLY);

	  if (value == skip_value)
	    continue;

	  if (k == 1)		/* on */
	  {
	    gd_bitmap = gfx->bitmap;
	    gd_x = gfx->src_x;
	    gd_y = gfx->src_y;
	  }
	  else			/* off */
	  {
	    gd_bitmap = gfx_bg->bitmap;
	    gd_x = gfx_bg->src_x + pos->x;
	    gd_y = gfx_bg->src_y + pos->y;
	  }

	  /* some tape graphics may be undefined -- only draw if defined */
	  if (gd_bitmap != NULL)
	    BlitBitmap(gd_bitmap, drawto, gd_x, gd_y, gfx->width, gfx->height,
		       VX + pos->x, VY + pos->y);

	  redraw_mask |= REDRAW_DOOR_2;
	}
      }
    }
  }
}


#define DATETIME_NONE			(0)

#define DATETIME_DATE_YYYY		(1 << 0)
#define DATETIME_DATE_YY		(1 << 1)
#define DATETIME_DATE_MON		(1 << 2)
#define DATETIME_DATE_MM		(1 << 3)
#define DATETIME_DATE_DD		(1 << 4)

#define DATETIME_TIME_HH		(1 << 5)
#define DATETIME_TIME_MIN		(1 << 6)
#define DATETIME_TIME_MM		(1 << 7)
#define DATETIME_TIME_SS		(1 << 8)

#define DATETIME_FRAME			(1 << 9)

#define DATETIME_XOFFSET_1		(1 << 10)
#define DATETIME_XOFFSET_2		(1 << 11)

#define DATETIME_DATE			(DATETIME_DATE_YYYY	|	\
					 DATETIME_DATE_YY	|	\
					 DATETIME_DATE_MON	|	\
					 DATETIME_DATE_MM	|	\
					 DATETIME_DATE_DD)

#define DATETIME_TIME			(DATETIME_TIME_HH	|	\
					 DATETIME_TIME_MIN	|	\
					 DATETIME_TIME_MM	|	\
					 DATETIME_TIME_SS)

#define MAX_DATETIME_STRING_SIZE	32

static void DrawVideoDisplay_DateTime(unsigned int state, unsigned int value)
{
  int i;

  static char *month_shortnames[] =
  {
    "JAN",
    "FEB",
    "MAR",
    "APR",
    "MAY",
    "JUN",
    "JUL",
    "AUG",
    "SEP",
    "OCT",
    "NOV",
    "DEC"
  };

  static struct
  {
    struct TextPosInfo *pos;
    int type;
  }
  datetime_info[] =
  {
    { &tape.text.date,		DATETIME_DATE_DD			},
    { &tape.text.date,		DATETIME_DATE_MON | DATETIME_XOFFSET_1	},
    { &tape.text.date,		DATETIME_DATE_YY  | DATETIME_XOFFSET_2	},
    { &tape.text.date_yyyy,	DATETIME_DATE_YYYY			},
    { &tape.text.date_yy,	DATETIME_DATE_YY			},
    { &tape.text.date_mon,	DATETIME_DATE_MON			},
    { &tape.text.date_mm,	DATETIME_DATE_MM			},
    { &tape.text.date_dd,	DATETIME_DATE_DD			},

    { &tape.text.time,		DATETIME_TIME_MIN			},
    { &tape.text.time,		DATETIME_TIME_SS  | DATETIME_XOFFSET_1	},
    { &tape.text.time_hh,	DATETIME_TIME_HH			},
    { &tape.text.time_mm,	DATETIME_TIME_MM			},
    { &tape.text.time_ss,	DATETIME_TIME_SS			},

    { &tape.text.frame,		DATETIME_FRAME				},

    { NULL,			DATETIME_NONE				},
  };

  for (i = 0; datetime_info[i].pos != NULL; i++)
  {
    struct TextPosInfo *pos = datetime_info[i].pos;
    int type = datetime_info[i].type;
    int xpos, ypos;

    if (pos->x == -1 &&
	pos->y == -1)
      continue;

    xpos = VX + pos->x + (type & DATETIME_XOFFSET_1 ? pos->xoffset  :
			  type & DATETIME_XOFFSET_2 ? pos->xoffset2 : 0);
    ypos = VY + pos->y;

    if ((type & DATETIME_DATE) && (state & VIDEO_STATE_DATE_ON))
    {
      char s[MAX_DATETIME_STRING_SIZE];
      int year2 = value / 10000;
      int year4 = (year2 < 70 ? 2000 + year2 : 1900 + year2);
      int month_index = (value / 100) % 100;
      int month = month_index + 1;
      int day = value % 100;

      strcpy(s, (type & DATETIME_DATE_YYYY ? int2str(year4, 4) :
		 type & DATETIME_DATE_YY   ? int2str(year2, 2) :
		 type & DATETIME_DATE_MON  ? month_shortnames[month_index] :
		 type & DATETIME_DATE_MM   ? int2str(month, 2) :
		 type & DATETIME_DATE_DD   ? int2str(day, 2) : ""));

      DrawText(xpos, ypos, s, pos->font);
    }
    else if ((type & DATETIME_TIME) && (state & VIDEO_STATE_TIME_ON))
    {
      char s[MAX_DATETIME_STRING_SIZE];
      int hh = (value / 3600) % 100;
      int min = value / 60;
      int mm = (value / 60) % 60;
      int ss = value % 60;

      strcpy(s, (type & DATETIME_TIME_HH  ? int2str(hh, 2) :
		 type & DATETIME_TIME_MIN ? int2str(min, 2) :
		 type & DATETIME_TIME_MM  ? int2str(mm, 2) :
		 type & DATETIME_TIME_SS  ? int2str(ss, 2) : ""));

      DrawText(xpos, ypos, s, pos->font);
    }
    else if ((type & DATETIME_FRAME) && (state & VIDEO_STATE_FRAME_ON))
    {
      DrawText(xpos, ypos, int2str(value, pos->size), pos->font);
    }
  }
}

void DrawVideoDisplay(unsigned int state, unsigned int value)
{
  DrawVideoDisplay_Graphics(state, value);
  DrawVideoDisplay_DateTime(state, value);
}

void DrawVideoDisplayLabel(unsigned int state)
{
  DrawVideoDisplay(state, VIDEO_DISPLAY_LABEL_ONLY);
}

void DrawVideoDisplaySymbol(unsigned int state)
{
  DrawVideoDisplay(state, VIDEO_DISPLAY_SYMBOL_ONLY);
}

void DrawVideoDisplayCurrentState()
{
  int state = 0;

  DrawVideoDisplay(VIDEO_STATE_OFF, 0);

  if (tape.pausing)
    state |= VIDEO_STATE_PAUSE_ON;

  if (tape.recording)
  {
    state |= VIDEO_STATE_REC_ON;

    if (tape.single_step)
      state |= VIDEO_STATE_1STEP_ON;
  }
  else if (tape.playing)
  {
    state |= VIDEO_STATE_PLAY_ON;

    if (!tape.pausing)
    {
      if (tape.deactivate_display)
	state |= VIDEO_STATE_WARP2_ON;
      else if (tape.warp_forward)
	state |= VIDEO_STATE_WARP_ON;
      else if (tape.fast_forward)
	state |= VIDEO_STATE_FFWD_ON;

      if (tape.pause_before_end)
	state |= VIDEO_STATE_PBEND_ON;
    }
  }

  // draw labels and symbols separately to prevent labels overlapping symbols
  DrawVideoDisplayLabel(state);
  DrawVideoDisplaySymbol(state);
}

void DrawCompleteVideoDisplay()
{
  struct GraphicInfo *g_tape = &graphic_info[IMG_BACKGROUND_TAPE];

  /* draw tape background */
  BlitBitmap(g_tape->bitmap, drawto, g_tape->src_x, g_tape->src_y,
	     gfx.vxsize, gfx.vysize, gfx.vx, gfx.vy);

  /* draw tape buttons (forced) */
  RedrawOrRemapTapeButtons();

  DrawVideoDisplay(VIDEO_ALL_OFF, 0);

  if (tape.recording)
  {
    DrawVideoDisplay(VIDEO_STATE_REC_ON, 0);
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, tape.length_frames);

    if (tape.pausing)
      DrawVideoDisplay(VIDEO_STATE_PAUSE_ON, 0);
  }
  else if (tape.playing)
  {
    DrawVideoDisplay(VIDEO_STATE_PLAY_ON, 0);
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, 0);

    if (tape.pausing)
      DrawVideoDisplay(VIDEO_STATE_PAUSE_ON, 0);
  }
  else if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, tape.length_frames);
  }

  BlitBitmap(drawto, bitmap_db_door_2, gfx.vx, gfx.vy, gfx.vxsize, gfx.vysize,
	     0, 0);
}

void TapeDeactivateDisplayOn()
{
  SetDrawDeactivationMask(REDRAW_FIELD);
  audio.sound_deactivated = TRUE;
}

void TapeDeactivateDisplayOff(boolean redraw_display)
{
  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;

  if (redraw_display)
  {
    RedrawPlayfield();
    DrawGameDoorValues();
  }
}


/* ========================================================================= */
/* tape logging functions                                                    */
/* ========================================================================= */

void PrintTapeReplayProgress(boolean replay_finished)
{
  static unsigned int counter_last = -1;
  unsigned int counter = Counter();
  unsigned int counter_seconds  = counter / 1000;

  if (!replay_finished)
  {
    unsigned int counter_delay = 50;

    if (counter > counter_last + counter_delay)
    {
      PrintNoLog("\r");
      PrintNoLog("Level %03d [%02d:%02d]: [%02d:%02d] - playing tape ... ",
		 level_nr, tape.length_seconds / 60, tape.length_seconds % 60,
		 TapeTime / 60, TapeTime % 60);

      counter_last = counter;
    }
  }
  else
  {
    float tape_length_seconds = GetTapeLengthSecondsFloat();

    PrintNoLog("\r");
    Print("Level %03d [%02d:%02d]: (%02d:%02d.%03d / %.2f %%) - %s.\n",
	  level_nr, tape.length_seconds / 60, tape.length_seconds % 60,
	  counter_seconds / 60, counter_seconds % 60, counter % 1000,
	  (float)counter / tape_length_seconds / 10,
	  tape.auto_play_level_solved ? "solved" : "NOT SOLVED");

    counter_last = -1;
  }
}


/* ========================================================================= */
/* tape control functions                                                    */
/* ========================================================================= */

void TapeSetDateFromEpochSeconds(time_t epoch_seconds)
{
  struct tm *lt = localtime(&epoch_seconds);

  tape.date = 10000 * (lt->tm_year % 100) + 100 * lt->tm_mon + lt->tm_mday;
}

void TapeSetDateFromNow()
{
  TapeSetDateFromEpochSeconds(time(NULL));
}

void TapeErase()
{
  int i;

  tape.counter = 0;
  tape.length = 0;
  tape.length_frames = 0;
  tape.length_seconds = 0;

  if (leveldir_current)
    setString(&tape.level_identifier, leveldir_current->identifier);

  tape.level_nr = level_nr;
  tape.pos[tape.counter].delay = 0;
  tape.changed = TRUE;

  tape.random_seed = InitRND(level.random_seed);

  tape.file_version = FILE_VERSION_ACTUAL;
  tape.game_version = GAME_VERSION_ACTUAL;
  tape.engine_version = level.game_version;

  TapeSetDateFromNow();

  for (i = 0; i < MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  tape.centered_player_nr_next = -1;
  tape.set_centered_player = FALSE;

  tape.use_mouse = (level.game_engine_type == GAME_ENGINE_TYPE_MM);
}

static void TapeRewind()
{
  tape.counter = 0;
  tape.delay_played = 0;
  tape.pause_before_end = FALSE;
  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.fast_forward = FALSE;
  tape.warp_forward = FALSE;
  tape.deactivate_display = FALSE;
  tape.auto_play = (global.autoplay_leveldir != NULL);
  tape.auto_play_level_solved = FALSE;
  tape.quick_resume = FALSE;
  tape.single_step = FALSE;

  tape.centered_player_nr_next = -1;
  tape.set_centered_player = FALSE;

  InitRND(tape.random_seed);
}

static void TapeSetRandomSeed(int random_seed)
{
  tape.random_seed = InitRND(random_seed);
}

void TapeStartRecording(int random_seed)
{
  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  TapeErase();
  TapeRewind();
  TapeSetRandomSeed(random_seed);

  tape.recording = TRUE;

  DrawVideoDisplay(VIDEO_STATE_REC_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_FRAME_ON, 0);

  MapTapeWarpButton();

  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;
}

static void TapeStartGameRecording()
{
  StartGameActions(options.network, TRUE, level.random_seed);
}

static void TapeAppendRecording()
{
  if (!tape.playing || !tape.pausing)
    return;

  // stop playing
  tape.playing = FALSE;
  tape.fast_forward = FALSE;
  tape.warp_forward = FALSE;
  tape.pause_before_end = FALSE;
  tape.deactivate_display = FALSE;

  // start recording
  tape.recording = TRUE;
  tape.changed = TRUE;

  // set current delay (for last played move)
  tape.pos[tape.counter].delay = tape.delay_played;

  // set current date
  TapeSetDateFromNow();

  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF | VIDEO_STATE_REC_ON, 0);

  UpdateAndDisplayGameControlValues();
}

void TapeHaltRecording()
{
  tape.counter++;

  // initialize delay for next tape entry (to be able to continue recording)
  if (tape.counter < MAX_TAPE_LEN)
    tape.pos[tape.counter].delay = 0;

  tape.length = tape.counter;
  tape.length_frames = GetTapeLengthFrames();
  tape.length_seconds = GetTapeLengthSeconds();
}

void TapeStopRecording()
{
  if (tape.recording)
    TapeHaltRecording();

  tape.recording = FALSE;
  tape.pausing = FALSE;

  DrawVideoDisplay(VIDEO_STATE_REC_OFF, 0);
  MapTapeEjectButton();
}

boolean TapeAddAction(byte action[MAX_PLAYERS])
{
  int i;

  if (tape.pos[tape.counter].delay > 0)		/* already stored action */
  {
    boolean changed_events = FALSE;

    for (i = 0; i < MAX_PLAYERS; i++)
      if (tape.pos[tape.counter].action[i] != action[i])
	changed_events = TRUE;

    if (changed_events || tape.pos[tape.counter].delay >= 255)
    {
      if (tape.counter >= MAX_TAPE_LEN - 1)
	return FALSE;

      tape.counter++;
      tape.pos[tape.counter].delay = 0;
    }
    else
      tape.pos[tape.counter].delay++;
  }

  if (tape.pos[tape.counter].delay == 0)	/* store new action */
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      tape.pos[tape.counter].action[i] = action[i];

    tape.pos[tape.counter].delay++;
  }

  return TRUE;
}

void TapeRecordAction(byte action_raw[MAX_PLAYERS])
{
  byte action[MAX_PLAYERS];
  int i;

  if (!tape.recording)		/* (record action even when tape is paused) */
    return;

  for (i = 0; i < MAX_PLAYERS; i++)
    action[i] = action_raw[i];

  if (!tape.use_mouse && tape.set_centered_player)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
      if (tape.centered_player_nr_next == i ||
	  tape.centered_player_nr_next == -1)
	action[i] |= KEY_SET_FOCUS;

    tape.set_centered_player = FALSE;
  }

  if (!TapeAddAction(action))
    TapeStopRecording();
}

void TapeTogglePause(boolean toggle_mode)
{
  if (tape.playing && tape.pausing && (toggle_mode & TAPE_TOGGLE_PLAY_PAUSE))
  {
    // continue playing in normal mode
    tape.fast_forward = FALSE;
    tape.warp_forward = FALSE;
    tape.deactivate_display = FALSE;

    tape.pause_before_end = FALSE;
  }

  tape.pausing = !tape.pausing;

  if (tape.single_step && (toggle_mode & TAPE_TOGGLE_MANUAL))
    tape.single_step = FALSE;

  DrawVideoDisplayCurrentState();

  if (tape.deactivate_display)
  {
    if (tape.pausing)
      TapeDeactivateDisplayOff(game_status == GAME_MODE_PLAYING);
    else
      TapeDeactivateDisplayOn();
  }

  if (tape.quick_resume)
  {
    tape.quick_resume = FALSE;

    TapeStopWarpForward();
    TapeAppendRecording();

    if (!CheckEngineSnapshotSingle())
      SaveEngineSnapshotSingle();

    // restart step/move snapshots after quick loading tape
    SaveEngineSnapshotToListInitial();

    // do not map undo/redo buttons after quick loading tape
    return;
  }

  if (setup.show_snapshot_buttons &&
      game_status == GAME_MODE_PLAYING &&
      CheckEngineSnapshotList())
  {
    if (tape.pausing)
      MapUndoRedoButtons();
    else if (!tape.single_step)
      UnmapUndoRedoButtons();
  }
}

void TapeStartPlaying()
{
  if (TAPE_IS_EMPTY(tape))
    return;

  if (!TAPE_IS_STOPPED(tape))
    TapeStop();

  TapeRewind();

  tape.playing = TRUE;

  DrawVideoDisplay(VIDEO_STATE_PLAY_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
  DrawVideoDisplay(VIDEO_STATE_TIME_ON, 0);
  DrawVideoDisplay(VIDEO_STATE_FRAME_ON, 0);

  MapTapeWarpButton();

  SetDrawDeactivationMask(REDRAW_NONE);
  audio.sound_deactivated = FALSE;
}

static void TapeStartGamePlaying()
{
  TapeStartPlaying();

  InitGame();
}

void TapeStopPlaying()
{
  tape.playing = FALSE;
  tape.pausing = FALSE;

  if (tape.warp_forward)
    TapeStopWarpForward();

  DrawVideoDisplay(VIDEO_STATE_PLAY_OFF, 0);
  MapTapeEjectButton();
}

byte *TapePlayAction()
{
  int update_delay = FRAMES_PER_SECOND / 2;
  boolean update_video_display = (FrameCounter % update_delay == 0);
  boolean update_draw_label_on = ((FrameCounter / update_delay) % 2 == 1);
  static byte action[MAX_PLAYERS];
  int i;

  if (!tape.playing || tape.pausing)
    return NULL;

  if (tape.pause_before_end)  // stop some seconds before end of tape
  {
    if (TapeTime > tape.length_seconds - TAPE_PAUSE_SECONDS_BEFORE_DEATH)
    {
      TapeStopWarpForward();
      TapeTogglePause(TAPE_TOGGLE_MANUAL);

      return NULL;
    }
  }

  if (tape.counter >= tape.length)	/* end of tape reached */
  {
    if (tape.warp_forward && !tape.auto_play)
    {
      TapeStopWarpForward();
      TapeTogglePause(TAPE_TOGGLE_MANUAL);
    }
    else
    {
      TapeStop();
    }

    return NULL;
  }

  if (update_video_display && !tape.deactivate_display)
  {
    int state = 0;

    if (tape.warp_forward)
      state |= VIDEO_STATE_WARP(update_draw_label_on);
    else if (tape.fast_forward)
      state |= VIDEO_STATE_FFWD(update_draw_label_on);

    if (tape.pause_before_end)
      state |= VIDEO_STATE_PBEND(update_draw_label_on);

    // draw labels and symbols separately to prevent labels overlapping symbols
    DrawVideoDisplayLabel(state);
    DrawVideoDisplaySymbol(state);
  }

  for (i = 0; i < MAX_PLAYERS; i++)
    action[i] = tape.pos[tape.counter].action[i];

#if DEBUG_TAPE_WHEN_PLAYING
  printf("%05d", FrameCounter);
  for (i = 0; i < MAX_PLAYERS; i++)
    printf("   %08x", action[i]);
  printf("\n");
#endif

  tape.set_centered_player = FALSE;
  tape.centered_player_nr_next = -999;

  if (!tape.use_mouse)
  {
    for (i = 0; i < MAX_PLAYERS; i++)
    {
      if (action[i] & KEY_SET_FOCUS)
      {
	tape.set_centered_player = TRUE;
	tape.centered_player_nr_next =
	  (tape.centered_player_nr_next == -999 ? i : -1);
      }

      action[i] &= ~KEY_SET_FOCUS;
    }
  }

  tape.delay_played++;
  if (tape.delay_played >= tape.pos[tape.counter].delay)
  {
    tape.counter++;
    tape.delay_played = 0;
  }

  if (tape.auto_play)
    PrintTapeReplayProgress(FALSE);

  return action;
}

void TapeStop()
{
  if (tape.pausing)
    TapeTogglePause(TAPE_TOGGLE_MANUAL);

  TapeStopRecording();
  TapeStopPlaying();

  DrawVideoDisplay(VIDEO_STATE_OFF, 0);

  if (tape.date && tape.length)
  {
    DrawVideoDisplay(VIDEO_STATE_DATE_ON, tape.date);
    DrawVideoDisplay(VIDEO_STATE_TIME_ON, tape.length_seconds);
    DrawVideoDisplay(VIDEO_STATE_FRAME_ON, tape.length_frames);
  }
}

unsigned int GetTapeLengthFrames()
{
  unsigned int tape_length_frames = 0;
  int i;

  if (TAPE_IS_EMPTY(tape))
    return(0);

  for (i = 0; i < tape.length; i++)
    tape_length_frames += tape.pos[i].delay;

  return tape_length_frames;
}

unsigned int GetTapeLengthSeconds()
{
  return (GetTapeLengthFrames() * GAME_FRAME_DELAY / 1000);
}

static float GetTapeLengthSecondsFloat()
{
  return ((float)GetTapeLengthFrames() * GAME_FRAME_DELAY / 1000);
}

static void TapeStartWarpForward(int mode)
{
  tape.fast_forward = (mode & AUTOPLAY_FFWD);
  tape.warp_forward = (mode & AUTOPLAY_WARP);
  tape.deactivate_display = (mode & AUTOPLAY_WARP_NO_DISPLAY);

  tape.pausing = FALSE;

  if (tape.deactivate_display)
    TapeDeactivateDisplayOn();

  DrawVideoDisplayCurrentState();
}

static void TapeStopWarpForward()
{
  tape.fast_forward = FALSE;
  tape.warp_forward = FALSE;
  tape.deactivate_display = FALSE;

  tape.pause_before_end = FALSE;

  TapeDeactivateDisplayOff(game_status == GAME_MODE_PLAYING);

  DrawVideoDisplayCurrentState();
}

static void TapeSingleStep()
{
  if (options.network)
    return;

  if (!tape.pausing)
    TapeTogglePause(TAPE_TOGGLE_MANUAL);

  tape.single_step = !tape.single_step;

  DrawVideoDisplay(VIDEO_STATE_1STEP(tape.single_step), 0);
}

void TapeQuickSave()
{
  if (game_status == GAME_MODE_MAIN)
  {
    Request("No game that can be saved!", REQ_CONFIRM);

    return;
  }

  if (game_status != GAME_MODE_PLAYING)
    return;

  if (tape.recording)
    TapeHaltRecording();	/* prepare tape for saving on-the-fly */

  if (TAPE_IS_EMPTY(tape))
  {
    Request("No tape that can be saved!", REQ_CONFIRM);

    return;
  }

  if (SaveTapeChecked(tape.level_nr))
    SaveEngineSnapshotSingle();
}

void TapeQuickLoad()
{
  char *filename = getTapeFilename(level_nr);

  if (!fileExists(filename))
  {
    Request("No tape for this level!", REQ_CONFIRM);

    return;
  }

  if (tape.recording && !Request("Stop recording and load tape?",
				 REQ_ASK | REQ_STAY_CLOSED))
  {
    OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);

    return;
  }

  if (game_status != GAME_MODE_PLAYING && game_status != GAME_MODE_MAIN)
    return;

  if (CheckEngineSnapshotSingle())
  {
    TapeStartGamePlaying();

    LoadEngineSnapshotSingle();

    DrawCompleteVideoDisplay();

    tape.playing = TRUE;
    tape.pausing = TRUE;

    TapeStopWarpForward();
    TapeAppendRecording();

    // restart step/move snapshots after quick loading tape
    SaveEngineSnapshotToListInitial();

    if (FrameCounter > 0)
      return;
  }

  TapeStop();
  TapeErase();

  LoadTape(level_nr);

  if (!TAPE_IS_EMPTY(tape))
  {
    TapeStartGamePlaying();
    TapeStartWarpForward(AUTOPLAY_MODE_WARP_NO_DISPLAY);

    tape.quick_resume = TRUE;
  }
  else	/* this should not happen (basically checked above) */
  {
    int reopen_door = (game_status == GAME_MODE_PLAYING ? REQ_REOPEN : 0);

    Request("No tape for this level!", REQ_CONFIRM | reopen_door);
  }
}

void InsertSolutionTape()
{
  boolean level_has_tape = (level.game_engine_type == GAME_ENGINE_TYPE_SP &&
			    level.native_sp_level->demo.is_available);

  if (!fileExists(getSolutionTapeFilename(level_nr)) && !level_has_tape)
  {
    Request("No solution tape for this level!", REQ_CONFIRM);

    return;
  }

  // if tape recorder already contains a tape, remove it without asking
  TapeErase();

  LoadSolutionTape(level_nr);

  if (TAPE_IS_EMPTY(tape))
    Request("Loading solution tape for this level failed!", REQ_CONFIRM);

  DrawCompleteVideoDisplay();
}


/* ------------------------------------------------------------------------- *
 * tape autoplay functions
 * ------------------------------------------------------------------------- */

void AutoPlayTape()
{
  static LevelDirTree *autoplay_leveldir = NULL;
  static boolean autoplay_initialized = FALSE;
  static int autoplay_level_nr = -1;
  static int num_levels_played = 0;
  static int num_levels_solved = 0;
  static int num_tape_missing = 0;
  static boolean level_failed[MAX_TAPES_PER_SET];
  int i;

  if (autoplay_initialized)
  {
    /* just finished auto-playing tape */
    PrintTapeReplayProgress(TRUE);

    num_levels_played++;

    if (tape.auto_play_level_solved)
      num_levels_solved++;
    else if (level_nr >= 0 && level_nr < MAX_TAPES_PER_SET)
      level_failed[level_nr] = TRUE;
  }
  else
  {
    DrawCompleteVideoDisplay();

    audio.sound_enabled = FALSE;
    setup.engine_snapshot_mode = getStringCopy(STR_SNAPSHOT_MODE_OFF);

    autoplay_leveldir = getTreeInfoFromIdentifier(leveldir_first,
						  global.autoplay_leveldir);

    if (autoplay_leveldir == NULL)
      Error(ERR_EXIT, "no such level identifier: '%s'",
	    global.autoplay_leveldir);

    leveldir_current = autoplay_leveldir;

    if (autoplay_leveldir->first_level < 0)
      autoplay_leveldir->first_level = 0;
    if (autoplay_leveldir->last_level >= MAX_TAPES_PER_SET)
      autoplay_leveldir->last_level = MAX_TAPES_PER_SET - 1;

    autoplay_level_nr = autoplay_leveldir->first_level;

    PrintLine("=", 79);
    Print("Automatically playing level tapes\n");
    PrintLine("-", 79);
    Print("Level series identifier: '%s'\n", autoplay_leveldir->identifier);
    Print("Level series name:       '%s'\n", autoplay_leveldir->name);
    Print("Level series author:     '%s'\n", autoplay_leveldir->author);
    Print("Number of levels:        %d\n",   autoplay_leveldir->levels);
    PrintLine("=", 79);
    Print("\n");

    for (i = 0; i < MAX_TAPES_PER_SET; i++)
      level_failed[i] = FALSE;

    autoplay_initialized = TRUE;
  }

  while (autoplay_level_nr <= autoplay_leveldir->last_level)
  {
    level_nr = autoplay_level_nr++;

    if (!global.autoplay_all && !global.autoplay_level[level_nr])
      continue;

    TapeErase();

    LoadLevel(level_nr);

    if (level.no_level_file || level.no_valid_file)
    {
      Print("Level %03d: (no level)\n", level_nr);

      continue;
    }

#if 0
    /* ACTIVATE THIS FOR LOADING/TESTING OF LEVELS ONLY */
    Print("Level %03d: (only testing level)\n", level_nr);
    continue;
#endif

    if (options.mytapes)
      LoadTape(level_nr);
    else
      LoadSolutionTape(level_nr);

    if (tape.no_valid_file)
    {
      num_tape_missing++;

      Print("Level %03d: (no tape)\n", level_nr);

      continue;
    }

    InitCounter();

    TapeStartGamePlaying();
    TapeStartWarpForward(global.autoplay_mode);

    return;
  }

  Print("\n");
  PrintLine("=", 79);
  Print("Number of levels played: %d\n", num_levels_played);
  Print("Number of levels solved: %d (%d%%)\n", num_levels_solved,
	 (num_levels_played ? num_levels_solved * 100 / num_levels_played :0));
  PrintLine("-", 79);
  Print("Summary (for automatic parsing by scripts):\n");
  Print("LEVELDIR '%s', SOLVED %d/%d (%d%%)",
	 autoplay_leveldir->identifier, num_levels_solved, num_levels_played,
	 (num_levels_played ? num_levels_solved * 100 / num_levels_played :0));

  if (num_levels_played != num_levels_solved)
  {
    Print(", FAILED:");
    for (i = 0; i < MAX_TAPES_PER_SET; i++)
      if (level_failed[i])
	Print(" %03d", i);
  }

  Print("\n");
  PrintLine("=", 79);

  CloseAllAndExit(0);
}


/* ---------- new tape button stuff ---------------------------------------- */

static struct
{
  int graphic;
  struct XY *pos;
  int gadget_id;
  char *infotext;
} tapebutton_info[NUM_TAPE_BUTTONS] =
{
  {
    IMG_GFX_TAPE_BUTTON_EJECT,		&tape.button.eject,
    TAPE_CTRL_ID_EJECT,			"eject tape"
  },
  {
    /* (same position as "eject" button) */
    IMG_GFX_TAPE_BUTTON_EXTRA,		&tape.button.eject,
    TAPE_CTRL_ID_EXTRA,			"extra functions"
  },
  {
    IMG_GFX_TAPE_BUTTON_STOP,		&tape.button.stop,
    TAPE_CTRL_ID_STOP,			"stop tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_PAUSE,		&tape.button.pause,
    TAPE_CTRL_ID_PAUSE,			"pause tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_RECORD,		&tape.button.record,
    TAPE_CTRL_ID_RECORD,		"record tape"
  },
  {
    IMG_GFX_TAPE_BUTTON_PLAY,		&tape.button.play,
    TAPE_CTRL_ID_PLAY,			"play tape"
  }
};

void CreateTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
  {
    struct GraphicInfo *gfx = &graphic_info[tapebutton_info[i].graphic];
    struct XY *pos = tapebutton_info[i].pos;
    struct GadgetInfo *gi;
    int gd_x = gfx->src_x;
    int gd_y = gfx->src_y;
    int gd_xp = gfx->src_x + gfx->pressed_xoffset;
    int gd_yp = gfx->src_y + gfx->pressed_yoffset;
    int id = i;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_INFO_TEXT, tapebutton_info[i].infotext,
		      GDI_X, VX + pos->x,
		      GDI_Y, VY + pos->y,
		      GDI_WIDTH, gfx->width,
		      GDI_HEIGHT, gfx->height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gfx->bitmap, gd_x, gd_y,
		      GDI_DESIGN_PRESSED, gfx->bitmap, gd_xp, gd_yp,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, GD_EVENT_RELEASED,
		      GDI_CALLBACK_ACTION, HandleTapeButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    tape_gadget[id] = gi;
  }
}

void FreeTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    FreeGadget(tape_gadget[i]);
}

void MapTapeEjectButton()
{
  UnmapGadget(tape_gadget[TAPE_CTRL_ID_EXTRA]);
  MapGadget(tape_gadget[TAPE_CTRL_ID_EJECT]);
}

void MapTapeWarpButton()
{
  UnmapGadget(tape_gadget[TAPE_CTRL_ID_EJECT]);
  MapGadget(tape_gadget[TAPE_CTRL_ID_EXTRA]);
}

void MapTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    if (i != TAPE_CTRL_ID_EXTRA)
      MapGadget(tape_gadget[i]);

  if (tape.recording || tape.playing)
    MapTapeWarpButton();

  if (tape.show_game_buttons)
    MapGameButtonsOnTape();
}

void UnmapTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    UnmapGadget(tape_gadget[i]);

  if (tape.show_game_buttons)
    UnmapGameButtonsOnTape();
}

void RedrawTapeButtons()
{
  int i;

  for (i = 0; i < NUM_TAPE_BUTTONS; i++)
    RedrawGadget(tape_gadget[i]);

  if (tape.show_game_buttons)
    RedrawGameButtonsOnTape();

  // RedrawGadget() may have set REDRAW_ALL if buttons are defined off-area
  redraw_mask &= ~REDRAW_ALL;
}

void RedrawOrRemapTapeButtons()
{
  if (tape_gadget[TAPE_CTRL_ID_PLAY]->mapped)
  {
    // tape buttons already mapped
    RedrawTapeButtons();
  }
  else
  {
    UnmapTapeButtons();
    MapTapeButtons();
  }
}

static void HandleTapeButtonsExt(int id)
{
  if (game_status != GAME_MODE_MAIN && game_status != GAME_MODE_PLAYING)
    return;

  switch (id)
  {
    case TAPE_CTRL_ID_EJECT:
      TapeStop();

      if (TAPE_IS_EMPTY(tape))
      {
	LoadTape(level_nr);

	if (TAPE_IS_EMPTY(tape))
	  Request("No tape for this level!", REQ_CONFIRM);
      }
      else
      {
	if (tape.changed)
	  SaveTapeChecked(level_nr);

	TapeErase();
      }

      DrawCompleteVideoDisplay();
      break;

    case TAPE_CTRL_ID_EXTRA:
      if (tape.playing)
      {
	tape.pause_before_end = !tape.pause_before_end;

	DrawVideoDisplayCurrentState();
      }
      else if (tape.recording)
      {
	TapeSingleStep();
      }

      break;

    case TAPE_CTRL_ID_STOP:
      TapeStop();

      break;

    case TAPE_CTRL_ID_PAUSE:
      TapeTogglePause(TAPE_TOGGLE_MANUAL);

      break;

    case TAPE_CTRL_ID_RECORD:
      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartGameRecording();
      }
      else if (tape.pausing)
      {
	if (tape.playing)			/* PLAY -> PAUSE -> RECORD */
	  TapeAppendRecording();
	else
	  TapeTogglePause(TAPE_TOGGLE_MANUAL);
      }

      break;

    case TAPE_CTRL_ID_PLAY:
      if (tape.recording && tape.pausing)	/* PAUSE -> RECORD */
      {
	// ("TAPE_IS_EMPTY(tape)" is TRUE here -- probably fix this)

	TapeTogglePause(TAPE_TOGGLE_MANUAL);
      }

      if (TAPE_IS_EMPTY(tape))
	break;

      if (TAPE_IS_STOPPED(tape))
      {
	TapeStartGamePlaying();
      }
      else if (tape.playing)
      {
	if (tape.pausing)			/* PAUSE -> PLAY */
	{
	  TapeTogglePause(TAPE_TOGGLE_MANUAL | TAPE_TOGGLE_PLAY_PAUSE);
	}
	else if (!tape.fast_forward)		/* PLAY -> FFWD */
	{
	  tape.fast_forward = TRUE;
	}
	else if (!tape.warp_forward)		/* FFWD -> WARP */
	{
	  tape.warp_forward = TRUE;
	}
	else if (!tape.deactivate_display)	/* WARP -> WARP BLIND */
	{
	  tape.deactivate_display = TRUE;

	  TapeDeactivateDisplayOn();
	}
	else					/* WARP BLIND -> PLAY */
	{
	  tape.fast_forward = FALSE;
	  tape.warp_forward = FALSE;
	  tape.deactivate_display = FALSE;

	  TapeDeactivateDisplayOff(game_status == GAME_MODE_PLAYING);
	}

	DrawVideoDisplayCurrentState();
      }

      break;

    default:
      break;
  }
}

static void HandleTapeButtons(struct GadgetInfo *gi)
{
  HandleTapeButtonsExt(gi->custom_id);
}

void HandleTapeButtonKeys(Key key)
{
  boolean eject_button_is_active = TAPE_IS_STOPPED(tape);
  boolean extra_button_is_active = !eject_button_is_active;

  if (key == setup.shortcut.tape_eject && eject_button_is_active)
    HandleTapeButtonsExt(TAPE_CTRL_ID_EJECT);
  else if (key == setup.shortcut.tape_extra && extra_button_is_active)
    HandleTapeButtonsExt(TAPE_CTRL_ID_EXTRA);
  else if (key == setup.shortcut.tape_stop)
    HandleTapeButtonsExt(TAPE_CTRL_ID_STOP);
  else if (key == setup.shortcut.tape_pause)
    HandleTapeButtonsExt(TAPE_CTRL_ID_PAUSE);
  else if (key == setup.shortcut.tape_record)
    HandleTapeButtonsExt(TAPE_CTRL_ID_RECORD);
  else if (key == setup.shortcut.tape_play)
    HandleTapeButtonsExt(TAPE_CTRL_ID_PLAY);
}
