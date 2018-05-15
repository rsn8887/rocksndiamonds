// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// screens.c
// ============================================================================

#include "libgame/libgame.h"

#include "screens.h"
#include "events.h"
#include "game.h"
#include "tools.h"
#include "editor.h"
#include "files.h"
#include "tape.h"
#include "anim.h"
#include "network.h"
#include "init.h"
#include "config.h"


#define DEBUG_JOYSTICKS		0


/* screens on the info screen */
#define INFO_MODE_MAIN			0
#define INFO_MODE_TITLE			1
#define INFO_MODE_ELEMENTS		2
#define INFO_MODE_MUSIC			3
#define INFO_MODE_CREDITS		4
#define INFO_MODE_PROGRAM		5
#define INFO_MODE_VERSION		6
#define INFO_MODE_LEVELSET		7

#define MAX_INFO_MODES			8

/* screens on the setup screen */
/* (must match GFX_SPECIAL_ARG_SETUP_* values as defined in src/main.h) */
/* (should also match corresponding entries in src/conf_gfx.c) */
#define SETUP_MODE_MAIN			0
#define SETUP_MODE_GAME			1
#define SETUP_MODE_EDITOR		2
#define SETUP_MODE_GRAPHICS		3
#define SETUP_MODE_SOUND		4
#define SETUP_MODE_ARTWORK		5
#define SETUP_MODE_INPUT		6
#define SETUP_MODE_TOUCH		7
#define SETUP_MODE_SHORTCUTS		8
#define SETUP_MODE_SHORTCUTS_1		9
#define SETUP_MODE_SHORTCUTS_2		10
#define SETUP_MODE_SHORTCUTS_3		11
#define SETUP_MODE_SHORTCUTS_4		12
#define SETUP_MODE_SHORTCUTS_5		13

/* sub-screens on the setup screen (generic) */
#define SETUP_MODE_CHOOSE_ARTWORK	14
#define SETUP_MODE_CHOOSE_OTHER		15

/* sub-screens on the setup screen (specific) */
#define SETUP_MODE_CHOOSE_GAME_SPEED	16
#define SETUP_MODE_CHOOSE_SCROLL_DELAY	17
#define SETUP_MODE_CHOOSE_SNAPSHOT_MODE	18
#define SETUP_MODE_CHOOSE_WINDOW_SIZE	19
#define SETUP_MODE_CHOOSE_SCALING_TYPE	20
#define SETUP_MODE_CHOOSE_RENDERING	21
#define SETUP_MODE_CHOOSE_GRAPHICS	22
#define SETUP_MODE_CHOOSE_SOUNDS	23
#define SETUP_MODE_CHOOSE_MUSIC		24
#define SETUP_MODE_CHOOSE_VOLUME_SIMPLE	25
#define SETUP_MODE_CHOOSE_VOLUME_LOOPS	26
#define SETUP_MODE_CHOOSE_VOLUME_MUSIC	27
#define SETUP_MODE_CHOOSE_TOUCH_CONTROL	28
#define SETUP_MODE_CHOOSE_MOVE_DISTANCE	29
#define SETUP_MODE_CHOOSE_DROP_DISTANCE	30

#define MAX_SETUP_MODES			31

#define MAX_MENU_MODES			MAX(MAX_INFO_MODES, MAX_SETUP_MODES)

/* setup screen titles */
#define STR_SETUP_MAIN			"Setup"
#define STR_SETUP_GAME			"Game & Menu"
#define STR_SETUP_EDITOR		"Editor"
#define STR_SETUP_GRAPHICS		"Graphics"
#define STR_SETUP_SOUND			"Sound & Music"
#define STR_SETUP_ARTWORK		"Custom Artwork"
#define STR_SETUP_INPUT			"Input Devices"
#define STR_SETUP_TOUCH			"Touch Controls"
#define STR_SETUP_SHORTCUTS		"Key Shortcuts"
#define STR_SETUP_EXIT			"Exit"
#define STR_SETUP_SAVE_AND_EXIT		"Save and Exit"

#define STR_SETUP_CHOOSE_GAME_SPEED	"Game Speed"
#define STR_SETUP_CHOOSE_SCROLL_DELAY	"Scroll Delay"
#define STR_SETUP_CHOOSE_SNAPSHOT_MODE	"Snapshot Mode"
#define STR_SETUP_CHOOSE_WINDOW_SIZE	"Window Scaling"
#define STR_SETUP_CHOOSE_SCALING_TYPE	"Anti-Aliasing"
#define STR_SETUP_CHOOSE_RENDERING	"Rendering Mode"
#define STR_SETUP_CHOOSE_VOLUME_SIMPLE	"Sound Volume"
#define STR_SETUP_CHOOSE_VOLUME_LOOPS	"Loops Volume"
#define STR_SETUP_CHOOSE_VOLUME_MUSIC	"Music Volume"
#define STR_SETUP_CHOOSE_TOUCH_CONTROL	"Control Type"
#define STR_SETUP_CHOOSE_MOVE_DISTANCE	"Move Distance"
#define STR_SETUP_CHOOSE_DROP_DISTANCE	"Drop Distance"

/* for input setup functions */
#define SETUPINPUT_SCREEN_POS_START	0
#define SETUPINPUT_SCREEN_POS_END	(SCR_FIELDY - 4)
#define SETUPINPUT_SCREEN_POS_EMPTY1	(SETUPINPUT_SCREEN_POS_START + 3)
#define SETUPINPUT_SCREEN_POS_EMPTY2	(SETUPINPUT_SCREEN_POS_END - 1)

#define MENU_SETUP_FONT_TITLE		FONT_TEXT_1
#define MENU_SETUP_FONT_TEXT		FONT_TITLE_2

/* for various menu stuff  */
#define MENU_SCREEN_START_XPOS		1
#define MENU_SCREEN_START_YPOS		2
#define MENU_SCREEN_VALUE_XPOS		(SCR_FIELDX - 3)
#define MENU_SCREEN_MAX_XPOS		(SCR_FIELDX - 1)
#define MENU_TITLE1_YPOS		8
#define MENU_TITLE2_YPOS		46
#define MENU_INFO_FONT_TITLE		FONT_TEXT_1
#define MENU_INFO_FONT_HEAD		FONT_TEXT_2
#define MENU_INFO_FONT_TEXT		FONT_TEXT_3
#define MENU_INFO_FONT_FOOT		FONT_TEXT_4
#define MENU_INFO_SPACE_HEAD		(menu.headline2_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_LEFT	(menu.left_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_RIGHT	(menu.right_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_TOP	(menu.top_spacing_info[info_mode])
#define MENU_SCREEN_INFO_SPACE_BOTTOM	(menu.bottom_spacing_info[info_mode])
#define MENU_SCREEN_INFO_YSTART1	MENU_SCREEN_INFO_SPACE_TOP
#define MENU_SCREEN_INFO_YSTART2	(MENU_SCREEN_INFO_YSTART1 +	       \
					 getMenuTextStep(MENU_INFO_SPACE_HEAD, \
							 MENU_INFO_FONT_TITLE))
#define MENU_SCREEN_INFO_YSTEP		(TILEY + 4)
#define MENU_SCREEN_INFO_YBOTTOM	(SYSIZE - MENU_SCREEN_INFO_SPACE_BOTTOM)
#define MENU_SCREEN_INFO_YSIZE		(MENU_SCREEN_INFO_YBOTTOM -	\
					 MENU_SCREEN_INFO_YSTART2 -	\
					 TILEY / 2)
#define MAX_INFO_ELEMENTS_ON_SCREEN	128
#define STD_INFO_ELEMENTS_ON_SCREEN	(MENU_SCREEN_INFO_YSIZE /	\
					 MENU_SCREEN_INFO_YSTEP)
#define NUM_INFO_ELEMENTS_FROM_CONF	\
  (menu.list_size_info[GFX_SPECIAL_ARG_INFO_ELEMENTS] > 0 ?		\
   menu.list_size_info[GFX_SPECIAL_ARG_INFO_ELEMENTS] :			\
   MAX_MENU_ENTRIES_ON_SCREEN)
#define NUM_INFO_ELEMENTS_ON_SCREEN	MIN(MIN(STD_INFO_ELEMENTS_ON_SCREEN, \
                                                MAX_INFO_ELEMENTS_ON_SCREEN), \
					    NUM_INFO_ELEMENTS_FROM_CONF)
#define MAX_MENU_ENTRIES_ON_SCREEN	(SCR_FIELDY - MENU_SCREEN_START_YPOS)
#define MAX_MENU_TEXT_LENGTH_BIG	13
#define MAX_MENU_TEXT_LENGTH_MEDIUM	(MAX_MENU_TEXT_LENGTH_BIG * 2)

/* buttons and scrollbars identifiers */
#define SCREEN_CTRL_ID_PREV_LEVEL	0
#define SCREEN_CTRL_ID_NEXT_LEVEL	1
#define SCREEN_CTRL_ID_PREV_PLAYER	2
#define SCREEN_CTRL_ID_NEXT_PLAYER	3
#define SCREEN_CTRL_ID_SCROLL_UP	4
#define SCREEN_CTRL_ID_SCROLL_DOWN	5
#define SCREEN_CTRL_ID_SCROLL_VERTICAL	6

#define NUM_SCREEN_GADGETS		7

#define NUM_SCREEN_MENUBUTTONS		4
#define NUM_SCREEN_SCROLLBUTTONS	2
#define NUM_SCREEN_SCROLLBARS		1

#define SCREEN_MASK_MAIN		(1 << 0)
#define SCREEN_MASK_INPUT		(1 << 1)

/* graphic position and size values for buttons and scrollbars */
#define SC_MENUBUTTON_XSIZE		TILEX
#define SC_MENUBUTTON_YSIZE		TILEY

#define SC_SCROLLBUTTON_XSIZE		TILEX
#define SC_SCROLLBUTTON_YSIZE		TILEY

#define SC_SCROLLBAR_XPOS		(SXSIZE - SC_SCROLLBUTTON_XSIZE)

#define SC_SCROLL_VERTICAL_XSIZE	SC_SCROLLBUTTON_XSIZE
#define SC_SCROLL_VERTICAL_YSIZE	((MAX_MENU_ENTRIES_ON_SCREEN - 2) * \
					 SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_UP_XPOS		SC_SCROLLBAR_XPOS
#define SC_SCROLL_UP_YPOS		(2 * SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_VERTICAL_XPOS		SC_SCROLLBAR_XPOS
#define SC_SCROLL_VERTICAL_YPOS		(SC_SCROLL_UP_YPOS + \
					 SC_SCROLLBUTTON_YSIZE)

#define SC_SCROLL_DOWN_XPOS		SC_SCROLLBAR_XPOS
#define SC_SCROLL_DOWN_YPOS		(SC_SCROLL_VERTICAL_YPOS + \
					 SC_SCROLL_VERTICAL_YSIZE)

#define SC_BORDER_SIZE			14


/* forward declarations of internal functions */
static void HandleScreenGadgets(struct GadgetInfo *);
static void HandleSetupScreen_Generic(int, int, int, int, int);
static void HandleSetupScreen_Input(int, int, int, int, int);
static void CustomizeKeyboard(int);
static void ConfigureJoystick(int);
static void execSetupGame(void);
static void execSetupGraphics(void);
static void execSetupSound(void);
static void execSetupTouch(void);
static void execSetupArtwork(void);
static void HandleChooseTree(int, int, int, int, int, TreeInfo **);

static void DrawChooseLevelSet(void);
static void DrawChooseLevelNr(void);
static void DrawInfoScreen(void);
static void DrawSetupScreen(void);

static void DrawInfoScreen_NotAvailable(char *, char *);
static void DrawInfoScreen_HelpAnim(int, int, boolean);
static void DrawInfoScreen_HelpText(int, int, int, int);
static void HandleInfoScreen_Main(int, int, int, int, int);
static void HandleInfoScreen_TitleScreen(int);
static void HandleInfoScreen_Elements(int);
static void HandleInfoScreen_Music(int);
static void HandleInfoScreen_Credits(int);
static void HandleInfoScreen_Program(int);
static void HandleInfoScreen_Version(int);

static void MapScreenMenuGadgets(int);
static void MapScreenGadgets(int);
static void MapScreenTreeGadgets(TreeInfo *);

static struct GadgetInfo *screen_gadget[NUM_SCREEN_GADGETS];

static int info_mode = INFO_MODE_MAIN;
static int setup_mode = SETUP_MODE_MAIN;

static TreeInfo *window_sizes = NULL;
static TreeInfo *window_size_current = NULL;

static TreeInfo *scaling_types = NULL;
static TreeInfo *scaling_type_current = NULL;

static TreeInfo *rendering_modes = NULL;
static TreeInfo *rendering_mode_current = NULL;

static TreeInfo *scroll_delays = NULL;
static TreeInfo *scroll_delay_current = NULL;

static TreeInfo *snapshot_modes = NULL;
static TreeInfo *snapshot_mode_current = NULL;

static TreeInfo *game_speeds = NULL;
static TreeInfo *game_speed_current = NULL;

static TreeInfo *volumes_simple = NULL;
static TreeInfo *volume_simple_current = NULL;

static TreeInfo *volumes_loops = NULL;
static TreeInfo *volume_loops_current = NULL;

static TreeInfo *volumes_music = NULL;
static TreeInfo *volume_music_current = NULL;

static TreeInfo *touch_controls = NULL;
static TreeInfo *touch_control_current = NULL;

static TreeInfo *move_distances = NULL;
static TreeInfo *move_distance_current = NULL;

static TreeInfo *drop_distances = NULL;
static TreeInfo *drop_distance_current = NULL;

static TreeInfo *level_number = NULL;
static TreeInfo *level_number_current = NULL;

static struct
{
  int value;
  char *text;
} window_sizes_list[] =
{
  {	50,	"50 %"				},
  {	80,	"80 %"				},
  {	90,	"90 %"				},
  {	100,	"100 % (Default)"		},
  {	110,	"110 %"				},
  {	120,	"120 %"				},
  {	130,	"130 %"				},
  {	140,	"140 %"				},
  {	150,	"150 %"				},
  {	200,	"200 %"				},
  {	250,	"250 %"				},
  {	300,	"300 %"				},

  {	-1,	NULL				},
};

static struct
{
  char *value;
  char *text;
} scaling_types_list[] =
{
  {	SCALING_QUALITY_NEAREST, "Off"		},
  {	SCALING_QUALITY_LINEAR,	 "Linear"	},
  {	SCALING_QUALITY_BEST,	 "Anisotropic"	},

  {	NULL,			 NULL		},
};

static struct
{
  char *value;
  char *text;
} rendering_modes_list[] =
{
  {	STR_SPECIAL_RENDERING_OFF,	"Off (May show artifacts, fast)" },
  {	STR_SPECIAL_RENDERING_BITMAP,	"Bitmap/Texture mode (slower)"	 },
#if DEBUG
  // this mode may work under certain conditions, but does not work on Windows
  {	STR_SPECIAL_RENDERING_TARGET,	"Target Texture mode (slower)"	 },
#endif
  {	STR_SPECIAL_RENDERING_DOUBLE,	"Double Texture mode (slower)"	 },

  {	NULL,				 NULL				 },
};

static struct
{
  int value;
  char *text;
} game_speeds_list[] =
{
#if 1
  {	30,	"Very Slow"			},
  {	25,	"Slow"				},
  {	20,	"Normal"			},
  {	15,	"Fast"				},
  {	10,	"Very Fast"			},
#else
  {	1000,	"1/1s (Extremely Slow)"		},
  {	500,	"1/2s"				},
  {	200,	"1/5s"				},
  {	100,	"1/10s"				},
  {	50,	"1/20s"				},
  {	29,	"1/35s (Original Supaplex)"	},
  {	25,	"1/40s"				},
  {	20,	"1/50s (Normal Speed)"		},
  {	14,	"1/70s (Maximum Supaplex)"	},
  {	10,	"1/100s"			},
  {	5,	"1/200s"			},
  {	2,	"1/500s"			},
  {	1,	"1/1000s (Extremely Fast)"	},
#endif

  {	-1,	NULL				},
};

static struct
{
  int value;
  char *text;
} scroll_delays_list[] =
{
  {	0,	"0 Tiles (No Scroll Delay)"	},
  {	1,	"1 Tile"			},
  {	2,	"2 Tiles"			},
  {	3,	"3 Tiles (Default)"		},
  {	4,	"4 Tiles"			},
  {	5,	"5 Tiles"			},
  {	6,	"6 Tiles"			},
  {	7,	"7 Tiles"			},
  {	8,	"8 Tiles (Maximum Scroll Delay)"},

  {	-1,	NULL				},
};

static struct
{
  char *value;
  char *text;
} snapshot_modes_list[] =
{
  {	STR_SNAPSHOT_MODE_OFF,			"Off"		},
  {	STR_SNAPSHOT_MODE_EVERY_STEP,		"Every Step"	},
  {	STR_SNAPSHOT_MODE_EVERY_MOVE,		"Every Move"	},
  {	STR_SNAPSHOT_MODE_EVERY_COLLECT,	"Every Collect"	},

  {	NULL,			 		NULL		},
};

static struct
{
  int value;
  char *text;
} volumes_list[] =
{
  {	0,	"0 %"				},
  {	1,	"1 %"				},
  {	2,	"2 %"				},
  {	5,	"5 %"				},
  {	10,	"10 %"				},
  {	20,	"20 %"				},
  {	30,	"30 %"				},
  {	40,	"40 %"				},
  {	50,	"50 %"				},
  {	60,	"60 %"				},
  {	70,	"70 %"				},
  {	80,	"80 %"				},
  {	90,	"90 %"				},
  {	100,	"100 %"				},

  {	-1,	NULL				},
};

static struct
{
  char *value;
  char *text;
} touch_controls_list[] =
{
  {	TOUCH_CONTROL_OFF,		"Off"			},
  {	TOUCH_CONTROL_VIRTUAL_BUTTONS,	"Virtual Buttons"	},
  {	TOUCH_CONTROL_WIPE_GESTURES,	"Wipe Gestures"		},
  {	TOUCH_CONTROL_FOLLOW_FINGER,	"Follow Finger"		},

  {	NULL,			 	NULL			},
};

static struct
{
  int value;
  char *text;
} distances_list[] =
{
  {	1,	"1 %"				},
  {	2,	"2 %"				},
  {	3,	"3 %"				},
  {	4,	"4 %"				},
  {	5,	"5 %"				},
  {	10,	"10 %"				},
  {	15,	"15 %"				},
  {	20,	"20 %"				},
  {	25,	"25 %"				},

  {	-1,	NULL				},
};

#define DRAW_MODE(s)		((s) >= GAME_MODE_MAIN &&		\
				 (s) <= GAME_MODE_SETUP ? (s) :		\
				 (s) == GAME_MODE_PSEUDO_TYPENAME ?	\
				 GAME_MODE_MAIN : GAME_MODE_DEFAULT)

/* (there are no draw offset definitions needed for INFO_MODE_TITLE) */
#define DRAW_MODE_INFO(i)	((i) >= INFO_MODE_TITLE &&		\
				 (i) <= INFO_MODE_LEVELSET ? (i) :	\
				 INFO_MODE_MAIN)

#define DRAW_MODE_SETUP(i)	((i) >= SETUP_MODE_MAIN &&		\
				 (i) <= SETUP_MODE_SHORTCUTS_5 ? (i) :	\
				 (i) >= SETUP_MODE_CHOOSE_GRAPHICS &&	\
				 (i) <= SETUP_MODE_CHOOSE_MUSIC ?	\
				 SETUP_MODE_CHOOSE_ARTWORK :		\
				 SETUP_MODE_CHOOSE_OTHER)

#define DRAW_XOFFSET_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ?	\
				 menu.draw_xoffset[GAME_MODE_INFO] :	\
				 menu.draw_xoffset_info[DRAW_MODE_INFO(i)])
#define DRAW_YOFFSET_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ?	\
				 menu.draw_yoffset[GAME_MODE_INFO] :	\
				 menu.draw_yoffset_info[DRAW_MODE_INFO(i)])
#define EXTRA_SPACING_INFO(i)	(DRAW_MODE_INFO(i) == INFO_MODE_MAIN ? \
				 menu.extra_spacing[GAME_MODE_INFO] :	\
				 menu.extra_spacing_info[DRAW_MODE_INFO(i)])

#define DRAW_XOFFSET_SETUP(i)	(DRAW_MODE_SETUP(i) == SETUP_MODE_MAIN ? \
				 menu.draw_xoffset[GAME_MODE_SETUP] :	\
				 menu.draw_xoffset_setup[DRAW_MODE_SETUP(i)])
#define DRAW_YOFFSET_SETUP(i)	(DRAW_MODE_SETUP(i) == SETUP_MODE_MAIN ? \
				 menu.draw_yoffset[GAME_MODE_SETUP] :	\
				 menu.draw_yoffset_setup[DRAW_MODE_SETUP(i)])
#define EXTRA_SPACING_SETUP(i)	(DRAW_MODE_SETUP(i) == SETUP_MODE_MAIN ? \
				 menu.extra_spacing[GAME_MODE_SETUP] :	\
				 menu.extra_spacing_setup[DRAW_MODE_SETUP(i)])

#define DRAW_XOFFSET(s)		((s) == GAME_MODE_INFO ?		\
				 DRAW_XOFFSET_INFO(info_mode) :		\
				 (s) == GAME_MODE_SETUP ?		\
				 DRAW_XOFFSET_SETUP(setup_mode) :	\
				 menu.draw_xoffset[DRAW_MODE(s)])
#define DRAW_YOFFSET(s)		((s) == GAME_MODE_INFO ?		\
				 DRAW_YOFFSET_INFO(info_mode) :		\
				 (s) == GAME_MODE_SETUP ?		\
				 DRAW_YOFFSET_SETUP(setup_mode) :	\
				 menu.draw_yoffset[DRAW_MODE(s)])
#define EXTRA_SPACING(s)	((s) == GAME_MODE_INFO ?		\
				 EXTRA_SPACING_INFO(info_mode) :	\
				 (s) == GAME_MODE_SETUP ?		\
				 EXTRA_SPACING_SETUP(setup_mode) :	\
				 menu.extra_spacing[DRAW_MODE(s)])

#define mSX			(SX + DRAW_XOFFSET(game_status))
#define mSY			(SY + DRAW_YOFFSET(game_status))

#define NUM_MENU_ENTRIES_ON_SCREEN (menu.list_size[game_status] > 2 ?	\
				    menu.list_size[game_status] :	\
				    MAX_MENU_ENTRIES_ON_SCREEN)

#define IN_VIS_MENU(x, y)	IN_FIELD(x, y, SCR_FIELDX,		\
					 NUM_MENU_ENTRIES_ON_SCREEN)


/* title display and control definitions */

#define MAX_NUM_TITLE_SCREENS	(2 * MAX_NUM_TITLE_IMAGES +		\
				 2 * MAX_NUM_TITLE_MESSAGES)

#define NO_DIRECT_LEVEL_SELECT	(-1)


static int num_title_screens = 0;

struct TitleControlInfo
{
  boolean is_image;
  boolean initial;
  boolean first;
  int local_nr;
  int sort_priority;
};

struct TitleControlInfo title_controls[MAX_NUM_TITLE_SCREENS];

/* main menu display and control definitions */

#define MAIN_CONTROL_NAME			0
#define MAIN_CONTROL_LEVELS			1
#define MAIN_CONTROL_SCORES			2
#define MAIN_CONTROL_EDITOR			3
#define MAIN_CONTROL_INFO			4
#define MAIN_CONTROL_GAME			5
#define MAIN_CONTROL_SETUP			6
#define MAIN_CONTROL_QUIT			7
#define MAIN_CONTROL_PREV_LEVEL			8
#define MAIN_CONTROL_NEXT_LEVEL			9
#define MAIN_CONTROL_FIRST_LEVEL		10
#define MAIN_CONTROL_LAST_LEVEL			11
#define MAIN_CONTROL_LEVEL_NUMBER		12
#define MAIN_CONTROL_LEVEL_INFO_1		13
#define MAIN_CONTROL_LEVEL_INFO_2		14
#define MAIN_CONTROL_LEVEL_NAME			15
#define MAIN_CONTROL_LEVEL_AUTHOR		16
#define MAIN_CONTROL_LEVEL_YEAR			17
#define MAIN_CONTROL_LEVEL_IMPORTED_FROM	18
#define MAIN_CONTROL_LEVEL_IMPORTED_BY		19
#define MAIN_CONTROL_LEVEL_TESTED_BY		20
#define MAIN_CONTROL_TITLE_1			21
#define MAIN_CONTROL_TITLE_2			22
#define MAIN_CONTROL_TITLE_3			23

static char str_main_text_name[10];
static char str_main_text_first_level[10];
static char str_main_text_last_level[10];
static char str_main_text_level_number[10];

static char *main_text_name			= str_main_text_name;
static char *main_text_first_level		= str_main_text_first_level;
static char *main_text_last_level		= str_main_text_last_level;
static char *main_text_level_number		= str_main_text_level_number;
static char *main_text_levels			= "Levelset";
static char *main_text_scores			= "Hall Of Fame";
static char *main_text_editor			= "Level Creator";
static char *main_text_info			= "Info Screen";
static char *main_text_game			= "Start Game";
static char *main_text_setup			= "Setup";
static char *main_text_quit			= "Quit";
static char *main_text_level_name		= level.name;
static char *main_text_level_author		= level.author;
static char *main_text_level_year		= NULL;
static char *main_text_level_imported_from	= NULL;
static char *main_text_level_imported_by	= NULL;
static char *main_text_level_tested_by		= NULL;
static char *main_text_title_1			= NULL;
static char *main_text_title_2			= NULL;
static char *main_text_title_3			= NULL;

struct MainControlInfo
{
  int nr;

  struct MenuPosInfo *pos_button;
  int button_graphic;

  struct TextPosInfo *pos_text;
  char **text;

  struct TextPosInfo *pos_input;
  char **input;
};

static struct MainControlInfo main_controls[] =
{
  {
    MAIN_CONTROL_NAME,
    &menu.main.button.name,		IMG_MENU_BUTTON_NAME,
    &menu.main.text.name,		&main_text_name,
    &menu.main.input.name,		&setup.player_name,
  },
  {
    MAIN_CONTROL_LEVELS,
    &menu.main.button.levels,		IMG_MENU_BUTTON_LEVELS,
    &menu.main.text.levels,		&main_text_levels,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_SCORES,
    &menu.main.button.scores,		IMG_MENU_BUTTON_SCORES,
    &menu.main.text.scores,		&main_text_scores,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_EDITOR,
    &menu.main.button.editor,		IMG_MENU_BUTTON_EDITOR,
    &menu.main.text.editor,		&main_text_editor,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_INFO,
    &menu.main.button.info,		IMG_MENU_BUTTON_INFO,
    &menu.main.text.info,		&main_text_info,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_GAME,
    &menu.main.button.game,		IMG_MENU_BUTTON_GAME,
    &menu.main.text.game,		&main_text_game,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_SETUP,
    &menu.main.button.setup,		IMG_MENU_BUTTON_SETUP,
    &menu.main.text.setup,		&main_text_setup,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_QUIT,
    &menu.main.button.quit,		IMG_MENU_BUTTON_QUIT,
    &menu.main.text.quit,		&main_text_quit,
    NULL,				NULL,
  },
#if 0
  /* (these two buttons are real gadgets) */
  {
    MAIN_CONTROL_PREV_LEVEL,
    &menu.main.button.prev_level,	IMG_MENU_BUTTON_PREV_LEVEL,
    NULL,				NULL,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_NEXT_LEVEL,
    &menu.main.button.next_level,	IMG_MENU_BUTTON_NEXT_LEVEL,
    NULL,				NULL,
    NULL,				NULL,
  },
#endif
  {
    MAIN_CONTROL_FIRST_LEVEL,
    NULL,				-1,
    &menu.main.text.first_level,	&main_text_first_level,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LAST_LEVEL,
    NULL,				-1,
    &menu.main.text.last_level,		&main_text_last_level,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_NUMBER,
    NULL,				-1,
    &menu.main.text.level_number,	&main_text_level_number,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_INFO_1,
    NULL,				-1,
    &menu.main.text.level_info_1,	NULL,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_INFO_2,
    NULL,				-1,
    &menu.main.text.level_info_2,	NULL,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_NAME,
    NULL,				-1,
    &menu.main.text.level_name,		&main_text_level_name,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_AUTHOR,
    NULL,				-1,
    &menu.main.text.level_author,	&main_text_level_author,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_YEAR,
    NULL,				-1,
    &menu.main.text.level_year,		&main_text_level_year,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_IMPORTED_FROM,
    NULL,				-1,
    &menu.main.text.level_imported_from, &main_text_level_imported_from,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_IMPORTED_BY,
    NULL,				-1,
    &menu.main.text.level_imported_by,	&main_text_level_imported_by,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_LEVEL_TESTED_BY,
    NULL,				-1,
    &menu.main.text.level_tested_by,	&main_text_level_tested_by,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_TITLE_1,
    NULL,				-1,
    &menu.main.text.title_1,		&main_text_title_1,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_TITLE_2,
    NULL,				-1,
    &menu.main.text.title_2,		&main_text_title_2,
    NULL,				NULL,
  },
  {
    MAIN_CONTROL_TITLE_3,
    NULL,				-1,
    &menu.main.text.title_3,		&main_text_title_3,
    NULL,				NULL,
  },

