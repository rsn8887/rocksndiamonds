// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_game.c
// ============================================================================

#include <math.h>

#include "main_mm.h"

#include "mm_main.h"
#include "mm_game.h"
#include "mm_tools.h"

/* graphic position values for game controls */
#define ENERGY_XSIZE		32
#define ENERGY_YSIZE		MAX_LASER_ENERGY
#define OVERLOAD_XSIZE		ENERGY_XSIZE
#define OVERLOAD_YSIZE		MAX_LASER_OVERLOAD

/* values for Explode_MM() */
#define EX_PHASE_START		0
#define EX_NORMAL		0
#define EX_KETTLE		1
#define EX_SHORT		2

/* special positions in the game control window (relative to control window) */
#define XX_LEVEL		36
#define YY_LEVEL		23
#define XX_KETTLES		29
#define YY_KETTLES		63
#define XX_SCORE		22
#define YY_SCORE		101
#define XX_ENERGY		8
#define YY_ENERGY		158
#define XX_OVERLOAD		60
#define YY_OVERLOAD		YY_ENERGY

/* special positions in the game control window (relative to main window) */
#define DX_LEVEL		(DX + XX_LEVEL)
#define DY_LEVEL		(DY + YY_LEVEL)
#define DX_KETTLES		(DX + XX_KETTLES)
#define DY_KETTLES		(DY + YY_KETTLES)
#define DX_SCORE		(DX + XX_SCORE)
#define DY_SCORE		(DY + YY_SCORE)
#define DX_ENERGY		(DX + XX_ENERGY)
#define DY_ENERGY		(DY + YY_ENERGY)
#define DX_OVERLOAD		(DX + XX_OVERLOAD)
#define DY_OVERLOAD		(DY + YY_OVERLOAD)

#define IS_LOOP_SOUND(s)	((s) == SND_FUEL)
#define IS_MUSIC_SOUND(s)	((s) == SND_TYGER || (s) == SND_VOYAGER)

/* game button identifiers */
#define GAME_CTRL_ID_LEFT	0
#define GAME_CTRL_ID_MIDDLE	1
#define GAME_CTRL_ID_RIGHT	2

#define NUM_GAME_BUTTONS	3

/* values for DrawLaser() */
#define DL_LASER_DISABLED	0
#define DL_LASER_ENABLED	1

/* values for 'click_delay_value' in ClickElement() */
#define CLICK_DELAY_FIRST	12	/* delay (frames) after first click */
#define CLICK_DELAY		6	/* delay (frames) for pressed butten */

#define AUTO_ROTATE_DELAY	CLICK_DELAY
#define INIT_GAME_ACTIONS_DELAY	(ONE_SECOND_DELAY / GAME_FRAME_DELAY)
#define NUM_INIT_CYCLE_STEPS	16
#define PACMAN_MOVE_DELAY	12
#define ENERGY_DELAY		(ONE_SECOND_DELAY / GAME_FRAME_DELAY)
#define HEALTH_DEC_DELAY	3
#define HEALTH_INC_DELAY	9
#define HEALTH_DELAY(x)		((x) ? HEALTH_DEC_DELAY : HEALTH_INC_DELAY)

#define BEGIN_NO_HEADLESS			\
  {						\
    boolean last_headless = program.headless;	\
						\
    program.headless = FALSE;			\

#define END_NO_HEADLESS				\
    program.headless = last_headless;		\
  }						\

/* forward declaration for internal use */
static int MovingOrBlocked2Element_MM(int, int);
static void Bang_MM(int, int);
static void RaiseScore_MM(int);
static void RaiseScoreElement_MM(int);
static void RemoveMovingField_MM(int, int);
static void InitMovingField_MM(int, int, int);
static void ContinueMoving_MM(int, int);
static void Moving2Blocked_MM(int, int, int *, int *);

/* bitmap for laser beam detection */
static Bitmap *laser_bitmap = NULL;

/* variables for laser control */
static int last_LX = 0, last_LY = 0, last_hit_mask = 0;
static int hold_x = -1, hold_y = -1;

/* variables for pacman control */
static int pacman_nr = -1;

/* various game engine delay counters */
static unsigned int rotate_delay = 0;
static unsigned int pacman_delay = 0;
static unsigned int energy_delay = 0;
static unsigned int overload_delay = 0;

/* element masks for scanning pixels of MM elements */
static const char mm_masks[10][16][16 + 1] =
{
  {
    "                ",
    "    XXXXX       ",
    "   XXXXXXX      ",
    "  XXXXXXXXXXX   ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXXX",
    "  XXXXXXXXXXXXXX",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
  },
  {
    "                ",
    "    XXXXXXXX    ",
    "  XXXXXXXXXXXX  ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    "  XXXXXXXXXXXXX ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
  },
  {
    "                ",
    "    XXXXXX      ",
    "  XXXXXXXXX     ",
    " XXXXXXXXXXX    ",
    "XXXXXXXXXXXXX   ",
    "XXXXXXXXXXXXX   ",
    "XXXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    " XXXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
  },
  {
    "                ",
    "    XXXXXX      ",
    "   XXXXXXXX     ",
    "  XXXXXXXXXX    ",
    "  XXXXXXXXXXX   ",
    "  XXXXXXXXXXX   ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "  XXXXX  XXXXX  ",
  },
  {
    " XXXXXX  XXXXXX ",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "                ",
    "                ",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    " XXXXXX  XXXXXX ",
  },
  {
    " XXXXXX  XXXXXX ",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    " XXXXXX  XXXXXX ",
    " XXXXXX  XXXXXX ",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    "XXXXXXX  XXXXXXX",
    " XXXXXX  XXXXXX ",
  },
  {
    "     XX  XXXXX  ",
    "    XXX  XXXX   ",
    "   XXXX  XXX   X",
    "  XXXXXXXXX   XX",
    " XXXXXXXXX   XXX",
    "XXXXXXXXX   XXXX",
    "XXXXXXXX   XXXXX",
    "   XXXX   XXX   ",
    "   XXX   XXXX   ",
    "XXXXX   XXXXXXXX",
    "XXXX   XXXXXXXXX",
    "XXX   XXXXXXXXX ",
    "XX   XXXXXXXXX  ",
    "X   XXX  XXXX   ",
    "   XXXX  XXX    ",
    "  XXXXX  XX     ",
  },
  {
    "  XXXXX  XX     ",
    "   XXXX  XXX    ",
    "X   XXX  XXXX   ",
    "XX   XXXXXXXXX  ",
    "XXX   XXXXXXXXX ",
    "XXXX   XXXXXXXXX",
    "XXXXX   XXXXXXXX",
    "   XXX   XXXX   ",
    "   XXXX   XXX   ",
    "XXXXXXXX   XXXXX",
    "XXXXXXXXX   XXXX",
    " XXXXXXXXX   XXX",
    "  XXXXXXXXX   XX",
    "   XXXX  XXX   X",
    "    XXX  XXXX   ",
    "     XX  XXXXX  ",
  },
  {
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
    "XXXXXXXXXXXXXXXX",
  },
  {
    "                ",
    "      XXXX      ",
    "    XXXXXXXX    ",
    "   XXXXXXXXXX   ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    " XXXXXXXXXXXXXX ",
    "  XXXXXXXXXXXX  ",
    "  XXXXXXXXXXXX  ",
    "   XXXXXXXXXX   ",
    "    XXXXXXXX    ",
    "      XXXX      ",
    "                ",
  },
};

static int get_element_angle(int element)
{
  int element_phase = get_element_phase(element);

  if (IS_MIRROR_FIXED(element) ||
      IS_MCDUFFIN(element) ||
      IS_LASER(element) ||
      IS_RECEIVER(element))
    return 4 * element_phase;
  else
    return element_phase;
}

static int get_opposite_angle(int angle)
{
  int opposite_angle = angle + ANG_RAY_180;

  /* make sure "opposite_angle" is in valid interval [0, 15] */
  return (opposite_angle + 16) % 16;
}

static int get_mirrored_angle(int laser_angle, int mirror_angle)
{
  int reflected_angle = 16 - laser_angle + mirror_angle;

  /* make sure "reflected_angle" is in valid interval [0, 15] */
  return (reflected_angle + 16) % 16;
}

static void DrawLaserLines(struct XY *points, int num_points, int mode)
{
  Pixel pixel_drawto = (mode == DL_LASER_ENABLED ? pen_ray     : pen_bg);
  Pixel pixel_buffer = (mode == DL_LASER_ENABLED ? WHITE_PIXEL : BLACK_PIXEL);

  DrawLines(drawto, points, num_points, pixel_drawto);

  BEGIN_NO_HEADLESS
  {
    DrawLines(laser_bitmap, points, num_points, pixel_buffer);
  }
  END_NO_HEADLESS
}

static boolean CheckLaserPixel(int x, int y)
{
  Pixel pixel;

  BEGIN_NO_HEADLESS
  {
    pixel = ReadPixel(laser_bitmap, x, y);
  }
  END_NO_HEADLESS

  return (pixel == WHITE_PIXEL);
}

static void CheckExitMM()
{
  int exit_element = EL_EMPTY;
  int exit_x = 0;
  int exit_y = 0;
  int x, y;
  static int xy[4][2] =
  {
    { +1,  0 },
    {  0, -1 },
    { -1,  0 },
    {  0, +1 }
  };

  for (y = 0; y < lev_fieldy; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
    {
      if (Feld[x][y] == EL_EXIT_CLOSED)
      {
	/* initiate opening animation of exit door */
	Feld[x][y] = EL_EXIT_OPENING;

	exit_element = EL_EXIT_OPEN;
	exit_x = x;
	exit_y = y;
      }
      else if (IS_RECEIVER(Feld[x][y]))
      {
	/* remove field that blocks receiver */
	int phase = Feld[x][y] - EL_RECEIVER_START;
	int blocking_x, blocking_y;

	blocking_x = x + xy[phase][0];
	blocking_y = y + xy[phase][1];

	if (IN_LEV_FIELD(blocking_x, blocking_y))
	{
	  Feld[blocking_x][blocking_y] = EL_EMPTY;

	  DrawField_MM(blocking_x, blocking_y);
	}

	exit_element = EL_RECEIVER;
	exit_x = x;
	exit_y = y;
      }
    }
  }

  if (exit_element != EL_EMPTY)
    PlayLevelSound_MM(exit_x, exit_y, exit_element, MM_ACTION_OPENING);
}

static void InitMovDir_MM(int x, int y)
{
  int element = Feld[x][y];
  static int direction[3][4] =
  {
    { MV_RIGHT, MV_UP,    MV_LEFT,  MV_DOWN },
    { MV_LEFT,  MV_DOWN,  MV_RIGHT, MV_UP   },
    { MV_LEFT,  MV_RIGHT, MV_UP,    MV_DOWN }
  };

  switch(element)
  {
    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
      Feld[x][y] = EL_PACMAN;
      MovDir[x][y] = direction[0][element - EL_PACMAN_RIGHT];
      break;

    default:
      break;
  }
}

static void InitField(int x, int y, boolean init_game)
{
  int element = Feld[x][y];

  switch (element)
  {
    case EL_DF_EMPTY:
      Feld[x][y] = EL_EMPTY;
      break;

    case EL_KETTLE:
    case EL_CELL:
      if (native_mm_level.auto_count_kettles)
	game_mm.kettles_still_needed++;
      break;

    case EL_LIGHTBULB_OFF:
      game_mm.lights_still_needed++;
      break;

    default:
      if (IS_MIRROR(element) ||
	  IS_BEAMER_OLD(element) ||
	  IS_BEAMER(element) ||
	  IS_POLAR(element) ||
	  IS_POLAR_CROSS(element) ||
	  IS_DF_MIRROR(element) ||
	  IS_DF_MIRROR_AUTO(element) ||
	  IS_GRID_STEEL_AUTO(element) ||
	  IS_GRID_WOOD_AUTO(element) ||
	  IS_FIBRE_OPTIC(element))
      {
	if (IS_BEAMER_OLD(element))
	{
	  Feld[x][y] = EL_BEAMER_BLUE_START + (element - EL_BEAMER_START);
	  element = Feld[x][y];
	}

	if (!IS_FIBRE_OPTIC(element))
	{
	  static int steps_grid_auto = 0;

	  if (game_mm.num_cycle == 0)	/* initialize cycle steps for grids */
	    steps_grid_auto = RND(16) * (RND(2) ? -1 : +1);

	  if (IS_GRID_STEEL_AUTO(element) ||
	      IS_GRID_WOOD_AUTO(element))
	    game_mm.cycle[game_mm.num_cycle].steps = steps_grid_auto;
	  else
	    game_mm.cycle[game_mm.num_cycle].steps = RND(16) * (RND(2) ? -1 : +1);

	  game_mm.cycle[game_mm.num_cycle].x = x;
	  game_mm.cycle[game_mm.num_cycle].y = y;
	  game_mm.num_cycle++;
	}

	if (IS_BEAMER(element) || IS_FIBRE_OPTIC(element))
	{
	  int beamer_nr = BEAMER_NR(element);
	  int nr = laser.beamer[beamer_nr][0].num;

	  laser.beamer[beamer_nr][nr].x = x;
	  laser.beamer[beamer_nr][nr].y = y;
	  laser.beamer[beamer_nr][nr].num = 1;
	}
      }
      else if (IS_PACMAN(element))
      {
	InitMovDir_MM(x, y);
      }
      else if (IS_MCDUFFIN(element) || IS_LASER(element))
      {
	laser.start_edge.x = x;
	laser.start_edge.y = y;
	laser.start_angle = get_element_angle(element);
      }

      break;
  }
}

static void InitCycleElements_RotateSingleStep()
{
  int i;

  if (game_mm.num_cycle == 0)	/* no elements to cycle */
    return;

  for (i = 0; i < game_mm.num_cycle; i++)
  {
    int x = game_mm.cycle[i].x;
    int y = game_mm.cycle[i].y;
    int step = SIGN(game_mm.cycle[i].steps);
    int last_element = Feld[x][y];
    int next_element = get_rotated_element(last_element, step);

    if (!game_mm.cycle[i].steps)
      continue;

    Feld[x][y] = next_element;

    DrawField_MM(x, y);
    game_mm.cycle[i].steps -= step;
  }
}

static void InitLaser()
{
  int start_element = Feld[laser.start_edge.x][laser.start_edge.y];
  int step = (IS_LASER(start_element) ? 4 : 0);

  LX = laser.start_edge.x * TILEX;
  if (laser.start_angle == ANG_RAY_UP || laser.start_angle == ANG_RAY_DOWN)
    LX += 14;
  else
    LX += (laser.start_angle == ANG_RAY_RIGHT ? 28 + step : 0 - step);

  LY = laser.start_edge.y * TILEY;
  if (laser.start_angle == ANG_RAY_UP || laser.start_angle == ANG_RAY_DOWN)
    LY += (laser.start_angle == ANG_RAY_DOWN ? 28 + step : 0 - step);
  else
    LY += 14;

  XS = 2 * Step[laser.start_angle].x;
  YS = 2 * Step[laser.start_angle].y;

  laser.current_angle = laser.start_angle;

  laser.num_damages = 0;
  laser.num_edges = 0;
  laser.num_beamers = 0;
  laser.beamer_edge[0] = 0;

  laser.dest_element = EL_EMPTY;
  laser.wall_mask = 0;

  AddLaserEdge(LX, LY);		/* set laser starting edge */

  pen_ray = GetPixelFromRGB(window,
			    native_mm_level.laser_red   * 0xFF,
			    native_mm_level.laser_green * 0xFF,
			    native_mm_level.laser_blue  * 0xFF);
}

void InitGameEngine_MM()
{
  int i, x, y;

  BEGIN_NO_HEADLESS
  {
    /* initialize laser bitmap to current playfield (screen) size */
    ReCreateBitmap(&laser_bitmap, drawto->width, drawto->height);
    ClearRectangle(laser_bitmap, 0, 0, drawto->width, drawto->height);
  }
  END_NO_HEADLESS

  /* set global game control values */
  game_mm.num_cycle = 0;
  game_mm.num_pacman = 0;

  game_mm.score = 0;
  game_mm.energy_left = 0;	// later set to "native_mm_level.time"
  game_mm.kettles_still_needed =
    (native_mm_level.auto_count_kettles ? 0 : native_mm_level.kettles_needed);
  game_mm.lights_still_needed = 0;
  game_mm.num_keys = 0;

  game_mm.level_solved = FALSE;
  game_mm.game_over = FALSE;
  game_mm.game_over_cause = 0;

  game_mm.laser_overload_value = 0;
  game_mm.laser_enabled = FALSE;

  /* set global laser control values (must be set before "InitLaser()") */
  laser.start_edge.x = 0;
  laser.start_edge.y = 0;
  laser.start_angle = 0;

  for (i = 0; i < MAX_NUM_BEAMERS; i++)
    laser.beamer[i][0].num = laser.beamer[i][1].num = 0;

  laser.overloaded = FALSE;
  laser.overload_value = 0;
  laser.fuse_off = FALSE;
  laser.fuse_x = laser.fuse_y = -1;

  laser.dest_element = EL_EMPTY;
  laser.wall_mask = 0;

  last_LX = 0;
  last_LY = 0;
  last_hit_mask = 0;

  hold_x = -1;
  hold_y = -1;

  pacman_nr = -1;

  CT = Ct = 0;

  rotate_delay = 0;
  pacman_delay = 0;
  energy_delay = 0;
  overload_delay = 0;

  ClickElement(-1, -1, -1);

  for (x = 0; x < lev_fieldx; x++)
  {
    for (y = 0; y < lev_fieldy; y++)
    {
      Feld[x][y] = Ur[x][y];
      Hit[x][y] = Box[x][y] = 0;
      Angle[x][y] = 0;
      MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
      Store[x][y] = Store2[x][y] = 0;
      Frame[x][y] = 0;
      Stop[x][y] = FALSE;

      InitField(x, y, TRUE);
    }
  }

#if 0
  CloseDoor(DOOR_CLOSE_1);
#endif

  DrawLevel_MM();
}

void InitGameActions_MM()
{
  int num_init_game_frames = INIT_GAME_ACTIONS_DELAY;
  int cycle_steps_done = 0;
  int i;

  InitLaser();

#if 0
  /* copy default game door content to main double buffer */
  BlitBitmap(pix[PIX_DOOR], drawto,
	     DOOR_GFX_PAGEX5, DOOR_GFX_PAGEY1, DXSIZE, DYSIZE, DX, DY);
#endif

#if 0
  DrawText(DX_LEVEL, DY_LEVEL,
	   int2str(level_nr, 2), FONT_TEXT_2);
  DrawText(DX_KETTLES, DY_KETTLES,
	   int2str(game_mm.kettles_still_needed, 3), FONT_TEXT_2);
  DrawText(DX_SCORE, DY_SCORE,
	   int2str(game_mm.score, 4), FONT_TEXT_2);
#endif

#if 0
  UnmapGameButtons();
  MapGameButtons();
#endif

#if 0
  /* copy actual game door content to door double buffer for OpenDoor() */
  BlitBitmap(drawto, pix[PIX_DB_DOOR],
	     DX, DY, DXSIZE, DYSIZE, DOOR_GFX_PAGEX1, DOOR_GFX_PAGEY1);
#endif

#if 0
  OpenDoor(DOOR_OPEN_ALL);
#endif

  for (i = 0; i <= num_init_game_frames; i++)
  {
    if (i == num_init_game_frames)
      StopSound_MM(SND_MM_GAME_LEVELTIME_CHARGING);
    else if (setup.sound_loops)
      PlaySoundLoop_MM(SND_MM_GAME_LEVELTIME_CHARGING);
    else
      PlaySound_MM(SND_MM_GAME_LEVELTIME_CHARGING);

    game_mm.energy_left = native_mm_level.time * i / num_init_game_frames;

    UpdateAndDisplayGameControlValues();

    while (cycle_steps_done < NUM_INIT_CYCLE_STEPS * i / num_init_game_frames)
    {
      InitCycleElements_RotateSingleStep();

      cycle_steps_done++;
    }

    BackToFront();

    ColorCycling();

#ifdef DEBUG
    if (setup.quick_doors)
      continue;
#endif
  }

#if 0
  if (setup.sound_music && num_bg_loops)
    PlayMusic(level_nr % num_bg_loops);
#endif

  ScanLaser();

  if (game_mm.kettles_still_needed == 0)
    CheckExitMM();

  SetTileCursorXY(laser.start_edge.x, laser.start_edge.y);
  SetTileCursorActive(TRUE);
}

void AddLaserEdge(int lx, int ly)
{
  int clx = dSX + lx;
  int cly = dSY + ly;

  if (clx < -2 || cly < -2 || clx >= SXSIZE + 2 || cly >= SYSIZE + 2)
  {
    Error(ERR_WARN, "AddLaserEdge: out of bounds: %d, %d", lx, ly);

    return;
  }

  laser.edge[laser.num_edges].x = cSX2 + lx;
  laser.edge[laser.num_edges].y = cSY2 + ly;
  laser.num_edges++;

  laser.redraw = TRUE;
}

void AddDamagedField(int ex, int ey)
{
  laser.damage[laser.num_damages].is_mirror = FALSE;
  laser.damage[laser.num_damages].angle = laser.current_angle;
  laser.damage[laser.num_damages].edge = laser.num_edges;
  laser.damage[laser.num_damages].x = ex;
  laser.damage[laser.num_damages].y = ey;
  laser.num_damages++;
}

boolean StepBehind()
{
  if (laser.num_edges)
  {
    int x = LX - XS;
    int y = LY - YS;
    int last_x = laser.edge[laser.num_edges - 1].x - cSX2;
    int last_y = laser.edge[laser.num_edges - 1].y - cSY2;

    return ((x - last_x) * XS < 0 || (y - last_y) * YS < 0);
  }

  return FALSE;
}

static int getMaskFromElement(int element)
{
  if (IS_GRID(element))
    return IMG_MM_MASK_GRID_1 + get_element_phase(element);
  else if (IS_MCDUFFIN(element))
    return IMG_MM_MASK_MCDUFFIN_RIGHT + get_element_phase(element);
  else if (IS_RECTANGLE(element) || IS_DF_GRID(element))
    return IMG_MM_MASK_RECTANGLE;
  else
    return IMG_MM_MASK_CIRCLE;
}

int ScanPixel()
{
  int hit_mask = 0;

#if 0
  printf("ScanPixel: start scanning at (%d, %d) [%d, %d] [%d, %d]\n",
	 LX, LY, LX / TILEX, LY / TILEY, LX % TILEX, LY % TILEY);
#endif

  /* follow laser beam until it hits something (at least the screen border) */
  while (hit_mask == HIT_MASK_NO_HIT)
  {
    int i;

#if 0
    /* for safety */
    if (SX + LX < REAL_SX || SX + LX >= REAL_SX + FULL_SXSIZE ||
	SY + LY < REAL_SY || SY + LY >= REAL_SY + FULL_SYSIZE)
    {
      printf("ScanPixel: touched screen border!\n");

      return HIT_MASK_ALL;
    }
#endif

    for (i = 0; i < 4; i++)
    {
      int px = LX + (i % 2) * 2;
      int py = LY + (i / 2) * 2;
      int dx = px % TILEX;
      int dy = py % TILEY;
      int lx = (px + TILEX) / TILEX - 1;  /* ...+TILEX...-1 to get correct */
      int ly = (py + TILEY) / TILEY - 1;  /* negative values!              */
      Pixel pixel;

      if (IN_LEV_FIELD(lx, ly))
      {
	int element = Feld[lx][ly];

	if (element == EL_EMPTY || element == EL_EXPLODING_TRANSP)
	{
	  pixel = 0;
	}
	else if (IS_WALL(element) || IS_WALL_CHANGING(element))
	{
	  int pos = dy / MINI_TILEY * 2 + dx / MINI_TILEX;

	  pixel = ((element & (1 << pos)) ? 1 : 0);
	}
	else
	{
	  int pos = getMaskFromElement(element) - IMG_MM_MASK_MCDUFFIN_RIGHT;

	  pixel = (mm_masks[pos][dy / 2][dx / 2] == 'X' ? 1 : 0);
	}
      }
      else
      {
	pixel = (cSX + px < REAL_SX || cSX + px >= REAL_SX + FULL_SXSIZE ||
		 cSY + py < REAL_SY || cSY + py >= REAL_SY + FULL_SYSIZE);
      }

      if ((Sign[laser.current_angle] & (1 << i)) && pixel)
	hit_mask |= (1 << i);
    }

    if (hit_mask == HIT_MASK_NO_HIT)
    {
      /* hit nothing -- go on with another step */
      LX += XS;
      LY += YS;
    }
  }

  return hit_mask;
}

void ScanLaser()
{
  int element;
  int end = 0, rf = laser.num_edges;

  /* do not scan laser again after the game was lost for whatever reason */
  if (game_mm.game_over)
    return;

  laser.overloaded = FALSE;
  laser.stops_inside_element = FALSE;

  DrawLaser(0, DL_LASER_ENABLED);

#if 0
  printf("Start scanning with LX == %d, LY == %d, XS == %d, YS == %d\n",
	 LX, LY, XS, YS);
#endif

  while (1)
  {
    int hit_mask;

    if (laser.num_edges > MAX_LASER_LEN || laser.num_damages > MAX_LASER_LEN)
    {
      end = 1;
      laser.overloaded = TRUE;

      break;
    }

    hit_mask = ScanPixel();

#if 0
    printf("Hit something at LX == %d, LY == %d, XS == %d, YS == %d\n",
	   LX, LY, XS, YS);
#endif

    /* hit something -- check out what it was */
    ELX = (LX + XS) / TILEX;
    ELY = (LY + YS) / TILEY;

#if 0
    printf("hit_mask (1) == '%x' (%d, %d) (%d, %d)\n",
	   hit_mask, LX, LY, ELX, ELY);
#endif

    if (!IN_LEV_FIELD(ELX, ELY) || !IN_PIX_FIELD(LX, LY))
    {
      element = EL_EMPTY;
      laser.dest_element = element;

      break;
    }

    if (hit_mask == (HIT_MASK_TOPRIGHT | HIT_MASK_BOTTOMLEFT))
    {
      /* we have hit the top-right and bottom-left element --
	 choose the bottom-left one */
      /* !!! THIS CAN BE DONE MORE INTELLIGENTLY, FOR EXAMPLE, IF ONE
	 ELEMENT WAS STEEL AND THE OTHER ONE WAS ICE => ALWAYS CHOOSE
	 THE ICE AND MELT IT AWAY INSTEAD OF OVERLOADING LASER !!! */
      ELX = (LX - 2) / TILEX;
      ELY = (LY + 2) / TILEY;
    }

    if (hit_mask == (HIT_MASK_TOPLEFT | HIT_MASK_BOTTOMRIGHT))
    {
      /* we have hit the top-left and bottom-right element --
	 choose the top-left one */
      /* !!! SEE ABOVE !!! */
      ELX = (LX - 2) / TILEX;
      ELY = (LY - 2) / TILEY;
    }

#if 0
    printf("hit_mask (2) == '%x' (%d, %d) (%d, %d)\n",
	   hit_mask, LX, LY, ELX, ELY);
#endif

    element = Feld[ELX][ELY];
    laser.dest_element = element;

#if 0
    printf("Hit element %d at (%d, %d) [%d, %d] [%d, %d] [%d]\n",
	   element, ELX, ELY,
	   LX, LY,
	   LX % TILEX, LY % TILEY,
	   hit_mask);
#endif

#if 0
    if (!IN_LEV_FIELD(ELX, ELY))
      printf("WARNING! (1) %d, %d (%d)\n", ELX, ELY, element);
#endif

    if (element == EL_EMPTY)
    {
      if (!HitOnlyAnEdge(element, hit_mask))
	break;
    }
    else if (element == EL_FUSE_ON)
    {
      if (HitPolarizer(element, hit_mask))
	break;
    }
    else if (IS_GRID(element) || IS_DF_GRID(element))
    {
      if (HitPolarizer(element, hit_mask))
	break;
    }
    else if (element == EL_BLOCK_STONE || element == EL_BLOCK_WOOD ||
	     element == EL_GATE_STONE || element == EL_GATE_WOOD)
    {
      if (HitBlock(element, hit_mask))
      {
	rf = 1;

	break;
      }
    }
    else if (IS_MCDUFFIN(element))
    {
      if (HitLaserSource(element, hit_mask))
	break;
    }
    else if ((element >= EL_EXIT_CLOSED && element <= EL_EXIT_OPEN) ||
	     IS_RECEIVER(element))
    {
      if (HitLaserDestination(element, hit_mask))
	break;
    }
    else if (IS_WALL(element))
    {
      if (IS_WALL_STEEL(element) || IS_DF_WALL_STEEL(element))
      {
	if (HitReflectingWalls(element, hit_mask))
	  break;
      }
      else
      {
	if (HitAbsorbingWalls(element, hit_mask))
	  break;
      }
    }
    else
    {
      if (HitElement(element, hit_mask))
	break;
    }

    if (rf)
      DrawLaser(rf - 1, DL_LASER_ENABLED);
    rf = laser.num_edges;
  }

#if 0
  if (laser.dest_element != Feld[ELX][ELY])
  {
    printf("ALARM: laser.dest_element == %d, Feld[ELX][ELY] == %d\n",
	   laser.dest_element, Feld[ELX][ELY]);
  }
#endif

  if (!end && !laser.stops_inside_element && !StepBehind())
  {
#if 0
    printf("ScanLaser: Go one step back\n");
#endif

    LX -= XS;
    LY -= YS;

    AddLaserEdge(LX, LY);
  }

  if (rf)
    DrawLaser(rf - 1, DL_LASER_ENABLED);

  Ct = CT = FrameCounter;

#if 0
    if (!IN_LEV_FIELD(ELX, ELY))
      printf("WARNING! (2) %d, %d\n", ELX, ELY);
#endif
}