  {
    -1,
    NULL,				-1,
    NULL,				NULL,
    NULL,				NULL,
  }
};


static int getTitleScreenGraphic(int nr, boolean initial)
{
  return (initial ? IMG_TITLESCREEN_INITIAL_1 : IMG_TITLESCREEN_1) + nr;
}

static struct TitleMessageInfo *getTitleMessageInfo(int nr, boolean initial)
{
  return (initial ? &titlemessage_initial[nr] : &titlemessage[nr]);
}

#if 0
static int getTitleScreenGameMode(boolean initial)
{
  return (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
}
#endif

static int getTitleMessageGameMode(boolean initial)
{
  return (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
}

static int getTitleAnimMode(struct TitleControlInfo *tci)
{
  int base = (tci->initial ? GAME_MODE_TITLE_INITIAL_1 : GAME_MODE_TITLE_1);

  return base + tci->local_nr;
}

#if 0
static int getTitleScreenBackground(boolean initial)
{
  return (initial ? IMG_BACKGROUND_TITLE_INITIAL : IMG_BACKGROUND_TITLE);
}
#endif

#if 0
static int getTitleMessageBackground(int nr, boolean initial)
{
  return (initial ? IMG_BACKGROUND_TITLE_INITIAL : IMG_BACKGROUND_TITLE);
}
#endif

static int getTitleBackground(int nr, boolean initial, boolean is_image)
{
  int base = (is_image ?
	      (initial ? IMG_BACKGROUND_TITLESCREEN_INITIAL_1 :
	                 IMG_BACKGROUND_TITLESCREEN_1) :
	      (initial ? IMG_BACKGROUND_TITLEMESSAGE_INITIAL_1 :
	                 IMG_BACKGROUND_TITLEMESSAGE_1));
  int graphic_global = (initial ? IMG_BACKGROUND_TITLE_INITIAL :
			          IMG_BACKGROUND_TITLE);
  int graphic_local = base + nr;

  if (graphic_info[graphic_local].bitmap != NULL)
    return graphic_local;

  if (graphic_info[graphic_global].bitmap != NULL)
    return graphic_global;

  return IMG_UNDEFINED;
}

static int getTitleSound(struct TitleControlInfo *tci)
{
  boolean is_image = tci->is_image;
  int initial = tci->initial;
  int nr = tci->local_nr;
  int mode = (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
  int base = (is_image ?
	      (initial ? SND_BACKGROUND_TITLESCREEN_INITIAL_1 :
	                 SND_BACKGROUND_TITLESCREEN_1) :
	      (initial ? SND_BACKGROUND_TITLEMESSAGE_INITIAL_1 :
	                 SND_BACKGROUND_TITLEMESSAGE_1));
  int sound_global = menu.sound[mode];
  int sound_local = base + nr;

#if 0
  printf("::: %d, %d, %d: %d ['%s'], %d ['%s']\n",
	 nr, initial, is_image,
	 sound_global, getSoundListEntry(sound_global)->filename,
	 sound_local, getSoundListEntry(sound_local)->filename);
#endif

  if (!strEqual(getSoundListEntry(sound_local)->filename, UNDEFINED_FILENAME))
    return sound_local;

  if (!strEqual(getSoundListEntry(sound_global)->filename, UNDEFINED_FILENAME))
    return sound_global;

  return SND_UNDEFINED;
}

static int getTitleMusic(struct TitleControlInfo *tci)
{
  boolean is_image = tci->is_image;
  int initial = tci->initial;
  int nr = tci->local_nr;
  int mode = (initial ? GAME_MODE_TITLE_INITIAL : GAME_MODE_TITLE);
  int base = (is_image ?
	      (initial ? MUS_BACKGROUND_TITLESCREEN_INITIAL_1 :
	                 MUS_BACKGROUND_TITLESCREEN_1) :
	      (initial ? MUS_BACKGROUND_TITLEMESSAGE_INITIAL_1 :
	                 MUS_BACKGROUND_TITLEMESSAGE_1));
  int music_global = menu.music[mode];
  int music_local = base + nr;

#if 0
  printf("::: %d, %d, %d: %d ['%s'], %d ['%s']\n",
	 nr, initial, is_image,
	 music_global, getMusicListEntry(music_global)->filename,
	 music_local, getMusicListEntry(music_local)->filename);
#endif

  if (!strEqual(getMusicListEntry(music_local)->filename, UNDEFINED_FILENAME))
    return music_local;

  if (!strEqual(getMusicListEntry(music_global)->filename, UNDEFINED_FILENAME))
    return music_global;

  return MUS_UNDEFINED;
}

static struct TitleFadingInfo getTitleFading(struct TitleControlInfo *tci)
{
  boolean is_image = tci->is_image;
  boolean initial = tci->initial;
  boolean first = tci->first;
  int nr = tci->local_nr;
  struct TitleMessageInfo tmi;
  struct TitleFadingInfo ti;

  tmi = (is_image ? (initial ? (first ?
				titlescreen_initial_first[nr] :
				titlescreen_initial[nr])
		             : (first ?
				titlescreen_first[nr] :
				titlescreen[nr]))
	          : (initial ? (first ?
				titlemessage_initial_first[nr] :
				titlemessage_initial[nr])
		             : (first ?
				titlemessage_first[nr] :
				titlemessage[nr])));

  ti.fade_mode  = tmi.fade_mode;
  ti.fade_delay = tmi.fade_delay;
  ti.post_delay = tmi.post_delay;
  ti.auto_delay = tmi.auto_delay;

  return ti;
}

static int compareTitleControlInfo(const void *object1, const void *object2)
{
  const struct TitleControlInfo *tci1 = (struct TitleControlInfo *)object1;
  const struct TitleControlInfo *tci2 = (struct TitleControlInfo *)object2;
  int compare_result;

  if (tci1->initial != tci2->initial)
    compare_result = (tci1->initial ? -1 : +1);
  else if (tci1->sort_priority != tci2->sort_priority)
    compare_result = tci1->sort_priority - tci2->sort_priority;
  else if (tci1->is_image != tci2->is_image)
    compare_result = (tci1->is_image ? -1 : +1);
  else
    compare_result = tci1->local_nr - tci2->local_nr;

  return compare_result;
}

static void InitializeTitleControlsExt_AddTitleInfo(boolean is_image,
						    boolean initial,
						    int nr, int sort_priority)
{
  title_controls[num_title_screens].is_image = is_image;
  title_controls[num_title_screens].initial = initial;
  title_controls[num_title_screens].local_nr = nr;
  title_controls[num_title_screens].sort_priority = sort_priority;

  title_controls[num_title_screens].first = FALSE;	/* will be set later */

  num_title_screens++;
}

static void InitializeTitleControls_CheckTitleInfo(boolean initial)
{
  int i;

  for (i = 0; i < MAX_NUM_TITLE_IMAGES; i++)
  {
    int graphic = getTitleScreenGraphic(i, initial);
    Bitmap *bitmap = graphic_info[graphic].bitmap;
    int sort_priority = graphic_info[graphic].sort_priority;

    if (bitmap != NULL)
      InitializeTitleControlsExt_AddTitleInfo(TRUE, initial, i, sort_priority);
  }

  for (i = 0; i < MAX_NUM_TITLE_MESSAGES; i++)
  {
    struct TitleMessageInfo *tmi = getTitleMessageInfo(i, initial);
    char *filename = getLevelSetTitleMessageFilename(i, initial);
    int sort_priority = tmi->sort_priority;

    if (filename != NULL)
      InitializeTitleControlsExt_AddTitleInfo(FALSE, initial, i, sort_priority);
  }
}

static void InitializeTitleControls(boolean show_title_initial)
{
  num_title_screens = 0;

  /* 1st step: initialize title screens for game start (only when starting) */
  if (show_title_initial)
    InitializeTitleControls_CheckTitleInfo(TRUE);

  /* 2nd step: initialize title screens for current level set */
  InitializeTitleControls_CheckTitleInfo(FALSE);

  /* sort title screens according to sort_priority and title number */
  qsort(title_controls, num_title_screens, sizeof(struct TitleControlInfo),
	compareTitleControlInfo);

  /* mark first title screen */
  title_controls[0].first = TRUE;
}

static boolean visibleMenuPos(struct MenuPosInfo *pos)
{
  return (pos != NULL && pos->x != -1 && pos->y != -1);
}

static boolean visibleTextPos(struct TextPosInfo *pos)
{
  return (pos != NULL && pos->x != -1 && pos->y != -1);
}

static void InitializeMainControls()
{
  boolean local_team_mode = (!options.network && setup.team_mode);
  int i;

  /* set main control text values to dynamically determined values */
  sprintf(main_text_name,         "%s",   local_team_mode ? "Team:" : "Name:");

  strcpy(main_text_first_level,  int2str(leveldir_current->first_level,
					 menu.main.text.first_level.size));
  strcpy(main_text_last_level,   int2str(leveldir_current->last_level,
					 menu.main.text.last_level.size));
  strcpy(main_text_level_number, int2str(level_nr,
					 menu.main.text.level_number.size));

  main_text_level_year		= leveldir_current->year;
  main_text_level_imported_from	= leveldir_current->imported_from;
  main_text_level_imported_by	= leveldir_current->imported_by;
  main_text_level_tested_by	= leveldir_current->tested_by;

  main_text_title_1 = getConfigProgramTitleString();
  main_text_title_2 = getConfigProgramCopyrightString();
  main_text_title_3 = getConfigProgramCompanyString();

  /* set main control screen positions to dynamically determined values */
  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];
    int nr                         = mci->nr;
    struct MenuPosInfo *pos_button = mci->pos_button;
    struct TextPosInfo *pos_text   = mci->pos_text;
    struct TextPosInfo *pos_input  = mci->pos_input;
    char *text                     = (mci->text  ? *mci->text  : NULL);
    char *input                    = (mci->input ? *mci->input : NULL);
    int button_graphic             = mci->button_graphic;
    int font_text                  = (pos_text  ? pos_text->font  : -1);
    int font_input                 = (pos_input ? pos_input->font : -1);

    int font_text_width   = (font_text  != -1 ? getFontWidth(font_text)   : 0);
    int font_text_height  = (font_text  != -1 ? getFontHeight(font_text)  : 0);
    int font_input_width  = (font_input != -1 ? getFontWidth(font_input)  : 0);
    int font_input_height = (font_input != -1 ? getFontHeight(font_input) : 0);
    int text_chars  = (text  != NULL ? strlen(text)  : 0);
    int input_chars = (input != NULL ? strlen(input) : 0);

    int button_width =
      (button_graphic != -1 ? graphic_info[button_graphic].width  : 0);
    int button_height =
      (button_graphic != -1 ? graphic_info[button_graphic].height : 0);
    int text_width   = font_text_width * text_chars;
    int text_height  = font_text_height;
    int input_width  = font_input_width * input_chars;
    int input_height = font_input_height;

    if (nr == MAIN_CONTROL_NAME)
    {
      menu.main.input.name.width  = input_width;
      menu.main.input.name.height = input_height;
    }

    if (pos_button != NULL)		/* (x/y may be -1/-1 here) */
    {
      pos_button->width  = button_width;
      pos_button->height = button_height;
    }

    if (pos_text != NULL)		/* (x/y may be -1/-1 here) */
    {
      /* calculate size for non-clickable text -- needed for text alignment */
      boolean calculate_text_size = (pos_button == NULL && text != NULL);

      if (pos_text->width == -1 || calculate_text_size)
	pos_text->width = text_width;
      if (pos_text->height == -1 || calculate_text_size)
	pos_text->height = text_height;

      if (visibleMenuPos(pos_button))
      {
	if (pos_text->x == -1)
	  pos_text->x = pos_button->x + pos_button->width;
	if (pos_text->y == -1)
	  pos_text->y =
	    pos_button->y + (pos_button->height - pos_text->height) / 2;
      }
    }

    if (pos_input != NULL)		/* (x/y may be -1/-1 here) */
    {
      if (visibleTextPos(pos_text))
      {
	if (pos_input->x == -1)
	  pos_input->x = pos_text->x + pos_text->width;
	if (pos_input->y == -1)
	  pos_input->y = pos_text->y;
      }

      if (pos_input->width == -1)
	pos_input->width = input_width;
      if (pos_input->height == -1)
	pos_input->height = input_height;
    }
  }
}

static void DrawPressedGraphicThruMask(int dst_x, int dst_y,
				       int graphic, boolean pressed)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  Bitmap *src_bitmap;
  int src_x, src_y;
  int xoffset = (pressed ? g->pressed_xoffset : 0);
  int yoffset = (pressed ? g->pressed_yoffset : 0);

  getFixedGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, drawto, src_x + xoffset, src_y + yoffset,
		   g->width, g->height, dst_x, dst_y);
}

static void DrawCursorAndText_Main_Ext(int nr, boolean active_text,
				       boolean active_input,
				       boolean pressed_button)
{
  int i;

  for (i = 0; main_controls[i].nr != -1; i++)
  {
    struct MainControlInfo *mci = &main_controls[i];

    if (mci->nr == nr || nr == -1)
    {
      struct MenuPosInfo *pos_button = mci->pos_button;
      struct TextPosInfo *pos_text   = mci->pos_text;
      struct TextPosInfo *pos_input  = mci->pos_input;
      char *text                     = (mci->text  ? *mci->text  : NULL);
      char *input                    = (mci->input ? *mci->input : NULL);
      int button_graphic             = mci->button_graphic;
      int font_text                  = (pos_text  ? pos_text->font  : -1);
      int font_input                 = (pos_input ? pos_input->font : -1);

      if (active_text)
      {
	button_graphic = BUTTON_ACTIVE(button_graphic);
	font_text = FONT_ACTIVE(font_text);
      }

      if (active_input)
      {
	font_input = FONT_ACTIVE(font_input);
      }

      if (visibleMenuPos(pos_button))
      {
	struct MenuPosInfo *pos = pos_button;
	int x = mSX + pos->x;
	int y = mSY + pos->y;

	DrawBackgroundForGraphic(x, y, pos->width, pos->height, button_graphic);
	DrawPressedGraphicThruMask(x, y, button_graphic, pressed_button);
      }

      if (visibleTextPos(pos_text) && text != NULL)
      {
	struct TextPosInfo *pos = pos_text;
	int x = mSX + ALIGNED_TEXT_XPOS(pos);
	int y = mSY + ALIGNED_TEXT_YPOS(pos);

#if 1
	/* (check why/if this is needed) */
	DrawBackgroundForFont(x, y, pos->width, pos->height, font_text);
#endif
	DrawText(x, y, text, font_text);
      }

      if (visibleTextPos(pos_input) && input != NULL)
      {
	struct TextPosInfo *pos = pos_input;
	int x = mSX + ALIGNED_TEXT_XPOS(pos);
	int y = mSY + ALIGNED_TEXT_YPOS(pos);

#if 1
	/* (check why/if this is needed) */
	DrawBackgroundForFont(x, y, pos->width, pos->height, font_input);
#endif
	DrawText(x, y, input, font_input);
      }
    }
  }
}

static void DrawCursorAndText_Main(int nr, boolean active_text,
				   boolean pressed_button)
{
  DrawCursorAndText_Main_Ext(nr, active_text, FALSE, pressed_button);
}

#if 0
static void DrawCursorAndText_Main_Input(int nr, boolean active_text,
					 boolean pressed_button)
{
  DrawCursorAndText_Main_Ext(nr, active_text, TRUE, pressed_button);
}
#endif

static struct MainControlInfo *getMainControlInfo(int nr)
{
  int i;

  for (i = 0; main_controls[i].nr != -1; i++)
    if (main_controls[i].nr == nr)
      return &main_controls[i];

  return NULL;
}

static boolean insideMenuPosRect(struct MenuPosInfo *rect, int x, int y)
{
  if (rect == NULL)
    return FALSE;

  int rect_x = ALIGNED_TEXT_XPOS(rect);
  int rect_y = ALIGNED_TEXT_YPOS(rect);

  return (x >= rect_x && x < rect_x + rect->width &&
	  y >= rect_y && y < rect_y + rect->height);
}

static boolean insideTextPosRect(struct TextPosInfo *rect, int x, int y)
{
  if (rect == NULL)
    return FALSE;

  int rect_x = ALIGNED_TEXT_XPOS(rect);
  int rect_y = ALIGNED_TEXT_YPOS(rect);

#if 0
  printf("::: insideTextPosRect: (%d, %d), (%d, %d) [%d, %d] (%d, %d) => %d\n",
	 x, y, rect_x, rect_y, rect->x, rect->y, rect->width, rect->height,
	 (x >= rect_x && x < rect_x + rect->width &&
	  y >= rect_y && y < rect_y + rect->height));
#endif

  return (x >= rect_x && x < rect_x + rect->width &&
	  y >= rect_y && y < rect_y + rect->height);
}

static boolean insidePreviewRect(struct PreviewInfo *preview, int x, int y)
{
  int rect_width  = preview->xsize * preview->tile_size;
  int rect_height = preview->ysize * preview->tile_size;
  int rect_x = ALIGNED_XPOS(preview->x, rect_width,  preview->align);
  int rect_y = ALIGNED_YPOS(preview->y, rect_height, preview->valign);

  return (x >= rect_x && x < rect_x + rect_width &&
	  y >= rect_y && y < rect_y + rect_height);
}

static void AdjustScrollbar(int id, int items_max, int items_visible,
			    int item_position)
{
  struct GadgetInfo *gi = screen_gadget[id];

  if (item_position > items_max - items_visible)
    item_position = items_max - items_visible;

  ModifyGadget(gi, GDI_SCROLLBAR_ITEMS_MAX, items_max,
	       GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
	       GDI_SCROLLBAR_ITEM_POSITION, item_position, GDI_END);
}

static void AdjustChooseTreeScrollbar(int id, int first_entry, TreeInfo *ti)
{
  AdjustScrollbar(id, numTreeInfoInGroup(ti), NUM_MENU_ENTRIES_ON_SCREEN,
		  first_entry);
}

static void clearMenuListArea()
{
  int scrollbar_xpos = mSX + SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset;

  /* correct scrollbar position if placed outside menu (playfield) area */
  if (scrollbar_xpos > SX + SC_SCROLLBAR_XPOS)
    scrollbar_xpos = SX + SC_SCROLLBAR_XPOS;

  /* clear menu list area, but not title or scrollbar */
  DrawBackground(mSX, mSY + MENU_SCREEN_START_YPOS * 32,
                 scrollbar_xpos - mSX, NUM_MENU_ENTRIES_ON_SCREEN * 32);
}

static void drawCursorExt(int xpos, int ypos, boolean active, int graphic)
{
  static int cursor_array[MAX_LEV_FIELDY];
  int x = mSX + TILEX * xpos;
  int y = mSY + TILEY * (MENU_SCREEN_START_YPOS + ypos);

  if (xpos == 0)
  {
    if (graphic != -1)
      cursor_array[ypos] = graphic;
    else
      graphic = cursor_array[ypos];
  }

  if (active)
    graphic = BUTTON_ACTIVE(graphic);

  DrawBackgroundForGraphic(x, y, TILEX, TILEY, graphic);
  DrawFixedGraphicThruMaskExt(drawto, x, y, graphic, 0);
}

static void initCursor(int ypos, int graphic)
{
  drawCursorExt(0, ypos, FALSE, graphic);
}

static void drawCursor(int ypos, boolean active)
{
  drawCursorExt(0, ypos, active, -1);
}

static void drawCursorXY(int xpos, int ypos, int graphic)
{
  drawCursorExt(xpos, ypos, FALSE, graphic);
}

static void drawChooseTreeCursor(int ypos, boolean active)
{
  drawCursorExt(0, ypos, active, -1);
}

void DrawHeadline()
{
  DrawTextSCentered(MENU_TITLE1_YPOS, FONT_TITLE_1, main_text_title_1);
  DrawTextSCentered(MENU_TITLE2_YPOS, FONT_TITLE_2, main_text_title_2);
}