void DrawLaserExt(int start_edge, int num_edges, int mode)
{
  int element;
  int elx, ely;

#if 0
  printf("DrawLaserExt: start_edge, num_edges, mode == %d, %d, %d\n",
	 start_edge, num_edges, mode);
#endif

  if (start_edge < 0)
  {
    Error(ERR_WARN, "DrawLaserExt: start_edge < 0");

    return;
  }

  if (num_edges < 0)
  {
    Error(ERR_WARN, "DrawLaserExt: num_edges < 0");

    return;
  }

#if 0
  if (mode == DL_LASER_DISABLED)
  {
    printf("DrawLaser: Delete laser from edge %d\n", start_edge);
  }
#endif

  /* now draw the laser to the backbuffer and (if enabled) to the screen */
  DrawLaserLines(&laser.edge[start_edge], num_edges, mode);

  redraw_mask |= REDRAW_FIELD;

  if (mode == DL_LASER_ENABLED)
    return;

  /* after the laser was deleted, the "damaged" graphics must be restored */
  if (laser.num_damages)
  {
    int damage_start = 0;
    int i;

    /* determine the starting edge, from which graphics need to be restored */
    if (start_edge > 0)
    {
      for (i = 0; i < laser.num_damages; i++)
      {
	if (laser.damage[i].edge == start_edge + 1)
	{
	  damage_start = i;

	  break;
	}
      }
    }

    /* restore graphics from this starting edge to the end of damage list */
    for (i = damage_start; i < laser.num_damages; i++)
    {
      int lx = laser.damage[i].x;
      int ly = laser.damage[i].y;
      int element = Feld[lx][ly];

      if (Hit[lx][ly] == laser.damage[i].edge)
	if (!((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
	       i == damage_start))
	  Hit[lx][ly] = 0;
      if (Box[lx][ly] == laser.damage[i].edge)
	Box[lx][ly] = 0;

      if (IS_DRAWABLE(element))
	DrawField_MM(lx, ly);
    }

    elx = laser.damage[damage_start].x;
    ely = laser.damage[damage_start].y;
    element = Feld[elx][ely];

#if 0
    if (IS_BEAMER(element))
    {
      int i;

      for (i = 0; i < laser.num_beamers; i++)
	printf("-> %d\n", laser.beamer_edge[i]);
      printf("DrawLaserExt: IS_BEAMER: [%d]: Hit[%d][%d] == %d [%d]\n",
	     mode, elx, ely, Hit[elx][ely], start_edge);
      printf("DrawLaserExt: IS_BEAMER: %d / %d\n",
	     get_element_angle(element), laser.damage[damage_start].angle);
    }
#endif

    if ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
	laser.num_beamers > 0 &&
	start_edge == laser.beamer_edge[laser.num_beamers - 1])
    {
      /* element is outgoing beamer */
      laser.num_damages = damage_start + 1;

      if (IS_BEAMER(element))
	laser.current_angle = get_element_angle(element);
    }
    else
    {
      /* element is incoming beamer or other element */
      laser.num_damages = damage_start;
      laser.current_angle = laser.damage[laser.num_damages].angle;
    }
  }
  else
  {
    /* no damages but McDuffin himself (who needs to be redrawn anyway) */

    elx = laser.start_edge.x;
    ely = laser.start_edge.y;
    element = Feld[elx][ely];
  }

  laser.num_edges = start_edge + 1;
  if (start_edge == 0)
    laser.current_angle = laser.start_angle;

  LX = laser.edge[start_edge].x - cSX2;
  LY = laser.edge[start_edge].y - cSY2;
  XS = 2 * Step[laser.current_angle].x;
  YS = 2 * Step[laser.current_angle].y;

#if 0
  printf("DrawLaser: Set (LX, LY) to (%d, %d) [%d]\n",
	 LX, LY, element);
#endif

  if (start_edge > 0)
  {
    if (IS_BEAMER(element) ||
	IS_FIBRE_OPTIC(element) ||
	IS_PACMAN(element) ||
	IS_POLAR(element) ||
	IS_POLAR_CROSS(element) ||
	element == EL_FUSE_ON)
    {
      int step_size;

#if 0
      printf("element == %d\n", element);
#endif

      if (IS_22_5_ANGLE(laser.current_angle))	/* neither 90° nor 45° angle */
	step_size = ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) ? 4 : 3);
      else
	step_size = 8;

      if (IS_POLAR(element) || IS_POLAR_CROSS(element) ||
	  ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
	   (laser.num_beamers == 0 ||
	    start_edge != laser.beamer_edge[laser.num_beamers - 1])))
      {
	/* element is incoming beamer or other element */
	step_size = -step_size;
	laser.num_edges--;
      }

#if 0
      if (IS_BEAMER(element))
      {
	printf("start_edge == %d, laser.beamer_edge == %d\n",
	       start_edge, laser.beamer_edge);
      }
#endif

      LX += step_size * XS;
      LY += step_size * YS;
    }
    else if (element != EL_EMPTY)
    {
      LX -= 3 * XS;
      LY -= 3 * YS;
      laser.num_edges--;
    }
  }

#if 0
  printf("DrawLaser: Finally: (LX, LY) to (%d, %d) [%d]\n",
	 LX, LY, element);
#endif
}

void DrawLaser(int start_edge, int mode)
{
  if (laser.num_edges - start_edge < 0)
  {
    Error(ERR_WARN, "DrawLaser: laser.num_edges - start_edge < 0");

    return;
  }

  /* check if laser is interrupted by beamer element */
  if (laser.num_beamers > 0 &&
      start_edge < laser.beamer_edge[laser.num_beamers - 1])
  {
    if (mode == DL_LASER_ENABLED)
    {
      int i;
      int tmp_start_edge = start_edge;

      /* draw laser segments forward from the start to the last beamer */
      for (i = 0; i < laser.num_beamers; i++)
      {
	int tmp_num_edges = laser.beamer_edge[i] - tmp_start_edge;

	if (tmp_num_edges <= 0)
	  continue;

#if 0
	printf("DrawLaser: DL_LASER_ENABLED: i==%d: %d, %d\n",
	       i, laser.beamer_edge[i], tmp_start_edge);
#endif

	DrawLaserExt(tmp_start_edge, tmp_num_edges, DL_LASER_ENABLED);

	tmp_start_edge = laser.beamer_edge[i];
      }

      /* draw last segment from last beamer to the end */
      DrawLaserExt(tmp_start_edge, laser.num_edges - tmp_start_edge,
		   DL_LASER_ENABLED);
    }
    else
    {
      int i;
      int last_num_edges = laser.num_edges;
      int num_beamers = laser.num_beamers;

      /* delete laser segments backward from the end to the first beamer */
      for (i = num_beamers - 1; i >= 0; i--)
      {
	int tmp_num_edges = last_num_edges - laser.beamer_edge[i];

	if (laser.beamer_edge[i] - start_edge <= 0)
	  break;

	DrawLaserExt(laser.beamer_edge[i], tmp_num_edges, DL_LASER_DISABLED);

	last_num_edges = laser.beamer_edge[i];
	laser.num_beamers--;
      }

#if 0
      if (last_num_edges - start_edge <= 0)
	printf("DrawLaser: DL_LASER_DISABLED: %d, %d\n",
	       last_num_edges, start_edge);
#endif

      // special case when rotating first beamer: delete laser edge on beamer
      // (but do not start scanning on previous edge to prevent mirror sound)
      if (last_num_edges - start_edge == 1 && start_edge > 0)
	DrawLaserLines(&laser.edge[start_edge - 1], 2, DL_LASER_DISABLED);

      /* delete first segment from start to the first beamer */
      DrawLaserExt(start_edge, last_num_edges - start_edge, DL_LASER_DISABLED);
    }
  }
  else
  {
    DrawLaserExt(start_edge, laser.num_edges - start_edge, mode);
  }

  game_mm.laser_enabled = mode;
}

void DrawLaser_MM()
{
  DrawLaser(0, game_mm.laser_enabled);
}

boolean HitElement(int element, int hit_mask)
{
  if (HitOnlyAnEdge(element, hit_mask))
    return FALSE;

  if (IS_MOVING(ELX, ELY) || IS_BLOCKED(ELX, ELY))
    element = MovingOrBlocked2Element_MM(ELX, ELY);

#if 0
  printf("HitElement (1): element == %d\n", element);
#endif

#if 0
  if ((ELX * TILEX + 14 - LX) * YS == (ELY * TILEY + 14 - LY) * XS)
    printf("HitElement (%d): EXACT MATCH @ (%d, %d)\n", element, ELX, ELY);
  else
    printf("HitElement (%d): FUZZY MATCH @ (%d, %d)\n", element, ELX, ELY);
#endif

  AddDamagedField(ELX, ELY);

  /* this is more precise: check if laser would go through the center */
  if ((ELX * TILEX + 14 - LX) * YS != (ELY * TILEY + 14 - LY) * XS)
  {
    /* skip the whole element before continuing the scan */
    do
    {
      LX += XS;
      LY += YS;
    }
    while (ELX == LX/TILEX && ELY == LY/TILEY && LX > 0 && LY > 0);

    if (LX/TILEX > ELX || LY/TILEY > ELY)
    {
      /* skipping scan positions to the right and down skips one scan
	 position too much, because this is only the top left scan position
	 of totally four scan positions (plus one to the right, one to the
	 bottom and one to the bottom right) */

      LX -= XS;
      LY -= YS;
    }

    return FALSE;
  }

#if 0
  printf("HitElement (2): element == %d\n", element);
#endif

  if (LX + 5 * XS < 0 ||
      LY + 5 * YS < 0)
  {
    LX += 2 * XS;
    LY += 2 * YS;

    return FALSE;
  }

#if 0
  printf("HitElement (3): element == %d\n", element);
#endif

  if (IS_POLAR(element) &&
      ((element - EL_POLAR_START) % 2 ||
       (element - EL_POLAR_START) / 2 != laser.current_angle % 8))
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    laser.num_damages--;

    return TRUE;
  }

  if (IS_POLAR_CROSS(element) &&
      (element - EL_POLAR_CROSS_START) != laser.current_angle % 4)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    laser.num_damages--;

    return TRUE;
  }

  if (!IS_BEAMER(element) &&
      !IS_FIBRE_OPTIC(element) &&
      !IS_GRID_WOOD(element) &&
      element != EL_FUEL_EMPTY)
  {
#if 0
    if ((ELX * TILEX + 14 - LX) * YS == (ELY * TILEY + 14 - LY) * XS)
      printf("EXACT MATCH @ (%d, %d)\n", ELX, ELY);
    else
      printf("FUZZY MATCH @ (%d, %d)\n", ELX, ELY);
#endif

    LX = ELX * TILEX + 14;
    LY = ELY * TILEY + 14;

    AddLaserEdge(LX, LY);
  }

  if (IS_MIRROR(element) ||
      IS_MIRROR_FIXED(element) ||
      IS_POLAR(element) ||
      IS_POLAR_CROSS(element) ||
      IS_DF_MIRROR(element) ||
      IS_DF_MIRROR_AUTO(element) ||
      element == EL_PRISM ||
      element == EL_REFRACTOR)
  {
    int current_angle = laser.current_angle;
    int step_size;

    laser.num_damages--;

    AddDamagedField(ELX, ELY);

    laser.damage[laser.num_damages - 1].is_mirror = TRUE;

    if (!Hit[ELX][ELY])
      Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

    if (IS_MIRROR(element) ||
	IS_MIRROR_FIXED(element) ||
	IS_DF_MIRROR(element) ||
	IS_DF_MIRROR_AUTO(element))
      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       get_element_angle(element));

    if (element == EL_PRISM || element == EL_REFRACTOR)
      laser.current_angle = RND(16);

    XS = 2 * Step[laser.current_angle].x;
    YS = 2 * Step[laser.current_angle].y;

    if (!IS_22_5_ANGLE(laser.current_angle))	/* 90° or 45° angle */
      step_size = 8;
    else
      step_size = 4;

    LX += step_size * XS;
    LY += step_size * YS;

#if 0
    /* draw sparkles on mirror */
    if ((IS_MIRROR(element) || IS_MIRROR_FIXED(element)) &&
	current_angle != laser.current_angle)
    {
      MoveSprite(vp, &Pfeil[2], 4 + 16 * ELX, 5 + 16 * ELY + 1);
    }
#endif

    if ((!IS_POLAR(element) && !IS_POLAR_CROSS(element)) &&
	current_angle != laser.current_angle)
      PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    laser.overloaded =
      (get_opposite_angle(laser.current_angle) ==
       laser.damage[laser.num_damages - 1].angle ? TRUE : FALSE);

    return (laser.overloaded ? TRUE : FALSE);
  }

  if (element == EL_FUEL_FULL)
  {
    laser.stops_inside_element = TRUE;

    return TRUE;
  }

  if (element == EL_BOMB || element == EL_MINE)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    if (element == EL_MINE)
      laser.overloaded = TRUE;
  }

  if (element == EL_KETTLE ||
      element == EL_CELL ||
      element == EL_KEY ||
      element == EL_LIGHTBALL ||
      element == EL_PACMAN ||
      IS_PACMAN(element))
  {
    if (!IS_PACMAN(element))
      Bang_MM(ELX, ELY);

    if (element == EL_PACMAN)
      Bang_MM(ELX, ELY);

    if (element == EL_KETTLE || element == EL_CELL)
    {
      if (game_mm.kettles_still_needed > 0)
	game_mm.kettles_still_needed--;

      game.snapshot.collected_item = TRUE;

      if (game_mm.kettles_still_needed == 0)
      {
	CheckExitMM();

	DrawLaser(0, DL_LASER_ENABLED);
      }
    }
    else if (element == EL_KEY)
    {
      game_mm.num_keys++;
    }
    else if (IS_PACMAN(element))
    {
      DeletePacMan(ELX, ELY);
    }

    RaiseScoreElement_MM(element);

    return FALSE;
  }

  if (element == EL_LIGHTBULB_OFF || element == EL_LIGHTBULB_ON)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    DrawLaser(0, DL_LASER_ENABLED);

    if (Feld[ELX][ELY] == EL_LIGHTBULB_OFF)
    {
      Feld[ELX][ELY] = EL_LIGHTBULB_ON;
      game_mm.lights_still_needed--;
    }
    else
    {
      Feld[ELX][ELY] = EL_LIGHTBULB_OFF;
      game_mm.lights_still_needed++;
    }

    DrawField_MM(ELX, ELY);
    DrawLaser(0, DL_LASER_ENABLED);

    /*
    BackToFront();
    */
    laser.stops_inside_element = TRUE;

    return TRUE;
  }

#if 0
  printf("HitElement (4): element == %d\n", element);
#endif

  if ((IS_BEAMER(element) || IS_FIBRE_OPTIC(element)) &&
      laser.num_beamers < MAX_NUM_BEAMERS &&
      laser.beamer[BEAMER_NR(element)][1].num)
  {
    int beamer_angle = get_element_angle(element);
    int beamer_nr = BEAMER_NR(element);
    int step_size;

#if 0
  printf("HitElement (BEAMER): element == %d\n", element);
#endif

    laser.num_damages--;

    if (IS_FIBRE_OPTIC(element) ||
	laser.current_angle == get_opposite_angle(beamer_angle))
    {
      int pos;

      LX = ELX * TILEX + 14;
      LY = ELY * TILEY + 14;

      AddLaserEdge(LX, LY);
      AddDamagedField(ELX, ELY);

      laser.damage[laser.num_damages - 1].is_mirror = TRUE;

      if (!Hit[ELX][ELY])
	Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

      pos = (ELX == laser.beamer[beamer_nr][0].x &&
	     ELY == laser.beamer[beamer_nr][0].y ? 1 : 0);
      ELX = laser.beamer[beamer_nr][pos].x;
      ELY = laser.beamer[beamer_nr][pos].y;
      LX = ELX * TILEX + 14;
      LY = ELY * TILEY + 14;

      if (IS_BEAMER(element))
      {
	laser.current_angle = get_element_angle(Feld[ELX][ELY]);
	XS = 2 * Step[laser.current_angle].x;
	YS = 2 * Step[laser.current_angle].y;
      }

      laser.beamer_edge[laser.num_beamers] = laser.num_edges;

      AddLaserEdge(LX, LY);
      AddDamagedField(ELX, ELY);

      laser.damage[laser.num_damages - 1].is_mirror = TRUE;

      if (!Hit[ELX][ELY])
	Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

      if (laser.current_angle == (laser.current_angle >> 1) << 1)
	step_size = 8;
      else
	step_size = 4;

      LX += step_size * XS;
      LY += step_size * YS;

      laser.num_beamers++;

      return FALSE;
    }
  }

  return TRUE;
}

boolean HitOnlyAnEdge(int element, int hit_mask)
{
  /* check if the laser hit only the edge of an element and, if so, go on */

#if 0
  printf("LX, LY, hit_mask == %d, %d, %d\n", LX, LY, hit_mask);
#endif

  if ((hit_mask == HIT_MASK_TOPLEFT ||
       hit_mask == HIT_MASK_TOPRIGHT ||
       hit_mask == HIT_MASK_BOTTOMLEFT ||
       hit_mask == HIT_MASK_BOTTOMRIGHT) &&
      laser.current_angle % 4)			/* angle is not 90° */
  {
    int dx, dy;

    if (hit_mask == HIT_MASK_TOPLEFT)
    {
      dx = -1;
      dy = -1;
    }
    else if (hit_mask == HIT_MASK_TOPRIGHT)
    {
      dx = +1;
      dy = -1;
    }
    else if (hit_mask == HIT_MASK_BOTTOMLEFT)
    {
      dx = -1;
      dy = +1;
    }
    else /* (hit_mask == HIT_MASK_BOTTOMRIGHT) */
    {
      dx = +1;
      dy = +1;
    }

    AddDamagedField((LX + 2 * dx) / TILEX, (LY + 2 * dy) / TILEY);

    LX += XS;
    LY += YS;

#if 0
    printf("[HitOnlyAnEdge() == TRUE]\n");
#endif

    return TRUE;
  }

#if 0
    printf("[HitOnlyAnEdge() == FALSE]\n");
#endif

  return FALSE;
}

boolean HitPolarizer(int element, int hit_mask)
{
  if (HitOnlyAnEdge(element, hit_mask))
    return FALSE;

  if (IS_DF_GRID(element))
  {
    int grid_angle = get_element_angle(element);

#if 0
    printf("HitPolarizer: angle: grid == %d, laser == %d\n",
	   grid_angle, laser.current_angle);
#endif

    AddLaserEdge(LX, LY);
    AddDamagedField(ELX, ELY);

    if (!Hit[ELX][ELY])
      Hit[ELX][ELY] = laser.damage[laser.num_damages - 1].edge;

    if (laser.current_angle == grid_angle ||
	laser.current_angle == get_opposite_angle(grid_angle))
    {
      /* skip the whole element before continuing the scan */
      do
      {
	LX += XS;
	LY += YS;
      }
      while (ELX == LX/TILEX && ELY == LY/TILEY && LX > 0 && LY > 0);

      if (LX/TILEX > ELX || LY/TILEY > ELY)
      {
	/* skipping scan positions to the right and down skips one scan
	   position too much, because this is only the top left scan position
	   of totally four scan positions (plus one to the right, one to the
	   bottom and one to the bottom right) */

	LX -= XS;
	LY -= YS;
      }

      AddLaserEdge(LX, LY);

      LX += XS;
      LY += YS;

#if 0
      printf("HitPolarizer: LX, LY == %d, %d [%d, %d] [%d, %d]\n",
	     LX, LY,
	     LX / TILEX, LY / TILEY,
	     LX % TILEX, LY % TILEY);
#endif

      return FALSE;
    }
    else if (IS_GRID_STEEL_FIXED(element) || IS_GRID_STEEL_AUTO(element))
    {
      return HitReflectingWalls(element, hit_mask);
    }
    else
    {
      return HitAbsorbingWalls(element, hit_mask);
    }
  }
  else if (IS_GRID_STEEL(element))
  {
    return HitReflectingWalls(element, hit_mask);
  }
  else	/* IS_GRID_WOOD */
  {
    return HitAbsorbingWalls(element, hit_mask);
  }

  return TRUE;
}

boolean HitBlock(int element, int hit_mask)
{
  boolean check = FALSE;

  if ((element == EL_GATE_STONE || element == EL_GATE_WOOD) &&
      game_mm.num_keys == 0)
    check = TRUE;

  if (element == EL_BLOCK_STONE || element == EL_BLOCK_WOOD)
  {
    int i, x, y;
    int ex = ELX * TILEX + 14;
    int ey = ELY * TILEY + 14;

    check = TRUE;

    for (i = 1; i < 32; i++)
    {
      x = LX + i * XS;
      y = LY + i * YS;

      if ((x == ex || x == ex + 1) && (y == ey || y == ey + 1))
	check = FALSE;
    }
  }

  if (check && (element == EL_BLOCK_WOOD || element == EL_GATE_WOOD))
    return HitAbsorbingWalls(element, hit_mask);

  if (check)
  {
    AddLaserEdge(LX - XS, LY - YS);
    AddDamagedField(ELX, ELY);

    if (!Box[ELX][ELY])
      Box[ELX][ELY] = laser.num_edges;

    return HitReflectingWalls(element, hit_mask);
  }

  if (element == EL_GATE_STONE || element == EL_GATE_WOOD)
  {
    int xs = XS / 2, ys = YS / 2;
    int hit_mask_diagonal1 = HIT_MASK_TOPRIGHT | HIT_MASK_BOTTOMLEFT;
    int hit_mask_diagonal2 = HIT_MASK_TOPLEFT | HIT_MASK_BOTTOMRIGHT;

    if ((hit_mask & hit_mask_diagonal1) == hit_mask_diagonal1 ||
	(hit_mask & hit_mask_diagonal2) == hit_mask_diagonal2)
    {
      laser.overloaded = (element == EL_GATE_STONE);

      return TRUE;
    }

    if (ABS(xs) == 1 && ABS(ys) == 1 &&
	(hit_mask == HIT_MASK_TOP ||
	 hit_mask == HIT_MASK_LEFT ||
	 hit_mask == HIT_MASK_RIGHT ||
	 hit_mask == HIT_MASK_BOTTOM))
      AddDamagedField(ELX - xs * (hit_mask == HIT_MASK_TOP ||
				  hit_mask == HIT_MASK_BOTTOM),
		      ELY - ys * (hit_mask == HIT_MASK_LEFT ||
				  hit_mask == HIT_MASK_RIGHT));
    AddLaserEdge(LX, LY);

    Bang_MM(ELX, ELY);

    game_mm.num_keys--;

    if (element == EL_GATE_STONE && Box[ELX][ELY])
    {
      DrawLaser(Box[ELX][ELY] - 1, DL_LASER_DISABLED);
      /*
      BackToFront();
      */
      ScanLaser();

      return TRUE;
    }

    return FALSE;
  }

  if (element == EL_BLOCK_STONE || element == EL_BLOCK_WOOD)
  {
    int xs = XS / 2, ys = YS / 2;
    int hit_mask_diagonal1 = HIT_MASK_TOPRIGHT | HIT_MASK_BOTTOMLEFT;
    int hit_mask_diagonal2 = HIT_MASK_TOPLEFT | HIT_MASK_BOTTOMRIGHT;

    if ((hit_mask & hit_mask_diagonal1) == hit_mask_diagonal1 ||
	(hit_mask & hit_mask_diagonal2) == hit_mask_diagonal2)
    {
      laser.overloaded = (element == EL_BLOCK_STONE);

      return TRUE;
    }

    if (ABS(xs) == 1 && ABS(ys) == 1 &&
	(hit_mask == HIT_MASK_TOP ||
	 hit_mask == HIT_MASK_LEFT ||
	 hit_mask == HIT_MASK_RIGHT ||
	 hit_mask == HIT_MASK_BOTTOM))
      AddDamagedField(ELX - xs * (hit_mask == HIT_MASK_TOP ||
				  hit_mask == HIT_MASK_BOTTOM),
		      ELY - ys * (hit_mask == HIT_MASK_LEFT ||
				  hit_mask == HIT_MASK_RIGHT));
    AddDamagedField(ELX, ELY);

    LX = ELX * TILEX + 14;
    LY = ELY * TILEY + 14;

    AddLaserEdge(LX, LY);

    laser.stops_inside_element = TRUE;

    return TRUE;
  }

  return TRUE;
}

boolean HitLaserSource(int element, int hit_mask)
{
  if (HitOnlyAnEdge(element, hit_mask))
    return FALSE;

  PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

  laser.overloaded = TRUE;

  return TRUE;
}

boolean HitLaserDestination(int element, int hit_mask)
{
  if (HitOnlyAnEdge(element, hit_mask))
    return FALSE;

  if (element != EL_EXIT_OPEN &&
      !(IS_RECEIVER(element) &&
	game_mm.kettles_still_needed == 0 &&
	laser.current_angle == get_opposite_angle(get_element_angle(element))))
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    return TRUE;
  }

  if (IS_RECEIVER(element) ||
      (IS_22_5_ANGLE(laser.current_angle) &&
       (ELX != (LX + 6 * XS) / TILEX ||
	ELY != (LY + 6 * YS) / TILEY ||
	LX + 6 * XS < 0 ||
	LY + 6 * YS < 0)))
  {
    LX -= XS;
    LY -= YS;
  }
  else
  {
    LX = ELX * TILEX + 14;
    LY = ELY * TILEY + 14;

    laser.stops_inside_element = TRUE;
  }

  AddLaserEdge(LX, LY);
  AddDamagedField(ELX, ELY);

  if (game_mm.lights_still_needed == 0)
  {
    game_mm.level_solved = TRUE;

    SetTileCursorActive(FALSE);
  }

  return TRUE;
}

boolean HitReflectingWalls(int element, int hit_mask)
{
  /* check if laser hits side of a wall with an angle that is not 90° */
  if (!IS_90_ANGLE(laser.current_angle) && (hit_mask == HIT_MASK_TOP ||
					    hit_mask == HIT_MASK_LEFT ||
					    hit_mask == HIT_MASK_RIGHT ||
					    hit_mask == HIT_MASK_BOTTOM))
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    LX -= XS;
    LY -= YS;

    if (!IS_DF_GRID(element))
      AddLaserEdge(LX, LY);

    /* check if laser hits wall with an angle of 45° */
    if (!IS_22_5_ANGLE(laser.current_angle))
    {
      if (hit_mask == HIT_MASK_TOP || hit_mask == HIT_MASK_BOTTOM)
      {
	LX += 2 * XS;
	laser.current_angle = get_mirrored_angle(laser.current_angle,
						 ANG_MIRROR_0);
      }
      else	/* hit_mask == HIT_MASK_LEFT || hit_mask == HIT_MASK_RIGHT */
      {
	LY += 2 * YS;
	laser.current_angle = get_mirrored_angle(laser.current_angle,
						 ANG_MIRROR_90);
      }

      AddLaserEdge(LX, LY);

      XS = 2 * Step[laser.current_angle].x;
      YS = 2 * Step[laser.current_angle].y;

      return FALSE;
    }
    else if (hit_mask == HIT_MASK_TOP || hit_mask == HIT_MASK_BOTTOM)
    {
      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       ANG_MIRROR_0);
      if (ABS(XS) == 4)
      {
	LX += 2 * XS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }
      else
      {
	LX += XS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY + YS / 2);

	LX += XS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }

      YS = 2 * Step[laser.current_angle].y;

      return FALSE;
    }
    else	/* hit_mask == HIT_MASK_LEFT || hit_mask == HIT_MASK_RIGHT */
    {
      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       ANG_MIRROR_90);
      if (ABS(YS) == 4)
      {
	LY += 2 * YS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }
      else
      {
	LY += YS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX + XS / 2, LY);

	LY += YS;
	if (!IS_DF_GRID(element))
	  AddLaserEdge(LX, LY);
      }

      XS = 2 * Step[laser.current_angle].x;

      return FALSE;
    }
  }

  /* reflection at the edge of reflecting DF style wall */
  if (IS_DF_WALL_STEEL(element) && IS_22_5_ANGLE(laser.current_angle))
  {
    if (((laser.current_angle == 1 || laser.current_angle == 3) &&
	 hit_mask == HIT_MASK_TOPRIGHT) ||
	((laser.current_angle == 5 || laser.current_angle == 7) &&
	 hit_mask == HIT_MASK_TOPLEFT) ||
	((laser.current_angle == 9 || laser.current_angle == 11) &&
	 hit_mask == HIT_MASK_BOTTOMLEFT) ||
	((laser.current_angle == 13 || laser.current_angle == 15) &&
	 hit_mask == HIT_MASK_BOTTOMRIGHT))
    {
      int mirror_angle =
	(hit_mask == HIT_MASK_TOPRIGHT || hit_mask == HIT_MASK_BOTTOMLEFT ?
	 ANG_MIRROR_135 : ANG_MIRROR_45);

      PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

      AddDamagedField(ELX, ELY);
      AddLaserEdge(LX, LY);

      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       mirror_angle);
      XS = 8 / -XS;
      YS = 8 / -YS;

      LX += XS;
      LY += YS;

      AddLaserEdge(LX, LY);

      return FALSE;
    }
  }

  /* reflection inside an edge of reflecting DF style wall */
  if (IS_DF_WALL_STEEL(element) && IS_22_5_ANGLE(laser.current_angle))
  {
    if (((laser.current_angle == 1 || laser.current_angle == 3) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_BOTTOMLEFT)) ||
	((laser.current_angle == 5 || laser.current_angle == 7) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_BOTTOMRIGHT)) ||
	((laser.current_angle == 9 || laser.current_angle == 11) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_TOPRIGHT)) ||
	((laser.current_angle == 13 || laser.current_angle == 15) &&
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_TOPLEFT)))
    {
      int mirror_angle =
	(hit_mask == (HIT_MASK_ALL ^ HIT_MASK_BOTTOMLEFT) ||
	 hit_mask == (HIT_MASK_ALL ^ HIT_MASK_TOPRIGHT) ?
	 ANG_MIRROR_135 : ANG_MIRROR_45);

      PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

      /*
      AddDamagedField(ELX, ELY);
      */

      AddLaserEdge(LX - XS, LY - YS);
      AddLaserEdge(LX - XS + (ABS(XS) == 4 ? XS/2 : 0),
		   LY - YS + (ABS(YS) == 4 ? YS/2 : 0));

      laser.current_angle = get_mirrored_angle(laser.current_angle,
					       mirror_angle);
      XS = 8 / -XS;
      YS = 8 / -YS;

      LX += XS;
      LY += YS;

      AddLaserEdge(LX, LY);

      return FALSE;
    }
  }

  /* check if laser hits DF style wall with an angle of 90° */
  if (IS_DF_WALL(element) && IS_90_ANGLE(laser.current_angle))
  {
    if ((IS_HORIZ_ANGLE(laser.current_angle) &&
	 (!(hit_mask & HIT_MASK_TOP) || !(hit_mask & HIT_MASK_BOTTOM))) ||
	(IS_VERT_ANGLE(laser.current_angle) &&
	 (!(hit_mask & HIT_MASK_LEFT) || !(hit_mask & HIT_MASK_RIGHT))))
    {
      /* laser at last step touched nothing or the same side of the wall */
      if (LX != last_LX || LY != last_LY || hit_mask == last_hit_mask)
      {
	AddDamagedField(ELX, ELY);

	LX += 8 * XS;
	LY += 8 * YS;

	last_LX = LX;
	last_LY = LY;
	last_hit_mask = hit_mask;

	return FALSE;
      }
    }
  }

  if (!HitOnlyAnEdge(element, hit_mask))
  {
    laser.overloaded = TRUE;

    return TRUE;
  }

  return FALSE;
}