void DrawTitleScreenImage(int nr, boolean initial)
{
  int graphic = getTitleScreenGraphic(nr, initial);
  Bitmap *bitmap = graphic_info[graphic].bitmap;
  int width  = graphic_info[graphic].width;
  int height = graphic_info[graphic].height;
  int src_x = graphic_info[graphic].src_x;
  int src_y = graphic_info[graphic].src_y;
  int dst_x, dst_y;

  if (bitmap == NULL)
    return;

  if (width > WIN_XSIZE)
  {
    /* image width too large for window => center image horizontally */
    src_x = (width - WIN_XSIZE) / 2;
    width = WIN_XSIZE;
  }

  if (height > WIN_YSIZE)
  {
    /* image height too large for window => center image vertically */
    src_y = (height - WIN_YSIZE) / 2;
    height = WIN_YSIZE;
  }

  /* always display title screens centered */
  dst_x = (WIN_XSIZE - width) / 2;
  dst_y = (WIN_YSIZE - height) / 2;

  SetDrawBackgroundMask(REDRAW_ALL);
  SetWindowBackgroundImage(getTitleBackground(nr, initial, TRUE));

  ClearRectangleOnBackground(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  if (DrawingOnBackground(dst_x, dst_y))
    BlitBitmapMasked(bitmap, drawto, src_x, src_y, width, height, dst_x, dst_y);
  else
    BlitBitmap(bitmap, drawto, src_x, src_y, width, height, dst_x, dst_y);

  redraw_mask = REDRAW_ALL;
}

void DrawTitleScreenMessage(int nr, boolean initial)
{
  char *filename = getLevelSetTitleMessageFilename(nr, initial);
  struct TitleMessageInfo *tmi = getTitleMessageInfo(nr, initial);

  if (filename == NULL)
    return;

  /* force TITLE font on title message screen */
  SetFontStatus(getTitleMessageGameMode(initial));

  /* if chars *and* width set to "-1", automatically determine width */
  if (tmi->chars == -1 && tmi->width == -1)
    tmi->width = viewport.window[game_status].width;

  /* if lines *and* height set to "-1", automatically determine height */
  if (tmi->lines == -1 && tmi->height == -1)
    tmi->height = viewport.window[game_status].height;

  /* if chars set to "-1", automatically determine by text and font width */
  if (tmi->chars == -1)
    tmi->chars = tmi->width / getFontWidth(tmi->font);
  else
    tmi->width = tmi->chars * getFontWidth(tmi->font);

  /* if lines set to "-1", automatically determine by text and font height */
  if (tmi->lines == -1)
    tmi->lines = tmi->height / getFontHeight(tmi->font);
  else
    tmi->height = tmi->lines * getFontHeight(tmi->font);

  /* if x set to "-1", automatically determine by width and alignment */
  if (tmi->x == -1)
    tmi->x = -1 * ALIGNED_XPOS(0, tmi->width, tmi->align);

  /* if y set to "-1", automatically determine by height and alignment */
  if (tmi->y == -1)
    tmi->y = -1 * ALIGNED_YPOS(0, tmi->height, tmi->valign);

  SetDrawBackgroundMask(REDRAW_ALL);
  SetWindowBackgroundImage(getTitleBackground(nr, initial, FALSE));

  ClearRectangleOnBackground(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  DrawTextFile(ALIGNED_TEXT_XPOS(tmi), ALIGNED_TEXT_YPOS(tmi),
	       filename, tmi->font, tmi->chars, -1, tmi->lines, 0, -1,
	       tmi->autowrap, tmi->centered, tmi->parse_comments);

  ResetFontStatus();
}

void DrawTitleScreen()
{
  KeyboardAutoRepeatOff();

  HandleTitleScreen(0, 0, 0, 0, MB_MENU_INITIALIZE);
}

boolean CheckTitleScreen(boolean levelset_has_changed)
{
  static boolean show_title_initial = TRUE;
  boolean show_titlescreen = FALSE;

  /* needed to be able to skip title screen, if no image or message defined */
  InitializeTitleControls(show_title_initial);

  if (setup.show_titlescreen && (show_title_initial || levelset_has_changed))
    show_titlescreen = TRUE;

  /* show initial title images and messages only once at program start */
  show_title_initial = FALSE;

  return (show_titlescreen && num_title_screens > 0);
}

void DrawMainMenu()
{
  static LevelDirTree *leveldir_last_valid = NULL;
  boolean levelset_has_changed = FALSE;
  int fade_mask = REDRAW_FIELD;

  LimitScreenUpdates(FALSE);

  FadeSetLeaveScreen();

  /* do not fade out here -- function may continue and fade on editor screen */

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  ExpireSoundLoops(FALSE);

  KeyboardAutoRepeatOn();

  audio.sound_deactivated = FALSE;

  GetPlayerConfig();

  /* needed if last screen was the playing screen, invoked from level editor */
  if (level_editor_test_game)
  {
    CloseDoor(DOOR_CLOSE_ALL);

    SetGameStatus(GAME_MODE_EDITOR);

    DrawLevelEd();

    return;
  }

  /* needed if last screen was the setup screen and fullscreen state changed */
  // (moved to "execSetupGraphics()" to change fullscreen state directly)
  // ToggleFullscreenOrChangeWindowScalingIfNeeded();

  /* leveldir_current may be invalid (level group, parent link) */
  if (!validLevelSeries(leveldir_current))
    leveldir_current = getFirstValidTreeInfoEntry(leveldir_last_valid);

  if (leveldir_current != leveldir_last_valid)
    levelset_has_changed = TRUE;

  /* store valid level series information */
  leveldir_last_valid = leveldir_current;

  init_last = init;			/* switch to new busy animation */

  /* needed if last screen (level choice) changed graphics, sounds or music */
  ReloadCustomArtwork(0);

  if (CheckTitleScreen(levelset_has_changed))
  {
    SetGameStatus(GAME_MODE_TITLE);

    DrawTitleScreen();

    return;
  }

  if (redraw_mask & REDRAW_ALL)
    fade_mask = REDRAW_ALL;

  if (CheckIfGlobalBorderOrPlayfieldViewportHasChanged())
    fade_mask = REDRAW_ALL;

  FadeOut(fade_mask);

  /* needed if different viewport properties defined for menues */
  ChangeViewportPropertiesIfNeeded();

  SetDrawtoField(DRAW_TO_BACKBUFFER);

  /* level_nr may have been set to value over handicap with level editor */
  if (setup.handicap && level_nr > leveldir_current->handicap_level)
    level_nr = leveldir_current->handicap_level;

  LoadLevel(level_nr);
  LoadScore(level_nr);

  SaveLevelSetup_SeriesInfo();

  // set this after "ChangeViewportPropertiesIfNeeded()" (which may reset it)
  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  SetMainBackgroundImage(IMG_BACKGROUND_MAIN);

#if 0
  if (fade_mask == REDRAW_ALL)
    RedrawGlobalBorder();
#endif

  ClearField();

  InitializeMainControls();

  DrawCursorAndText_Main(-1, FALSE, FALSE);
  DrawPreviewLevelInitial();

  HandleMainMenu(0, 0, 0, 0, MB_MENU_INITIALIZE);

  TapeStop();
  if (TAPE_IS_EMPTY(tape))
    LoadTape(level_nr);
  DrawCompleteVideoDisplay();

  PlayMenuSoundsAndMusic();

  /* create gadgets for main menu screen */
  FreeScreenGadgets();
  CreateScreenGadgets();

  /* map gadgets for main menu screen */
  MapTapeButtons();
  MapScreenMenuGadgets(SCREEN_MASK_MAIN);

  /* copy actual game door content to door double buffer for OpenDoor() */
  BlitBitmap(drawto, bitmap_db_door_1, DX, DY, DXSIZE, DYSIZE, 0, 0);
  BlitBitmap(drawto, bitmap_db_door_2, VX, VY, VXSIZE, VYSIZE, 0, 0);

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
  FadeSetEnterMenu();

  /* update screen area with special editor door */
  redraw_mask |= REDRAW_ALL;
  BackToFront();

  SetMouseCursor(CURSOR_DEFAULT);

  OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2);
}

static void gotoTopLevelDir()
{
  /* move upwards until inside (but not above) top level directory */
  while (leveldir_current->node_parent &&
	 !strEqual(leveldir_current->node_parent->subdir, STRING_TOP_DIRECTORY))
  {
    /* write a "path" into level tree for easy navigation to last level */
    if (leveldir_current->node_parent->node_group->cl_first == -1)
    {
      int num_leveldirs = numTreeInfoInGroup(leveldir_current);
      int leveldir_pos = posTreeInfo(leveldir_current);
      int num_page_entries;
      int cl_first, cl_cursor;

      if (num_leveldirs <= NUM_MENU_ENTRIES_ON_SCREEN)
	num_page_entries = num_leveldirs;
      else
	num_page_entries = NUM_MENU_ENTRIES_ON_SCREEN;

      cl_first = MAX(0, leveldir_pos - num_page_entries + 1);
      cl_cursor = leveldir_pos - cl_first;

      leveldir_current->node_parent->node_group->cl_first = cl_first;
      leveldir_current->node_parent->node_group->cl_cursor = cl_cursor;
    }

    leveldir_current = leveldir_current->node_parent;
  }
}

void HandleTitleScreen(int mx, int my, int dx, int dy, int button)
{
  static unsigned int title_delay = 0;
  static int title_screen_nr = 0;
  static int last_sound = -1, last_music = -1;
  boolean return_to_main_menu = FALSE;
  struct TitleControlInfo *tci;
  int sound, music;

  if (button == MB_MENU_INITIALIZE)
  {
    title_delay = 0;
    title_screen_nr = 0;
    tci = &title_controls[title_screen_nr];

    SetAnimStatus(getTitleAnimMode(tci));

    last_sound = SND_UNDEFINED;
    last_music = MUS_UNDEFINED;

    if (num_title_screens != 0)
    {
      FadeSetEnterScreen();

      /* use individual title fading instead of global "enter screen" fading */
      fading = getTitleFading(tci);
    }

    if (game_status_last_screen == GAME_MODE_INFO)
    {
      if (num_title_screens == 0)
      {
	/* switch game mode from title screen mode back to info screen mode */
	SetGameStatus(GAME_MODE_INFO);

	/* store that last screen was info screen, not main menu screen */
	game_status_last_screen = GAME_MODE_INFO;

	DrawInfoScreen_NotAvailable("Title screen information:",
				    "No title screen for this level set.");
	return;
      }

      FadeMenuSoundsAndMusic();
    }

    FadeOut(REDRAW_ALL);

    /* title screens may have different window size */
    ChangeViewportPropertiesIfNeeded();

    /* only required to update logic for redrawing global border */
    ClearField();

    if (tci->is_image)
      DrawTitleScreenImage(tci->local_nr, tci->initial);
    else
      DrawTitleScreenMessage(tci->local_nr, tci->initial);

    sound = getTitleSound(tci);
    music = getTitleMusic(tci);

    if (sound != last_sound)
      PlayMenuSoundExt(sound);
    if (music != last_music)
      PlayMenuMusicExt(music);

    last_sound = sound;
    last_music = music;

    SetMouseCursor(CURSOR_NONE);

    FadeIn(REDRAW_ALL);

    DelayReached(&title_delay, 0);	/* reset delay counter */

    return;
  }

  if (fading.auto_delay > 0 && DelayReached(&title_delay, fading.auto_delay))
    button = MB_MENU_CHOICE;

  if (button == MB_MENU_LEAVE)
  {
    return_to_main_menu = TRUE;
  }
  else if (button == MB_MENU_CHOICE)
  {
    if (game_status_last_screen == GAME_MODE_INFO && num_title_screens == 0)
    {
      SetGameStatus(GAME_MODE_INFO);

      info_mode = INFO_MODE_MAIN;

      DrawInfoScreen();

      return;
    }

    title_screen_nr++;

    if (title_screen_nr < num_title_screens)
    {
      tci = &title_controls[title_screen_nr];

      SetAnimStatus(getTitleAnimMode(tci));

      sound = getTitleSound(tci);
      music = getTitleMusic(tci);

      if (last_sound != SND_UNDEFINED && sound != last_sound)
	FadeSound(last_sound);
      if (last_music != MUS_UNDEFINED && music != last_music)
	FadeMusic();

      fading = getTitleFading(tci);

      FadeOut(REDRAW_ALL);

      if (tci->is_image)
	DrawTitleScreenImage(tci->local_nr, tci->initial);
      else
	DrawTitleScreenMessage(tci->local_nr, tci->initial);

      sound = getTitleSound(tci);
      music = getTitleMusic(tci);

      if (sound != last_sound)
	PlayMenuSoundExt(sound);
      if (music != last_music)
	PlayMenuMusicExt(music);

      last_sound = sound;
      last_music = music;

      FadeIn(REDRAW_ALL);

      DelayReached(&title_delay, 0);	/* reset delay counter */
    }
    else
    {
      FadeMenuSoundsAndMusic();

      return_to_main_menu = TRUE;
    }
  }

  if (return_to_main_menu)
  {
    SetMouseCursor(CURSOR_DEFAULT);

    /* force full menu screen redraw after displaying title screens */
    redraw_mask = REDRAW_ALL;

    if (game_status_last_screen == GAME_MODE_INFO)
    {
      SetGameStatus(GAME_MODE_INFO);

      info_mode = INFO_MODE_MAIN;

      DrawInfoScreen();
    }
    else	/* default: return to main menu */
    {
      SetGameStatus(GAME_MODE_MAIN);

      DrawMainMenu();
    }
  }
}

void HandleMainMenu_SelectLevel(int step, int direction, int selected_level_nr)
{
  int old_level_nr = level_nr;
  int new_level_nr;

  if (selected_level_nr != NO_DIRECT_LEVEL_SELECT)
    new_level_nr = selected_level_nr;
  else
    new_level_nr = old_level_nr + step * direction;

  if (new_level_nr < leveldir_current->first_level)
    new_level_nr = leveldir_current->first_level;
  if (new_level_nr > leveldir_current->last_level)
    new_level_nr = leveldir_current->last_level;

  if (setup.handicap && new_level_nr > leveldir_current->handicap_level)
  {
    /* skipping levels is only allowed when trying to skip single level */
    if (setup.skip_levels && new_level_nr == old_level_nr + 1 &&
	Request("Level still unsolved! Skip despite handicap?", REQ_ASK))
    {
      leveldir_current->handicap_level++;
      SaveLevelSetup_SeriesInfo();
    }

    new_level_nr = leveldir_current->handicap_level;
  }

  if (new_level_nr != old_level_nr)
  {
    struct MainControlInfo *mci= getMainControlInfo(MAIN_CONTROL_LEVEL_NUMBER);

    PlaySound(SND_MENU_ITEM_SELECTING);

    level_nr = new_level_nr;

    DrawText(mSX + mci->pos_text->x, mSY + mci->pos_text->y,
	     int2str(level_nr, menu.main.text.level_number.size),
	     mci->pos_text->font);

    LoadLevel(level_nr);
    DrawPreviewLevelInitial();

    TapeErase();
    LoadTape(level_nr);
    DrawCompleteVideoDisplay();

    SaveLevelSetup_SeriesInfo();

    /* needed because DrawPreviewLevelInitial() takes some time */
    BackToFront();
    /* SyncDisplay(); */
  }
}

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = MAIN_CONTROL_GAME;
  static boolean button_pressed_last = FALSE;
  boolean button_pressed = FALSE;
  int pos = choice;
  int i;

  if (button == MB_MENU_INITIALIZE)
  {
    DrawCursorAndText_Main(choice, TRUE, FALSE);

    return;
  }

  if (mx || my)		/* mouse input */
  {
    pos = -1;

    for (i = 0; main_controls[i].nr != -1; i++)
    {
      if (insideMenuPosRect(main_controls[i].pos_button, mx - mSX, my - mSY) ||
	  insideTextPosRect(main_controls[i].pos_text,   mx - mSX, my - mSY) ||
	  insideTextPosRect(main_controls[i].pos_input,  mx - mSX, my - mSY))
      {
	pos = main_controls[i].nr;

	break;
      }
    }

    /* check if level preview was clicked */
    if (insidePreviewRect(&preview, mx - SX, my - SY))
      pos = MAIN_CONTROL_GAME;

    // handle pressed/unpressed state for active/inactive menu buttons
    // (if pos != -1, "i" contains index position corresponding to "pos")
    if (button &&
	pos >= MAIN_CONTROL_NAME && pos <= MAIN_CONTROL_QUIT &&
	insideMenuPosRect(main_controls[i].pos_button, mx - mSX, my - mSY))
      button_pressed = TRUE;

    if (button_pressed != button_pressed_last)
    {
      DrawCursorAndText_Main(choice, TRUE, button_pressed);

      if (button_pressed)
	PlaySound(SND_MENU_BUTTON_PRESSING);
      else
	PlaySound(SND_MENU_BUTTON_RELEASING);
    }
  }
  else if (dx || dy)	/* keyboard input */
  {
    if (dx > 0 && (choice == MAIN_CONTROL_INFO ||
		   choice == MAIN_CONTROL_SETUP))
      button = MB_MENU_CHOICE;
    else if (dy)
      pos = choice + dy;
  }

  if (pos == MAIN_CONTROL_FIRST_LEVEL && !button)
  {
    HandleMainMenu_SelectLevel(MAX_LEVELS, -1, NO_DIRECT_LEVEL_SELECT);
  }
  else if (pos == MAIN_CONTROL_LAST_LEVEL && !button)
  {
    HandleMainMenu_SelectLevel(MAX_LEVELS, +1, NO_DIRECT_LEVEL_SELECT);
  }
  else if (pos == MAIN_CONTROL_LEVEL_NUMBER && !button)
  {
    CloseDoor(DOOR_CLOSE_2);

    SetGameStatus(GAME_MODE_LEVELNR);

    DrawChooseLevelNr();
  }
  else if (pos >= MAIN_CONTROL_NAME && pos <= MAIN_CONTROL_QUIT)
  {
    if (button)
    {
      if (pos != choice)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	DrawCursorAndText_Main(choice, FALSE, FALSE);
	DrawCursorAndText_Main(pos, TRUE, button_pressed);

	choice = pos;
      }
      else if (dx != 0)
      {
	if (choice != MAIN_CONTROL_INFO &&
	    choice != MAIN_CONTROL_SETUP)
	  HandleMainMenu_SelectLevel(1, dx, NO_DIRECT_LEVEL_SELECT);
      }
    }
    else
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      if (pos == MAIN_CONTROL_NAME)
      {
	SetGameStatus(GAME_MODE_PSEUDO_TYPENAME);

	HandleTypeName(strlen(setup.player_name), 0);
      }
      else if (pos == MAIN_CONTROL_LEVELS)
      {
	if (leveldir_first)
	{
	  CloseDoor(DOOR_CLOSE_2);

	  SetGameStatus(GAME_MODE_LEVELS);

	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();

	  if (setup.internal.choose_from_top_leveldir)
	    gotoTopLevelDir();

	  DrawChooseLevelSet();
	}
      }
      else if (pos == MAIN_CONTROL_SCORES)
      {
	CloseDoor(DOOR_CLOSE_2);

	SetGameStatus(GAME_MODE_SCORES);

	DrawHallOfFame(-1);
      }
      else if (pos == MAIN_CONTROL_EDITOR)
      {
	if (leveldir_current->readonly &&
	    !strEqual(setup.player_name, "Artsoft"))
	  Request("This level is read only!", REQ_CONFIRM);

	CloseDoor(DOOR_CLOSE_2);

	SetGameStatus(GAME_MODE_EDITOR);

	FadeSetEnterScreen();

	DrawLevelEd();
      }
      else if (pos == MAIN_CONTROL_INFO)
      {
	CloseDoor(DOOR_CLOSE_2);

	SetGameStatus(GAME_MODE_INFO);

	info_mode = INFO_MODE_MAIN;

	DrawInfoScreen();
      }
      else if (pos == MAIN_CONTROL_GAME)
      {
	StartGameActions(options.network, setup.autorecord, level.random_seed);
      }
      else if (pos == MAIN_CONTROL_SETUP)
      {
	CloseDoor(DOOR_CLOSE_2);

	SetGameStatus(GAME_MODE_SETUP);

	setup_mode = SETUP_MODE_MAIN;

	DrawSetupScreen();
      }
      else if (pos == MAIN_CONTROL_QUIT)
      {
	SaveLevelSetup_LastSeries();
	SaveLevelSetup_SeriesInfo();

        if (Request("Do you really want to quit?", REQ_ASK | REQ_STAY_CLOSED))
	  SetGameStatus(GAME_MODE_QUIT);
      }
    }
  }

  button_pressed_last = button_pressed;
}


/* ========================================================================= */
/* info screen functions                                                     */
/* ========================================================================= */

static struct TokenInfo *info_info;
static int num_info_info;	/* number of info entries shown on screen */
static int max_info_info;	/* total number of info entries in list */

static void execInfoTitleScreen()
{
  info_mode = INFO_MODE_TITLE;

  DrawInfoScreen();
}

static void execInfoElements()
{
  info_mode = INFO_MODE_ELEMENTS;

  DrawInfoScreen();
}

static void execInfoMusic()
{
  info_mode = INFO_MODE_MUSIC;

  DrawInfoScreen();
}

static void execInfoCredits()
{
  info_mode = INFO_MODE_CREDITS;

  DrawInfoScreen();
}

static void execInfoProgram()
{
  info_mode = INFO_MODE_PROGRAM;

  DrawInfoScreen();
}

static void execInfoVersion()
{
  info_mode = INFO_MODE_VERSION;

  DrawInfoScreen();
}

static void execInfoLevelSet()
{
  info_mode = INFO_MODE_LEVELSET;

  DrawInfoScreen();
}

static void execExitInfo()
{
  SetGameStatus(GAME_MODE_MAIN);

  DrawMainMenu();
}

static struct TokenInfo info_info_main[] =
{
  { TYPE_ENTER_SCREEN,	execInfoTitleScreen,	"Title Screen"		},
  { TYPE_ENTER_SCREEN,	execInfoElements,	"Elements Info"		},
  { TYPE_ENTER_SCREEN,	execInfoMusic,		"Music Info"		},
  { TYPE_ENTER_SCREEN,	execInfoCredits,	"Credits"		},
  { TYPE_ENTER_SCREEN,	execInfoProgram,	"Program Info"		},
  { TYPE_ENTER_SCREEN,	execInfoVersion,	"Version Info"		},
  { TYPE_ENTER_SCREEN,	execInfoLevelSet,	"Level Set Info"	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execExitInfo, 		"Exit"			},

  { 0,			NULL,			NULL			}
};

static int getMenuTextFont(int type)
{
  if (type & (TYPE_SWITCH	|
	      TYPE_YES_NO	|
	      TYPE_YES_NO_AUTO	|
	      TYPE_STRING	|
	      TYPE_ECS_AGA	|
	      TYPE_KEYTEXT	|
	      TYPE_ENTER_LIST))
    return FONT_MENU_2;
  else
    return FONT_MENU_1;
}

static struct TokenInfo *setup_info;
static struct TokenInfo setup_info_input[];

static struct TokenInfo *menu_info;

static void DrawCursorAndText_Menu_Ext(struct TokenInfo *token_info,
				       int screen_pos, int menu_info_pos_raw,
				       boolean active)
{
  int pos = (menu_info_pos_raw < 0 ? screen_pos : menu_info_pos_raw);
  struct TokenInfo *ti = &token_info[pos];
  int xpos = MENU_SCREEN_START_XPOS;
  int ypos = MENU_SCREEN_START_YPOS + screen_pos;
  int font_nr = getMenuTextFont(ti->type);

  if (token_info == setup_info_input)
    font_nr = FONT_MENU_1;

  if (active)
    font_nr = FONT_ACTIVE(font_nr);

  DrawText(mSX + xpos * 32, mSY + ypos * 32, ti->text, font_nr);

  if (ti->type & ~TYPE_SKIP_ENTRY)
    drawCursor(screen_pos, active);
}

static void DrawCursorAndText_Menu(int screen_pos, int menu_info_pos_raw,
				   boolean active)
{
  DrawCursorAndText_Menu_Ext(menu_info, screen_pos, menu_info_pos_raw, active);
}

static void DrawCursorAndText_Setup(int screen_pos, int menu_info_pos_raw,
				    boolean active)
{
  DrawCursorAndText_Menu_Ext(setup_info, screen_pos, menu_info_pos_raw, active);
}

static char *window_size_text;
static char *scaling_type_text;

static void drawSetupValue(int, int);

static void drawMenuInfoList(int first_entry, int num_page_entries,
			     int max_page_entries)
{
  int i;

  if (first_entry + num_page_entries > max_page_entries)
    first_entry = 0;

  clearMenuListArea();

  for (i = 0; i < num_page_entries; i++)
  {
    int menu_info_pos = first_entry + i;
    struct TokenInfo *si = &menu_info[menu_info_pos];
    void *value_ptr = si->value;

    /* set some entries to "unchangeable" according to other variables */
    if ((value_ptr == &setup.sound_simple && !audio.sound_available) ||
	(value_ptr == &setup.sound_loops  && !audio.loops_available) ||
	(value_ptr == &setup.sound_music  && !audio.music_available) ||
	(value_ptr == &setup.fullscreen   && !video.fullscreen_available) ||
	(value_ptr == &window_size_text   && !video.window_scaling_available) ||
	(value_ptr == &scaling_type_text  && !video.window_scaling_available))
      si->type |= TYPE_GHOSTED;

    if (si->type & (TYPE_ENTER_MENU|TYPE_ENTER_LIST))
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else if (si->type & (TYPE_LEAVE_MENU|TYPE_LEAVE_LIST))
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (si->type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

    DrawCursorAndText_Menu(i, menu_info_pos, FALSE);

    if (si->type & TYPE_VALUE &&
	menu_info == setup_info)
      drawSetupValue(i, menu_info_pos);
  }
}

static void DrawInfoScreen_Main()
{
  int fade_mask = REDRAW_FIELD;
  int i;

  if (redraw_mask & REDRAW_ALL)
    fade_mask = REDRAW_ALL;

  if (CheckIfGlobalBorderOrPlayfieldViewportHasChanged())
    fade_mask = REDRAW_ALL;

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  FreeScreenGadgets();
  CreateScreenGadgets();

  /* (needed after displaying title screens which disable auto repeat) */
  KeyboardAutoRepeatOn();

  FadeSetLeaveScreen();

  FadeOut(fade_mask);

  /* needed if different viewport properties defined for info screen */
  ChangeViewportPropertiesIfNeeded();

  SetMainBackgroundImage(IMG_BACKGROUND_INFO);

  ClearField();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, "Info Screen");

  info_info = info_info_main;

  // determine maximal number of info entries that can be displayed on screen
  num_info_info = 0;
  for (i = 0; info_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
    num_info_info++;

  // determine maximal number of info entries available for menu of info screen
  max_info_info = 0;
  for (i = 0; info_info[i].type != 0; i++)
    max_info_info++;

  HandleInfoScreen_Main(0, 0, 0, 0, MB_MENU_INITIALIZE);

  MapScreenGadgets(max_info_info);

  PlayMenuSoundsAndMusic();

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
}

static void changeSetupValue(int, int, int);

void HandleMenuScreen(int mx, int my, int dx, int dy, int button,
		      int mode, int num_page_entries, int max_page_entries)
{
  static int num_page_entries_all_last[NUM_SPECIAL_GFX_ARGS][MAX_MENU_MODES];
  static int choice_stores[NUM_SPECIAL_GFX_ARGS][MAX_MENU_MODES];
  static int first_entry_stores[NUM_SPECIAL_GFX_ARGS][MAX_MENU_MODES];
  int *num_page_entries_last = num_page_entries_all_last[game_status];
  int *choice_store = choice_stores[game_status];
  int *first_entry_store = first_entry_stores[game_status];
  int choice = choice_store[mode];		/* starts with 0 */
  int first_entry = first_entry_store[mode];	/* starts with 0 */
  int x = 0;
  int y = choice - first_entry;
  int y_old = y;
  boolean position_set_by_scrollbar = (dx == 999);
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int i;

  if (button == MB_MENU_INITIALIZE)
  {
    // check if number of menu page entries has changed (may happen by change
    // of custom artwork definition value for 'list_size' for this menu screen)
    // (in this case, the last menu position most probably has to be corrected)
    if (num_page_entries != num_page_entries_last[mode])
    {
      choice_store[mode] = first_entry_store[mode] = 0;

      choice = first_entry = 0;
      y = y_old = 0;

      num_page_entries_last[mode] = num_page_entries;
    }

    /* advance to first valid menu entry */
    while (choice < num_page_entries &&
	   menu_info[choice].type & TYPE_SKIP_ENTRY)
      choice++;

    if (position_set_by_scrollbar)
      first_entry = first_entry_store[mode] = dy;
    else
      AdjustScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL, max_page_entries,
		      NUM_MENU_ENTRIES_ON_SCREEN, first_entry);

    drawMenuInfoList(first_entry, num_page_entries, max_page_entries);

    if (choice < first_entry)
    {
      choice = first_entry;

      if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	choice++;
    }
    else if (choice > first_entry + num_page_entries - 1)
    {
      choice = first_entry + num_page_entries - 1;

      if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	choice--;
    }

    choice_store[mode] = choice;

    DrawCursorAndText_Menu(choice - first_entry, choice, TRUE);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    for (i = 0; i < max_page_entries; i++)
    {
      if (menu_info[i].type & TYPE_LEAVE_MENU)
      {
	void (*menu_callback_function)(void) = menu_info[i].value;

	FadeSetLeaveMenu();

	menu_callback_function();

	break;	/* absolutely needed because function changes 'menu_info'! */
      }
    }

    return;
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	/* keyboard or scrollbar/scrollbutton input */
  {
    /* move cursor instead of scrolling when already at start/end of list */
    if (dy == -1 * SCROLL_LINE && first_entry == 0)
      dy = -1;
    else if (dy == +1 * SCROLL_LINE &&
	     first_entry + num_page_entries == max_page_entries)
      dy = 1;

    /* handle scrolling screen one line or page */
    if (y + dy < 0 ||
	y + dy > num_page_entries - 1)
    {
      boolean redraw = FALSE;

      if (ABS(dy) == SCROLL_PAGE)
	step = num_page_entries - 1;

      if (dy < 0 && first_entry > 0)
      {
	/* scroll page/line up */

	first_entry -= step;
	if (first_entry < 0)
	  first_entry = 0;

	redraw = TRUE;
      }
      else if (dy > 0 && first_entry + num_page_entries < max_page_entries)
      {
	/* scroll page/line down */

	first_entry += step;
	if (first_entry + num_page_entries > max_page_entries)
	  first_entry = MAX(0, max_page_entries - num_page_entries);

	redraw = TRUE;
      }

      if (redraw)
      {
	choice += first_entry - first_entry_store[mode];

	if (choice < first_entry)
	{
	  choice = first_entry;

	  if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	    choice++;
	}
	else if (choice > first_entry + num_page_entries - 1)
	{
	  choice = first_entry + num_page_entries - 1;

	  if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	    choice--;
	}
	else if (menu_info[choice].type & TYPE_SKIP_ENTRY)
	{
	  choice += SIGN(dy);

	  if (choice < first_entry ||
	      choice > first_entry + num_page_entries - 1)
	  first_entry += SIGN(dy);
	}

	first_entry_store[mode] = first_entry;
	choice_store[mode] = choice;

	drawMenuInfoList(first_entry, num_page_entries, max_page_entries);

	DrawCursorAndText_Menu(choice - first_entry, choice, TRUE);

	AdjustScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL, max_page_entries,
			NUM_MENU_ENTRIES_ON_SCREEN, first_entry);
      }

      return;
    }

    if (dx)
    {
      int menu_navigation_type = (dx < 0 ? TYPE_LEAVE : TYPE_ENTER);

      if (menu_info[choice].type & menu_navigation_type ||
	  menu_info[choice].type & TYPE_BOOLEAN_STYLE ||
	  menu_info[choice].type & TYPE_YES_NO_AUTO)
	button = MB_MENU_CHOICE;
    }
    else if (dy)
      y += dy;

    /* jump to next non-empty menu entry (up or down) */
    while (first_entry + y > 0 &&
	   first_entry + y < max_page_entries - 1 &&
	   menu_info[first_entry + y].type & TYPE_SKIP_ENTRY)
      y += dy;

    if (!IN_VIS_MENU(x, y))
    {
      choice += y - y_old;

      if (choice < first_entry)
	first_entry = choice;
      else if (choice > first_entry + num_page_entries - 1)
	first_entry = choice - num_page_entries + 1;

      if (first_entry >= 0 &&
	  first_entry + num_page_entries <= max_page_entries)
      {
	first_entry_store[mode] = first_entry;

	if (choice < first_entry)
	  choice = first_entry;
	else if (choice > first_entry + num_page_entries - 1)
	  choice = first_entry + num_page_entries - 1;

	choice_store[mode] = choice;

	drawMenuInfoList(first_entry, num_page_entries, max_page_entries);

	DrawCursorAndText_Menu(choice - first_entry, choice, TRUE);

	AdjustScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL, max_page_entries,
			NUM_MENU_ENTRIES_ON_SCREEN, first_entry);
      }

      return;
    }
  }

  if (!anyScrollbarGadgetActive() &&
      IN_VIS_MENU(x, y) &&
      mx < screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->x &&
      y >= 0 && y < num_page_entries)
  {
    if (button)
    {
      if (first_entry + y != choice &&
	  menu_info[first_entry + y].type & ~TYPE_SKIP_ENTRY)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	DrawCursorAndText_Menu(choice - first_entry, choice, FALSE);
	DrawCursorAndText_Menu(y, first_entry + y, TRUE);

	choice = choice_store[mode] = first_entry + y;
      }
      else if (dx < 0)
      {
	PlaySound(SND_MENU_ITEM_SELECTING);

	for (i = 0; menu_info[i].type != 0; i++)
	{
	  if (menu_info[i].type & TYPE_LEAVE_MENU)
	  {
	    void (*menu_callback_function)(void) = menu_info[i].value;

	    FadeSetLeaveMenu();

	    menu_callback_function();

	    /* absolutely needed because function changes 'menu_info'! */
	    break;
	  }
	}

	return;
      }
    }
    else if (!(menu_info[first_entry + y].type & TYPE_GHOSTED))
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      /* when selecting key headline, execute function for key value change */
      if (menu_info[first_entry + y].type & TYPE_KEYTEXT &&
	  menu_info[first_entry + y + 1].type & TYPE_KEY)
	y++;

      /* when selecting string value, execute function for list selection */
      if (menu_info[first_entry + y].type & TYPE_STRING && y > 0 &&
	  menu_info[first_entry + y - 1].type & TYPE_ENTER_LIST)
	y--;

      if (menu_info[first_entry + y].type & TYPE_ENTER_OR_LEAVE)
      {
	void (*menu_callback_function)(void) =
	  menu_info[first_entry + y].value;

	FadeSetFromType(menu_info[first_entry + y].type);

	menu_callback_function();
      }
      else if (menu_info[first_entry + y].type & TYPE_VALUE &&
	       menu_info == setup_info)
      {
	changeSetupValue(y, first_entry + y, dx);
      }
    }
  }
}