boolean HitAbsorbingWalls(int element, int hit_mask)
{
  if (HitOnlyAnEdge(element, hit_mask))
    return FALSE;

  if (ABS(XS) == 4 &&
      (hit_mask == HIT_MASK_LEFT || hit_mask == HIT_MASK_RIGHT))
  {
    AddLaserEdge(LX - XS, LY - YS);

    LX = LX + XS / 2;
    LY = LY + YS;
  }

  if (ABS(YS) == 4 &&
      (hit_mask == HIT_MASK_TOP || hit_mask == HIT_MASK_BOTTOM))
  {
    AddLaserEdge(LX - XS, LY - YS);

    LX = LX + XS;
    LY = LY + YS / 2;
  }

  if (IS_WALL_WOOD(element) ||
      IS_DF_WALL_WOOD(element) ||
      IS_GRID_WOOD(element) ||
      IS_GRID_WOOD_FIXED(element) ||
      IS_GRID_WOOD_AUTO(element) ||
      element == EL_FUSE_ON ||
      element == EL_BLOCK_WOOD ||
      element == EL_GATE_WOOD)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_HITTING);

    return TRUE;
  }

  if (IS_WALL_ICE(element))
  {
    int mask;

    mask = (LX + XS) / MINI_TILEX - ELX * 2 + 1;    /* Quadrant (horizontal) */
    mask <<= (((LY + YS) / MINI_TILEY - ELY * 2) > 0) * 2;  /* || (vertical) */

    /* check if laser hits wall with an angle of 90° */
    if (IS_90_ANGLE(laser.current_angle))
      mask += mask * (2 + IS_HORIZ_ANGLE(laser.current_angle) * 2);

    if (mask == 1 || mask == 2 || mask == 4 || mask == 8)
    {
      int i;

      for (i = 0; i < 4; i++)
      {
	if (mask == (1 << i) && (XS > 0) == (i % 2) && (YS > 0) == (i / 2))
	  mask = 15 - (8 >> i);
	else if (ABS(XS) == 4 &&
		 mask == (1 << i) &&
		 (XS > 0) == (i % 2) &&
		 (YS < 0) == (i / 2))
	  mask = 3 + (i / 2) * 9;
	else if (ABS(YS) == 4 &&
		 mask == (1 << i) &&
		 (XS < 0) == (i % 2) &&
		 (YS > 0) == (i / 2))
	  mask = 5 + (i % 2) * 5;
      }
    }

    laser.wall_mask = mask;
  }
  else if (IS_WALL_AMOEBA(element))
  {
    int elx = (LX - 2 * XS) / TILEX;
    int ely = (LY - 2 * YS) / TILEY;
    int element2 = Feld[elx][ely];
    int mask;

    if (element2 != EL_EMPTY && !IS_WALL_AMOEBA(element2))
    {
      laser.dest_element = EL_EMPTY;

      return TRUE;
    }

    ELX = elx;
    ELY = ely;

    mask = (LX - 2 * XS) / 16 - ELX * 2 + 1;
    mask <<= ((LY - 2 * YS) / 16 - ELY * 2) * 2;

    if (IS_90_ANGLE(laser.current_angle))
      mask += mask * (2 + IS_HORIZ_ANGLE(laser.current_angle) * 2);

    laser.dest_element = element2 | EL_WALL_AMOEBA;

    laser.wall_mask = mask;
  }

  return TRUE;
}

void OpenExit(int x, int y)
{
  int delay = 6;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 4 * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;

    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
      DrawGraphicAnimation_MM(x, y, IMG_MM_EXIT_OPENING, 3 - phase);

    if (!MovDelay[x][y])
    {
      Feld[x][y] = EL_EXIT_OPEN;
      DrawField_MM(x, y);
    }
  }
}

void OpenSurpriseBall(int x, int y)
{
  int delay = 2;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = 50 * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    MovDelay[x][y]--;

    if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
    {
      Bitmap *bitmap;
      int graphic = el2gfx(Store[x][y]);
      int gx, gy;
      int dx = RND(26), dy = RND(26);

      getGraphicSource(graphic, 0, &bitmap, &gx, &gy);

      BlitBitmap(bitmap, drawto, gx + dx, gy + dy, 6, 6,
		 cSX + x * TILEX + dx, cSY + y * TILEY + dy);

      MarkTileDirty(x, y);
    }

    if (!MovDelay[x][y])
    {
      Feld[x][y] = Store[x][y];
      Store[x][y] = 0;
      DrawField_MM(x, y);

      ScanLaser();
    }
  }
}

void MeltIce(int x, int y)
{
  int frames = 5;
  int delay = 5;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = frames * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;
    int wall_mask = Store2[x][y];
    int real_element = Feld[x][y] - EL_WALL_CHANGING + EL_WALL_ICE;

    MovDelay[x][y]--;
    phase = frames - MovDelay[x][y] / delay - 1;

    if (!MovDelay[x][y])
    {
      int i;

      Feld[x][y] = real_element & (wall_mask ^ 0xFF);
      Store[x][y] = Store2[x][y] = 0;

      DrawWalls_MM(x, y, Feld[x][y]);

      if (Feld[x][y] == EL_WALL_ICE)
	Feld[x][y] = EL_EMPTY;

      for (i = (laser.num_damages > 0 ? laser.num_damages - 1 : 0); i >= 0; i--)
	if (laser.damage[i].is_mirror)
	  break;

      if (i > 0)
	DrawLaser(laser.damage[i].edge - 1, DL_LASER_DISABLED);
      else
	DrawLaser(0, DL_LASER_DISABLED);

      ScanLaser();
    }
    else if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
    {
      DrawWallsAnimation_MM(x, y, real_element, phase, wall_mask);

      laser.redraw = TRUE;
    }
  }
}

void GrowAmoeba(int x, int y)
{
  int frames = 5;
  int delay = 1;

  if (!MovDelay[x][y])		/* next animation frame */
    MovDelay[x][y] = frames * delay;

  if (MovDelay[x][y])		/* wait some time before next frame */
  {
    int phase;
    int wall_mask = Store2[x][y];
    int real_element = Feld[x][y] - EL_WALL_CHANGING + EL_WALL_AMOEBA;

    MovDelay[x][y]--;
    phase = MovDelay[x][y] / delay;

    if (!MovDelay[x][y])
    {
      Feld[x][y] = real_element;
      Store[x][y] = Store2[x][y] = 0;

      DrawWalls_MM(x, y, Feld[x][y]);
      DrawLaser(0, DL_LASER_ENABLED);
    }
    else if (!(MovDelay[x][y] % delay) && IN_SCR_FIELD(x, y))
    {
      DrawWallsAnimation_MM(x, y, real_element, phase, wall_mask);
    }
  }
}

static void Explode_MM(int x, int y, int phase, int mode)
{
  int num_phase = 9, delay = 2;
  int last_phase = num_phase * delay;
  int half_phase = (num_phase / 2) * delay;

  laser.redraw = TRUE;

  if (phase == EX_PHASE_START)		/* initialize 'Store[][]' field */
  {
    int center_element = Feld[x][y];

    if (IS_MOVING(x, y) || IS_BLOCKED(x, y))
    {
      /* put moving element to center field (and let it explode there) */
      center_element = MovingOrBlocked2Element_MM(x, y);
      RemoveMovingField_MM(x, y);

      Feld[x][y] = center_element;
    }

    if (center_element == EL_BOMB || IS_MCDUFFIN(center_element))
      Store[x][y] = center_element;
    else
      Store[x][y] = EL_EMPTY;

    Store2[x][y] = mode;
    Feld[x][y] = EL_EXPLODING_OPAQUE;
    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;
    Frame[x][y] = 1;

    return;
  }

  Frame[x][y] = (phase < last_phase ? phase + 1 : 0);

  if (phase == half_phase)
  {
    Feld[x][y] = EL_EXPLODING_TRANSP;

    if (x == ELX && y == ELY)
      ScanLaser();
  }

  if (phase == last_phase)
  {
    if (Store[x][y] == EL_BOMB)
    {
      DrawLaser(0, DL_LASER_DISABLED);
      InitLaser();

      Bang_MM(laser.start_edge.x, laser.start_edge.y);
      Store[x][y] = EL_EMPTY;

      game_mm.game_over = TRUE;
      game_mm.game_over_cause = GAME_OVER_BOMB;

      SetTileCursorActive(FALSE);

      laser.overloaded = FALSE;
    }
    else if (IS_MCDUFFIN(Store[x][y]))
    {
      Store[x][y] = EL_EMPTY;

      game.restart_game_message = "Bomb killed Mc Duffin ! Play it again ?";
    }

    Feld[x][y] = Store[x][y];
    Store[x][y] = Store2[x][y] = 0;
    MovDir[x][y] = MovPos[x][y] = MovDelay[x][y] = 0;

    InitField(x, y, FALSE);
    DrawField_MM(x, y);
  }
  else if (!(phase % delay) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
  {
    int graphic = IMG_MM_DEFAULT_EXPLODING;
    int graphic_phase = (phase / delay - 1);
    Bitmap *bitmap;
    int src_x, src_y;

    if (Store2[x][y] == EX_KETTLE)
    {
      if (graphic_phase < 3)
      {
	graphic = IMG_MM_KETTLE_EXPLODING;
      }
      else if (graphic_phase < 5)
      {
	graphic_phase += 3;
      }
      else
      {
	graphic = IMG_EMPTY;
	graphic_phase = 0;
      }
    }
    else if (Store2[x][y] == EX_SHORT)
    {
      if (graphic_phase < 4)
      {
	graphic_phase += 4;
      }
      else
      {
	graphic = IMG_EMPTY;
	graphic_phase = 0;
      }
    }

    getGraphicSource(graphic, graphic_phase, &bitmap, &src_x, &src_y);

    BlitBitmap(bitmap, drawto_field, src_x, src_y, TILEX, TILEY,
	       cFX + x * TILEX, cFY + y * TILEY);

    MarkTileDirty(x, y);
  }
}

static void Bang_MM(int x, int y)
{
  int element = Feld[x][y];
  int mode = EX_NORMAL;

#if 0
  DrawLaser(0, DL_LASER_ENABLED);
#endif

  switch(element)
  {
    case EL_KETTLE:
      mode = EX_KETTLE;
      break;

    case EL_GATE_STONE:
    case EL_GATE_WOOD:
      mode = EX_SHORT;
      break;

    default:
      mode = EX_NORMAL;
      break;
  }

  if (IS_PACMAN(element))
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);
  else if (element == EL_BOMB || IS_MCDUFFIN(element))
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);
  else if (element == EL_KEY)
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);
  else
    PlayLevelSound_MM(x, y, element, MM_ACTION_EXPLODING);

  Explode_MM(x, y, EX_PHASE_START, mode);
}

void TurnRound(int x, int y)
{
  static struct
  {
    int x, y;
  } move_xy[] =
  {
    { 0, 0 },
    {-1, 0 },
    {+1, 0 },
    { 0, 0 },
    { 0, -1 },
    { 0, 0 }, { 0, 0 }, { 0, 0 },
    { 0, +1 }
  };
  static struct
  {
    int left, right, back;
  } turn[] =
  {
    { 0,	0,		0 },
    { MV_DOWN,	MV_UP,		MV_RIGHT },
    { MV_UP,	MV_DOWN,	MV_LEFT },
    { 0,	0,		0 },
    { MV_LEFT,	MV_RIGHT,	MV_DOWN },
    { 0,0,0 },	{ 0,0,0 },	{ 0,0,0 },
    { MV_RIGHT,	MV_LEFT,	MV_UP }
  };

  int element = Feld[x][y];
  int old_move_dir = MovDir[x][y];
  int right_dir = turn[old_move_dir].right;
  int back_dir = turn[old_move_dir].back;
  int right_dx = move_xy[right_dir].x, right_dy = move_xy[right_dir].y;
  int right_x = x + right_dx, right_y = y + right_dy;

  if (element == EL_PACMAN)
  {
    boolean can_turn_right = FALSE;

    if (IN_LEV_FIELD(right_x, right_y) &&
	IS_EATABLE4PACMAN(Feld[right_x][right_y]))
      can_turn_right = TRUE;

    if (can_turn_right)
      MovDir[x][y] = right_dir;
    else
      MovDir[x][y] = back_dir;

    MovDelay[x][y] = 0;
  }
}

static void StartMoving_MM(int x, int y)
{
  int element = Feld[x][y];

  if (Stop[x][y])
    return;

  if (CAN_MOVE(element))
  {
    int newx, newy;

    if (MovDelay[x][y])		/* wait some time before next movement */
    {
      MovDelay[x][y]--;

      if (MovDelay[x][y])
	return;
    }

    /* now make next step */

    Moving2Blocked_MM(x, y, &newx, &newy);	/* get next screen position */

    if (element == EL_PACMAN &&
	IN_LEV_FIELD(newx, newy) && IS_EATABLE4PACMAN(Feld[newx][newy]) &&
	!ObjHit(newx, newy, HIT_POS_CENTER))
    {
      Store[newx][newy] = Feld[newx][newy];
      Feld[newx][newy] = EL_EMPTY;

      DrawField_MM(newx, newy);
    }
    else if (!IN_LEV_FIELD(newx, newy) || !IS_FREE(newx, newy) ||
	     ObjHit(newx, newy, HIT_POS_CENTER))
    {
      /* object was running against a wall */

      TurnRound(x, y);

      return;
    }

    InitMovingField_MM(x, y, MovDir[x][y]);
  }

  if (MovDir[x][y])
    ContinueMoving_MM(x, y);
}

static void ContinueMoving_MM(int x, int y)
{
  int element = Feld[x][y];
  int direction = MovDir[x][y];
  int dx = (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int dy = (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);
  int horiz_move = (dx!=0);
  int newx = x + dx, newy = y + dy;
  int step = (horiz_move ? dx : dy) * TILEX / 8;

  MovPos[x][y] += step;

  if (ABS(MovPos[x][y]) >= TILEX)	/* object reached its destination */
  {
    Feld[x][y] = EL_EMPTY;
    Feld[newx][newy] = element;

    MovPos[x][y] = MovDir[x][y] = MovDelay[x][y] = 0;
    MovDelay[newx][newy] = 0;

    if (!CAN_MOVE(element))
      MovDir[newx][newy] = 0;

    DrawField_MM(x, y);
    DrawField_MM(newx, newy);

    Stop[newx][newy] = TRUE;

    if (element == EL_PACMAN)
    {
      if (Store[newx][newy] == EL_BOMB)
	Bang_MM(newx, newy);

      if (IS_WALL_AMOEBA(Store[newx][newy]) &&
	  (LX + 2 * XS) / TILEX == newx &&
	  (LY + 2 * YS) / TILEY == newy)
      {
	laser.num_edges--;
	ScanLaser();
      }
    }
  }
  else				/* still moving on */
  {
    DrawField_MM(x, y);
  }

  laser.redraw = TRUE;
}

boolean ClickElement(int x, int y, int button)
{
  static unsigned int click_delay = 0;
  static int click_delay_value = CLICK_DELAY;
  static boolean new_button = TRUE;
  boolean element_clicked = FALSE;
  int element;

  if (button == -1)
  {
    /* initialize static variables */
    click_delay = 0;
    click_delay_value = CLICK_DELAY;
    new_button = TRUE;

    return FALSE;
  }

  /* do not rotate objects hit by the laser after the game was solved */
  if (game_mm.level_solved && Hit[x][y])
    return FALSE;

  if (button == MB_RELEASED)
  {
    new_button = TRUE;
    click_delay_value = CLICK_DELAY;

    /* release eventually hold auto-rotating mirror */
    RotateMirror(x, y, MB_RELEASED);

    return FALSE;
  }

  if (!FrameReached(&click_delay, click_delay_value) && !new_button)
    return FALSE;

  if (button == MB_MIDDLEBUTTON)	/* middle button has no function */
    return FALSE;

  if (!IN_LEV_FIELD(x, y))
    return FALSE;

  if (Feld[x][y] == EL_EMPTY)
    return FALSE;

  element = Feld[x][y];

  if (IS_MIRROR(element) ||
      IS_BEAMER(element) ||
      IS_POLAR(element) ||
      IS_POLAR_CROSS(element) ||
      IS_DF_MIRROR(element) ||
      IS_DF_MIRROR_AUTO(element))
  {
    RotateMirror(x, y, button);

    element_clicked = TRUE;
  }
  else if (IS_MCDUFFIN(element))
  {
    if (!laser.fuse_off)
    {
      DrawLaser(0, DL_LASER_DISABLED);

      /*
      BackToFront();
      */
    }

    element = get_rotated_element(element, BUTTON_ROTATION(button));
    laser.start_angle = get_element_angle(element);

    InitLaser();

    Feld[x][y] = element;
    DrawField_MM(x, y);

    /*
    BackToFront();
    */

    if (!laser.fuse_off)
      ScanLaser();

    element_clicked = TRUE;
  }
  else if (element == EL_FUSE_ON && laser.fuse_off)
  {
    if (x != laser.fuse_x || y != laser.fuse_y)
      return FALSE;

    laser.fuse_off = FALSE;
    laser.fuse_x = laser.fuse_y = -1;

    DrawGraphic_MM(x, y, IMG_MM_FUSE_ACTIVE);
    ScanLaser();

    element_clicked = TRUE;
  }
  else if (element == EL_FUSE_ON && !laser.fuse_off && new_button)
  {
    laser.fuse_off = TRUE;
    laser.fuse_x = x;
    laser.fuse_y = y;
    laser.overloaded = FALSE;

    DrawLaser(0, DL_LASER_DISABLED);
    DrawGraphic_MM(x, y, IMG_MM_FUSE);

    element_clicked = TRUE;
  }
  else if (element == EL_LIGHTBALL)
  {
    Bang_MM(x, y);
    RaiseScoreElement_MM(element);
    DrawLaser(0, DL_LASER_ENABLED);

    element_clicked = TRUE;
  }

  click_delay_value = (new_button ? CLICK_DELAY_FIRST : CLICK_DELAY);
  new_button = FALSE;

  return element_clicked;
}

void RotateMirror(int x, int y, int button)
{
  if (button == MB_RELEASED)
  {
    /* release eventually hold auto-rotating mirror */
    hold_x = -1;
    hold_y = -1;

    return;
  }

  if (IS_MIRROR(Feld[x][y]) ||
      IS_POLAR_CROSS(Feld[x][y]) ||
      IS_POLAR(Feld[x][y]) ||
      IS_BEAMER(Feld[x][y]) ||
      IS_DF_MIRROR(Feld[x][y]) ||
      IS_GRID_STEEL_AUTO(Feld[x][y]) ||
      IS_GRID_WOOD_AUTO(Feld[x][y]))
  {
    Feld[x][y] = get_rotated_element(Feld[x][y], BUTTON_ROTATION(button));
  }
  else if (IS_DF_MIRROR_AUTO(Feld[x][y]))
  {
    if (button == MB_LEFTBUTTON)
    {
      /* left mouse button only for manual adjustment, no auto-rotating;
	 freeze mirror for until mouse button released */
      hold_x = x;
      hold_y = y;
    }
    else if (button == MB_RIGHTBUTTON && (hold_x != x || hold_y != y))
    {
      Feld[x][y] = get_rotated_element(Feld[x][y], ROTATE_RIGHT);
    }
  }

  if (IS_GRID_STEEL_AUTO(Feld[x][y]) || IS_GRID_WOOD_AUTO(Feld[x][y]))
  {
    int edge = Hit[x][y];

    DrawField_MM(x, y);

    if (edge > 0)
    {
      DrawLaser(edge - 1, DL_LASER_DISABLED);
      ScanLaser();
    }
  }
  else if (ObjHit(x, y, HIT_POS_CENTER))
  {
    int edge = Hit[x][y];

    if (edge == 0)
    {
      Error(ERR_WARN, "RotateMirror: inconsistent field Hit[][]!\n");
      edge = 1;
    }

    DrawLaser(edge - 1, DL_LASER_DISABLED);
    ScanLaser();
  }
  else
  {
    int check = 1;

    if (ObjHit(x, y, HIT_POS_EDGE | HIT_POS_BETWEEN))
      check = 2;

    DrawField_MM(x, y);

    if ((IS_BEAMER(Feld[x][y]) ||
	 IS_POLAR(Feld[x][y]) ||
	 IS_POLAR_CROSS(Feld[x][y])) && x == ELX && y == ELY)
    {
      check = 0;

      if (IS_BEAMER(Feld[x][y]))
      {
#if 0
	printf("TEST (%d, %d) [%d] [%d]\n",
	       LX, LY,
	       laser.beamer_edge, laser.beamer[1].num);
#endif

	laser.num_edges--;
      }

      ScanLaser();
    }

    if (check == 2)
      DrawLaser(0, DL_LASER_ENABLED);
  }
}

void AutoRotateMirrors()
{
  int x, y;

  if (!FrameReached(&rotate_delay, AUTO_ROTATE_DELAY))
    return;

  for (x = 0; x < lev_fieldx; x++)
  {
    for (y = 0; y < lev_fieldy; y++)
    {
      int element = Feld[x][y];

      /* do not rotate objects hit by the laser after the game was solved */
      if (game_mm.level_solved && Hit[x][y])
	continue;

      if (IS_DF_MIRROR_AUTO(element) ||
	  IS_GRID_WOOD_AUTO(element) ||
	  IS_GRID_STEEL_AUTO(element) ||
	  element == EL_REFRACTOR)
	RotateMirror(x, y, MB_RIGHTBUTTON);
    }
  }
}

boolean ObjHit(int obx, int oby, int bits)
{
  int i;

  obx *= TILEX;
  oby *= TILEY;

  if (bits & HIT_POS_CENTER)
  {
    if (CheckLaserPixel(cSX + obx + 15,
			cSY + oby + 15))
      return TRUE;
  }

  if (bits & HIT_POS_EDGE)
  {
    for (i = 0; i < 4; i++)
      if (CheckLaserPixel(cSX + obx + 31 * (i % 2),
			  cSY + oby + 31 * (i / 2)))
	return TRUE;
  }

  if (bits & HIT_POS_BETWEEN)
  {
    for (i = 0; i < 4; i++)
      if (CheckLaserPixel(cSX + 4 + obx + 22 * (i % 2),
			  cSY + 4 + oby + 22 * (i / 2)))
	return TRUE;
  }

  return FALSE;
}

void DeletePacMan(int px, int py)
{
  int i, j;

  Bang_MM(px, py);

  if (game_mm.num_pacman <= 1)
  {
    game_mm.num_pacman = 0;
    return;
  }

  for (i = 0; i < game_mm.num_pacman; i++)
    if (game_mm.pacman[i].x == px && game_mm.pacman[i].y == py)
      break;

  game_mm.num_pacman--;

  for (j = i; j < game_mm.num_pacman; j++)
  {
    game_mm.pacman[j].x   = game_mm.pacman[j + 1].x;
    game_mm.pacman[j].y   = game_mm.pacman[j + 1].y;
    game_mm.pacman[j].dir = game_mm.pacman[j + 1].dir;
  }
}

void ColorCycling(void)
{
  static int CC, Cc = 0;

  static int color, old = 0xF00, new = 0x010, mult = 1;
  static unsigned short red, green, blue;

  if (color_status == STATIC_COLORS)
    return;

  CC = FrameCounter;

  if (CC < Cc || CC > Cc + 2)
  {
    Cc = CC;

    color = old + new * mult;
    if (mult > 0)
      mult++;
    else
      mult--;

    if (ABS(mult) == 16)
    {
      mult =- mult / 16;
      old = color;
      new = new << 4;

      if (new > 0x100)
	new = 0x001;
    }

    red   = 0x0e00 * ((color & 0xF00) >> 8);
    green = 0x0e00 * ((color & 0x0F0) >> 4);
    blue  = 0x0e00 * ((color & 0x00F));
    SetRGB(pen_magicolor[0], red, green, blue);

    red   = 0x1111 * ((color & 0xF00) >> 8);
    green = 0x1111 * ((color & 0x0F0) >> 4);
    blue  = 0x1111 * ((color & 0x00F));
    SetRGB(pen_magicolor[1], red, green, blue);
  }
}

static void GameActions_MM_Ext(struct MouseActionInfo action, boolean warp_mode)
{
  int element;
  int x, y, i;

  int r, d;

  for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
    Stop[x][y] = FALSE;

  for (y = 0; y < lev_fieldy; y++) for (x = 0; x < lev_fieldx; x++)
  {
    element = Feld[x][y];

    if (!IS_MOVING(x, y) && CAN_MOVE(element))
      StartMoving_MM(x, y);
    else if (IS_MOVING(x, y))
      ContinueMoving_MM(x, y);
    else if (IS_EXPLODING(element))
      Explode_MM(x, y, Frame[x][y], EX_NORMAL);
    else if (element == EL_EXIT_OPENING)
      OpenExit(x, y);
    else if (element == EL_GRAY_BALL_OPENING)
      OpenSurpriseBall(x, y);
    else if (IS_WALL_CHANGING(element) && Store[x][y] == EL_WALL_ICE)
      MeltIce(x, y);
    else if (IS_WALL_CHANGING(element) && Store[x][y] == EL_WALL_AMOEBA)
      GrowAmoeba(x, y);
  }

  AutoRotateMirrors();

#if 1
  /* !!! CHANGE THIS: REDRAW ONLY WHEN NEEDED !!! */

  /* redraw after Explode_MM() ... */
  if (laser.redraw)
    DrawLaser(0, DL_LASER_ENABLED);
  laser.redraw = FALSE;
#endif

  CT = FrameCounter;

  if (game_mm.num_pacman && FrameReached(&pacman_delay, PACMAN_MOVE_DELAY))
  {
    MovePacMen();

    if (laser.num_damages > MAX_LASER_LEN && !laser.fuse_off)
    {
      DrawLaser(0, DL_LASER_DISABLED);
      ScanLaser();
    }
  }

  if (FrameReached(&energy_delay, ENERGY_DELAY))
  {
    if (game_mm.energy_left > 0)
    {
      game_mm.energy_left--;

#if 0
      BlitBitmap(pix[PIX_DOOR], drawto,
		 DOOR_GFX_PAGEX5 + XX_ENERGY, DOOR_GFX_PAGEY1 + YY_ENERGY,
		 ENERGY_XSIZE, ENERGY_YSIZE - game_mm.energy_left,
		 DX_ENERGY, DY_ENERGY);
#endif
      redraw_mask |= REDRAW_DOOR_1;
    }
    else if (setup.time_limit && !game_mm.game_over)
    {
      int i;

      for (i = 15; i >= 0; i--)
      {
#if 0
	SetRGB(pen_ray, 0x0000, 0x0000, i * color_scale);
#endif
	pen_ray = GetPixelFromRGB(window,
				  native_mm_level.laser_red   * 0x11 * i,
				  native_mm_level.laser_green * 0x11 * i,
				  native_mm_level.laser_blue  * 0x11 * i);

	DrawLaser(0, DL_LASER_ENABLED);
	BackToFront();
	Delay(50);
      }

      StopSound_MM(SND_MM_GAME_HEALTH_CHARGING);
#if 0
      FadeMusic();
#endif

      DrawLaser(0, DL_LASER_DISABLED);
      game_mm.game_over = TRUE;
      game_mm.game_over_cause = GAME_OVER_NO_ENERGY;

      SetTileCursorActive(FALSE);

      game.restart_game_message = "Out of magic energy ! Play it again ?";

#if 0
      if (Request("Out of magic energy ! Play it again ?",
		  REQ_ASK | REQ_STAY_CLOSED))
      {
	InitGame();
      }
      else
      {
	game_status = MAINMENU;
	DrawMainMenu();
      }
#endif

      return;
    }
  }

  element = laser.dest_element;

#if 0
  if (element != Feld[ELX][ELY])
  {
    printf("element == %d, Feld[ELX][ELY] == %d\n",
	   element, Feld[ELX][ELY]);
  }
#endif

  if (!laser.overloaded && laser.overload_value == 0 &&
      element != EL_BOMB &&
      element != EL_MINE &&
      element != EL_BALL_GRAY &&
      element != EL_BLOCK_STONE &&
      element != EL_BLOCK_WOOD &&
      element != EL_FUSE_ON &&
      element != EL_FUEL_FULL &&
      !IS_WALL_ICE(element) &&
      !IS_WALL_AMOEBA(element))
    return;

  if (((laser.overloaded && laser.overload_value < MAX_LASER_OVERLOAD) ||
       (!laser.overloaded && laser.overload_value > 0)) &&
      FrameReached(&overload_delay, HEALTH_DELAY(laser.overloaded)))
  {
    if (laser.overloaded)
      laser.overload_value++;
    else
      laser.overload_value--;

    if (game_mm.cheat_no_overload)
    {
      laser.overloaded = FALSE;
      laser.overload_value = 0;
    }

    game_mm.laser_overload_value = laser.overload_value;

    if (laser.overload_value < MAX_LASER_OVERLOAD - 8)
    {
      int color_up = 0xFF * laser.overload_value / MAX_LASER_OVERLOAD;
      int color_down = 0xFF - color_up;

#if 0
      SetRGB(pen_ray, (laser.overload_value / 6) * color_scale, 0x0000,
	     (15 - (laser.overload_value / 6)) * color_scale);
#endif
      pen_ray =
	GetPixelFromRGB(window,
			(native_mm_level.laser_red  ? 0xFF : color_up),
			(native_mm_level.laser_green ? color_down : 0x00),
			(native_mm_level.laser_blue  ? color_down : 0x00));

      DrawLaser(0, DL_LASER_ENABLED);
#if 0
      BackToFront();
#endif
    }

    if (!laser.overloaded)
      StopSound_MM(SND_MM_GAME_HEALTH_CHARGING);
    else if (setup.sound_loops)
      PlaySoundLoop_MM(SND_MM_GAME_HEALTH_CHARGING);
    else
      PlaySound_MM(SND_MM_GAME_HEALTH_CHARGING);

    if (laser.overloaded)
    {
#if 0
      BlitBitmap(pix[PIX_DOOR], drawto,
		 DOOR_GFX_PAGEX4 + XX_OVERLOAD,
		 DOOR_GFX_PAGEY1 + YY_OVERLOAD + OVERLOAD_YSIZE
		 - laser.overload_value,
		 OVERLOAD_XSIZE, laser.overload_value,
		 DX_OVERLOAD, DY_OVERLOAD + OVERLOAD_YSIZE
		 - laser.overload_value);
#endif
      redraw_mask |= REDRAW_DOOR_1;
    }
    else
    {
#if 0
      BlitBitmap(pix[PIX_DOOR], drawto,
		 DOOR_GFX_PAGEX5 + XX_OVERLOAD, DOOR_GFX_PAGEY1 + YY_OVERLOAD,
		 OVERLOAD_XSIZE, OVERLOAD_YSIZE - laser.overload_value,
		 DX_OVERLOAD, DY_OVERLOAD);
#endif
      redraw_mask |= REDRAW_DOOR_1;
    }

    if (laser.overload_value == MAX_LASER_OVERLOAD)
    {
      int i;

      for (i = 15; i >= 0; i--)
      {
#if 0
	SetRGB(pen_ray, i * color_scale, 0x0000, 0x0000);
#endif

	pen_ray = GetPixelFromRGB(window, 0x11 * i, 0x00, 0x00);

	DrawLaser(0, DL_LASER_ENABLED);
	BackToFront();
	Delay(50);
      }

      DrawLaser(0, DL_LASER_DISABLED);

      game_mm.game_over = TRUE;
      game_mm.game_over_cause = GAME_OVER_OVERLOADED;

      SetTileCursorActive(FALSE);

      game.restart_game_message = "Magic spell hit Mc Duffin ! Play it again ?";

#if 0
      if (Request("Magic spell hit Mc Duffin ! Play it again ?",
		  REQ_ASK | REQ_STAY_CLOSED))
      {
	InitGame();
      }
      else
      {
	game_status = MAINMENU;
	DrawMainMenu();
      }
#endif

      return;
    }
  }

  if (laser.fuse_off)
    return;

  CT -= Ct;

  if (element == EL_BOMB && CT > native_mm_level.time_bomb)
  {
    if (game_mm.cheat_no_explosion)
      return;

#if 0
    laser.num_damages--;
    DrawLaser(0, DL_LASER_DISABLED);
    laser.num_edges = 0;
#endif

    Bang_MM(ELX, ELY);

    laser.dest_element = EL_EXPLODING_OPAQUE;

#if 0
    Bang_MM(ELX, ELY);
    laser.num_damages--;
    DrawLaser(0, DL_LASER_DISABLED);

    laser.num_edges = 0;
    Bang_MM(laser.start_edge.x, laser.start_edge.y);

    if (Request("Bomb killed Mc Duffin ! Play it again ?",
		REQ_ASK | REQ_STAY_CLOSED))
    {
      InitGame();
    }
    else
    {
      game_status = MAINMENU;
      DrawMainMenu();
    }
#endif

    return;
  }

  if (element == EL_FUSE_ON && CT > native_mm_level.time_fuse)
  {
    laser.fuse_off = TRUE;
    laser.fuse_x = ELX;
    laser.fuse_y = ELY;

    DrawLaser(0, DL_LASER_DISABLED);
    DrawGraphic_MM(ELX, ELY, IMG_MM_FUSE);
  }

  if (element == EL_BALL_GRAY && CT > native_mm_level.time_ball)
  {
    static int new_elements[] =
    {
      EL_MIRROR_START,
      EL_MIRROR_FIXED_START,
      EL_POLAR_START,
      EL_POLAR_CROSS_START,
      EL_PACMAN_START,
      EL_KETTLE,
      EL_BOMB,
      EL_PRISM
    };
    int num_new_elements = sizeof(new_elements) / sizeof(int);
    int new_element = new_elements[RND(num_new_elements)];

    Store[ELX][ELY] = new_element + RND(get_num_elements(new_element));
    Feld[ELX][ELY] = EL_GRAY_BALL_OPENING;

    /* !!! CHECK AGAIN: Laser on Polarizer !!! */
    ScanLaser();

    return;

#if 0
    int graphic;

    switch (RND(5))
    {
      case 0:
        element = EL_MIRROR_START + RND(16);
	break;
      case 1:
	{
	  int rnd = RND(3);

	  element = (rnd == 0 ? EL_KETTLE : rnd == 1 ? EL_BOMB : EL_PRISM);
	}
	break;
      default:
	{
	  int rnd = RND(3);

	  element = (rnd == 0 ? EL_FUSE_ON :
		     rnd >= 1 && rnd <= 4 ? EL_PACMAN_RIGHT + rnd - 1 :
		     rnd >= 5 && rnd <= 20 ? EL_POLAR_START + rnd - 5 :
		     rnd >= 21 && rnd <= 24 ? EL_POLAR_CROSS_START + rnd - 21 :
		     EL_MIRROR_FIXED_START + rnd - 25);
	}
	break;
    }

    graphic = el2gfx(element);

    for (i = 0; i < 50; i++)
    {
      int x = RND(26);
      int y = RND(26);

#if 0
      BlitBitmap(pix[PIX_BACK], drawto,
		 SX + (graphic % GFX_PER_LINE) * TILEX + x,
		 SY + (graphic / GFX_PER_LINE) * TILEY + y, 6, 6,
		 SX + ELX * TILEX + x,
		 SY + ELY * TILEY + y);
#endif
      MarkTileDirty(ELX, ELY);
      BackToFront();

      DrawLaser(0, DL_LASER_ENABLED);

      Delay(50);
    }

    Feld[ELX][ELY] = element;
    DrawField_MM(ELX, ELY);

#if 0
    printf("NEW ELEMENT: (%d, %d)\n", ELX, ELY);
#endif

    /* above stuff: GRAY BALL -> PRISM !!! */
/*
    LX = ELX * TILEX + 14;
    LY = ELY * TILEY + 14;
    if (laser.current_angle == (laser.current_angle >> 1) << 1)
      OK = 8;
    else
      OK = 4;
    LX -= OK * XS;
    LY -= OK * YS;

    laser.num_edges -= 2;
    laser.num_damages--;
*/

#if 0
    for (i = (laser.num_damages > 0 ? laser.num_damages - 1 : 0); i>=0; i--)
      if (laser.damage[i].is_mirror)
	break;

    if (i > 0)
      DrawLaser(laser.damage[i].edge - 1, DL_LASER_DISABLED);
    else
      DrawLaser(0, DL_LASER_DISABLED);
#else
    DrawLaser(0, DL_LASER_DISABLED);
#endif

    ScanLaser();

    /*
    printf("TEST ELEMENT: %d\n", Feld[0][0]);
    */
#endif

    return;
  }

  if (IS_WALL_ICE(element) && CT > 50)
  {
    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_SHRINKING);

    {
      Feld[ELX][ELY] = Feld[ELX][ELY] - EL_WALL_ICE + EL_WALL_CHANGING;
      Store[ELX][ELY] = EL_WALL_ICE;
      Store2[ELX][ELY] = laser.wall_mask;

      laser.dest_element = Feld[ELX][ELY];

      return;
    }

    for (i = 0; i < 5; i++)
    {
      int phase = i + 1;

      if (i == 4)
      {
	Feld[ELX][ELY] &= (laser.wall_mask ^ 0xFF);
	phase = 0;
      }

      DrawWallsAnimation_MM(ELX, ELY, Feld[ELX][ELY], phase, laser.wall_mask);
      BackToFront();
      Delay(100);
    }

    if (Feld[ELX][ELY] == EL_WALL_ICE)
      Feld[ELX][ELY] = EL_EMPTY;

/*
    laser.num_edges--;
    LX = laser.edge[laser.num_edges].x - cSX2;
    LY = laser.edge[laser.num_edges].y - cSY2;
*/

    for (i = (laser.num_damages > 0 ? laser.num_damages - 1 : 0); i >= 0; i--)
      if (laser.damage[i].is_mirror)
	break;

    if (i > 0)
      DrawLaser(laser.damage[i].edge - 1, DL_LASER_DISABLED);
    else
      DrawLaser(0, DL_LASER_DISABLED);

    ScanLaser();

    return;
  }

  if (IS_WALL_AMOEBA(element) && CT > 60)
  {
    int k1, k2, k3, dx, dy, de, dm;
    int element2 = Feld[ELX][ELY];

    if (element2 != EL_EMPTY && !IS_WALL_AMOEBA(element))
      return;

    for (i = laser.num_damages - 1; i >= 0; i--)
      if (laser.damage[i].is_mirror)
	break;

    r = laser.num_edges;
    d = laser.num_damages;
    k1 = i;

    if (k1 > 0)
    {
      int x, y;

      DrawLaser(laser.damage[k1].edge - 1, DL_LASER_DISABLED);

      laser.num_edges++;
      DrawLaser(0, DL_LASER_ENABLED);
      laser.num_edges--;

      x = laser.damage[k1].x;
      y = laser.damage[k1].y;

      DrawField_MM(x, y);
    }

    for (i = 0; i < 4; i++)
    {
      if (laser.wall_mask & (1 << i))
      {
	if (CheckLaserPixel(cSX + ELX * TILEX + 14 + (i % 2) * 2,
			    cSY + ELY * TILEY + 31 * (i / 2)))
	  break;

	if (CheckLaserPixel(cSX + ELX * TILEX + 31 * (i % 2),
			    cSY + ELY * TILEY + 14 + (i / 2) * 2))
	  break;
      }
    }

    k2 = i;

    for (i = 0; i < 4; i++)
    {
      if (laser.wall_mask & (1 << i))
      {
	if (CheckLaserPixel(cSX + ELX * TILEX + 31 * (i % 2),
			    cSY + ELY * TILEY + 31 * (i / 2)))
	  break;
      }
    }

    k3 = i;

    if (laser.num_beamers > 0 ||
	k1 < 1 || k2 < 4 || k3 < 4 ||
	CheckLaserPixel(cSX + ELX * TILEX + 14,
			cSY + ELY * TILEY + 14))
    {
      laser.num_edges = r;
      laser.num_damages = d;

      DrawLaser(0, DL_LASER_DISABLED);
    }

    Feld[ELX][ELY] = element | laser.wall_mask;

    dx = ELX;
    dy = ELY;
    de = Feld[ELX][ELY];
    dm = laser.wall_mask;

#if 1
    {
      int x = ELX, y = ELY;
      int wall_mask = laser.wall_mask;

      ScanLaser();
      DrawLaser(0, DL_LASER_ENABLED);

      PlayLevelSound_MM(dx, dy, element, MM_ACTION_GROWING);

      Feld[x][y] = Feld[x][y] - EL_WALL_AMOEBA + EL_WALL_CHANGING;
      Store[x][y] = EL_WALL_AMOEBA;
      Store2[x][y] = wall_mask;

      return;
    }
#endif

    DrawWallsAnimation_MM(dx, dy, de, 4, dm);
    ScanLaser();
    DrawLaser(0, DL_LASER_ENABLED);

    PlayLevelSound_MM(dx, dy, element, MM_ACTION_GROWING);

    for (i = 4; i >= 0; i--)
    {
      DrawWallsAnimation_MM(dx, dy, de, i, dm);

      BackToFront();
      Delay(20);
    }

    DrawLaser(0, DL_LASER_ENABLED);

    return;
  }

  if ((element == EL_BLOCK_WOOD || element == EL_BLOCK_STONE) &&
      laser.stops_inside_element && CT > native_mm_level.time_block)
  {
    int x, y;
    int k;

    if (ABS(XS) > ABS(YS))
      k = 0;
    else
      k = 1;
    if (XS < YS)
      k += 2;

    for (i = 0; i < 4; i++)
    {
      if (i)
	k++;
      if (k > 3)
	k = 0;

      x = ELX + Step[k * 4].x;
      y = ELY + Step[k * 4].y;

      if (!IN_LEV_FIELD(x, y) || Feld[x][y] != EL_EMPTY)
	continue;

      if (ObjHit(x, y, HIT_POS_CENTER | HIT_POS_EDGE | HIT_POS_BETWEEN))
	continue;

      break;
    }

    if (i > 3)
    {
      laser.overloaded = (element == EL_BLOCK_STONE);

      return;
    }

    PlayLevelSound_MM(ELX, ELY, element, MM_ACTION_PUSHING);

    Feld[ELX][ELY] = 0;
    Feld[x][y] = element;

    DrawGraphic_MM(ELX, ELY, IMG_EMPTY);
    DrawField_MM(x, y);

    if (element == EL_BLOCK_STONE && Box[ELX][ELY])
    {
      DrawLaser(Box[ELX][ELY] - 1, DL_LASER_DISABLED);
      DrawLaser(laser.num_edges - 1, DL_LASER_ENABLED);
    }

    ScanLaser();

    return;
  }

  if (element == EL_FUEL_FULL && CT > 10)
  {
    for (i = game_mm.energy_left; i <= MAX_LASER_ENERGY; i+=2)
    {
#if 0
      BlitBitmap(pix[PIX_DOOR], drawto,
		 DOOR_GFX_PAGEX4 + XX_ENERGY,
		 DOOR_GFX_PAGEY1 + YY_ENERGY + ENERGY_YSIZE - i,
		 ENERGY_XSIZE, i, DX_ENERGY,
		 DY_ENERGY + ENERGY_YSIZE - i);
#endif

      redraw_mask |= REDRAW_DOOR_1;
      BackToFront();

      Delay(20);
    }

    game_mm.energy_left = MAX_LASER_ENERGY;
    Feld[ELX][ELY] = EL_FUEL_EMPTY;
    DrawField_MM(ELX, ELY);

    DrawLaser(0, DL_LASER_ENABLED);

    return;
  }

  return;
}