void HandleInfoScreen_Main(int mx, int my, int dx, int dy, int button)
{
  menu_info = info_info;

  HandleMenuScreen(mx, my, dx, dy, button,
		   info_mode, num_info_info, max_info_info);
}

static int getMenuFontSpacing(int spacing_height, int font_nr)
{
  int font_spacing = getFontHeight(font_nr) + EXTRA_SPACING(game_status);

  return (spacing_height < 0 ? ABS(spacing_height) * font_spacing :
	  spacing_height);
}

static int getMenuTextSpacing(int spacing_height, int font_nr)
{
  return (getMenuFontSpacing(spacing_height, font_nr) +
	  EXTRA_SPACING(game_status));
}

static int getMenuTextStep(int spacing_height, int font_nr)
{
  return getFontHeight(font_nr) + getMenuTextSpacing(spacing_height, font_nr);
}

void DrawInfoScreen_NotAvailable(char *text_title, char *text_error)
{
  int font_title = MENU_INFO_FONT_TITLE;
  int font_error = FONT_TEXT_2;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int spacing_title = menu.headline1_spacing_info[info_mode];
  int ystep_title = getMenuTextStep(spacing_title, font_title);
  int ystart1 = mSY - SY + MENU_SCREEN_INFO_YSTART1;
  int ystart2 = ystart1 + ystep_title;
  int ybottom = mSY - SY + MENU_SCREEN_INFO_YBOTTOM;

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO);

  FadeOut(REDRAW_FIELD);

  ClearField();
  DrawHeadline();

  DrawTextSCentered(ystart1, font_title, text_title);
  DrawTextSCentered(ystart2, font_error, text_error);

  DrawTextSCentered(ybottom, font_foot,
		    "Press any key or button for info menu");

  FadeIn(REDRAW_FIELD);
}

void DrawInfoScreen_HelpAnim(int start, int max_anims, boolean init)
{
  static int infoscreen_step[MAX_INFO_ELEMENTS_ON_SCREEN];
  static int infoscreen_frame[MAX_INFO_ELEMENTS_ON_SCREEN];
  int font_title = MENU_INFO_FONT_TITLE;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int xstart  = mSX + MENU_SCREEN_INFO_SPACE_LEFT;
  int ystart1 = mSY - SY + MENU_SCREEN_INFO_YSTART1;
  int ystart2 = mSY + MENU_SCREEN_INFO_YSTART2;
  int ybottom = mSY - SY + MENU_SCREEN_INFO_YBOTTOM;
  int ystep = MENU_SCREEN_INFO_YSTEP;
  int element, action, direction;
  int graphic;
  int delay;
  int sync_frame;
  int i, j;

  if (init)
  {
    for (i = 0; i < NUM_INFO_ELEMENTS_ON_SCREEN; i++)
      infoscreen_step[i] = infoscreen_frame[i] = 0;

    ClearField();
    DrawHeadline();

    DrawTextSCentered(ystart1, font_title, "The Game Elements:");

    DrawTextSCentered(ybottom, font_foot,
		      "Press any key or button for next page");

    FrameCounter = 0;
  }

  i = j = 0;
  while (helpanim_info[j].element != HELPANIM_LIST_END)
  {
    if (i >= start + NUM_INFO_ELEMENTS_ON_SCREEN ||
	i >= max_anims)
      break;
    else if (i < start)
    {
      while (helpanim_info[j].element != HELPANIM_LIST_NEXT)
	j++;

      j++;
      i++;

      continue;
    }

    j += infoscreen_step[i - start];

    element = helpanim_info[j].element;
    action = helpanim_info[j].action;
    direction = helpanim_info[j].direction;

    if (element < 0)
      element = EL_UNKNOWN;

    if (action != -1 && direction != -1)
      graphic = el_act_dir2img(element, action, direction);
    else if (action != -1)
      graphic = el_act2img(element, action);
    else if (direction != -1)
      graphic = el_dir2img(element, direction);
    else
      graphic = el2img(element);

    delay = helpanim_info[j++].delay;

    if (delay == -1)
      delay = 1000000;

    if (infoscreen_frame[i - start] == 0)
    {
      sync_frame = 0;
      infoscreen_frame[i - start] = delay - 1;
    }
    else
    {
      sync_frame = delay - infoscreen_frame[i - start];
      infoscreen_frame[i - start]--;
    }

    if (helpanim_info[j].element == HELPANIM_LIST_NEXT)
    {
      if (!infoscreen_frame[i - start])
	infoscreen_step[i - start] = 0;
    }
    else
    {
      if (!infoscreen_frame[i - start])
	infoscreen_step[i - start]++;
      while (helpanim_info[j].element != HELPANIM_LIST_NEXT)
	j++;
    }

    j++;

    ClearRectangleOnBackground(drawto, xstart, ystart2 + (i - start) * ystep,
			       TILEX, TILEY);
    DrawFixedGraphicAnimationExt(drawto, xstart, ystart2 + (i - start) * ystep,
				 graphic, sync_frame, USE_MASKING);

    if (init)
      DrawInfoScreen_HelpText(element, action, direction, i - start);

    i++;
  }

  redraw_mask |= REDRAW_FIELD;

  FrameCounter++;
}

static char *getHelpText(int element, int action, int direction)
{
  char token[MAX_LINE_LEN];

  strcpy(token, element_info[element].token_name);

  if (action != -1)
    strcat(token, element_action_info[action].suffix);

  if (direction != -1)
    strcat(token, element_direction_info[MV_DIR_TO_BIT(direction)].suffix);

  return getHashEntry(helptext_info, token);
}

void DrawInfoScreen_HelpText(int element, int action, int direction, int ypos)
{
  int font_nr = FONT_INFO_ELEMENTS;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int yoffset = (TILEX - 2 * font_height) / 2;
  int xstart = mSX + MENU_SCREEN_INFO_SPACE_LEFT + TILEX + MINI_TILEX;
  int ystart = mSY + MENU_SCREEN_INFO_YSTART2 + yoffset;
  int ystep = TILEY + 4;
  int pad_left = xstart - SX;
  int pad_right = MENU_SCREEN_INFO_SPACE_RIGHT;
  int max_chars_per_line = (SXSIZE - pad_left - pad_right) / font_width;
  int max_lines_per_text = 2;    
  char *text = NULL;

  if (action != -1 && direction != -1)		/* element.action.direction */
    text = getHelpText(element, action, direction);

  if (text == NULL && action != -1)		/* element.action */
    text = getHelpText(element, action, -1);

  if (text == NULL && direction != -1)		/* element.direction */
    text = getHelpText(element, -1, direction);

  if (text == NULL)				/* base element */
    text = getHelpText(element, -1, -1);

  if (text == NULL)				/* not found */
    text = "No description available";

  if (strlen(text) <= max_chars_per_line)	/* only one line of text */
    ystart += getFontHeight(font_nr) / 2;

  DrawTextBuffer(xstart, ystart + ypos * ystep, text, font_nr,
		 max_chars_per_line, -1, max_lines_per_text, 0, -1,
		 TRUE, FALSE, FALSE);
}

void DrawInfoScreen_TitleScreen()
{
  SetGameStatus(GAME_MODE_TITLE);

  DrawTitleScreen();
}

void HandleInfoScreen_TitleScreen(int button)
{
  HandleTitleScreen(0, 0, 0, 0, button);
}

void DrawInfoScreen_Elements()
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_ELEMENTS);

  FadeOut(REDRAW_FIELD);

  LoadHelpAnimInfo();
  LoadHelpTextInfo();

  HandleInfoScreen_Elements(MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Elements(int button)
{
  static unsigned int info_delay = 0;
  static int num_anims;
  static int num_pages;
  static int page;
  int anims_per_page = NUM_INFO_ELEMENTS_ON_SCREEN;
  int i;

  if (button == MB_MENU_INITIALIZE)
  {
    boolean new_element = TRUE;

    num_anims = 0;

    for (i = 0; helpanim_info[i].element != HELPANIM_LIST_END; i++)
    {
      if (helpanim_info[i].element == HELPANIM_LIST_NEXT)
	new_element = TRUE;
      else if (new_element)
      {
	num_anims++;
	new_element = FALSE;
      }
    }

    num_pages = (num_anims + anims_per_page - 1) / anims_per_page;
    page = 0;
  }

  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE || button == MB_MENU_INITIALIZE)
  {
    if (button != MB_MENU_INITIALIZE)
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      page++;
    }

    if (page >= num_pages)
    {
      FadeMenuSoundsAndMusic();

      info_mode = INFO_MODE_MAIN;
      DrawInfoScreen();

      return;
    }

    if (page > 0)
      FadeSetNextScreen();

    if (button != MB_MENU_INITIALIZE)
      FadeOut(REDRAW_FIELD);

    DrawInfoScreen_HelpAnim(page * anims_per_page, num_anims, TRUE);

    if (button != MB_MENU_INITIALIZE)
      FadeIn(REDRAW_FIELD);
  }
  else
  {
    if (DelayReached(&info_delay, GameFrameDelay))
      if (page < num_pages)
	DrawInfoScreen_HelpAnim(page * anims_per_page, num_anims, FALSE);

    PlayMenuSoundIfLoop();
  }
}

void DrawInfoScreen_Music()
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_MUSIC);

  FadeOut(REDRAW_FIELD);

  ClearField();
  DrawHeadline();

  LoadMusicInfo();

  HandleInfoScreen_Music(MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Music(int button)
{
  static struct MusicFileInfo *list = NULL;
  int font_title = MENU_INFO_FONT_TITLE;
  int font_head  = MENU_INFO_FONT_HEAD;
  int font_text  = MENU_INFO_FONT_TEXT;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int spacing_title = menu.headline1_spacing_info[info_mode];
  int spacing_head  = menu.headline2_spacing_info[info_mode];
  int ystep_title = getMenuTextStep(spacing_title, font_title);
  int ystep_head  = getMenuTextStep(spacing_head,  font_head);
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART1;
  int ybottom = mSY - SY + MENU_SCREEN_INFO_YBOTTOM;

  if (button == MB_MENU_INITIALIZE)
  {
    list = music_file_info;

    if (list == NULL)
    {
      FadeMenuSoundsAndMusic();

      ClearField();
      DrawHeadline();

      DrawTextSCentered(ystart, font_title,
			"No music info for this level set.");

      DrawTextSCentered(ybottom, font_foot,
			"Press any key or button for info menu");

      return;
    }
  }

  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeMenuSoundsAndMusic();

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE || button == MB_MENU_INITIALIZE)
  {
    if (button != MB_MENU_INITIALIZE)
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      if (list != NULL)
	list = list->next;
    }

    if (list == NULL)
    {
      FadeMenuSoundsAndMusic();

      info_mode = INFO_MODE_MAIN;
      DrawInfoScreen();

      return;
    }

    FadeMenuSoundsAndMusic();

    if (list != music_file_info)
      FadeSetNextScreen();

    if (button != MB_MENU_INITIALIZE)
      FadeOut(REDRAW_FIELD);

    ClearField();
    DrawHeadline();

    if (list->is_sound)
    {
      int sound = list->music;

      if (sound_info[sound].loop)
	PlaySoundLoop(sound);
      else
	PlaySound(sound);

      DrawTextSCentered(ystart, font_title, "The Game Background Sounds:");
    }
    else
    {
      PlayMusic(list->music);

      DrawTextSCentered(ystart, font_title, "The Game Background Music:");
    }

    ystart += ystep_title;

    if (!strEqual(list->title, UNKNOWN_NAME))
    {
      if (!strEqual(list->title_header, UNKNOWN_NAME))
      {
	DrawTextSCentered(ystart, font_head, list->title_header);
	ystart += ystep_head;
      }

      DrawTextFCentered(ystart, font_text, "\"%s\"", list->title);
      ystart += ystep_head;
    }

    if (!strEqual(list->artist, UNKNOWN_NAME))
    {
      if (!strEqual(list->artist_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->artist_header);
      else
	DrawTextSCentered(ystart, font_head, "by");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "%s", list->artist);
      ystart += ystep_head;
    }

    if (!strEqual(list->album, UNKNOWN_NAME))
    {
      if (!strEqual(list->album_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->album_header);
      else
	DrawTextSCentered(ystart, font_head, "from the album");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "\"%s\"", list->album);
      ystart += ystep_head;
    }

    if (!strEqual(list->year, UNKNOWN_NAME))
    {
      if (!strEqual(list->year_header, UNKNOWN_NAME))
	DrawTextSCentered(ystart, font_head, list->year_header);
      else
	DrawTextSCentered(ystart, font_head, "from the year");

      ystart += ystep_head;

      DrawTextFCentered(ystart, font_text, "%s", list->year);
      ystart += ystep_head;
    }

    DrawTextSCentered(ybottom, FONT_TEXT_4,
		      "Press any key or button for next page");

    if (button != MB_MENU_INITIALIZE)
      FadeIn(REDRAW_FIELD);
  }

  if (list != NULL && list->is_sound && sound_info[list->music].loop)
    PlaySoundLoop(list->music);
}

static void DrawInfoScreen_CreditsScreen(int screen_nr)
{
  int font_title = MENU_INFO_FONT_TITLE;
  int font_head  = MENU_INFO_FONT_HEAD;
  int font_text  = MENU_INFO_FONT_TEXT;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int spacing_title = menu.headline1_spacing_info[info_mode];
  int spacing_head  = menu.headline2_spacing_info[info_mode];
  int spacing_para  = menu.paragraph_spacing_info[info_mode];
  int spacing_line  = menu.line_spacing_info[info_mode];
  int ystep_title = getMenuTextStep(spacing_title, font_title);
  int ystep_head  = getMenuTextStep(spacing_head,  font_head);
  int ystep_para  = getMenuTextStep(spacing_para,  font_text);
  int ystep_line  = getMenuTextStep(spacing_line,  font_text);
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART1;
  int ybottom = mSY - SY + MENU_SCREEN_INFO_YBOTTOM;

  ClearField();
  DrawHeadline();

  DrawTextSCentered(ystart, font_title, "Credits:");
  ystart += ystep_title;

  if (screen_nr == 0)
  {
    DrawTextSCentered(ystart, font_head,
		      "Special thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Peter Liepa");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for creating");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "\"Boulder Dash\"");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "in the year");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "1984");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "published by");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "First Star Software");
  }
  else if (screen_nr == 1)
  {
    DrawTextSCentered(ystart, font_head,
		      "Special thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Klaus Heinz & Volker Wertich");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for creating");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "\"Emerald Mine\"");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "in the year");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "1987");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "published by");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Kingsoft");
  }
  else if (screen_nr == 2)
  {
    DrawTextSCentered(ystart, font_head,
		      "Special thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Michael Stopp & Philip Jespersen");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for creating");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "\"Supaplex\"");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "in the year");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "1991");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "published by");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Digital Integration");
  }
  else if (screen_nr == 3)
  {
    DrawTextSCentered(ystart, font_head,
		      "Special thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Hiroyuki Imabayashi");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for creating");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "\"Sokoban\"");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "in the year");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "1982");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "published by");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Thinking Rabbit");
  }
  else if (screen_nr == 4)
  {
    DrawTextSCentered(ystart, font_head,
		      "Special thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Alan Bond");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "and");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "J\xfcrgen Bonhagen");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for the continuous creation");
    ystart += ystep_line;
    DrawTextSCentered(ystart, font_head,
		      "of outstanding level sets");
  }
  else if (screen_nr == 5)
  {
    DrawTextSCentered(ystart, font_head,
		      "Thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Peter Elzner");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for ideas and inspiration by");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Diamond Caves");
    ystart += ystep_para;

    DrawTextSCentered(ystart, font_head,
		      "Thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Steffest");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for ideas and inspiration by");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "DX-Boulderdash");
  }
  else if (screen_nr == 6)
  {
    DrawTextSCentered(ystart, font_head,
		      "Thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "David Tritscher");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for the code base used for the");
    ystart += ystep_line;
    DrawTextSCentered(ystart, font_head,
		      "native Emerald Mine engine");
  }
  else if (screen_nr == 7)
  {
    DrawTextSCentered(ystart, font_head,
		      "Thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Guido Schulz");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for the initial DOS port");
    ystart += ystep_para;

    DrawTextSCentered(ystart, font_head,
		      "Thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "Karl H\xf6rnell");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "for some additional toons");
  }
  else if (screen_nr == 8)
  {
    DrawTextSCentered(ystart, font_head,
		      "And not to forget:");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_head,
		      "Many thanks to");
    ystart += ystep_head;
    DrawTextSCentered(ystart, font_text,
		      "All those who contributed");
    ystart += ystep_line;
    DrawTextSCentered(ystart, font_text,
		      "levels to this game");
    ystart += ystep_line;
    DrawTextSCentered(ystart, font_text,
		      "since 1995");
  }

  DrawTextSCentered(ybottom, font_foot,
		    "Press any key or button for next page");
}