void GameActions_MM(struct MouseActionInfo action, boolean warp_mode)
{
  boolean element_clicked = ClickElement(action.lx, action.ly, action.button);
  boolean button_released = (action.button == MB_RELEASED);

  GameActions_MM_Ext(action, warp_mode);

  CheckSingleStepMode_MM(element_clicked, button_released);
}

void MovePacMen()
{
  int mx, my, ox, oy, nx, ny;
  int element;
  int l;

  if (++pacman_nr >= game_mm.num_pacman)
    pacman_nr = 0;

  game_mm.pacman[pacman_nr].dir--;

  for (l = 1; l < 5; l++)
  {
    game_mm.pacman[pacman_nr].dir++;

    if (game_mm.pacman[pacman_nr].dir > 4)
      game_mm.pacman[pacman_nr].dir = 1;

    if (game_mm.pacman[pacman_nr].dir % 2)
    {
      mx = 0;
      my = game_mm.pacman[pacman_nr].dir - 2;
    }
    else
    {
      my = 0;
      mx = 3 - game_mm.pacman[pacman_nr].dir;
    }

    ox = game_mm.pacman[pacman_nr].x;
    oy = game_mm.pacman[pacman_nr].y;
    nx = ox + mx;
    ny = oy + my;
    element = Feld[nx][ny];

    if (nx < 0 || nx > 15 || ny < 0 || ny > 11)
      continue;

    if (!IS_EATABLE4PACMAN(element))
      continue;

    if (ObjHit(nx, ny, HIT_POS_CENTER))
      continue;

    Feld[ox][oy] = EL_EMPTY;
    Feld[nx][ny] =
      EL_PACMAN_RIGHT - 1 +
      (game_mm.pacman[pacman_nr].dir - 1 +
       (game_mm.pacman[pacman_nr].dir % 2) * 2);

    game_mm.pacman[pacman_nr].x = nx;
    game_mm.pacman[pacman_nr].y = ny;

    DrawGraphic_MM(ox, oy, IMG_EMPTY);

    if (element != EL_EMPTY)
    {
      int graphic = el2gfx(Feld[nx][ny]);
      Bitmap *bitmap;
      int src_x, src_y;
      int i;

      getGraphicSource(graphic, 0, &bitmap, &src_x, &src_y);

      CT = FrameCounter;
      ox = cSX + ox * TILEX;
      oy = cSY + oy * TILEY;

      for (i = 1; i < 33; i += 2)
	BlitBitmap(bitmap, window,
		   src_x, src_y, TILEX, TILEY,
		   ox + i * mx, oy + i * my);
      Ct = Ct + FrameCounter - CT;
    }

    DrawField_MM(nx, ny);
    BackToFront();

    if (!laser.fuse_off)
    {
      DrawLaser(0, DL_LASER_ENABLED);

      if (ObjHit(nx, ny, HIT_POS_BETWEEN))
      {
	AddDamagedField(nx, ny);

	laser.damage[laser.num_damages - 1].edge = 0;
      }
    }

    if (element == EL_BOMB)
      DeletePacMan(nx, ny);

    if (IS_WALL_AMOEBA(element) &&
	(LX + 2 * XS) / TILEX == nx &&
	(LY + 2 * YS) / TILEY == ny)
    {
      laser.num_edges--;
      ScanLaser();
    }

    break;
  }
}