void DrawInfoScreen_Credits()
{
  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_CREDITS);

  FadeMenuSoundsAndMusic();

  FadeOut(REDRAW_FIELD);

  HandleInfoScreen_Credits(MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Credits(int button)
{
  static int screen_nr = 0;
  int num_screens = 9;

  if (button == MB_MENU_INITIALIZE)
  {
    screen_nr = 0;

    // DrawInfoScreen_CreditsScreen(screen_nr);
  }

  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE || button == MB_MENU_INITIALIZE)
  {
    if (button != MB_MENU_INITIALIZE)
    {
      PlaySound(SND_MENU_ITEM_SELECTING);

      screen_nr++;
    }

    if (screen_nr >= num_screens)
    {
      FadeMenuSoundsAndMusic();

      info_mode = INFO_MODE_MAIN;
      DrawInfoScreen();

      return;
    }

    if (screen_nr > 0)
      FadeSetNextScreen();

    if (button != MB_MENU_INITIALIZE)
      FadeOut(REDRAW_FIELD);

    DrawInfoScreen_CreditsScreen(screen_nr);

    if (button != MB_MENU_INITIALIZE)
      FadeIn(REDRAW_FIELD);
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

void DrawInfoScreen_Program()
{
  int font_title = MENU_INFO_FONT_TITLE;
  int font_head  = MENU_INFO_FONT_HEAD;
  int font_text  = MENU_INFO_FONT_TEXT;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int spacing_title = menu.headline1_spacing_info[info_mode];
  int spacing_head  = menu.headline2_spacing_info[info_mode];
  int spacing_para  = menu.paragraph_spacing_info[info_mode];
  int spacing_line  = menu.line_spacing_info[info_mode];
  int ystep_title = getMenuTextStep(spacing_title, font_title);
  int ystep_head  = getMenuTextStep(spacing_head,  font_head);
  int ystep_para  = getMenuTextStep(spacing_para,  font_text);
  int ystep_line  = getMenuTextStep(spacing_line,  font_text);
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART1;
  int ybottom = mSY - SY + MENU_SCREEN_INFO_YBOTTOM;

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_PROGRAM);

  FadeOut(REDRAW_FIELD);

  ClearField();
  DrawHeadline();

  DrawTextSCentered(ystart, font_title, "Program Information:");
  ystart += ystep_title;

  DrawTextSCentered(ystart, font_head,
		    "This game is Freeware!");
  ystart += ystep_head;
  DrawTextSCentered(ystart, font_head,
		    "If you like it, send e-mail to:");
  ystart += ystep_head;
  DrawTextSCentered(ystart, font_text,
		    setup.internal.program_email);
  ystart += ystep_para;

  DrawTextSCentered(ystart, font_head,
		    "More information and levels:");
  ystart += ystep_head;
  DrawTextSCentered(ystart, font_text,
		    setup.internal.program_website);
  ystart += ystep_para;

  DrawTextSCentered(ystart, font_head,
		    "If you have created new levels,");
  ystart += ystep_line;
  DrawTextSCentered(ystart, font_head,
		    "send them to me to include them!");
  ystart += ystep_head;
  DrawTextSCentered(ystart, font_head,
		    ":-)");

  DrawTextSCentered(ybottom, font_foot,
		    "Press any key or button for info menu");

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Program(int button)
{
  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeMenuSoundsAndMusic();

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

void DrawInfoScreen_Version()
{
  int font_title = MENU_INFO_FONT_TITLE;
  int font_head  = MENU_INFO_FONT_HEAD;
  int font_text  = MENU_INFO_FONT_TEXT;
  int font_foot  = MENU_INFO_FONT_FOOT;
  int spacing_title = menu.headline1_spacing_info[info_mode];
  int spacing_head  = menu.headline2_spacing_info[info_mode];
  int spacing_para  = menu.paragraph_spacing_info[info_mode];
  int spacing_line  = menu.line_spacing_info[info_mode];
  int xstep = getFontWidth(font_text);
  int ystep_title = getMenuTextStep(spacing_title, font_title);
  int ystep_head  = getMenuTextStep(spacing_head,  font_head);
  int ystep_para  = getMenuTextStep(spacing_para,  font_text);
  int ystep_line  = getMenuTextStep(spacing_line,  font_text);
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART1;
  int ybottom = mSY - SY + MENU_SCREEN_INFO_YBOTTOM;
  int xstart1 = mSX - SX + 2 * xstep;
  int xstart2 = mSX - SX + 18 * xstep;
  int xstart3 = mSX - SX + 28 * xstep;
  SDL_version sdl_version_compiled;
  const SDL_version *sdl_version_linked;
  int driver_name_len = 10;
#if defined(TARGET_SDL2)
  SDL_version sdl_version_linked_ext;
  const char *driver_name = NULL;
#else
  char driver_name[driver_name_len];
#endif

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_VERSION);

  FadeOut(REDRAW_FIELD);

  ClearField();
  DrawHeadline();

  DrawTextSCentered(ystart, font_title, "Version Information:");
  ystart += ystep_title;

  DrawTextF(xstart1, ystart, font_head, "Name");
  DrawTextF(xstart2, ystart, font_text, getProgramTitleString());
  ystart += ystep_line;

  if (!strEqual(getProgramVersionString(), getProgramRealVersionString()))
  {
    DrawTextF(xstart1, ystart, font_head, "Version (fake)");
    DrawTextF(xstart2, ystart, font_text, getProgramVersionString());
    ystart += ystep_line;

    DrawTextF(xstart1, ystart, font_head, "Version (real)");
    DrawTextF(xstart2, ystart, font_text, getProgramRealVersionString());
    ystart += ystep_line;
  }
  else
  {
    DrawTextF(xstart1, ystart, font_head, "Version");
    DrawTextF(xstart2, ystart, font_text, getProgramVersionString());
    ystart += ystep_line;
  }

  DrawTextF(xstart1, ystart, font_head, "Platform");
  DrawTextF(xstart2, ystart, font_text, PLATFORM_STRING);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Target");
  DrawTextF(xstart2, ystart, font_text, TARGET_STRING);
  ystart += ystep_line;

  DrawTextF(xstart1, ystart, font_head, "Source date");
  DrawTextF(xstart2, ystart, font_text, getSourceDateString());
  ystart += ystep_para;

  DrawTextF(xstart1, ystart, font_head, "Library");
  DrawTextF(xstart2, ystart, font_head, "compiled");
  DrawTextF(xstart3, ystart, font_head, "linked");
  ystart += ystep_head;

  SDL_VERSION(&sdl_version_compiled);
#if defined(TARGET_SDL2)
  SDL_GetVersion(&sdl_version_linked_ext);
  sdl_version_linked = &sdl_version_linked_ext;
#else
  sdl_version_linked = SDL_Linked_Version();
#endif

  DrawTextF(xstart1, ystart, font_text, "SDL");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_line;

  SDL_IMAGE_VERSION(&sdl_version_compiled);
  sdl_version_linked = IMG_Linked_Version();

  DrawTextF(xstart1, ystart, font_text, "SDL_image");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_line;

  SDL_MIXER_VERSION(&sdl_version_compiled);
  sdl_version_linked = Mix_Linked_Version();

  DrawTextF(xstart1, ystart, font_text, "SDL_mixer");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_line;

  SDL_NET_VERSION(&sdl_version_compiled);
  sdl_version_linked = SDLNet_Linked_Version();

  DrawTextF(xstart1, ystart, font_text, "SDL_net");
  DrawTextF(xstart2, ystart, font_text, "%d.%d.%d",
	    sdl_version_compiled.major,
	    sdl_version_compiled.minor,
	    sdl_version_compiled.patch);
  DrawTextF(xstart3, ystart, font_text, "%d.%d.%d",
	    sdl_version_linked->major,
	    sdl_version_linked->minor,
	    sdl_version_linked->patch);
  ystart += ystep_para;

  DrawTextF(xstart1, ystart, font_head, "Driver");
  DrawTextF(xstart2, ystart, font_head, "Requested");
  DrawTextF(xstart3, ystart, font_head, "Used");
  ystart += ystep_head;

#if defined(TARGET_SDL2)
  driver_name = getStringCopyNStatic(SDL_GetVideoDriver(0), driver_name_len);
#else
  SDL_VideoDriverName(driver_name, driver_name_len);
#endif

  DrawTextF(xstart1, ystart, font_text, "SDL_VideoDriver");
  DrawTextF(xstart2, ystart, font_text, "%s", setup.system.sdl_videodriver);
  DrawTextF(xstart3, ystart, font_text, "%s", driver_name);
  ystart += ystep_line;

#if defined(TARGET_SDL2)
  driver_name = getStringCopyNStatic(SDL_GetAudioDriver(0), driver_name_len);
#else
  SDL_AudioDriverName(driver_name, driver_name_len);
#endif

  DrawTextF(xstart1, ystart, font_text, "SDL_AudioDriver");
  DrawTextF(xstart2, ystart, font_text, "%s", setup.system.sdl_audiodriver);
  DrawTextF(xstart3, ystart, font_text, "%s", driver_name);

  DrawTextSCentered(ybottom, font_foot,
		    "Press any key or button for info menu");

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_Version(int button)
{
  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeMenuSoundsAndMusic();

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

void DrawInfoScreen_LevelSet()
{
  struct TitleMessageInfo *tmi = &readme;
  char *filename = getLevelSetInfoFilename();
  char *title = "Level Set Information:";
  int ystart  = mSY - SY + MENU_SCREEN_INFO_YSTART1;
  int ybottom = mSY - SY + MENU_SCREEN_INFO_YBOTTOM;

  if (filename == NULL)
  {
    DrawInfoScreen_NotAvailable(title, "No information for this level set.");

    return;
  }

  SetMainBackgroundImageIfDefined(IMG_BACKGROUND_INFO_LEVELSET);

  FadeOut(REDRAW_FIELD);

  ClearField();
  DrawHeadline();

  DrawTextSCentered(ystart, FONT_TEXT_1, title);

  /* if x position set to "-1", automatically determine by playfield width */
  if (tmi->x == -1)
    tmi->x = SXSIZE / 2;

  /* if y position set to "-1", use static default value */
  if (tmi->y == -1)
    tmi->y = 150;

  /* if width set to "-1", automatically determine by playfield width */
  if (tmi->width == -1)
    tmi->width = SXSIZE - 2 * TILEX;

  /* if height set to "-1", automatically determine by playfield height */
  if (tmi->height == -1)
    tmi->height = MENU_SCREEN_INFO_YBOTTOM - tmi->y - 10;

  /* if chars set to "-1", automatically determine by text and font width */
  if (tmi->chars == -1)
    tmi->chars = tmi->width / getFontWidth(tmi->font);
  else
    tmi->width = tmi->chars * getFontWidth(tmi->font);

  /* if lines set to "-1", automatically determine by text and font height */
  if (tmi->lines == -1)
    tmi->lines = tmi->height / getFontHeight(tmi->font);
  else
    tmi->height = tmi->lines * getFontHeight(tmi->font);

  DrawTextFile(mSX + ALIGNED_TEXT_XPOS(tmi), mSY + ALIGNED_TEXT_YPOS(tmi),
	       filename, tmi->font, tmi->chars, -1, tmi->lines, 0, -1,
	       tmi->autowrap, tmi->centered, tmi->parse_comments);

  DrawTextSCentered(ybottom, FONT_TEXT_4,
		    "Press any key or button for info menu");

  FadeIn(REDRAW_FIELD);
}

void HandleInfoScreen_LevelSet(int button)
{
  if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();

    return;
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeMenuSoundsAndMusic();

    info_mode = INFO_MODE_MAIN;
    DrawInfoScreen();
  }
  else
  {
    PlayMenuSoundIfLoop();
  }
}

static void DrawInfoScreen()
{
  if (info_mode == INFO_MODE_TITLE)
    DrawInfoScreen_TitleScreen();
  else if (info_mode == INFO_MODE_ELEMENTS)
    DrawInfoScreen_Elements();
  else if (info_mode == INFO_MODE_MUSIC)
    DrawInfoScreen_Music();
  else if (info_mode == INFO_MODE_CREDITS)
    DrawInfoScreen_Credits();
  else if (info_mode == INFO_MODE_PROGRAM)
    DrawInfoScreen_Program();
  else if (info_mode == INFO_MODE_VERSION)
    DrawInfoScreen_Version();
  else if (info_mode == INFO_MODE_LEVELSET)
    DrawInfoScreen_LevelSet();
  else
    DrawInfoScreen_Main();

  if (info_mode != INFO_MODE_MAIN &&
      info_mode != INFO_MODE_TITLE &&
      info_mode != INFO_MODE_MUSIC)
    PlayMenuSoundsAndMusic();
}

void HandleInfoScreen(int mx, int my, int dx, int dy, int button)
{
  if (info_mode == INFO_MODE_TITLE)
    HandleInfoScreen_TitleScreen(button);
  else if (info_mode == INFO_MODE_ELEMENTS)
    HandleInfoScreen_Elements(button);
  else if (info_mode == INFO_MODE_MUSIC)
    HandleInfoScreen_Music(button);
  else if (info_mode == INFO_MODE_CREDITS)
    HandleInfoScreen_Credits(button);
  else if (info_mode == INFO_MODE_PROGRAM)
    HandleInfoScreen_Program(button);
  else if (info_mode == INFO_MODE_VERSION)
    HandleInfoScreen_Version(button);
  else if (info_mode == INFO_MODE_LEVELSET)
    HandleInfoScreen_LevelSet(button);
  else
    HandleInfoScreen_Main(mx, my, dx, dy, button);
}


/* ========================================================================= */
/* type name functions                                                       */
/* ========================================================================= */

void HandleTypeName(int newxpos, Key key)
{
  static char last_player_name[MAX_PLAYER_NAME_LEN + 1];
  struct MainControlInfo *mci = getMainControlInfo(MAIN_CONTROL_NAME);
  struct TextPosInfo *pos = mci->pos_input;
  int startx = mSX + ALIGNED_TEXT_XPOS(pos);
  int starty = mSY + ALIGNED_TEXT_YPOS(pos);
  static int xpos = 0;
  int font_nr = pos->font;
  int font_active_nr = FONT_ACTIVE(font_nr);
  int font_width = getFontWidth(font_active_nr);
  char key_char = getValidConfigValueChar(getCharFromKey(key));
  boolean is_valid_key_char = (key_char != 0 && (key_char != ' ' || xpos > 0));
  boolean is_active = TRUE;

  DrawBackgroundForFont(startx,starty, pos->width, pos->height, font_active_nr);

  if (newxpos)
  {
    strcpy(last_player_name, setup.player_name);

    xpos = newxpos;

    StartTextInput(startx, starty, pos->width, pos->height);
  }
  else if (is_valid_key_char && xpos < MAX_PLAYER_NAME_LEN)
  {
    setup.player_name[xpos] = key_char;
    setup.player_name[xpos + 1] = 0;

    xpos++;
  }
  else if ((key == KSYM_Delete || key == KSYM_BackSpace) && xpos > 0)
  {
    xpos--;

    setup.player_name[xpos] = 0;
  }
  else if (key == KSYM_Return && xpos > 0)
  {
    SaveSetup();

    is_active = FALSE;

    SetGameStatus(GAME_MODE_MAIN);
  }
  else if (key == KSYM_Escape)
  {
    strcpy(setup.player_name, last_player_name);

    is_active = FALSE;

    SetGameStatus(GAME_MODE_MAIN);
  }

  if (is_active)
  {
    pos->width = (strlen(setup.player_name) + 1) * font_width;
    startx = mSX + ALIGNED_TEXT_XPOS(pos);

    DrawText(startx, starty, setup.player_name, font_active_nr);
    DrawText(startx + xpos * font_width, starty, "_", font_active_nr);
  }
  else
  {
    pos->width = strlen(setup.player_name) * font_width;
    startx = mSX + ALIGNED_TEXT_XPOS(pos);

    DrawText(startx, starty, setup.player_name, font_nr);

    StopTextInput();
  }
}


/* ========================================================================= */
/* tree menu functions                                                       */
/* ========================================================================= */

static void DrawChooseTree(TreeInfo **ti_ptr)
{
  int fade_mask = REDRAW_FIELD;

  if (CheckIfGlobalBorderOrPlayfieldViewportHasChanged())
    fade_mask = REDRAW_ALL;

  if (strEqual((*ti_ptr)->subdir, STRING_TOP_DIRECTORY))
  {
    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();

    return;
  }

  UnmapAllGadgets();

  FreeScreenGadgets();
  CreateScreenGadgets();

  FadeOut(fade_mask);

  /* needed if different viewport properties defined for choosing level (set) */
  ChangeViewportPropertiesIfNeeded();

  if (game_status == GAME_MODE_LEVELNR)
    SetMainBackgroundImage(IMG_BACKGROUND_LEVELNR);
  else if (game_status == GAME_MODE_LEVELS)
    SetMainBackgroundImage(IMG_BACKGROUND_LEVELS);

  ClearField();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  HandleChooseTree(0, 0, 0, 0, MB_MENU_INITIALIZE, ti_ptr);
  MapScreenTreeGadgets(*ti_ptr);

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
}

static void drawChooseTreeList(int first_entry, int num_page_entries,
			       TreeInfo *ti)
{
  int i;
  char *title_string = NULL;
  int yoffset_sets = MENU_TITLE1_YPOS;
  int yoffset_setup = 16;
  int yoffset = (ti->type == TREE_TYPE_LEVEL_DIR ||
		 ti->type == TREE_TYPE_LEVEL_NR ? yoffset_sets : yoffset_setup);

  title_string = ti->infotext;

  DrawTextSCentered(mSY - SY + yoffset, FONT_TITLE_1, title_string);

  clearMenuListArea();

  for (i = 0; i < num_page_entries; i++)
  {
    TreeInfo *node, *node_first;
    int entry_pos = first_entry + i;
    int xpos = MENU_SCREEN_START_XPOS;
    int ypos = MENU_SCREEN_START_YPOS + i;
    int startx = mSX + xpos * 32;
    int starty = mSY + ypos * 32;
    int font_nr = FONT_TEXT_1;
    int font_xoffset = getFontBitmapInfo(font_nr)->draw_xoffset;
    int startx_text = startx + font_xoffset;
    int startx_scrollbar = mSX + SC_SCROLLBAR_XPOS + menu.scrollbar_xoffset;
    int text_size = startx_scrollbar - startx_text;
    int max_buffer_len = text_size / getFontWidth(font_nr);
    char buffer[max_buffer_len + 1];

    node_first = getTreeInfoFirstGroupEntry(ti);
    node = getTreeInfoFromPos(node_first, entry_pos);

    strncpy(buffer, node->name, max_buffer_len);
    buffer[max_buffer_len] = '\0';

    DrawText(startx, starty, buffer, font_nr + node->color);

    if (node->parent_link)
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (node->level_group)
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else
      initCursor(i, IMG_MENU_BUTTON);
  }

  redraw_mask |= REDRAW_FIELD;
}

static void drawChooseTreeInfo(int entry_pos, TreeInfo *ti)
{
  TreeInfo *node, *node_first;
  int x, last_redraw_mask = redraw_mask;
  int ypos = MENU_TITLE2_YPOS;
  int font_nr = FONT_TITLE_2;

  if (ti->type == TREE_TYPE_LEVEL_NR)
    DrawTextFCentered(ypos, font_nr, leveldir_current->name);

  if (ti->type != TREE_TYPE_LEVEL_DIR)
    return;

  node_first = getTreeInfoFirstGroupEntry(ti);
  node = getTreeInfoFromPos(node_first, entry_pos);

  DrawBackgroundForFont(SX, SY + ypos, SXSIZE, getFontHeight(font_nr), font_nr);

  if (node->parent_link)
    DrawTextFCentered(ypos, font_nr, "leave \"%s\"",
		      node->node_parent->name);
  else if (node->level_group)
    DrawTextFCentered(ypos, font_nr, "enter \"%s\"",
		      node->name);
  else if (ti->type == TREE_TYPE_LEVEL_DIR)
    DrawTextFCentered(ypos, font_nr, "%3d %s (%s)",
		      node->levels, (node->levels > 1 ? "levels" : "level"),
		      node->class_desc);

  /* let BackToFront() redraw only what is needed */
  redraw_mask = last_redraw_mask;
  for (x = 0; x < SCR_FIELDX; x++)
    MarkTileDirty(x, 1);
}

static void HandleChooseTree(int mx, int my, int dx, int dy, int button,
			     TreeInfo **ti_ptr)
{
  TreeInfo *ti = *ti_ptr;
  int x = 0;
  int y = ti->cl_cursor;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);
  int num_entries = numTreeInfoInGroup(ti);
  int num_page_entries;
  boolean position_set_by_scrollbar = (dx == 999);

  if (num_entries <= NUM_MENU_ENTRIES_ON_SCREEN)
    num_page_entries = num_entries;
  else
    num_page_entries = NUM_MENU_ENTRIES_ON_SCREEN;

  if (button == MB_MENU_INITIALIZE)
  {
    int num_entries = numTreeInfoInGroup(ti);
    int entry_pos = posTreeInfo(ti);

    if (ti->cl_first == -1)
    {
      /* only on initialization */
      ti->cl_first = MAX(0, entry_pos - num_page_entries + 1);
      ti->cl_cursor = entry_pos - ti->cl_first;
    }
    else if (ti->cl_cursor >= num_page_entries ||
	     (num_entries > num_page_entries &&
	      num_entries - ti->cl_first < num_page_entries))
    {
      /* only after change of list size (by custom graphic configuration) */
      ti->cl_first = MAX(0, entry_pos - num_page_entries + 1);
      ti->cl_cursor = entry_pos - ti->cl_first;
    }

    if (position_set_by_scrollbar)
      ti->cl_first = dy;
    else
      AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				ti->cl_first, ti);

    drawChooseTreeList(ti->cl_first, num_page_entries, ti);
    drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
    drawChooseTreeCursor(ti->cl_cursor, TRUE);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    FadeSetLeaveMenu();

    PlaySound(SND_MENU_ITEM_SELECTING);

    if (ti->node_parent)
    {
      *ti_ptr = ti->node_parent;
      DrawChooseTree(ti_ptr);
    }
    else if (game_status == GAME_MODE_SETUP)
    {
      if (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED ||
	  setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY ||
	  setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
	execSetupGame();
      else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE ||
	       setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE ||
	       setup_mode == SETUP_MODE_CHOOSE_RENDERING)
	execSetupGraphics();
      else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE ||
	       setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS ||
	       setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
	execSetupSound();
      else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL ||
	       setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE ||
	       setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE)
	execSetupTouch();
      else
	execSetupArtwork();
    }
    else
    {
      if (game_status == GAME_MODE_LEVELNR)
      {
	int new_level_nr = atoi(level_number_current->identifier);

	HandleMainMenu_SelectLevel(0, 0, new_level_nr);
      }

      SetGameStatus(GAME_MODE_MAIN);

      DrawMainMenu();
    }

    return;
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	/* keyboard or scrollbar/scrollbutton input */
  {
    /* move cursor instead of scrolling when already at start/end of list */
    if (dy == -1 * SCROLL_LINE && ti->cl_first == 0)
      dy = -1;
    else if (dy == +1 * SCROLL_LINE &&
	     ti->cl_first + num_page_entries == num_entries)
      dy = 1;

    /* handle scrolling screen one line or page */
    if (ti->cl_cursor + dy < 0 ||
	ti->cl_cursor + dy > num_page_entries - 1)
    {
      boolean redraw = FALSE;

      if (ABS(dy) == SCROLL_PAGE)
	step = num_page_entries - 1;

      if (dy < 0 && ti->cl_first > 0)
      {
	/* scroll page/line up */

	ti->cl_first -= step;
	if (ti->cl_first < 0)
	  ti->cl_first = 0;

	redraw = TRUE;
      }
      else if (dy > 0 && ti->cl_first + num_page_entries < num_entries)
      {
	/* scroll page/line down */

	ti->cl_first += step;
	if (ti->cl_first + num_page_entries > num_entries)
	  ti->cl_first = MAX(0, num_entries - num_page_entries);

	redraw = TRUE;
      }

      if (redraw)
      {
	drawChooseTreeList(ti->cl_first, num_page_entries, ti);
	drawChooseTreeInfo(ti->cl_first + ti->cl_cursor, ti);
	drawChooseTreeCursor(ti->cl_cursor, TRUE);

	AdjustChooseTreeScrollbar(SCREEN_CTRL_ID_SCROLL_VERTICAL,
				  ti->cl_first, ti);
      }

      return;
    }

    /* handle moving cursor one line */
    y = ti->cl_cursor + dy;
  }

  if (dx == 1)
  {
    TreeInfo *node_first, *node_cursor;
    int entry_pos = ti->cl_first + y;

    node_first = getTreeInfoFirstGroupEntry(ti);
    node_cursor = getTreeInfoFromPos(node_first, entry_pos);

    if (node_cursor->node_group)
    {
      FadeSetEnterMenu();

      PlaySound(SND_MENU_ITEM_SELECTING);

      node_cursor->cl_first = ti->cl_first;
      node_cursor->cl_cursor = ti->cl_cursor;
      *ti_ptr = node_cursor->node_group;
      DrawChooseTree(ti_ptr);

      return;
    }
  }
  else if (dx == -1 && ti->node_parent)
  {
    FadeSetLeaveMenu();

    PlaySound(SND_MENU_ITEM_SELECTING);

    *ti_ptr = ti->node_parent;
    DrawChooseTree(ti_ptr);

    return;
  }

  if (!anyScrollbarGadgetActive() &&
      IN_VIS_MENU(x, y) &&
      mx < screen_gadget[SCREEN_CTRL_ID_SCROLL_VERTICAL]->x &&
      y >= 0 && y < num_page_entries)
  {
    if (button)
    {
      if (y != ti->cl_cursor)
      {
	PlaySound(SND_MENU_ITEM_ACTIVATING);

	drawChooseTreeCursor(ti->cl_cursor, FALSE);
	drawChooseTreeCursor(y, TRUE);
	drawChooseTreeInfo(ti->cl_first + y, ti);

	ti->cl_cursor = y;
      }
      else if (dx < 0)
      {
	if (game_status == GAME_MODE_SETUP)
	{
	  if (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED ||
	      setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY ||
	      setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
	    execSetupGame();
	  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE ||
		   setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE ||
		   setup_mode == SETUP_MODE_CHOOSE_RENDERING)
	    execSetupGraphics();
	  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
	    execSetupSound();
	  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL ||
		   setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE ||
		   setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE)
	    execSetupTouch();
	  else
	    execSetupArtwork();
	}
      }
    }
    else
    {
      TreeInfo *node_first, *node_cursor;
      int entry_pos = ti->cl_first + y;

      PlaySound(SND_MENU_ITEM_SELECTING);

      node_first = getTreeInfoFirstGroupEntry(ti);
      node_cursor = getTreeInfoFromPos(node_first, entry_pos);

      if (node_cursor->node_group)
      {
	FadeSetEnterMenu();

	node_cursor->cl_first = ti->cl_first;
	node_cursor->cl_cursor = ti->cl_cursor;
	*ti_ptr = node_cursor->node_group;
	DrawChooseTree(ti_ptr);
      }
      else if (node_cursor->parent_link)
      {
	FadeSetLeaveMenu();

	*ti_ptr = node_cursor->node_parent;
	DrawChooseTree(ti_ptr);
      }
      else
      {
	FadeSetEnterMenu();

	node_cursor->cl_first = ti->cl_first;
	node_cursor->cl_cursor = ti->cl_cursor;
	*ti_ptr = node_cursor;

	if (ti->type == TREE_TYPE_LEVEL_DIR)
	{
	  LoadLevelSetup_SeriesInfo();

	  SaveLevelSetup_LastSeries();
	  SaveLevelSetup_SeriesInfo();
	  TapeErase();
	}

	if (game_status == GAME_MODE_SETUP)
	{
	  if (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED ||
	      setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY ||
	      setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
	    execSetupGame();
	  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE ||
		   setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE ||
		   setup_mode == SETUP_MODE_CHOOSE_RENDERING)
	    execSetupGraphics();
	  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS ||
		   setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
	    execSetupSound();
	  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL ||
		   setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE ||
		   setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE)
	    execSetupTouch();
	  else
	    execSetupArtwork();
	}
	else
	{
	  if (game_status == GAME_MODE_LEVELNR)
	  {
	    int new_level_nr = atoi(level_number_current->identifier);

	    HandleMainMenu_SelectLevel(0, 0, new_level_nr);
	  }

	  SetGameStatus(GAME_MODE_MAIN);

	  DrawMainMenu();
	}
      }
    }
  }
}

void DrawChooseLevelSet()
{
  FadeMenuSoundsAndMusic();

  DrawChooseTree(&leveldir_current);

  PlayMenuSoundsAndMusic();
}

void HandleChooseLevelSet(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &leveldir_current);
}

void DrawChooseLevelNr()
{
  int i;

  FadeMenuSoundsAndMusic();

  if (level_number != NULL)
  {
    freeTreeInfo(level_number);

    level_number = NULL;
  }

  for (i = leveldir_current->first_level; i <= leveldir_current->last_level;i++)
  {
    TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_LEVEL_NR);
    char identifier[32], name[32];
    int value = i;

    /* temporarily load level info to get level name */
    LoadLevelInfoOnly(i);

    ti->node_top = &level_number;
    ti->sort_priority = 10000 + value;
    ti->color = (level.no_level_file ? FC_BLUE :
		 LevelStats_getSolved(i) ? FC_GREEN :
		 LevelStats_getPlayed(i) ? FC_YELLOW : FC_RED);

    snprintf(identifier, sizeof(identifier), "%d", value);
    snprintf(name, sizeof(name), "%03d: %s", value,
	     (level.no_level_file ? "(no file)" : level.name));

    setString(&ti->identifier, identifier);
    setString(&ti->name, name);
    setString(&ti->name_sorting, name);

    pushTreeInfo(&level_number, ti);
  }

  /* sort level number values to start with lowest level number */
  sortTreeInfo(&level_number);

  /* set current level number to current level number */
  level_number_current =
    getTreeInfoFromIdentifier(level_number, i_to_a(level_nr));

  /* if that also fails, set current level number to first available level */
  if (level_number_current == NULL)
    level_number_current = level_number;

  DrawChooseTree(&level_number_current);

  PlayMenuSoundsAndMusic();
}

void HandleChooseLevelNr(int mx, int my, int dx, int dy, int button)
{
  HandleChooseTree(mx, my, dx, dy, button, &level_number_current);
}

void DrawHallOfFame(int highlight_position)
{
  int fade_mask = REDRAW_FIELD;

  if (CheckIfGlobalBorderOrPlayfieldViewportHasChanged())
    fade_mask = REDRAW_ALL;

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  /* (this is needed when called from GameEnd() after winning a game) */
  KeyboardAutoRepeatOn();

  /* (this is needed when called from GameEnd() after winning a game) */
  SetDrawDeactivationMask(REDRAW_NONE);
  SetDrawBackgroundMask(REDRAW_FIELD);

  if (highlight_position < 0) 
    LoadScore(level_nr);
  else
    SetAnimStatus(GAME_MODE_PSEUDO_SCORESNEW);

  FadeSetEnterScreen();

  FadeOut(fade_mask);

  /* needed if different viewport properties defined for scores */
  ChangeViewportPropertiesIfNeeded();

  PlayMenuSoundsAndMusic();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  HandleHallOfFame(highlight_position, 0, 0, 0, MB_MENU_INITIALIZE);

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
}

static void drawHallOfFameList(int first_entry, int highlight_position)
{
  int i, j;

  SetMainBackgroundImage(IMG_BACKGROUND_SCORES);
  ClearField();

  DrawTextSCentered(MENU_TITLE1_YPOS, FONT_TITLE_1, "Hall Of Fame");
  DrawTextFCentered(MENU_TITLE2_YPOS, FONT_TITLE_2,
		    "HighScores of Level %d", level_nr);

  for (i = 0; i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
  {
    int entry = first_entry + i;
    boolean active = (entry == highlight_position);
    int font_nr1 = (active ? FONT_TEXT_1_ACTIVE : FONT_TEXT_1);
    int font_nr2 = (active ? FONT_TEXT_2_ACTIVE : FONT_TEXT_2);
    int font_nr3 = (active ? FONT_TEXT_3_ACTIVE : FONT_TEXT_3);
    int font_nr4 = (active ? FONT_TEXT_4_ACTIVE : FONT_TEXT_4);
    int dxoff = getFontDrawOffsetX(font_nr1);
    int dx1 = 3 * getFontWidth(font_nr1);
    int dx2 = dx1 + getFontWidth(font_nr1);
    int dx3 = SXSIZE - 2 * (mSX - SX + dxoff) - 5 * getFontWidth(font_nr4);
    int num_dots = (dx3 - dx2) / getFontWidth(font_nr3);
    int sy = mSY + 64 + i * 32;

    DrawText(mSX, sy, int2str(entry + 1, 3), font_nr1);
    DrawText(mSX + dx1, sy, ".", font_nr1);

    for (j = 0; j < num_dots; j++)
      DrawText(mSX + dx2 + j * getFontWidth(font_nr3), sy, ".", font_nr3);

    if (!strEqual(highscore[entry].Name, EMPTY_PLAYER_NAME))
      DrawText(mSX + dx2, sy, highscore[entry].Name, font_nr2);

    DrawText(mSX + dx3, sy, int2str(highscore[entry].Score, 5), font_nr4);
  }

  redraw_mask |= REDRAW_FIELD;
}

void HandleHallOfFame(int mx, int my, int dx, int dy, int button)
{
  static int first_entry = 0;
  static int highlight_position = 0;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);

  if (button == MB_MENU_INITIALIZE)
  {
    first_entry = 0;
    highlight_position = mx;
    drawHallOfFameList(first_entry, highlight_position);

    return;
  }

  if (ABS(dy) == SCROLL_PAGE)		/* handle scrolling one page */
    step = NUM_MENU_ENTRIES_ON_SCREEN - 1;

  if (dy < 0)
  {
    if (first_entry > 0)
    {
      first_entry -= step;
      if (first_entry < 0)
	first_entry = 0;

      drawHallOfFameList(first_entry, highlight_position);
    }
  }
  else if (dy > 0)
  {
    if (first_entry + NUM_MENU_ENTRIES_ON_SCREEN < MAX_SCORE_ENTRIES)
    {
      first_entry += step;
      if (first_entry + NUM_MENU_ENTRIES_ON_SCREEN > MAX_SCORE_ENTRIES)
	first_entry = MAX(0, MAX_SCORE_ENTRIES - NUM_MENU_ENTRIES_ON_SCREEN);

      drawHallOfFameList(first_entry, highlight_position);
    }
  }
  else if (button == MB_MENU_LEAVE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeSound(SND_BACKGROUND_SCORES);

    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();
  }
  else if (button == MB_MENU_CHOICE)
  {
    PlaySound(SND_MENU_ITEM_SELECTING);

    FadeSound(SND_BACKGROUND_SCORES);

    SetGameStatus(GAME_MODE_MAIN);

    DrawMainMenu();
  }

  if (game_status == GAME_MODE_SCORES)
    PlayMenuSoundIfLoop();
}


/* ========================================================================= */
/* setup screen functions                                                    */
/* ========================================================================= */

static struct TokenInfo *setup_info;
static int num_setup_info;	/* number of setup entries shown on screen */
static int max_setup_info;	/* total number of setup entries in list */

static char *window_size_text;
static char *scaling_type_text;
static char *rendering_mode_text;
static char *scroll_delay_text;
static char *snapshot_mode_text;
static char *game_speed_text;
static char *graphics_set_name;
static char *sounds_set_name;
static char *music_set_name;
static char *volume_simple_text;
static char *volume_loops_text;
static char *volume_music_text;
static char *touch_controls_text;
static char *move_distance_text;
static char *drop_distance_text;

static void execSetupMain()
{
  setup_mode = SETUP_MODE_MAIN;

  DrawSetupScreen();
}

static void execSetupGame_setGameSpeeds()
{
  if (game_speeds == NULL)
  {
    int i;

    for (i = 0; game_speeds_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = game_speeds_list[i].value;
      char *text = game_speeds_list[i].text;

      ti->node_top = &game_speeds;
      ti->sort_priority = 10000 - value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_GAME_SPEED);

      pushTreeInfo(&game_speeds, ti);
    }

    /* sort game speed values to start with slowest game speed */
    sortTreeInfo(&game_speeds);

    /* set current game speed to configured game speed value */
    game_speed_current =
      getTreeInfoFromIdentifier(game_speeds, i_to_a(setup.game_frame_delay));

    /* if that fails, set current game speed to reliable default value */
    if (game_speed_current == NULL)
      game_speed_current =
	getTreeInfoFromIdentifier(game_speeds, i_to_a(GAME_FRAME_DELAY));

    /* if that also fails, set current game speed to first available speed */
    if (game_speed_current == NULL)
      game_speed_current = game_speeds;
  }

  setup.game_frame_delay = atoi(game_speed_current->identifier);

  /* needed for displaying game speed text instead of identifier */
  game_speed_text = game_speed_current->name;
}

static void execSetupGame_setScrollDelays()
{
  if (scroll_delays == NULL)
  {
    int i;

    for (i = 0; scroll_delays_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = scroll_delays_list[i].value;
      char *text = scroll_delays_list[i].text;

      ti->node_top = &scroll_delays;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_SCROLL_DELAY);

      pushTreeInfo(&scroll_delays, ti);
    }

    /* sort scroll delay values to start with lowest scroll delay value */
    sortTreeInfo(&scroll_delays);

    /* set current scroll delay value to configured scroll delay value */
    scroll_delay_current =
      getTreeInfoFromIdentifier(scroll_delays,i_to_a(setup.scroll_delay_value));

    /* if that fails, set current scroll delay to reliable default value */
    if (scroll_delay_current == NULL)
      scroll_delay_current =
	getTreeInfoFromIdentifier(scroll_delays, i_to_a(STD_SCROLL_DELAY));

    /* if that also fails, set current scroll delay to first available value */
    if (scroll_delay_current == NULL)
      scroll_delay_current = scroll_delays;
  }

  setup.scroll_delay_value = atoi(scroll_delay_current->identifier);

  /* needed for displaying scroll delay text instead of identifier */
  scroll_delay_text = scroll_delay_current->name;
}

static void execSetupGame_setSnapshotModes()
{
  if (snapshot_modes == NULL)
  {
    int i;

    for (i = 0; snapshot_modes_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = snapshot_modes_list[i].value;
      char *text = snapshot_modes_list[i].text;

      ti->node_top = &snapshot_modes;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_SNAPSHOT_MODE);

      pushTreeInfo(&snapshot_modes, ti);
    }

    /* sort snapshot mode values to start with lowest snapshot mode value */
    sortTreeInfo(&snapshot_modes);

    /* set current snapshot mode value to configured snapshot mode value */
    snapshot_mode_current =
      getTreeInfoFromIdentifier(snapshot_modes, setup.engine_snapshot_mode);

    /* if that fails, set current snapshot mode to reliable default value */
    if (snapshot_mode_current == NULL)
      snapshot_mode_current =
	getTreeInfoFromIdentifier(snapshot_modes, STR_SNAPSHOT_MODE_DEFAULT);

    /* if that also fails, set current snapshot mode to first available value */
    if (snapshot_mode_current == NULL)
      snapshot_mode_current = snapshot_modes;
  }

  setup.engine_snapshot_mode = snapshot_mode_current->identifier;

  /* needed for displaying snapshot mode text instead of identifier */
  snapshot_mode_text = snapshot_mode_current->name;
}

static void execSetupGame()
{
  execSetupGame_setGameSpeeds();
  execSetupGame_setScrollDelays();
  execSetupGame_setSnapshotModes();

  setup_mode = SETUP_MODE_GAME;

  DrawSetupScreen();
}

static void execSetupChooseGameSpeed()
{
  setup_mode = SETUP_MODE_CHOOSE_GAME_SPEED;

  DrawSetupScreen();
}

static void execSetupChooseScrollDelay()
{
  setup_mode = SETUP_MODE_CHOOSE_SCROLL_DELAY;

  DrawSetupScreen();
}

static void execSetupChooseSnapshotMode()
{
  setup_mode = SETUP_MODE_CHOOSE_SNAPSHOT_MODE;

  DrawSetupScreen();
}

static void execSetupEditor()
{
  setup_mode = SETUP_MODE_EDITOR;

  DrawSetupScreen();
}

static void execSetupGraphics_setWindowSizes(boolean update_list)
{
  if (window_sizes != NULL && update_list)
  {
    freeTreeInfo(window_sizes);

    window_sizes = NULL;
  }

  if (window_sizes == NULL)
  {
    boolean current_window_size_found = FALSE;
    int i;

    for (i = 0; window_sizes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = window_sizes_list[i].value;
      char *text = window_sizes_list[i].text;

      ti->node_top = &window_sizes;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_WINDOW_SIZE);

      pushTreeInfo(&window_sizes, ti);

      if (value == setup.window_scaling_percent)
	current_window_size_found = TRUE;
    }

    if (!current_window_size_found)
    {
      // add entry for non-preset window scaling value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.window_scaling_percent;

      ti->node_top = &window_sizes;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_WINDOW_SIZE);

      pushTreeInfo(&window_sizes, ti);
    }

    /* sort window size values to start with lowest window size value */
    sortTreeInfo(&window_sizes);

    /* set current window size value to configured window size value */
    window_size_current =
      getTreeInfoFromIdentifier(window_sizes,
				i_to_a(setup.window_scaling_percent));

    /* if that fails, set current window size to reliable default value */
    if (window_size_current == NULL)
      window_size_current =
	getTreeInfoFromIdentifier(window_sizes,
				  i_to_a(STD_WINDOW_SCALING_PERCENT));

    /* if that also fails, set current window size to first available value */
    if (window_size_current == NULL)
      window_size_current = window_sizes;
  }

  setup.window_scaling_percent = atoi(window_size_current->identifier);

  /* needed for displaying window size text instead of identifier */
  window_size_text = window_size_current->name;
}

static void execSetupGraphics_setScalingTypes()
{
  if (scaling_types == NULL)
  {
    int i;

    for (i = 0; scaling_types_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = scaling_types_list[i].value;
      char *text = scaling_types_list[i].text;

      ti->node_top = &scaling_types;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_SCALING_TYPE);

      pushTreeInfo(&scaling_types, ti);
    }

    /* sort scaling type values to start with lowest scaling type value */
    sortTreeInfo(&scaling_types);

    /* set current scaling type value to configured scaling type value */
    scaling_type_current =
      getTreeInfoFromIdentifier(scaling_types, setup.window_scaling_quality);

    /* if that fails, set current scaling type to reliable default value */
    if (scaling_type_current == NULL)
      scaling_type_current =
	getTreeInfoFromIdentifier(scaling_types, SCALING_QUALITY_DEFAULT);

    /* if that also fails, set current scaling type to first available value */
    if (scaling_type_current == NULL)
      scaling_type_current = scaling_types;
  }

  setup.window_scaling_quality = scaling_type_current->identifier;

  /* needed for displaying scaling type text instead of identifier */
  scaling_type_text = scaling_type_current->name;
}

static void execSetupGraphics_setRenderingModes()
{
  if (rendering_modes == NULL)
  {
    int i;

    for (i = 0; rendering_modes_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = rendering_modes_list[i].value;
      char *text = rendering_modes_list[i].text;

      ti->node_top = &rendering_modes;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_RENDERING);

      pushTreeInfo(&rendering_modes, ti);
    }

    /* sort rendering mode values to start with lowest rendering mode value */
    sortTreeInfo(&rendering_modes);

    /* set current rendering mode value to configured rendering mode value */
    rendering_mode_current =
      getTreeInfoFromIdentifier(rendering_modes, setup.screen_rendering_mode);

    /* if that fails, set current rendering mode to reliable default value */
    if (rendering_mode_current == NULL)
      rendering_mode_current =
	getTreeInfoFromIdentifier(rendering_modes,
				  STR_SPECIAL_RENDERING_DEFAULT);

    /* if that also fails, set current rendering mode to first available one */
    if (rendering_mode_current == NULL)
      rendering_mode_current = rendering_modes;
  }

  setup.screen_rendering_mode = rendering_mode_current->identifier;

  /* needed for displaying rendering mode text instead of identifier */
  rendering_mode_text = rendering_mode_current->name;
}

static void execSetupGraphics()
{
  // update "setup.window_scaling_percent" from list selection
  // (in this case, window scaling was changed on setup screen)
  if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
    execSetupGraphics_setWindowSizes(FALSE);

  // update list selection from "setup.window_scaling_percent"
  // (window scaling may have changed by resizing the window)
  execSetupGraphics_setWindowSizes(TRUE);

  execSetupGraphics_setScalingTypes();
  execSetupGraphics_setRenderingModes();

  setup_mode = SETUP_MODE_GRAPHICS;

  DrawSetupScreen();

#if defined(TARGET_SDL2)
  // window scaling may have changed at this point
  ToggleFullscreenOrChangeWindowScalingIfNeeded();

  // window scaling quality may have changed at this point
  if (!strEqual(setup.window_scaling_quality, video.window_scaling_quality))
    SDLSetWindowScalingQuality(setup.window_scaling_quality);

  // screen rendering mode may have changed at this point
  SDLSetScreenRenderingMode(setup.screen_rendering_mode);
#endif
}

#if defined(TARGET_SDL2) && !defined(PLATFORM_ANDROID)
static void execSetupChooseWindowSize()
{
  setup_mode = SETUP_MODE_CHOOSE_WINDOW_SIZE;

  DrawSetupScreen();
}

static void execSetupChooseScalingType()
{
  setup_mode = SETUP_MODE_CHOOSE_SCALING_TYPE;

  DrawSetupScreen();
}

static void execSetupChooseRenderingMode()
{
  setup_mode = SETUP_MODE_CHOOSE_RENDERING;

  DrawSetupScreen();
}
#endif

static void execSetupChooseVolumeSimple()
{
  setup_mode = SETUP_MODE_CHOOSE_VOLUME_SIMPLE;

  DrawSetupScreen();
}

static void execSetupChooseVolumeLoops()
{
  setup_mode = SETUP_MODE_CHOOSE_VOLUME_LOOPS;

  DrawSetupScreen();
}

static void execSetupChooseVolumeMusic()
{
  setup_mode = SETUP_MODE_CHOOSE_VOLUME_MUSIC;

  DrawSetupScreen();
}

static void execSetupSound()
{
  if (volumes_simple == NULL)
  {
    boolean current_volume_simple_found = FALSE;
    int i;

    for (i = 0; volumes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = volumes_list[i].value;
      char *text = volumes_list[i].text;

      ti->node_top = &volumes_simple;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_SIMPLE);

      pushTreeInfo(&volumes_simple, ti);

      if (value == setup.volume_simple)
        current_volume_simple_found = TRUE;
    }

    if (!current_volume_simple_found)
    {
      // add entry for non-preset volume value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.volume_simple;

      ti->node_top = &volumes_simple;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_SIMPLE);

      pushTreeInfo(&volumes_simple, ti);
    }

    /* sort volume values to start with lowest volume value */
    sortTreeInfo(&volumes_simple);

    /* set current volume value to configured volume value */
    volume_simple_current =
      getTreeInfoFromIdentifier(volumes_simple,i_to_a(setup.volume_simple));

    /* if that fails, set current volume to reliable default value */
    if (volume_simple_current == NULL)
      volume_simple_current =
	getTreeInfoFromIdentifier(volumes_simple, i_to_a(100));

    /* if that also fails, set current volume to first available value */
    if (volume_simple_current == NULL)
      volume_simple_current = volumes_simple;
  }

  if (volumes_loops == NULL)
  {
    boolean current_volume_loops_found = FALSE;
    int i;

    for (i = 0; volumes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = volumes_list[i].value;
      char *text = volumes_list[i].text;

      ti->node_top = &volumes_loops;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_LOOPS);

      pushTreeInfo(&volumes_loops, ti);

      if (value == setup.volume_loops)
        current_volume_loops_found = TRUE;
    }

    if (!current_volume_loops_found)
    {
      // add entry for non-preset volume value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.volume_loops;

      ti->node_top = &volumes_loops;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_LOOPS);

      pushTreeInfo(&volumes_loops, ti);
    }

    /* sort volume values to start with lowest volume value */
    sortTreeInfo(&volumes_loops);

    /* set current volume value to configured volume value */
    volume_loops_current =
      getTreeInfoFromIdentifier(volumes_loops,i_to_a(setup.volume_loops));

    /* if that fails, set current volume to reliable default value */
    if (volume_loops_current == NULL)
      volume_loops_current =
	getTreeInfoFromIdentifier(volumes_loops, i_to_a(100));

    /* if that also fails, set current volume to first available value */
    if (volume_loops_current == NULL)
      volume_loops_current = volumes_loops;
  }

  if (volumes_music == NULL)
  {
    boolean current_volume_music_found = FALSE;
    int i;

    for (i = 0; volumes_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = volumes_list[i].value;
      char *text = volumes_list[i].text;

      ti->node_top = &volumes_music;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_MUSIC);

      pushTreeInfo(&volumes_music, ti);

      if (value == setup.volume_music)
        current_volume_music_found = TRUE;
    }

    if (!current_volume_music_found)
    {
      // add entry for non-preset volume value

      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = setup.volume_music;

      ti->node_top = &volumes_music;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%d %% (Current)", value);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_VOLUME_MUSIC);

      pushTreeInfo(&volumes_music, ti);
    }

    /* sort volume values to start with lowest volume value */
    sortTreeInfo(&volumes_music);

    /* set current volume value to configured volume value */
    volume_music_current =
      getTreeInfoFromIdentifier(volumes_music,i_to_a(setup.volume_music));

    /* if that fails, set current volume to reliable default value */
    if (volume_music_current == NULL)
      volume_music_current =
	getTreeInfoFromIdentifier(volumes_music, i_to_a(100));

    /* if that also fails, set current volume to first available value */
    if (volume_music_current == NULL)
      volume_music_current = volumes_music;
  }

  setup.volume_simple = atoi(volume_simple_current->identifier);
  setup.volume_loops  = atoi(volume_loops_current->identifier);
  setup.volume_music  = atoi(volume_music_current->identifier);

  /* needed for displaying volume text instead of identifier */
  volume_simple_text = volume_simple_current->name;
  volume_loops_text = volume_loops_current->name;
  volume_music_text = volume_music_current->name;

  setup_mode = SETUP_MODE_SOUND;

  DrawSetupScreen();
}

static void execSetupChooseTouchControls()
{
  setup_mode = SETUP_MODE_CHOOSE_TOUCH_CONTROL;

  DrawSetupScreen();
}

static void execSetupChooseMoveDistance()
{
  setup_mode = SETUP_MODE_CHOOSE_MOVE_DISTANCE;

  DrawSetupScreen();
}

static void execSetupChooseDropDistance()
{
  setup_mode = SETUP_MODE_CHOOSE_DROP_DISTANCE;

  DrawSetupScreen();
}

static void execSetupTouch()
{
  if (touch_controls == NULL)
  {
    int i;

    for (i = 0; touch_controls_list[i].value != NULL; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      char *value = touch_controls_list[i].value;
      char *text = touch_controls_list[i].text;

      ti->node_top = &touch_controls;
      ti->sort_priority = i;

      sprintf(identifier, "%s", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_TOUCH_CONTROL);

      pushTreeInfo(&touch_controls, ti);
    }

    /* sort touch control values to start with lowest touch control value */
    sortTreeInfo(&touch_controls);

    /* set current touch control value to configured touch control value */
    touch_control_current =
      getTreeInfoFromIdentifier(touch_controls, setup.touch.control_type);

    /* if that fails, set current touch control to reliable default value */
    if (touch_control_current == NULL)
      touch_control_current =
	getTreeInfoFromIdentifier(touch_controls, TOUCH_CONTROL_DEFAULT);

    /* if that also fails, set current touch control to first available value */
    if (touch_control_current == NULL)
      touch_control_current = touch_controls;
  }

  if (move_distances == NULL)
  {
    int i;

    for (i = 0; distances_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = distances_list[i].value;
      char *text = distances_list[i].text;

      ti->node_top = &move_distances;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_MOVE_DISTANCE);

      pushTreeInfo(&move_distances, ti);
    }

    /* sort distance values to start with lowest distance value */
    sortTreeInfo(&move_distances);

    /* set current distance value to configured distance value */
    move_distance_current =
      getTreeInfoFromIdentifier(move_distances,
				i_to_a(setup.touch.move_distance));

    /* if that fails, set current distance to reliable default value */
    if (move_distance_current == NULL)
      move_distance_current =
	getTreeInfoFromIdentifier(move_distances, i_to_a(1));

    /* if that also fails, set current distance to first available value */
    if (move_distance_current == NULL)
      move_distance_current = move_distances;
  }

  if (drop_distances == NULL)
  {
    int i;

    for (i = 0; distances_list[i].value != -1; i++)
    {
      TreeInfo *ti = newTreeInfo_setDefaults(TREE_TYPE_UNDEFINED);
      char identifier[32], name[32];
      int value = distances_list[i].value;
      char *text = distances_list[i].text;

      ti->node_top = &drop_distances;
      ti->sort_priority = value;

      sprintf(identifier, "%d", value);
      sprintf(name, "%s", text);

      setString(&ti->identifier, identifier);
      setString(&ti->name, name);
      setString(&ti->name_sorting, name);
      setString(&ti->infotext, STR_SETUP_CHOOSE_DROP_DISTANCE);

      pushTreeInfo(&drop_distances, ti);
    }

    /* sort distance values to start with lowest distance value */
    sortTreeInfo(&drop_distances);

    /* set current distance value to configured distance value */
    drop_distance_current =
      getTreeInfoFromIdentifier(drop_distances,
				i_to_a(setup.touch.drop_distance));

    /* if that fails, set current distance to reliable default value */
    if (drop_distance_current == NULL)
      drop_distance_current =
	getTreeInfoFromIdentifier(drop_distances, i_to_a(1));

    /* if that also fails, set current distance to first available value */
    if (drop_distance_current == NULL)
      drop_distance_current = drop_distances;
  }

  setup.touch.control_type = touch_control_current->identifier;
  setup.touch.move_distance = atoi(move_distance_current->identifier);
  setup.touch.drop_distance = atoi(drop_distance_current->identifier);

  /* needed for displaying volume text instead of identifier */
  touch_controls_text = touch_control_current->name;
  move_distance_text = move_distance_current->name;
  drop_distance_text = drop_distance_current->name;

  setup_mode = SETUP_MODE_TOUCH;

  DrawSetupScreen();
}

static void execSetupArtwork()
{
#if 0
  printf("::: '%s', '%s', '%s'\n",
	 artwork.gfx_current->subdir,
	 artwork.gfx_current->fullpath,
	 artwork.gfx_current->basepath);
#endif

  setup.graphics_set = artwork.gfx_current->identifier;
  setup.sounds_set = artwork.snd_current->identifier;
  setup.music_set = artwork.mus_current->identifier;

  /* needed if last screen (setup choice) changed graphics, sounds or music */
  ReloadCustomArtwork(0);

  /* needed for displaying artwork name instead of artwork identifier */
  graphics_set_name = artwork.gfx_current->name;
  sounds_set_name = artwork.snd_current->name;
  music_set_name = artwork.mus_current->name;

  setup_mode = SETUP_MODE_ARTWORK;

  DrawSetupScreen();
}

static void execSetupChooseGraphics()
{
  setup_mode = SETUP_MODE_CHOOSE_GRAPHICS;

  DrawSetupScreen();
}

static void execSetupChooseSounds()
{
  setup_mode = SETUP_MODE_CHOOSE_SOUNDS;

  DrawSetupScreen();
}

static void execSetupChooseMusic()
{
  setup_mode = SETUP_MODE_CHOOSE_MUSIC;

  DrawSetupScreen();
}

static void execSetupInput()
{
  setup_mode = SETUP_MODE_INPUT;

  DrawSetupScreen();
}

static void execSetupShortcuts()
{
  setup_mode = SETUP_MODE_SHORTCUTS;

  DrawSetupScreen();
}

static void execSetupShortcuts1()
{
  setup_mode = SETUP_MODE_SHORTCUTS_1;

  DrawSetupScreen();
}

static void execSetupShortcuts2()
{
  setup_mode = SETUP_MODE_SHORTCUTS_2;

  DrawSetupScreen();
}

static void execSetupShortcuts3()
{
  setup_mode = SETUP_MODE_SHORTCUTS_3;

  DrawSetupScreen();
}

static void execSetupShortcuts4()
{
  setup_mode = SETUP_MODE_SHORTCUTS_4;

  DrawSetupScreen();
}

static void execSetupShortcuts5()
{
  setup_mode = SETUP_MODE_SHORTCUTS_5;

  DrawSetupScreen();
}

static void execExitSetup()
{
  SetGameStatus(GAME_MODE_MAIN);

  DrawMainMenu();
}

static void execSaveAndExitSetup()
{
  SaveSetup();
  execExitSetup();
}