void GameWon_MM()
{
  int hi_pos;
  boolean raise_level = FALSE;

#if 0
  if (local_player->MovPos)
    return;

  local_player->LevelSolved = FALSE;
#endif

  if (game_mm.energy_left)
  {
    if (setup.sound_loops)
      PlaySoundExt(SND_SIRR, SOUND_MAX_VOLUME, SOUND_MAX_RIGHT,
		   SND_CTRL_PLAY_LOOP);

    while (game_mm.energy_left > 0)
    {
      if (!setup.sound_loops)
	PlaySoundStereo(SND_SIRR, SOUND_MAX_RIGHT);

      /*
      if (game_mm.energy_left > 0 && !(game_mm.energy_left % 10))
	RaiseScore_MM(native_mm_level.score[SC_ZEITBONUS]);
      */

      RaiseScore_MM(5);

      game_mm.energy_left--;
      if (game_mm.energy_left >= 0)
      {
#if 0
	BlitBitmap(pix[PIX_DOOR], drawto,
		   DOOR_GFX_PAGEX5 + XX_ENERGY, DOOR_GFX_PAGEY1 + YY_ENERGY,
		   ENERGY_XSIZE, ENERGY_YSIZE - game_mm.energy_left,
		   DX_ENERGY, DY_ENERGY);
#endif
	redraw_mask |= REDRAW_DOOR_1;
      }

      BackToFront();
      Delay(10);
    }

    if (setup.sound_loops)
      StopSound(SND_SIRR);
  }
  else if (native_mm_level.time == 0)		/* level without time limit */
  {
    if (setup.sound_loops)
      PlaySoundExt(SND_SIRR, SOUND_MAX_VOLUME, SOUND_MAX_RIGHT,
		   SND_CTRL_PLAY_LOOP);

    while (TimePlayed < 999)
    {
      if (!setup.sound_loops)
	PlaySoundStereo(SND_SIRR, SOUND_MAX_RIGHT);
      if (TimePlayed < 999 && !(TimePlayed % 10))
	RaiseScore_MM(native_mm_level.score[SC_TIME_BONUS]);
      if (TimePlayed < 900 && !(TimePlayed % 10))
	TimePlayed += 10;
      else
	TimePlayed++;

      /*
      DrawText(DX_TIME, DY_TIME, int2str(TimePlayed, 3), FONT_TEXT_2);
      */

      BackToFront();
      Delay(10);
    }

    if (setup.sound_loops)
      StopSound(SND_SIRR);
  }

#if 0
  FadeSounds();
#endif

  CloseDoor(DOOR_CLOSE_1);

  Request("Level solved !", REQ_CONFIRM);

  if (level_nr == leveldir_current->handicap_level)
  {
    leveldir_current->handicap_level++;
    SaveLevelSetup_SeriesInfo();
  }

  if (level_editor_test_game)
    game_mm.score = -1;		/* no highscore when playing from editor */
  else if (level_nr < leveldir_current->last_level)
    raise_level = TRUE;		/* advance to next level */

  if ((hi_pos = NewHiScore_MM()) >= 0)
  {
    game_status = HALLOFFAME;

    // DrawHallOfFame(hi_pos);

    if (raise_level)
      level_nr++;
  }
  else
  {
    game_status = MAINMENU;

    if (raise_level)
      level_nr++;

    // DrawMainMenu();
  }

  BackToFront();
}

int NewHiScore_MM()
{
  int k, l;
  int position = -1;

  // LoadScore(level_nr);

  if (strcmp(setup.player_name, EMPTY_PLAYER_NAME) == 0 ||
      game_mm.score < highscore[MAX_SCORE_ENTRIES - 1].Score)
    return -1;

  for (k = 0; k < MAX_SCORE_ENTRIES; k++)
  {
    if (game_mm.score > highscore[k].Score)
    {
      /* player has made it to the hall of fame */

      if (k < MAX_SCORE_ENTRIES - 1)
      {
	int m = MAX_SCORE_ENTRIES - 1;

#ifdef ONE_PER_NAME
	for (l = k; l < MAX_SCORE_ENTRIES; l++)
	  if (!strcmp(setup.player_name, highscore[l].Name))
	    m = l;
	if (m == k)	/* player's new highscore overwrites his old one */
	  goto put_into_list;
#endif

	for (l = m; l>k; l--)
	{
	  strcpy(highscore[l].Name, highscore[l - 1].Name);
	  highscore[l].Score = highscore[l - 1].Score;
	}
      }

#ifdef ONE_PER_NAME
      put_into_list:
#endif
      strncpy(highscore[k].Name, setup.player_name, MAX_PLAYER_NAME_LEN);
      highscore[k].Name[MAX_PLAYER_NAME_LEN] = '\0';
      highscore[k].Score = game_mm.score;
      position = k;

      break;
    }

#ifdef ONE_PER_NAME
    else if (!strncmp(setup.player_name, highscore[k].Name,
		      MAX_PLAYER_NAME_LEN))
      break;	/* player already there with a higher score */
#endif

  }

  // if (position >= 0)
  //   SaveScore(level_nr);

  return position;
}

static void InitMovingField_MM(int x, int y, int direction)
{
  int newx = x + (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int newy = y + (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);

  MovDir[x][y] = direction;
  MovDir[newx][newy] = direction;

  if (Feld[newx][newy] == EL_EMPTY)
    Feld[newx][newy] = EL_BLOCKED;
}

static void Moving2Blocked_MM(int x, int y, int *goes_to_x, int *goes_to_y)
{
  int direction = MovDir[x][y];
  int newx = x + (direction == MV_LEFT ? -1 : direction == MV_RIGHT ? +1 : 0);
  int newy = y + (direction == MV_UP   ? -1 : direction == MV_DOWN  ? +1 : 0);

  *goes_to_x = newx;
  *goes_to_y = newy;
}

static void Blocked2Moving_MM(int x, int y,
			      int *comes_from_x, int *comes_from_y)
{
  int oldx = x, oldy = y;
  int direction = MovDir[x][y];

  if (direction == MV_LEFT)
    oldx++;
  else if (direction == MV_RIGHT)
    oldx--;
  else if (direction == MV_UP)
    oldy++;
  else if (direction == MV_DOWN)
    oldy--;

  *comes_from_x = oldx;
  *comes_from_y = oldy;
}

static int MovingOrBlocked2Element_MM(int x, int y)
{
  int element = Feld[x][y];

  if (element == EL_BLOCKED)
  {
    int oldx, oldy;

    Blocked2Moving_MM(x, y, &oldx, &oldy);

    return Feld[oldx][oldy];
  }

  return element;
}

#if 0
static void RemoveField(int x, int y)
{
  Feld[x][y] = EL_EMPTY;
  MovPos[x][y] = 0;
  MovDir[x][y] = 0;
  MovDelay[x][y] = 0;
}
#endif

static void RemoveMovingField_MM(int x, int y)
{
  int oldx = x, oldy = y, newx = x, newy = y;

  if (Feld[x][y] != EL_BLOCKED && !IS_MOVING(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    Moving2Blocked_MM(x, y, &newx, &newy);
    if (Feld[newx][newy] != EL_BLOCKED)
      return;
  }
  else if (Feld[x][y] == EL_BLOCKED)
  {
    Blocked2Moving_MM(x, y, &oldx, &oldy);
    if (!IS_MOVING(oldx, oldy))
      return;
  }

  Feld[oldx][oldy] = EL_EMPTY;
  Feld[newx][newy] = EL_EMPTY;
  MovPos[oldx][oldy] = MovDir[oldx][oldy] = MovDelay[oldx][oldy] = 0;
  MovPos[newx][newy] = MovDir[newx][newy] = MovDelay[newx][newy] = 0;

  DrawLevelField_MM(oldx, oldy);
  DrawLevelField_MM(newx, newy);
}

void PlaySoundLevel(int x, int y, int sound_nr)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int volume, stereo;
  int silence_distance = 8;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound_nr)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound_nr)))
    return;

  if (!IN_LEV_FIELD(x, y) ||
      sx < -silence_distance || sx >= SCR_FIELDX+silence_distance ||
      sy < -silence_distance || sy >= SCR_FIELDY+silence_distance)
    return;

  volume = SOUND_MAX_VOLUME;

#ifndef MSDOS
  stereo = (sx - SCR_FIELDX/2) * 12;
#else
  stereo = SOUND_MIDDLE + (2 * sx - (SCR_FIELDX - 1)) * 5;
  if (stereo > SOUND_MAX_RIGHT)
    stereo = SOUND_MAX_RIGHT;
  if (stereo < SOUND_MAX_LEFT)
    stereo = SOUND_MAX_LEFT;
#endif

  if (!IN_SCR_FIELD(sx, sy))
  {
    int dx = ABS(sx - SCR_FIELDX/2) - SCR_FIELDX/2;
    int dy = ABS(sy - SCR_FIELDY/2) - SCR_FIELDY/2;

    volume -= volume * (dx > dy ? dx : dy) / silence_distance;
  }

  PlaySoundExt(sound_nr, volume, stereo, SND_CTRL_PLAY_SOUND);
}

static void RaiseScore_MM(int value)
{
  game_mm.score += value;

#if 0
  DrawText(DX_SCORE, DY_SCORE, int2str(game_mm.score, 4),
	   FONT_TEXT_2);
#endif
}

void RaiseScoreElement_MM(int element)
{
  switch(element)
  {
    case EL_PACMAN:
    case EL_PACMAN_RIGHT:
    case EL_PACMAN_UP:
    case EL_PACMAN_LEFT:
    case EL_PACMAN_DOWN:
      RaiseScore_MM(native_mm_level.score[SC_PACMAN]);
      break;

    case EL_KEY:
      RaiseScore_MM(native_mm_level.score[SC_KEY]);
      break;

    case EL_KETTLE:
    case EL_CELL:
      RaiseScore_MM(native_mm_level.score[SC_COLLECTIBLE]);
      break;

    case EL_LIGHTBALL:
      RaiseScore_MM(native_mm_level.score[SC_LIGHTBALL]);
      break;

    default:
      break;
  }
}


/* ------------------------------------------------------------------------- */
/* Mirror Magic game engine snapshot handling functions                      */
/* ------------------------------------------------------------------------- */

void SaveEngineSnapshotValues_MM(ListNode **buffers)
{
  int x, y;

  engine_snapshot_mm.game_mm = game_mm;
  engine_snapshot_mm.laser = laser;

  for (x = 0; x < MAX_PLAYFIELD_WIDTH; x++)
  {
    for (y = 0; y < MAX_PLAYFIELD_HEIGHT; y++)
    {
      engine_snapshot_mm.Ur[x][y]    = Ur[x][y];
      engine_snapshot_mm.Hit[x][y]   = Hit[x][y];
      engine_snapshot_mm.Box[x][y]   = Box[x][y];
      engine_snapshot_mm.Angle[x][y] = Angle[x][y];
      engine_snapshot_mm.Frame[x][y] = Frame[x][y];
    }
  }

  engine_snapshot_mm.LX = LX;
  engine_snapshot_mm.LY = LY;
  engine_snapshot_mm.XS = XS;
  engine_snapshot_mm.YS = YS;
  engine_snapshot_mm.ELX = ELX;
  engine_snapshot_mm.ELY = ELY;
  engine_snapshot_mm.CT = CT;
  engine_snapshot_mm.Ct = Ct;

  engine_snapshot_mm.last_LX = last_LX;
  engine_snapshot_mm.last_LY = last_LY;
  engine_snapshot_mm.last_hit_mask = last_hit_mask;
  engine_snapshot_mm.hold_x = hold_x;
  engine_snapshot_mm.hold_y = hold_y;
  engine_snapshot_mm.pacman_nr = pacman_nr;

  engine_snapshot_mm.rotate_delay = rotate_delay;
  engine_snapshot_mm.pacman_delay = pacman_delay;
  engine_snapshot_mm.energy_delay = energy_delay;
  engine_snapshot_mm.overload_delay = overload_delay;
}

void LoadEngineSnapshotValues_MM()
{
  int x, y;

  /* stored engine snapshot buffers already restored at this point */

  game_mm = engine_snapshot_mm.game_mm;
  laser   = engine_snapshot_mm.laser;

  for (x = 0; x < MAX_PLAYFIELD_WIDTH; x++)
  {
    for (y = 0; y < MAX_PLAYFIELD_HEIGHT; y++)
    {
      Ur[x][y]    = engine_snapshot_mm.Ur[x][y];
      Hit[x][y]   = engine_snapshot_mm.Hit[x][y];
      Box[x][y]   = engine_snapshot_mm.Box[x][y];
      Angle[x][y] = engine_snapshot_mm.Angle[x][y];
      Frame[x][y] = engine_snapshot_mm.Frame[x][y];
    }
  }

  LX  = engine_snapshot_mm.LX;
  LY  = engine_snapshot_mm.LY;
  XS  = engine_snapshot_mm.XS;
  YS  = engine_snapshot_mm.YS;
  ELX = engine_snapshot_mm.ELX;
  ELY = engine_snapshot_mm.ELY;
  CT  = engine_snapshot_mm.CT;
  Ct  = engine_snapshot_mm.Ct;

  last_LX       = engine_snapshot_mm.last_LX;
  last_LY       = engine_snapshot_mm.last_LY;
  last_hit_mask = engine_snapshot_mm.last_hit_mask;
  hold_x        = engine_snapshot_mm.hold_x;
  hold_y        = engine_snapshot_mm.hold_y;
  pacman_nr     = engine_snapshot_mm.pacman_nr;

  rotate_delay   = engine_snapshot_mm.rotate_delay;
  pacman_delay   = engine_snapshot_mm.pacman_delay;
  energy_delay   = engine_snapshot_mm.energy_delay;
  overload_delay = engine_snapshot_mm.overload_delay;

  RedrawPlayfield_MM(TRUE);
}

static int getAngleFromTouchDelta(int dx, int dy,  int base)
{
  double pi = 3.141592653;
  double rad = atan2((double)-dy, (double)dx);
  double rad2 = (rad < 0 ? rad + 2 * pi : rad);
  double deg = rad2 * 180.0 / pi;

  return (int)(deg * base / 360.0 + 0.5) % base;
}

int getButtonFromTouchPosition(int x, int y, int dst_mx, int dst_my)
{
  // calculate start (source) position to be at the middle of the tile
  int src_mx = cSX + x * TILESIZE_VAR + TILESIZE_VAR / 2;
  int src_my = cSY + y * TILESIZE_VAR + TILESIZE_VAR / 2;
  int dx = dst_mx - src_mx;
  int dy = dst_my - src_my;
  int element;
  int base = 16;
  int phases = 16;
  int angle_old = -1;
  int angle_new = -1;
  int button = 0;
  int i;

  if (!IN_LEV_FIELD(x, y))
    return 0;

  element = Feld[x][y];

  if (!IS_MCDUFFIN(element) &&
      !IS_MIRROR(element) &&
      !IS_BEAMER(element) &&
      !IS_POLAR(element) &&
      !IS_POLAR_CROSS(element) &&
      !IS_DF_MIRROR(element))
    return 0;

  angle_old = get_element_angle(element);

  if (IS_MCDUFFIN(element))
  {
    angle_new = (dx > 0 && ABS(dy) < ABS(dx) ? ANG_RAY_RIGHT :
		 dy < 0 && ABS(dx) < ABS(dy) ? ANG_RAY_UP :
		 dx < 0 && ABS(dy) < ABS(dx) ? ANG_RAY_LEFT :
		 dy > 0 && ABS(dx) < ABS(dy) ? ANG_RAY_DOWN :
		 -1);
  }
  else if (IS_MIRROR(element) ||
	   IS_DF_MIRROR(element))
  {
    for (i = 0; i < laser.num_damages; i++)
    {
      if (laser.damage[i].x == x &&
	  laser.damage[i].y == y &&
	  ObjHit(x, y, HIT_POS_CENTER))
      {
	angle_old = get_mirrored_angle(laser.damage[i].angle, angle_old);
	angle_new = getAngleFromTouchDelta(dx, dy, base) % phases;

	break;
      }
    }
  }

  if (angle_new == -1)
  {
    if (IS_MIRROR(element) ||
	IS_DF_MIRROR(element) ||
	IS_POLAR(element))
      base = 32;

    if (IS_POLAR_CROSS(element))
      phases = 4;

    angle_new = getAngleFromTouchDelta(dx, dy, base) % phases;
  }

  button = (angle_new == angle_old ? 0 :
	    (angle_new - angle_old + phases) % phases < (phases / 2) ?
	    MB_LEFTBUTTON : MB_RIGHTBUTTON);

  return button;
}