static struct TokenInfo setup_info_main[] =
{
  { TYPE_ENTER_MENU,	execSetupGame,		STR_SETUP_GAME		},
  { TYPE_ENTER_MENU,	execSetupEditor,	STR_SETUP_EDITOR	},
  { TYPE_ENTER_MENU,	execSetupGraphics,	STR_SETUP_GRAPHICS	},
  { TYPE_ENTER_MENU,	execSetupSound,		STR_SETUP_SOUND		},
  { TYPE_ENTER_MENU,	execSetupArtwork,	STR_SETUP_ARTWORK	},
  { TYPE_ENTER_MENU,	execSetupInput,		STR_SETUP_INPUT		},
  { TYPE_ENTER_MENU,	execSetupTouch,		STR_SETUP_TOUCH		},
  { TYPE_ENTER_MENU,	execSetupShortcuts,	STR_SETUP_SHORTCUTS	},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execExitSetup, 		STR_SETUP_EXIT		},
  { TYPE_LEAVE_MENU,	execSaveAndExitSetup,	STR_SETUP_SAVE_AND_EXIT	},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_game[] =
{
  { TYPE_SWITCH,	&setup.team_mode,	"Team-Mode (Multi-Player):" },
  { TYPE_YES_NO,	&setup.input_on_focus,	"Only Move Focussed Player:" },
  { TYPE_SWITCH,	&setup.time_limit,	"Time Limit:"		},
  { TYPE_SWITCH,	&setup.handicap,	"Handicap:"		},
  { TYPE_SWITCH,	&setup.skip_levels,	"Skip Unsolved Levels:"	},
  { TYPE_SWITCH,	&setup.increment_levels,"Increment Solved Levels:" },
  { TYPE_SWITCH,	&setup.autorecord,	"Auto-Record Tapes:"	},
  { TYPE_ENTER_LIST,	execSetupChooseGameSpeed, "Game Speed:"		},
  { TYPE_STRING,	&game_speed_text,	""			},
#if 1
  { TYPE_ENTER_LIST,	execSetupChooseScrollDelay, "Scroll Delay:"	},
  { TYPE_STRING,	&scroll_delay_text,	""			},
#endif
  { TYPE_ENTER_LIST, execSetupChooseSnapshotMode,"Game Engine Snapshot Mode:" },
  { TYPE_STRING,	&snapshot_mode_text,	""			},
  { TYPE_SWITCH,	&setup.show_snapshot_buttons,"Show Snapshot Buttons:" },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_editor[] =
{
#if 0
  { TYPE_SWITCH,	&setup.editor.el_boulderdash,	"Boulder Dash:" },
  { TYPE_SWITCH,	&setup.editor.el_emerald_mine,	"Emerald Mine:"	},
  { TYPE_SWITCH, &setup.editor.el_emerald_mine_club,	"Emerald Mine Club:" },
  { TYPE_SWITCH,	&setup.editor.el_more,		"Rocks'n'Diamonds:" },
  { TYPE_SWITCH,	&setup.editor.el_sokoban,	"Sokoban:"	},
  { TYPE_SWITCH,	&setup.editor.el_supaplex,	"Supaplex:"	},
  { TYPE_SWITCH,	&setup.editor.el_diamond_caves,	"Diamond Caves II:" },
  { TYPE_SWITCH,	&setup.editor.el_dx_boulderdash,"DX-Boulderdash:" },
  { TYPE_SWITCH,	&setup.editor.el_chars,		"Text Characters:" },
  { TYPE_SWITCH, &setup.editor.el_steel_chars, "Text Characters (Steel):" },
#endif
  { TYPE_SWITCH,	&setup.editor.el_classic,  "Classic Elements:" },
  { TYPE_SWITCH,	&setup.editor.el_custom,  "Custom & Group Elements:" },
#if 0
  { TYPE_SWITCH,	&setup.editor.el_headlines,	"Headlines:"	},
#endif
  { TYPE_SWITCH, &setup.editor.el_user_defined, "User defined element list:" },
  { TYPE_SWITCH,	&setup.editor.el_dynamic,  "Dynamic level elements:" },
  { TYPE_EMPTY,		NULL,			""			},
#if 0
  { TYPE_SWITCH,	&setup.editor.el_by_game,   "Show elements by game:" },
  { TYPE_SWITCH,	&setup.editor.el_by_type,   "Show elements by type:" },
  { TYPE_EMPTY,		NULL,			""			},
#endif
  { TYPE_SWITCH, &setup.editor.show_element_token,	"Show element token:" },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_graphics[] =
{
#if defined(TARGET_SDL2) && !defined(PLATFORM_ANDROID)
  { TYPE_SWITCH,	&setup.fullscreen,	"Fullscreen:"		},
  { TYPE_ENTER_LIST,	execSetupChooseWindowSize, "Window Scaling:"	},
  { TYPE_STRING,	&window_size_text,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseScalingType, "Anti-Aliasing:"	},
  { TYPE_STRING,	&scaling_type_text,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseRenderingMode, "Special Rendering:" },
  { TYPE_STRING,	&rendering_mode_text,	""			},
#endif
#if 0
  { TYPE_ENTER_LIST,	execSetupChooseScrollDelay, "Scroll Delay:"	},
  { TYPE_STRING,	&scroll_delay_text,	""			},
#endif
  { TYPE_SWITCH,	&setup.fade_screens,	"Fade Screens:"		},
  { TYPE_SWITCH,	&setup.quick_switch,	"Quick Player Focus Switch:" },
  { TYPE_SWITCH,	&setup.quick_doors,	"Quick Menu Doors:"	},
  { TYPE_SWITCH,	&setup.show_titlescreen,"Show Title Screens:"	},
  { TYPE_SWITCH,	&setup.toons,		"Show Menu Animations:"	},
  { TYPE_ECS_AGA,	&setup.prefer_aga_graphics,"EMC graphics preference:" },
  { TYPE_SWITCH, &setup.sp_show_border_elements,"Supaplex Border Elements:" },
  { TYPE_SWITCH,	&setup.small_game_graphics, "Small Game Graphics:" },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_sound[] =
{
  { TYPE_SWITCH,	&setup.sound_simple,	"Sound Effects (Normal):"  },
  { TYPE_SWITCH,	&setup.sound_loops,	"Sound Effects (Looping):" },
  { TYPE_SWITCH,	&setup.sound_music,	"Music:"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_ENTER_LIST,	execSetupChooseVolumeSimple, "Sound Volume (Normal):" },
  { TYPE_STRING,	&volume_simple_text,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseVolumeLoops, "Sound Volume (Looping):" },
  { TYPE_STRING,	&volume_loops_text,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseVolumeMusic, "Music Volume:"	},
  { TYPE_STRING,	&volume_music_text,	""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_artwork[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseGraphics,"Custom Graphics:"	},
  { TYPE_STRING,	&graphics_set_name,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseSounds,	"Custom Sounds:"	},
  { TYPE_STRING,	&sounds_set_name,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseMusic,	"Custom Music:"		},
  { TYPE_STRING,	&music_set_name,	""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_YES_NO_AUTO,&setup.override_level_graphics,"Override Level Graphics:"},
  { TYPE_YES_NO_AUTO,&setup.override_level_sounds,  "Override Level Sounds:"  },
  { TYPE_YES_NO_AUTO,&setup.override_level_music,   "Override Level Music:"   },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_input[] =
{
  { TYPE_SWITCH,	NULL,			"Player:"		},
  { TYPE_SWITCH,	NULL,			"Device:"		},
  { TYPE_SWITCH,	NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_touch[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseTouchControls, "Touch Control Type:" },
  { TYPE_STRING,	&touch_controls_text,	""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_touch_wipe_gestures[] =
{
  { TYPE_ENTER_LIST,	execSetupChooseTouchControls, "Touch Control Type:" },
  { TYPE_STRING,	&touch_controls_text,	""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_ENTER_LIST,	execSetupChooseMoveDistance, "Move Trigger Distance:" },
  { TYPE_STRING,	&move_distance_text,	""			},
  { TYPE_ENTER_LIST,	execSetupChooseDropDistance, "Drop Trigger Distance:" },
  { TYPE_STRING,	&drop_distance_text,	""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcuts[] =
{
  { TYPE_ENTER_MENU,	execSetupShortcuts1,	"Various Keys"		},
  { TYPE_ENTER_MENU,	execSetupShortcuts2,	"Player Focus"		},
  { TYPE_ENTER_MENU,	execSetupShortcuts3,	"Tape Buttons"		},
  { TYPE_ENTER_MENU,	execSetupShortcuts4,	"Sound & Music"		},
  { TYPE_ENTER_MENU,	execSetupShortcuts5,	"TAS Snap Keys"		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupMain, 		"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcuts_1[] =
{
  { TYPE_KEYTEXT,	NULL,		"Quick Save Game to Tape:",	},
  { TYPE_KEY,		&setup.shortcut.save_game, ""			},
  { TYPE_KEYTEXT,	NULL,		"Quick Load Game from Tape:",	},
  { TYPE_KEY,		&setup.shortcut.load_game, ""			},
  { TYPE_KEYTEXT,	NULL,		"Start Game & Toggle Pause:",	},
  { TYPE_KEY,		&setup.shortcut.toggle_pause, ""		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_YES_NO,	&setup.ask_on_escape,	"Ask on 'Esc' Key:"	},
  { TYPE_YES_NO, &setup.ask_on_escape_editor,	"Ask on 'Esc' Key (Editor):" },
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,	"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcuts_2[] =
{
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 1:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[0], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 2:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[1], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 3:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[2], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to Player 4:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player[3], ""		},
  { TYPE_KEYTEXT,	NULL,		"Set Focus to All Players:",	},
  { TYPE_KEY,		&setup.shortcut.focus_player_all, ""		},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,	"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcuts_3[] =
{
  { TYPE_KEYTEXT,	NULL,			"Eject Tape:",		},
  { TYPE_KEY,		&setup.shortcut.tape_eject, ""			},
  { TYPE_KEYTEXT,	NULL,			"Warp / Single Step:",	},
  { TYPE_KEY,		&setup.shortcut.tape_extra, ""			},
  { TYPE_KEYTEXT,	NULL,			"Stop Tape:",		},
  { TYPE_KEY,		&setup.shortcut.tape_stop, ""			},
  { TYPE_KEYTEXT,	NULL,			"Pause / Unpause Tape:",},
  { TYPE_KEY,		&setup.shortcut.tape_pause, ""			},
  { TYPE_KEYTEXT,	NULL,			"Record Tape:",		},
  { TYPE_KEY,		&setup.shortcut.tape_record, ""			},
  { TYPE_KEYTEXT,	NULL,			"Play Tape:",		},
  { TYPE_KEY,		&setup.shortcut.tape_play, ""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,	"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcuts_4[] =
{
  { TYPE_KEYTEXT,	NULL,		"Toggle Sound Effects (Normal):", },
  { TYPE_KEY,		&setup.shortcut.sound_simple, ""		},
  { TYPE_KEYTEXT,	NULL,		"Toggle Sound Effects (Looping):", },
  { TYPE_KEY,		&setup.shortcut.sound_loops, ""			},
  { TYPE_KEYTEXT,	NULL,		"Toggle Music:",		},
  { TYPE_KEY,		&setup.shortcut.sound_music, ""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,	"Back"			},

  { 0,			NULL,			NULL			}
};

static struct TokenInfo setup_info_shortcuts_5[] =
{
  { TYPE_KEYTEXT,	NULL,			"Snap Left:",		},
  { TYPE_KEY,		&setup.shortcut.snap_left, ""			},
  { TYPE_KEYTEXT,	NULL,			"Snap Right:",		},
  { TYPE_KEY,		&setup.shortcut.snap_right, ""			},
  { TYPE_KEYTEXT,	NULL,			"Snap Up:",		},
  { TYPE_KEY,		&setup.shortcut.snap_up, ""			},
  { TYPE_KEYTEXT,	NULL,			"Snap Down:",		},
  { TYPE_KEY,		&setup.shortcut.snap_down, ""			},
  { TYPE_EMPTY,		NULL,			""			},
  { TYPE_LEAVE_MENU,	execSetupShortcuts,	"Back"			},

  { 0,			NULL,			NULL			}
};

static Key getSetupKey()
{
  Key key = KSYM_UNDEFINED;
  boolean got_key_event = FALSE;

  while (!got_key_event)
  {
    Event event;

    if (NextValidEvent(&event))
    {
      switch (event.type)
      {
        case EVENT_KEYPRESS:
	  {
	    key = GetEventKey((KeyEvent *)&event, TRUE);

	    /* press 'Escape' or 'Enter' to keep the existing key binding */
	    if (key == KSYM_Escape || key == KSYM_Return)
	      key = KSYM_UNDEFINED;	/* keep old value */

	    got_key_event = TRUE;
	  }
	  break;

        case EVENT_KEYRELEASE:
	  key_joystick_mapping = 0;
	  break;

        default:
	  HandleOtherEvents(&event);
	  break;
      }
    }

    BackToFront();
  }

  return key;
}

static int getSetupValueFont(int type, void *value)
{
  if (type & TYPE_GHOSTED)
    return FONT_OPTION_OFF;
  else if (type & TYPE_KEY)
    return (type & TYPE_QUERY ? FONT_INPUT_1_ACTIVE : FONT_VALUE_1);
  else if (type & TYPE_STRING)
    return FONT_VALUE_2;
  else if (type & TYPE_ECS_AGA)
    return FONT_VALUE_1;
  else if (type & TYPE_BOOLEAN_STYLE)
    return (*(boolean *)value ? FONT_OPTION_ON : FONT_OPTION_OFF);
  else if (type & TYPE_YES_NO_AUTO)
    return (*(int *)value == AUTO  ? FONT_OPTION_ON :
	    *(int *)value == FALSE ? FONT_OPTION_OFF : FONT_OPTION_ON);
  else
    return FONT_VALUE_1;
}

static int getSetupValueFontNarrow(int type, int font_nr)
{
  return (font_nr == FONT_VALUE_1    ? FONT_VALUE_NARROW :
	  font_nr == FONT_OPTION_ON  ? FONT_OPTION_ON_NARROW :
	  font_nr == FONT_OPTION_OFF ? FONT_OPTION_OFF_NARROW :
	  font_nr);
}

static void drawSetupValue(int screen_pos, int setup_info_pos_raw)
{
  int si_pos = (setup_info_pos_raw < 0 ? screen_pos : setup_info_pos_raw);
  struct TokenInfo *si = &setup_info[si_pos];
  boolean font_draw_xoffset_modified = FALSE;
  boolean scrollbar_needed = (num_setup_info < max_setup_info);
  int font_draw_xoffset_old = -1;
  int xoffset = (scrollbar_needed ? -1 : 0);
  int menu_screen_value_xpos = MENU_SCREEN_VALUE_XPOS + xoffset;
  int menu_screen_max_xpos = MENU_SCREEN_MAX_XPOS + xoffset;
  int xpos = menu_screen_value_xpos;
  int ypos = MENU_SCREEN_START_YPOS + screen_pos;
  int startx = mSX + xpos * 32;
  int starty = mSY + ypos * 32;
  int font_nr, font_nr_default, font_width_default;
  int type = si->type;
  void *value = si->value;
  char *value_string = getSetupValue(type, value);
  int i;

  if (value_string == NULL)
    return;

  if (type & TYPE_KEY)
  {
    xpos = MENU_SCREEN_START_XPOS;

    if (type & TYPE_QUERY)
      value_string = "<press key>";
  }
  else if (type & TYPE_STRING)
  {
    int max_value_len = (SCR_FIELDX - 2) * 2;

    xpos = MENU_SCREEN_START_XPOS;

    if (strlen(value_string) > max_value_len)
      value_string[max_value_len] = '\0';
  }
  else if (type & TYPE_YES_NO_AUTO)
  {
    xpos = menu_screen_value_xpos - 1;
  }

  startx = mSX + xpos * 32;
  starty = mSY + ypos * 32;
  font_nr_default = getSetupValueFont(type, value);
  font_width_default = getFontWidth(font_nr_default);

  font_nr = font_nr_default;

  // special check if right-side setup values moved left due to scrollbar
  if (scrollbar_needed && xpos > MENU_SCREEN_START_XPOS)
  {
    int max_menu_text_length = 26;	// maximum text length for classic menu
    int font_xoffset = getFontBitmapInfo(font_nr)->draw_xoffset;
    int text_startx = mSX + MENU_SCREEN_START_XPOS * 32;
    int text_font_nr = getMenuTextFont(FONT_MENU_2);
    int text_font_xoffset = getFontBitmapInfo(text_font_nr)->draw_xoffset;
    int text_width = max_menu_text_length * getFontWidth(text_font_nr);

    if (startx + font_xoffset < text_startx + text_width + text_font_xoffset)
    {
      xpos += 1;
      startx = mSX + xpos * 32;

      font_nr = getSetupValueFontNarrow(type, font_nr);
    }
  }

  /* downward compatibility correction for Juergen Bonhagen's menu settings */
  if (setup_mode != SETUP_MODE_INPUT)
  {
    int max_menu_text_length_big = (menu_screen_value_xpos -
				    MENU_SCREEN_START_XPOS);
    int max_menu_text_length_medium = max_menu_text_length_big * 2;
    int check_font_nr = FONT_OPTION_ON; /* known font that needs correction */
    int font1_xoffset = getFontBitmapInfo(font_nr)->draw_xoffset;
    int font2_xoffset = getFontBitmapInfo(check_font_nr)->draw_xoffset;
    int text_startx = mSX + MENU_SCREEN_START_XPOS * 32;
    int text_font_nr = getMenuTextFont(FONT_MENU_2);
    int text_font_xoffset = getFontBitmapInfo(text_font_nr)->draw_xoffset;
    int text_width = max_menu_text_length_medium * getFontWidth(text_font_nr);
    boolean correct_font_draw_xoffset = FALSE;

    if (xpos == MENU_SCREEN_START_XPOS &&
	startx + font1_xoffset < text_startx + text_font_xoffset)
      correct_font_draw_xoffset = TRUE;

    if (xpos == menu_screen_value_xpos &&
	startx + font2_xoffset < text_startx + text_width + text_font_xoffset)
      correct_font_draw_xoffset = TRUE;

    /* check if setup value would overlap with setup text when printed */
    /* (this can happen for extreme/wrong values for font draw offset) */
    if (correct_font_draw_xoffset)
    {
      font_draw_xoffset_old = getFontBitmapInfo(font_nr)->draw_xoffset;
      font_draw_xoffset_modified = TRUE;

      if (type & TYPE_KEY)
	getFontBitmapInfo(font_nr)->draw_xoffset += 2 * getFontWidth(font_nr);
      else if (!(type & TYPE_STRING))
	getFontBitmapInfo(font_nr)->draw_xoffset = text_font_xoffset + 20 -
	  max_menu_text_length_medium * (16 - getFontWidth(text_font_nr));
    }
  }

  for (i = 0; i <= menu_screen_max_xpos - xpos; i++)
    DrawText(startx + i * font_width_default, starty, " ", font_nr_default);

  DrawText(startx, starty, value_string, font_nr);

  if (font_draw_xoffset_modified)
    getFontBitmapInfo(font_nr)->draw_xoffset = font_draw_xoffset_old;
}

static void changeSetupValue(int screen_pos, int setup_info_pos_raw, int dx)
{
  int si_pos = (setup_info_pos_raw < 0 ? screen_pos : setup_info_pos_raw);
  struct TokenInfo *si = &setup_info[si_pos];

  if (si->type & TYPE_BOOLEAN_STYLE)
  {
    *(boolean *)si->value ^= TRUE;
  }
  else if (si->type & TYPE_YES_NO_AUTO)
  {
    *(int *)si->value =
      (dx == -1 ?
       (*(int *)si->value == AUTO ? TRUE :
	*(int *)si->value == TRUE ? FALSE : AUTO) :
       (*(int *)si->value == TRUE ? AUTO :
	*(int *)si->value == AUTO ? FALSE : TRUE));
  }
  else if (si->type & TYPE_KEY)
  {
    Key key;

    si->type |= TYPE_QUERY;
    drawSetupValue(screen_pos, setup_info_pos_raw);
    si->type &= ~TYPE_QUERY;

    key = getSetupKey();
    if (key != KSYM_UNDEFINED)
      *(Key *)si->value = key;
  }

  drawSetupValue(screen_pos, setup_info_pos_raw);

  // fullscreen state may have changed at this point
  if (si->value == &setup.fullscreen)
    ToggleFullscreenOrChangeWindowScalingIfNeeded();
}

static struct TokenInfo *getSetupInfoFinal(struct TokenInfo *setup_info_orig)
{
  static struct TokenInfo *setup_info_hide = NULL;
  int list_size = 0;
  int list_pos = 0;
  int i;

  /* determine maximum list size of target list */
  while (setup_info_orig[list_size++].type != 0);

  /* free, allocate and clear memory for target list */
  checked_free(setup_info_hide);
  setup_info_hide = checked_calloc(list_size * sizeof(struct TokenInfo));

  /* copy setup info list without setup entries marked as hidden */
  for (i = 0; setup_info_orig[i].type != 0; i++)
    if (!hideSetupEntry(setup_info_orig[i].value))
      setup_info_hide[list_pos++] = setup_info_orig[i];

  return setup_info_hide;
}

static void DrawSetupScreen_Generic()
{
  int fade_mask = REDRAW_FIELD;
  boolean redraw_all = FALSE;
  char *title_string = NULL;
  int i;

  if (CheckIfGlobalBorderOrPlayfieldViewportHasChanged())
    fade_mask = REDRAW_ALL;

  UnmapAllGadgets();
  FadeMenuSoundsAndMusic();

  FreeScreenGadgets();
  CreateScreenGadgets();

  if (redraw_mask & REDRAW_ALL)
    redraw_all = TRUE;

  FadeOut(fade_mask);

  /* needed if different viewport properties defined for setup screen */
  ChangeViewportPropertiesIfNeeded();

  SetMainBackgroundImage(IMG_BACKGROUND_SETUP);

  ClearField();

  OpenDoor(GetDoorState() | DOOR_NO_DELAY | DOOR_FORCE_REDRAW);

  if (setup_mode == SETUP_MODE_MAIN)
  {
    setup_info = setup_info_main;
    title_string = STR_SETUP_MAIN;
  }
  else if (setup_mode == SETUP_MODE_GAME)
  {
    setup_info = setup_info_game;
    title_string = STR_SETUP_GAME;
  }
  else if (setup_mode == SETUP_MODE_EDITOR)
  {
    setup_info = setup_info_editor;
    title_string = STR_SETUP_EDITOR;
  }
  else if (setup_mode == SETUP_MODE_GRAPHICS)
  {
    setup_info = setup_info_graphics;
    title_string = STR_SETUP_GRAPHICS;
  }
  else if (setup_mode == SETUP_MODE_SOUND)
  {
    setup_info = setup_info_sound;
    title_string = STR_SETUP_SOUND;
  }
  else if (setup_mode == SETUP_MODE_ARTWORK)
  {
    setup_info = setup_info_artwork;
    title_string = STR_SETUP_ARTWORK;
  }
  else if (setup_mode == SETUP_MODE_TOUCH)
  {
    setup_info = setup_info_touch;
    title_string = STR_SETUP_TOUCH;

    if (strEqual(setup.touch.control_type, TOUCH_CONTROL_WIPE_GESTURES))
      setup_info = setup_info_touch_wipe_gestures;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS)
  {
    setup_info = setup_info_shortcuts;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_1)
  {
    setup_info = setup_info_shortcuts_1;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_2)
  {
    setup_info = setup_info_shortcuts_2;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_3)
  {
    setup_info = setup_info_shortcuts_3;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_4)
  {
    setup_info = setup_info_shortcuts_4;
    title_string = STR_SETUP_SHORTCUTS;
  }
  else if (setup_mode == SETUP_MODE_SHORTCUTS_5)
  {
    setup_info = setup_info_shortcuts_5;
    title_string = STR_SETUP_SHORTCUTS;
  }

  /* use modified setup info without setup entries marked as hidden */
  setup_info = getSetupInfoFinal(setup_info);

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, title_string);

  // determine maximal number of setup entries that can be displayed on screen
  num_setup_info = 0;
  for (i = 0; setup_info[i].type != 0 && i < NUM_MENU_ENTRIES_ON_SCREEN; i++)
    num_setup_info++;

  // determine maximal number of setup entries available for this setup screen
  max_setup_info = 0;
  for (i = 0; setup_info[i].type != 0; i++)
    max_setup_info++;

  HandleSetupScreen_Generic(0, 0, 0, 0, MB_MENU_INITIALIZE);

  MapScreenGadgets(max_setup_info);

  if (redraw_all)
    redraw_mask = fade_mask = REDRAW_ALL;

  DrawMaskedBorder(fade_mask);

  FadeIn(fade_mask);
}

void HandleSetupScreen_Generic(int mx, int my, int dx, int dy, int button)
{
  menu_info = setup_info;

  HandleMenuScreen(mx, my, dx, dy, button,
		   setup_mode, num_setup_info, max_setup_info);
}

void DrawSetupScreen_Input()
{
  int i;

  FadeOut(REDRAW_FIELD);

  ClearField();

  setup_info = setup_info_input;

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, STR_SETUP_INPUT);

  for (i = 0; setup_info[i].type != 0 && i < MAX_MENU_ENTRIES_ON_SCREEN; i++)
  {
    if (setup_info[i].type & (TYPE_ENTER_MENU|TYPE_ENTER_LIST))
      initCursor(i, IMG_MENU_BUTTON_ENTER_MENU);
    else if (setup_info[i].type & (TYPE_LEAVE_MENU|TYPE_LEAVE_LIST))
      initCursor(i, IMG_MENU_BUTTON_LEAVE_MENU);
    else if (setup_info[i].type & ~TYPE_SKIP_ENTRY)
      initCursor(i, IMG_MENU_BUTTON);

    DrawCursorAndText_Setup(i, -1, FALSE);
  }

  /* create gadgets for setup input menu screen */
  FreeScreenGadgets();
  CreateScreenGadgets();

  /* map gadgets for setup input menu screen */
  MapScreenMenuGadgets(SCREEN_MASK_INPUT);

  HandleSetupScreen_Input(0, 0, 0, 0, MB_MENU_INITIALIZE);

  FadeIn(REDRAW_FIELD);
}

static void setJoystickDeviceToNr(char *device_name, int device_nr)
{
  if (device_name == NULL)
    return;

  if (device_nr < 0 || device_nr >= MAX_PLAYERS)
    device_nr = 0;

  if (strlen(device_name) > 1)
  {
    char c1 = device_name[strlen(device_name) - 1];
    char c2 = device_name[strlen(device_name) - 2];

    if (c1 >= '0' && c1 <= '9' && !(c2 >= '0' && c2 <= '9'))
      device_name[strlen(device_name) - 1] = '0' + (char)(device_nr % 10);
  }
  else
    strncpy(device_name, getDeviceNameFromJoystickNr(device_nr),
	    strlen(device_name));
}

static void drawPlayerSetupInputInfo(int player_nr, boolean active)
{
  int i;
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    Key *key;
    char *text;
  } custom[] =
  {
    { &custom_key.left,  "Axis/Pad Left"  },
    { &custom_key.right, "Axis/Pad Right" },
    { &custom_key.up,    "Axis/Pad Up"    },
    { &custom_key.down,  "Axis/Pad Down"  },
    { &custom_key.snap,  "Button 1/A/X"   },
    { &custom_key.drop,  "Button 2/B/Y"   }
  };
  static char *joystick_name[MAX_PLAYERS] =
  {
    "Joystick1",
    "Joystick2",
    "Joystick3",
    "Joystick4"
  };
  int text_font_nr = (active ? FONT_MENU_1_ACTIVE : FONT_MENU_1);

  custom_key = setup.input[player_nr].key;

  DrawText(mSX + 11 * 32, mSY + 2 * 32, int2str(player_nr + 1, 1),
	   FONT_INPUT_1_ACTIVE);

  ClearRectangleOnBackground(drawto, mSX + 8 * TILEX, mSY + 2 * TILEY,
			     TILEX, TILEY);
  DrawFixedGraphicThruMaskExt(drawto, mSX + 8 * TILEX, mSY + 2 * TILEY,
			      PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0);

  if (setup.input[player_nr].use_joystick)
  {
    char *device_name = setup.input[player_nr].joy.device_name;
    int joystick_nr = getJoystickNrFromDeviceName(device_name);
    boolean joystick_active = CheckJoystickOpened(joystick_nr);
    char *text = joystick_name[joystick_nr];
    int font_nr = (joystick_active ? FONT_VALUE_1 : FONT_VALUE_OLD);

    DrawText(mSX + 8 * 32, mSY + 3 * 32, text, font_nr);
    DrawText(mSX + 32, mSY + 4 * 32, "Configure", text_font_nr);
  }
  else
  {
    DrawText(mSX + 8 * 32, mSY + 3 * 32, "Keyboard ", FONT_VALUE_1);
    DrawText(mSX + 1 * 32, mSY + 4 * 32, "Customize", text_font_nr);
  }

  DrawText(mSX + 32, mSY + 5 * 32, "Actual Settings:", FONT_MENU_1);

  drawCursorXY(1, 4, IMG_MENU_BUTTON_LEFT);
  drawCursorXY(1, 5, IMG_MENU_BUTTON_RIGHT);
  drawCursorXY(1, 6, IMG_MENU_BUTTON_UP);
  drawCursorXY(1, 7, IMG_MENU_BUTTON_DOWN);

  DrawText(mSX + 2 * 32, mSY +  6 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 2 * 32, mSY +  7 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 2 * 32, mSY +  8 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 2 * 32, mSY +  9 * 32, ":", FONT_VALUE_OLD);
  DrawText(mSX + 1 * 32, mSY + 10 * 32, "Snap Field:", FONT_VALUE_OLD);
  DrawText(mSX + 1 * 32, mSY + 12 * 32, "Drop Element:", FONT_VALUE_OLD);

  for (i = 0; i < 6; i++)
  {
    int ypos = 6 + i + (i > 3 ? i-3 : 0);

    DrawText(mSX + 3 * 32, mSY + ypos * 32,
	     "              ", FONT_VALUE_1);
    DrawText(mSX + 3 * 32, mSY + ypos * 32,
	     (setup.input[player_nr].use_joystick ?
	      custom[i].text :
	      getKeyNameFromKey(*custom[i].key)), FONT_VALUE_1);
  }
}

static int input_player_nr = 0;

void HandleSetupScreen_Input_Player(int step, int direction)
{
  int old_player_nr = input_player_nr;
  int new_player_nr;

  new_player_nr = old_player_nr + step * direction;
  if (new_player_nr < 0)
    new_player_nr = 0;
  if (new_player_nr > MAX_PLAYERS - 1)
    new_player_nr = MAX_PLAYERS - 1;

  if (new_player_nr != old_player_nr)
  {
    input_player_nr = new_player_nr;

    drawPlayerSetupInputInfo(input_player_nr, FALSE);
  }
}

void HandleSetupScreen_Input(int mx, int my, int dx, int dy, int button)
{
  static int choice = 0;
  int x = 0;
  int y = choice;
  int pos_start  = SETUPINPUT_SCREEN_POS_START;
  int pos_empty1 = SETUPINPUT_SCREEN_POS_EMPTY1;
  int pos_empty2 = SETUPINPUT_SCREEN_POS_EMPTY2;
  int pos_end    = SETUPINPUT_SCREEN_POS_END;

  if (button == MB_MENU_INITIALIZE)
  {
    drawPlayerSetupInputInfo(input_player_nr, (choice == 2));

    DrawCursorAndText_Setup(choice, -1, TRUE);

    return;
  }
  else if (button == MB_MENU_LEAVE)
  {
    setup_mode = SETUP_MODE_MAIN;
    DrawSetupScreen();
    InitJoysticks();

    return;
  }

  if (mx || my)		/* mouse input */
  {
    x = (mx - mSX) / 32;
    y = (my - mSY) / 32 - MENU_SCREEN_START_YPOS;
  }
  else if (dx || dy)	/* keyboard input */
  {
    if (dx && choice == 0)
      x = (dx < 0 ? 10 : 12);
    else if ((dx && choice == 1) ||
	     (dx == -1 && choice == pos_end))
      button = MB_MENU_CHOICE;
    else if (dy)
      y = choice + dy;

    if (y >= pos_empty1 && y <= pos_empty2)
      y = (dy > 0 ? pos_empty2 + 1 : pos_empty1 - 1);
  }

  if (y == 0 && dx != 0 && button)
  {
    HandleSetupScreen_Input_Player(1, dx < 0 ? -1 : +1);
  }
  else if (IN_VIS_FIELD(x, y) &&	// (does not use "IN_VIS_MENU()" yet)
	   y >= pos_start && y <= pos_end &&
	   !(y >= pos_empty1 && y <= pos_empty2))
  {
    if (button)
    {
      if (y != choice)
      {
	DrawCursorAndText_Setup(choice, -1, FALSE);
	DrawCursorAndText_Setup(y, -1, TRUE);

	drawPlayerSetupInputInfo(input_player_nr, (y == 2));

	choice = y;
      }
    }
    else
    {
      if (y == 1)
      {
	char *device_name = setup.input[input_player_nr].joy.device_name;

	if (!setup.input[input_player_nr].use_joystick)
	{
	  int new_device_nr = (dx >= 0 ? 0 : MAX_PLAYERS - 1);

	  setJoystickDeviceToNr(device_name, new_device_nr);
	  setup.input[input_player_nr].use_joystick = TRUE;
	}
	else
	{
	  int device_nr = getJoystickNrFromDeviceName(device_name);
	  int new_device_nr = device_nr + (dx >= 0 ? +1 : -1);

	  if (new_device_nr < 0 || new_device_nr >= MAX_PLAYERS)
	    setup.input[input_player_nr].use_joystick = FALSE;
	  else
	    setJoystickDeviceToNr(device_name, new_device_nr);
	}

	drawPlayerSetupInputInfo(input_player_nr, FALSE);
      }
      else if (y == 2)
      {
	if (setup.input[input_player_nr].use_joystick)
	  ConfigureJoystick(input_player_nr);
	else
	  CustomizeKeyboard(input_player_nr);
      }
      else if (y == pos_end)
      {
	InitJoysticks();

	FadeSetLeaveMenu();

	setup_mode = SETUP_MODE_MAIN;
	DrawSetupScreen();
      }
    }
  }
}

void CustomizeKeyboard(int player_nr)
{
  int i;
  int step_nr;
  boolean finished = FALSE;
  static struct SetupKeyboardInfo custom_key;
  static struct
  {
    Key *key;
    char *text;
  } customize_step[] =
  {
    { &custom_key.left,  "Move Left"	},
    { &custom_key.right, "Move Right"	},
    { &custom_key.up,    "Move Up"	},
    { &custom_key.down,  "Move Down"	},
    { &custom_key.snap,  "Snap Field"	},
    { &custom_key.drop,  "Drop Element"	}
  };

  /* read existing key bindings from player setup */
  custom_key = setup.input[player_nr].key;

  FadeSetEnterMenu();
  FadeOut(REDRAW_FIELD);

  ClearField();

  DrawTextSCentered(mSY - SY + 16, FONT_TITLE_1, "Keyboard Input");

  step_nr = 0;
  DrawText(mSX, mSY + (2 + 2 * step_nr) * 32,
	   customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
  DrawText(mSX, mSY + (2 + 2 * step_nr + 1) * 32,
	   "Key:", FONT_INPUT_1_ACTIVE);
  DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
	   getKeyNameFromKey(*customize_step[step_nr].key), FONT_VALUE_OLD);

  FadeIn(REDRAW_FIELD);

  while (!finished)
  {
    Event event;

    if (NextValidEvent(&event))
    {
      switch (event.type)
      {
        case EVENT_KEYPRESS:
	  {
	    Key key = GetEventKey((KeyEvent *)&event, FALSE);

	    if (key == KSYM_Escape || (key == KSYM_Return && step_nr == 6))
	    {
	      if (key == KSYM_Escape)
		FadeSkipNextFadeIn();

	      finished = TRUE;
	      break;
	    }

	    /* all keys configured -- wait for "Escape" or "Return" key */
	    if (step_nr == 6)
	      break;

	    /* press 'Enter' to keep the existing key binding */
	    if (key == KSYM_Return)
	      key = *customize_step[step_nr].key;

	    /* check if key already used */
	    for (i = 0; i < step_nr; i++)
	      if (*customize_step[i].key == key)
		break;
	    if (i < step_nr)
	      break;

	    /* got new key binding */
	    *customize_step[step_nr].key = key;
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     "             ", FONT_VALUE_1);
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     getKeyNameFromKey(key), FONT_VALUE_1);
	    step_nr++;

	    /* un-highlight last query */
	    DrawText(mSX, mSY + (2 + 2 * (step_nr - 1)) * 32,
		     customize_step[step_nr - 1].text, FONT_MENU_1);
	    DrawText(mSX, mSY + (2 + 2 * (step_nr - 1) + 1) * 32,
		     "Key:", FONT_MENU_1);

	    /* press 'Enter' to leave */
	    if (step_nr == 6)
	    {
	      DrawText(mSX + 16, mSY + 15 * 32 + 16,
		       "Press Enter", FONT_TITLE_1);
	      break;
	    }

	    /* query next key binding */
	    DrawText(mSX, mSY + (2 + 2 * step_nr) * 32,
		     customize_step[step_nr].text, FONT_INPUT_1_ACTIVE);
	    DrawText(mSX, mSY + (2 + 2 * step_nr + 1) * 32,
		     "Key:", FONT_INPUT_1_ACTIVE);
	    DrawText(mSX + 4 * 32, mSY + (2 + 2 * step_nr + 1) * 32,
		     getKeyNameFromKey(*customize_step[step_nr].key),
		     FONT_VALUE_OLD);
	  }
	  break;

        case EVENT_KEYRELEASE:
	  key_joystick_mapping = 0;
	  break;

        default:
	  HandleOtherEvents(&event);
	  break;
      }
    }

    BackToFront();
  }

  /* write new key bindings back to player setup */
  setup.input[player_nr].key = custom_key;

  DrawSetupScreen_Input();
}

/* game controller mapping generator by Gabriel Jacobo <gabomdq@gmail.com> */

#define MARKER_BUTTON		1
#define MARKER_AXIS_X		2
#define MARKER_AXIS_Y		3

static boolean ConfigureJoystickMapButtonsAndAxes(SDL_Joystick *joystick)
{
#if defined(TARGET_SDL2)
  static boolean bitmaps_initialized = FALSE;
  boolean screen_initialized = FALSE;
  static Bitmap *controller, *button, *axis_x, *axis_y;
  char *name;
  boolean success = TRUE;
  boolean done = FALSE, next = FALSE;
  Event event;
  int alpha = 200, alpha_step = -1;
  int alpha_ticks = 0;
  char mapping[4096], temp[4096];
  int font_name = MENU_SETUP_FONT_TITLE;
  int font_info = MENU_SETUP_FONT_TEXT;
  int spacing_name = menu.line_spacing_setup[SETUP_MODE_INPUT];
  int spacing_line = menu.line_spacing_setup[SETUP_MODE_INPUT];
  int spacing_para = menu.paragraph_spacing_setup[SETUP_MODE_INPUT];
  int ystep_name = getMenuTextStep(spacing_name, font_name);
  int ystep_line = getMenuTextStep(spacing_line, font_info);
  int ystep_para = getMenuTextStep(spacing_para, font_info);
  int i, j;

  struct
  {
    int x, y;
    int marker;
    char *field;
    int axis, button, hat, hat_value;
    char mapping[4096];
  }
  *step, *prev_step, steps[] =
  {
    { 356, 155, MARKER_BUTTON, "a",		},
    { 396, 122, MARKER_BUTTON, "b",		},
    { 320, 125, MARKER_BUTTON, "x",		},
    { 358,  95, MARKER_BUTTON, "y",		},
    { 162, 125, MARKER_BUTTON, "back",		},
    { 216, 125, MARKER_BUTTON, "guide",		},
    { 271, 125, MARKER_BUTTON, "start",		},
    { 110, 200, MARKER_BUTTON, "dpleft",	},
    { 146, 228, MARKER_BUTTON, "dpdown",	},
    { 178, 200, MARKER_BUTTON, "dpright",	},
    { 146, 172, MARKER_BUTTON, "dpup",		},
    {  50,  40, MARKER_BUTTON, "leftshoulder",	},
    {  88, -10, MARKER_AXIS_Y, "lefttrigger",	},
    { 382,  40, MARKER_BUTTON, "rightshoulder",	},
    { 346, -10, MARKER_AXIS_Y, "righttrigger",	},
    {  73, 141, MARKER_BUTTON, "leftstick",	},
    { 282, 210, MARKER_BUTTON, "rightstick",	},
    {  73, 141, MARKER_AXIS_X, "leftx",		},
    {  73, 141, MARKER_AXIS_Y, "lefty",		},
    { 282, 210, MARKER_AXIS_X, "rightx",	},
    { 282, 210, MARKER_AXIS_Y, "righty",	},
  };

  unsigned int event_frame_delay = 0;
  unsigned int event_frame_delay_value = GAME_FRAME_DELAY;

  ResetDelayCounter(&event_frame_delay);

  if (!bitmaps_initialized)
  {
    controller = LoadCustomImage("joystick/controller.png");
    button     = LoadCustomImage("joystick/button.png");
    axis_x     = LoadCustomImage("joystick/axis_x.png");
    axis_y     = LoadCustomImage("joystick/axis_y.png");

    bitmaps_initialized = TRUE;
  }

  name = getFormattedJoystickName(SDL_JoystickName(joystick));

#if DEBUG_JOYSTICKS
  /* print info about the joystick we are watching */
  Error(ERR_DEBUG, "watching joystick %d: (%s)\n",
	SDL_JoystickInstanceID(joystick), name);
  Error(ERR_DEBUG, "joystick has %d axes, %d hats, %d balls, and %d buttons\n",
	SDL_JoystickNumAxes(joystick), SDL_JoystickNumHats(joystick),
	SDL_JoystickNumBalls(joystick), SDL_JoystickNumButtons(joystick));
#endif

  /* initialize mapping with GUID and name */
  SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(joystick), temp, sizeof(temp));

  snprintf(mapping, sizeof(mapping), "%s,%s,platform:%s,",
	   temp, name, SDL_GetPlatform());

  /* loop through all steps (buttons and axes), getting joystick events */
  for (i = 0; i < SDL_arraysize(steps) && !done;)
  {
    Bitmap *marker = button;	/* initialize with reliable default value */

    step = &steps[i];
    strcpy(step->mapping, mapping);
    step->axis = -1;
    step->button = -1;
    step->hat = -1;
    step->hat_value = -1;

    marker = (step->marker == MARKER_BUTTON ? button :
	      step->marker == MARKER_AXIS_X ? axis_x :
	      step->marker == MARKER_AXIS_Y ? axis_y : marker);

    next = FALSE;

    while (!done && !next)
    {
      alpha += alpha_step * (int)(SDL_GetTicks() - alpha_ticks) / 5;
      alpha_ticks = SDL_GetTicks();

      if (alpha >= 255)
      {
	alpha = 255;
	alpha_step = -1;
      }
      else if (alpha < 128)
      {
	alpha = 127;
	alpha_step = 1;
      }

      int controller_x = SX + (SXSIZE - controller->width) / 2;
      int controller_y = SY + ystep_line;

      int marker_x = controller_x + step->x;
      int marker_y = controller_y + step->y;

      int ystart1 = mSY - 2 * SY + controller_y + controller->height;
      int ystart2 = ystart1 + ystep_name + ystep_line;

      ClearField();

      DrawTextSCentered(ystart1, font_name, name);

      DrawTextSCentered(ystart2, font_info,
			"Press buttons and move axes on");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"your controller when indicated.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"(Your controller may look different.)");
      ystart2 += ystep_para;

#if defined(PLATFORM_ANDROID)
      DrawTextSCentered(ystart2, font_info,
			"To correct a mistake,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'back' button.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"To skip a button or axis,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'menu' button.");
#else
      DrawTextSCentered(ystart2, font_info,
			"To correct a mistake,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'backspace' key.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"To skip a button or axis,");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"press the 'return' key.");
      ystart2 += ystep_line;
      DrawTextSCentered(ystart2, font_info,
			"To exit, press the 'escape' key.");
#endif

      BlitBitmapMasked(controller, drawto, 0, 0,
		       controller->width, controller->height,
		       controller_x, controller_y);

      SDL_SetSurfaceAlphaMod(marker->surface_masked, alpha);

      BlitBitmapMasked(marker, drawto, 0, 0,
		       marker->width, marker->height,
		       marker_x, marker_y);

      if (!screen_initialized)
	FadeIn(REDRAW_FIELD);
      else
	BackToFront();

      screen_initialized = TRUE;

      while (NextValidEvent(&event))
      {
	switch (event.type)
	{
	  case SDL_JOYAXISMOTION:
	    if (event.jaxis.value > 20000 ||
		event.jaxis.value < -20000)
	    {
	      for (j = 0; j < i; j++)
		if (steps[j].axis == event.jaxis.axis)
		  break;

	      if (j == i)
	      {
		if (step->marker != MARKER_AXIS_X &&
		    step->marker != MARKER_AXIS_Y)
		  break;

		step->axis = event.jaxis.axis;
		strcat(mapping, step->field);
		snprintf(temp, sizeof(temp), ":a%u,", event.jaxis.axis);
		strcat(mapping, temp);
		i++;
		next = TRUE;
	      }
	    }

	    break;

	  case SDL_JOYHATMOTION:
	    /* ignore centering; we're probably just coming back
	       to the center from the previous item we set */
	    if (event.jhat.value == SDL_HAT_CENTERED)
	      break;

	    for (j = 0; j < i; j++)
	      if (steps[j].hat == event.jhat.hat &&
		  steps[j].hat_value == event.jhat.value)
		break;

	    if (j == i)
	    {
	      step->hat = event.jhat.hat;
	      step->hat_value = event.jhat.value;
	      strcat(mapping, step->field);
	      snprintf(temp, sizeof(temp), ":h%u.%u,",
		       event.jhat.hat, event.jhat.value );
	      strcat(mapping, temp);
	      i++;
	      next = TRUE;
	    }

	    break;

	  case SDL_JOYBALLMOTION:
	    break;

	  case SDL_JOYBUTTONUP:
	    for (j = 0; j < i; j++)
	      if (steps[j].button == event.jbutton.button)
		break;

	    if (j == i)
	    {
	      step->button = event.jbutton.button;
	      strcat(mapping, step->field);
	      snprintf(temp, sizeof(temp), ":b%u,", event.jbutton.button);
	      strcat(mapping, temp);
	      i++;
	      next = TRUE;
	    }

	    break;

	  case SDL_FINGERDOWN:
	  case SDL_MOUSEBUTTONDOWN:
	    /* skip this step */
	    i++;
	    next = TRUE;

	    break;

	  case SDL_KEYDOWN:
	    if (event.key.keysym.sym == KSYM_BackSpace ||
		event.key.keysym.sym == KSYM_Back)
	    {
	      if (i == 0)
	      {
		/* leave screen */
		success = FALSE;
		done = TRUE;
	      }

	      /* undo this step */
	      prev_step = &steps[i - 1];
	      strcpy(mapping, prev_step->mapping);
	      i--;
	      next = TRUE;

	      break;
	    }

	    if (event.key.keysym.sym == KSYM_space ||
		event.key.keysym.sym == KSYM_Return ||
		event.key.keysym.sym == KSYM_Menu)
	    {
	      /* skip this step */
	      i++;
	      next = TRUE;

	      break;
	    }

	    if (event.key.keysym.sym == KSYM_Escape)
	    {
	      /* leave screen */
	      success = FALSE;
	      done = TRUE;
	    }

	    break;

	  case SDL_QUIT:
	    program.exit_function(0);
	    break;

	  default:
	    break;
	}

	// do not handle events for longer than standard frame delay period
	if (DelayReached(&event_frame_delay, event_frame_delay_value))
	  break;
      }
    }
  }

  if (success)
  {
#if DEBUG_JOYSTICKS
    Error(ERR_DEBUG, "New game controller mapping:\n\n%s\n\n", mapping);
#endif

    // activate mapping for this game
    SDL_GameControllerAddMapping(mapping);

    // save mapping to personal mappings
    SaveSetup_AddGameControllerMapping(mapping);
  }

  /* wait until the last pending event was removed from event queue */
  while (NextValidEvent(&event));

  return success;
#else
  return TRUE;
#endif
}

static int ConfigureJoystickMain(int player_nr)
{
  char *device_name = setup.input[player_nr].joy.device_name;
  int joystick_nr = getJoystickNrFromDeviceName(device_name);
  boolean joystick_active = CheckJoystickOpened(joystick_nr);
  int success = FALSE;
  int i;

  if (joystick.status == JOYSTICK_NOT_AVAILABLE)
    return JOYSTICK_NOT_AVAILABLE;

  if (!joystick_active || !setup.input[player_nr].use_joystick)
    return JOYSTICK_NOT_AVAILABLE;

  FadeSetEnterMenu();
  FadeOut(REDRAW_FIELD);

  // close all joystick devices (potentially opened as game controllers)
  for (i = 0; i < SDL_NumJoysticks(); i++)
    SDLCloseJoystick(i);

  // open joystick device as plain joystick to configure as game controller
  SDL_Joystick *joystick = SDL_JoystickOpen(joystick_nr);

  // as the joystick was successfully opened before, this should not happen
  if (joystick == NULL)
    return FALSE;

  // create new game controller mapping (buttons and axes) for joystick device
  success = ConfigureJoystickMapButtonsAndAxes(joystick);

  // close joystick (and maybe re-open as configured game controller later)
  SDL_JoystickClose(joystick);

  // re-open all joystick devices (potentially as game controllers)
  for (i = 0; i < SDL_NumJoysticks(); i++)
    SDLOpenJoystick(i);

  // clear all joystick input actions for all joystick devices
  SDLClearJoystickState();

  return (success ? JOYSTICK_CONFIGURED : JOYSTICK_NOT_CONFIGURED);
}

void ConfigureJoystick(int player_nr)
{
  boolean state = ConfigureJoystickMain(player_nr);

  if (state != JOYSTICK_NOT_CONFIGURED)
  {
    boolean success = (state == JOYSTICK_CONFIGURED);
    char *message = (success ? " IS CONFIGURED! " : " NOT AVAILABLE! ");
    char *device_name = setup.input[player_nr].joy.device_name;
    int nr = getJoystickNrFromDeviceName(device_name) + 1;
    int xpos = mSX - SX;
    int ypos = mSY - SY;
    unsigned int wait_frame_delay = 0;
    unsigned int wait_frame_delay_value = 2000;

    ResetDelayCounter(&wait_frame_delay);

    ClearField();

    DrawTextF(xpos + 16, ypos + 6 * 32, FONT_TITLE_1, "   JOYSTICK %d   ", nr);
    DrawTextF(xpos + 16, ypos + 7 * 32, FONT_TITLE_1, message);

    while (!DelayReached(&wait_frame_delay, wait_frame_delay_value))
      BackToFront();

    ClearEventQueue();
  }

  DrawSetupScreen_Input();
}

void DrawSetupScreen()
{
  if (setup_mode == SETUP_MODE_INPUT)
    DrawSetupScreen_Input();
  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED)
    DrawChooseTree(&game_speed_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY)
    DrawChooseTree(&scroll_delay_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
    DrawChooseTree(&snapshot_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
    DrawChooseTree(&window_size_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE)
    DrawChooseTree(&scaling_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_RENDERING)
    DrawChooseTree(&rendering_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    DrawChooseTree(&artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    DrawChooseTree(&artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    DrawChooseTree(&artwork.mus_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE)
    DrawChooseTree(&volume_simple_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS)
    DrawChooseTree(&volume_loops_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
    DrawChooseTree(&volume_music_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL)
    DrawChooseTree(&touch_control_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE)
    DrawChooseTree(&move_distance_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE)
    DrawChooseTree(&drop_distance_current);
  else
    DrawSetupScreen_Generic();

  PlayMenuSoundsAndMusic();
}

void RedrawSetupScreenAfterFullscreenToggle()
{
  if (setup_mode == SETUP_MODE_GRAPHICS ||
      setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
  {
    // update list selection from "setup.window_scaling_percent"
    execSetupGraphics_setWindowSizes(TRUE);

    DrawSetupScreen();
  }
}

void HandleSetupScreen(int mx, int my, int dx, int dy, int button)
{
  if (setup_mode == SETUP_MODE_INPUT)
    HandleSetupScreen_Input(mx, my, dx, dy, button);
  else if (setup_mode == SETUP_MODE_CHOOSE_GAME_SPEED)
    HandleChooseTree(mx, my, dx, dy, button, &game_speed_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCROLL_DELAY)
    HandleChooseTree(mx, my, dx, dy, button, &scroll_delay_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SNAPSHOT_MODE)
    HandleChooseTree(mx, my, dx, dy, button, &snapshot_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_WINDOW_SIZE)
    HandleChooseTree(mx, my, dx, dy, button, &window_size_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SCALING_TYPE)
    HandleChooseTree(mx, my, dx, dy, button, &scaling_type_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_RENDERING)
    HandleChooseTree(mx, my, dx, dy, button, &rendering_mode_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_GRAPHICS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.gfx_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_SOUNDS)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.snd_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MUSIC)
    HandleChooseTree(mx, my, dx, dy, button, &artwork.mus_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_SIMPLE)
    HandleChooseTree(mx, my, dx, dy, button, &volume_simple_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_LOOPS)
    HandleChooseTree(mx, my, dx, dy, button, &volume_loops_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_VOLUME_MUSIC)
    HandleChooseTree(mx, my, dx, dy, button, &volume_music_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_TOUCH_CONTROL)
    HandleChooseTree(mx, my, dx, dy, button, &touch_control_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_MOVE_DISTANCE)
    HandleChooseTree(mx, my, dx, dy, button, &move_distance_current);
  else if (setup_mode == SETUP_MODE_CHOOSE_DROP_DISTANCE)
    HandleChooseTree(mx, my, dx, dy, button, &drop_distance_current);
  else
    HandleSetupScreen_Generic(mx, my, dx, dy, button);
}

void HandleGameActions()
{
  if (game.restart_game_message != NULL)
    RequestRestartGame(game.restart_game_message);

  if (game_status != GAME_MODE_PLAYING)
    return;

  GameActions();	/* main game loop */

  if (tape.auto_play && !tape.playing)
    AutoPlayTape();	/* continue automatically playing next tape */
}


/* ---------- new screen button stuff -------------------------------------- */

static void getScreenMenuButtonPos(int *x, int *y, int gadget_id)
{
  switch (gadget_id)
  {
    case SCREEN_CTRL_ID_PREV_LEVEL:
      *x = mSX + GDI_ACTIVE_POS(menu.main.button.prev_level.x);
      *y = mSY + GDI_ACTIVE_POS(menu.main.button.prev_level.y);
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL:
      *x = mSX + GDI_ACTIVE_POS(menu.main.button.next_level.x);
      *y = mSY + GDI_ACTIVE_POS(menu.main.button.next_level.y);
      break;

    case SCREEN_CTRL_ID_PREV_PLAYER:
      *x = mSX + TILEX * 10;
      *y = mSY + TILEY * MENU_SCREEN_START_YPOS;
      break;

    case SCREEN_CTRL_ID_NEXT_PLAYER:
      *x = mSX + TILEX * 12;
      *y = mSY + TILEY * MENU_SCREEN_START_YPOS;
      break;

    default:
      Error(ERR_EXIT, "unknown gadget ID %d", gadget_id);
  }
}

static struct
{
  int gfx_unpressed, gfx_pressed;
  void (*get_gadget_position)(int *, int *, int);
  int gadget_id;
  int screen_mask;
  char *infotext;
} menubutton_info[NUM_SCREEN_MENUBUTTONS] =
{
  {
    IMG_MENU_BUTTON_PREV_LEVEL, IMG_MENU_BUTTON_PREV_LEVEL_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_PREV_LEVEL,
    SCREEN_MASK_MAIN,
    "last level"
  },
  {
    IMG_MENU_BUTTON_NEXT_LEVEL, IMG_MENU_BUTTON_NEXT_LEVEL_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_NEXT_LEVEL,
    SCREEN_MASK_MAIN,
    "next level"
  },
  {
    IMG_MENU_BUTTON_LEFT, IMG_MENU_BUTTON_LEFT_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_PREV_PLAYER,
    SCREEN_MASK_INPUT,
    "last player"
  },
  {
    IMG_MENU_BUTTON_RIGHT, IMG_MENU_BUTTON_RIGHT_ACTIVE,
    getScreenMenuButtonPos,
    SCREEN_CTRL_ID_NEXT_PLAYER,
    SCREEN_MASK_INPUT,
    "next player"
  },
};

static struct
{
  int gfx_unpressed, gfx_pressed;
  int x, y;
  int gadget_id;
  char *infotext;
} scrollbutton_info[NUM_SCREEN_SCROLLBUTTONS] =
{
  {
    IMG_MENU_BUTTON_UP, IMG_MENU_BUTTON_UP_ACTIVE,
    -1, -1,	/* these values are not constant, but can change at runtime */
    SCREEN_CTRL_ID_SCROLL_UP,
    "scroll up"
  },
  {
    IMG_MENU_BUTTON_DOWN, IMG_MENU_BUTTON_DOWN_ACTIVE,
    -1, -1,	/* these values are not constant, but can change at runtime */
    SCREEN_CTRL_ID_SCROLL_DOWN,
    "scroll down"
  }
};

static struct
{
  int gfx_unpressed, gfx_pressed;
  int x, y;
  int width, height;
  int type;
  int gadget_id;
  char *infotext;
} scrollbar_info[NUM_SCREEN_SCROLLBARS] =
{
  {
    IMG_MENU_SCROLLBAR, IMG_MENU_SCROLLBAR_ACTIVE,
    -1, -1,	/* these values are not constant, but can change at runtime */
    -1, -1,	/* these values are not constant, but can change at runtime */
    GD_TYPE_SCROLLBAR_VERTICAL,
    SCREEN_CTRL_ID_SCROLL_VERTICAL,
    "scroll level series vertically"
  }
};

static void CreateScreenMenubuttons()
{
  struct GadgetInfo *gi;
  unsigned int event_mask;
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
  {
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    int gfx_unpressed, gfx_pressed;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int id = menubutton_info[i].gadget_id;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    menubutton_info[i].get_gadget_position(&x, &y, id);

    width = SC_MENUBUTTON_XSIZE;
    height = SC_MENUBUTTON_YSIZE;

    gfx_unpressed = menubutton_info[i].gfx_unpressed;
    gfx_pressed   = menubutton_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, menubutton_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    screen_gadget[id] = gi;
  }
}

static void CreateScreenScrollbuttons()
{
  struct GadgetInfo *gi;
  unsigned int event_mask;
  int i;

  /* these values are not constant, but can change at runtime */
  scrollbutton_info[0].x = SC_SCROLL_UP_XPOS;
  scrollbutton_info[0].y = SC_SCROLL_UP_YPOS;
  scrollbutton_info[1].x = SC_SCROLL_DOWN_XPOS;
  scrollbutton_info[1].y = SC_SCROLL_DOWN_YPOS;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
  {
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    int gfx_unpressed, gfx_pressed;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    int id = scrollbutton_info[i].gadget_id;

    event_mask = GD_EVENT_PRESSED | GD_EVENT_REPEATED;

    x = mSX + scrollbutton_info[i].x + menu.scrollbar_xoffset;
    y = mSY + scrollbutton_info[i].y;
    width = SC_SCROLLBUTTON_XSIZE;
    height = SC_SCROLLBUTTON_YSIZE;

    /* correct scrollbar position if placed outside menu (playfield) area */
    if (x > SX + SC_SCROLL_UP_XPOS)
      x = SX + SC_SCROLL_UP_XPOS;

    if (id == SCREEN_CTRL_ID_SCROLL_DOWN)
      y = mSY + (SC_SCROLL_VERTICAL_YPOS +
		 (NUM_MENU_ENTRIES_ON_SCREEN - 2) * SC_SCROLLBUTTON_YSIZE);

    gfx_unpressed = scrollbutton_info[i].gfx_unpressed;
    gfx_pressed   = scrollbutton_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, scrollbutton_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    screen_gadget[id] = gi;
  }
}

static void CreateScreenScrollbars()
{
  int i;

  /* these values are not constant, but can change at runtime */
  scrollbar_info[0].x = SC_SCROLL_VERTICAL_XPOS;
  scrollbar_info[0].y = SC_SCROLL_VERTICAL_YPOS;
  scrollbar_info[0].width  = SC_SCROLL_VERTICAL_XSIZE;
  scrollbar_info[0].height = SC_SCROLL_VERTICAL_YSIZE;

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
  {
    Bitmap *gd_bitmap_unpressed, *gd_bitmap_pressed;
    int gfx_unpressed, gfx_pressed;
    int x, y, width, height;
    int gd_x1, gd_x2, gd_y1, gd_y2;
    struct GadgetInfo *gi;
    int items_max, items_visible, item_position;
    unsigned int event_mask;
    int num_page_entries = NUM_MENU_ENTRIES_ON_SCREEN;
    int id = scrollbar_info[i].gadget_id;

    event_mask = GD_EVENT_MOVING | GD_EVENT_OFF_BORDERS;

    x = mSX + scrollbar_info[i].x + menu.scrollbar_xoffset;
    y = mSY + scrollbar_info[i].y;
    width  = scrollbar_info[i].width;
    height = scrollbar_info[i].height;

    /* correct scrollbar position if placed outside menu (playfield) area */
    if (x > SX + SC_SCROLL_VERTICAL_XPOS)
      x = SX + SC_SCROLL_VERTICAL_XPOS;

    if (id == SCREEN_CTRL_ID_SCROLL_VERTICAL)
      height = (NUM_MENU_ENTRIES_ON_SCREEN - 2) * SC_SCROLLBUTTON_YSIZE;

    items_max = num_page_entries;
    items_visible = num_page_entries;
    item_position = 0;

    gfx_unpressed = scrollbar_info[i].gfx_unpressed;
    gfx_pressed   = scrollbar_info[i].gfx_pressed;
    gd_bitmap_unpressed = graphic_info[gfx_unpressed].bitmap;
    gd_bitmap_pressed   = graphic_info[gfx_pressed].bitmap;
    gd_x1 = graphic_info[gfx_unpressed].src_x;
    gd_y1 = graphic_info[gfx_unpressed].src_y;
    gd_x2 = graphic_info[gfx_pressed].src_x;
    gd_y2 = graphic_info[gfx_pressed].src_y;

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_CUSTOM_TYPE_ID, i,
		      GDI_INFO_TEXT, scrollbar_info[i].infotext,
		      GDI_X, x,
		      GDI_Y, y,
		      GDI_WIDTH, width,
		      GDI_HEIGHT, height,
		      GDI_TYPE, scrollbar_info[i].type,
		      GDI_SCROLLBAR_ITEMS_MAX, items_max,
		      GDI_SCROLLBAR_ITEMS_VISIBLE, items_visible,
		      GDI_SCROLLBAR_ITEM_POSITION, item_position,
		      GDI_WHEEL_AREA_X, SX,
		      GDI_WHEEL_AREA_Y, SY,
		      GDI_WHEEL_AREA_WIDTH, SXSIZE,
		      GDI_WHEEL_AREA_HEIGHT, SYSIZE,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gd_bitmap_unpressed, gd_x1, gd_y1,
		      GDI_DESIGN_PRESSED, gd_bitmap_pressed, gd_x2, gd_y2,
		      GDI_BORDER_SIZE, SC_BORDER_SIZE, SC_BORDER_SIZE,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleScreenGadgets,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    screen_gadget[id] = gi;
  }
}

void CreateScreenGadgets()
{
  CreateScreenMenubuttons();

  CreateScreenScrollbuttons();
  CreateScreenScrollbars();
}

void FreeScreenGadgets()
{
  int i;

  for (i = 0; i < NUM_SCREEN_GADGETS; i++)
    FreeGadget(screen_gadget[i]);
}

void MapScreenMenuGadgets(int screen_mask)
{
  int i;

  for (i = 0; i < NUM_SCREEN_MENUBUTTONS; i++)
    if (screen_mask & menubutton_info[i].screen_mask)
      MapGadget(screen_gadget[menubutton_info[i].gadget_id]);
}

void MapScreenGadgets(int num_entries)
{
  int i;

  if (num_entries <= NUM_MENU_ENTRIES_ON_SCREEN)
    return;

  for (i = 0; i < NUM_SCREEN_SCROLLBUTTONS; i++)
    MapGadget(screen_gadget[scrollbutton_info[i].gadget_id]);

  for (i = 0; i < NUM_SCREEN_SCROLLBARS; i++)
    MapGadget(screen_gadget[scrollbar_info[i].gadget_id]);
}

void MapScreenTreeGadgets(TreeInfo *ti)
{
  MapScreenGadgets(numTreeInfoInGroup(ti));
}

static void HandleScreenGadgets(struct GadgetInfo *gi)
{
  int id = gi->custom_id;
  int button = gi->event.button;
  int step = (button == 1 ? 1 : button == 2 ? 5 : 10);

  switch (id)
  {
    case SCREEN_CTRL_ID_PREV_LEVEL:
      HandleMainMenu_SelectLevel(step, -1, NO_DIRECT_LEVEL_SELECT);
      break;

    case SCREEN_CTRL_ID_NEXT_LEVEL:
      HandleMainMenu_SelectLevel(step, +1, NO_DIRECT_LEVEL_SELECT);
      break;

    case SCREEN_CTRL_ID_PREV_PLAYER:
      HandleSetupScreen_Input_Player(step, -1);
      break;

    case SCREEN_CTRL_ID_NEXT_PLAYER:
      HandleSetupScreen_Input_Player(step, +1);
      break;

    case SCREEN_CTRL_ID_SCROLL_UP:
      if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevelSet(0,0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELNR)
	HandleChooseLevelNr(0,0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0,0, 0, -1 * SCROLL_LINE, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_DOWN:
      if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevelSet(0,0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_LEVELNR)
	HandleChooseLevelNr(0,0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0,0, 0, +1 * SCROLL_LINE, MB_MENU_MARK);
      break;

    case SCREEN_CTRL_ID_SCROLL_VERTICAL:
      if (game_status == GAME_MODE_LEVELS)
	HandleChooseLevelSet(0,0,999,gi->event.item_position,MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_LEVELNR)
	HandleChooseLevelNr(0,0,999,gi->event.item_position,MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_SETUP)
	HandleSetupScreen(0,0, 999,gi->event.item_position,MB_MENU_INITIALIZE);
      else if (game_status == GAME_MODE_INFO)
	HandleInfoScreen(0,0, 999,gi->event.item_position,MB_MENU_INITIALIZE);
      break;

    default:
      break;
  }
}
