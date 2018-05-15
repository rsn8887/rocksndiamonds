// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// main.c
// ============================================================================

#include "libgame/libgame.h"

#include "main.h"
#include "init.h"
#include "game.h"
#include "tape.h"
#include "tools.h"
#include "files.h"
#include "events.h"
#include "config.h"

Bitmap		       *bitmap_db_field;
Bitmap		       *bitmap_db_panel;
Bitmap		       *bitmap_db_door_1;
Bitmap		       *bitmap_db_door_2;
Bitmap		       *bitmap_db_store_1;
Bitmap		       *bitmap_db_store_2;
DrawBuffer	       *fieldbuffer;
DrawBuffer	       *drawto_field;

int			game_status = -1;
boolean			game_status_last_screen = -1;
boolean			level_editor_test_game = FALSE;
boolean			network_playing = FALSE;

#if defined(TARGET_SDL)
boolean			network_server = FALSE;
SDL_Thread	       *server_thread;
#endif

int			key_joystick_mapping = 0;

short			Feld[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			MovPos[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			MovDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			MovDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ChangeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ChangePage[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			CustomValue[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			Store[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			Store2[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			StorePlayer[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			Back[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean			Stop[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
boolean			Pushed[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ChangeCount[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ChangeEvent[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			WasJustMoving[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			WasJustFalling[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			CheckCollision[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			CheckImpact[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			AmoebaNr[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			AmoebaCnt[MAX_NUM_AMOEBA];
short			AmoebaCnt2[MAX_NUM_AMOEBA];
short			ExplodeField[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ExplodePhase[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short			ExplodeDelay[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int			RunnerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int			PlayerVisit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

int			GfxFrame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 			GfxRandom[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 			GfxElement[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int			GfxAction[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 			GfxDir[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
int 			GfxRedraw[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

int			ActiveElement[MAX_NUM_ELEMENTS];
int			ActiveButton[NUM_IMAGE_FILES];
int			ActiveFont[NUM_FONTS];

int			lev_fieldx, lev_fieldy;
int			scroll_x, scroll_y;

int			WIN_XSIZE = WIN_XSIZE_DEFAULT;
int			WIN_YSIZE = WIN_YSIZE_DEFAULT;

int			SCR_FIELDX = SCR_FIELDX_DEFAULT;
int			SCR_FIELDY = SCR_FIELDY_DEFAULT;

int			REAL_SX = 6, REAL_SY = 6;
int			SX = 8, SY = 8;
int			DX = 566, DY = 60;
int			VX = 566, VY = 400;
int			EX = 566, EY = 356;
int			dDX, dDY;

int			FULL_SXSIZE = 2 + SXSIZE_DEFAULT + 2;
int			FULL_SYSIZE = 2 + SYSIZE_DEFAULT + 2;
int			SXSIZE = SXSIZE_DEFAULT;
int			SYSIZE = SYSIZE_DEFAULT;

int			FADE_SX = 6, FADE_SY = 6;
int			FADE_SXSIZE = 2 + SXSIZE_DEFAULT + 2;
int			FADE_SYSIZE = 2 + SXSIZE_DEFAULT + 2;

int			DXSIZE = 100;
int			DYSIZE = 280;
int			VXSIZE = 100;
int			VYSIZE = 100;
int			EXSIZE = 100;
int			EYSIZE = 144;
int			TILESIZE_VAR = TILESIZE;

int			FX, FY;
int			ScrollStepSize;
int			ScreenMovDir = MV_NONE, ScreenMovPos = 0;
int			ScreenGfxPos = 0;
int			BorderElement = EL_STEELWALL;
int			MenuFrameDelay = MENU_FRAME_DELAY;
int			GameFrameDelay = GAME_FRAME_DELAY;
int			FfwdFrameDelay = FFWD_FRAME_DELAY;
int			BX1, BY1;
int			BX2, BY2;
int			SBX_Left, SBX_Right;
int			SBY_Upper, SBY_Lower;
int			ZX, ZY;
int			ExitX, ExitY;
int			AllPlayersGone;

int			TimeFrames, TimePlayed, TimeLeft, TapeTime;

boolean			network_player_action_received = FALSE;

struct LevelSetInfo	levelset;
struct LevelInfo	level, level_template;
struct PlayerInfo	stored_player[MAX_PLAYERS], *local_player = NULL;
struct HiScore		highscore[MAX_SCORE_ENTRIES];
struct TapeInfo		tape;
struct SetupInfo	setup;
struct GameInfo		game;
struct GlobalInfo	global;
struct BorderInfo	border;
struct ViewportInfo	viewport;
struct TitleFadingInfo	fading;
struct TitleFadingInfo	title_initial_first_default;
struct TitleFadingInfo	title_initial_default;
struct TitleFadingInfo	title_first_default;
struct TitleFadingInfo	title_default;
struct TitleMessageInfo	titlescreen_initial_first_default;
struct TitleMessageInfo	titlescreen_initial_first[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo	titlescreen_initial_default;
struct TitleMessageInfo	titlescreen_initial[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo	titlescreen_first_default;
struct TitleMessageInfo	titlescreen_first[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo	titlescreen_default;
struct TitleMessageInfo	titlescreen[MAX_NUM_TITLE_IMAGES];
struct TitleMessageInfo	titlemessage_initial_first_default;
struct TitleMessageInfo	titlemessage_initial_first[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo	titlemessage_initial_default;
struct TitleMessageInfo	titlemessage_initial[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo	titlemessage_first_default;
struct TitleMessageInfo	titlemessage_first[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo	titlemessage_default;
struct TitleMessageInfo	titlemessage[MAX_NUM_TITLE_MESSAGES];
struct TitleMessageInfo	readme;
struct InitInfo		init, init_last;
struct MenuInfo		menu;
struct DoorInfo		door_1, door_2;
struct RequestInfo	request;
struct PreviewInfo	preview;
struct EditorInfo	editor;

struct GraphicInfo     *graphic_info = NULL;
struct SoundInfo       *sound_info = NULL;
struct MusicInfo       *music_info = NULL;
struct MusicFileInfo   *music_file_info = NULL;
struct HelpAnimInfo    *helpanim_info = NULL;

SetupFileHash          *helptext_info = NULL;
SetupFileHash	       *image_config_hash = NULL;
SetupFileHash	       *element_token_hash = NULL;
SetupFileHash	       *graphic_token_hash = NULL;
SetupFileHash	       *font_token_hash = NULL;
SetupFileHash	       *hide_setup_hash = NULL;


/* ------------------------------------------------------------------------- */
/* element definitions                                                       */
/* ------------------------------------------------------------------------- */

struct ElementInfo element_info[MAX_NUM_ELEMENTS + 1];

/* this contains predefined structure elements to initialize "element_info" */
struct ElementNameInfo element_name_info[MAX_NUM_ELEMENTS + 1] =
{
  /* keyword to start parser: "ELEMENT_INFO_START" <-- do not change! */

  /* ----------------------------------------------------------------------- */
  /* "real" level file elements                                              */
  /* ----------------------------------------------------------------------- */

  {
    "empty_space",
    "empty_space",
    "empty space"
  },
  {
    "sand",
    "sand",
    "sand"
  },
  {
    "wall",
    "wall",
    "normal wall"
  },
  {
    "wall_slippery",
    "wall",
    "slippery wall"
  },
  {
    "rock",
    "rock",
    "rock"
  },
  {
    "key_obsolete",
    "obsolete",
    "key (OBSOLETE)"
  },
  {
    "emerald",
    "emerald",
    "emerald"
  },
  {
    "exit_closed",
    "exit",
    "closed exit"
  },
  {
    "player_obsolete",
    "obsolete",
    "player (OBSOLETE)"
  },
  {
    "bug",
    "bug",
    "bug (random start direction)"
  },
  {
    "spaceship",
    "spaceship",
    "spaceship (random start direction)"
  },
  {
    "yamyam",
    "yamyam",
    "yam yam (random start direction)"
  },
  {
    "robot",
    "robot",
    "robot"
  },
  {
    "steelwall",
    "steelwall",
    "steel wall"
  },
  {
    "diamond",
    "diamond",
    "diamond"
  },
  {
    "amoeba_dead",
    "amoeba",
    "dead amoeba"
  },
  {
    "quicksand_empty",
    "quicksand",
    "quicksand (empty)"
  },
  {
    "quicksand_full",
    "quicksand",
    "quicksand (with rock)"
  },
  {
    "amoeba_drop",
    "amoeba",
    "amoeba drop"
  },
  {
    "bomb",
    "bomb",
    "bomb"
  },
  {
    "magic_wall",
    "magic_wall",
    "magic wall"
  },
  {
    "speed_pill",
    "speed_pill",
    "speed pill"
  },
  {
    "acid",
    "acid",
    "acid"
  },
  {
    "amoeba_wet",
    "amoeba",
    "dropping amoeba (EM style)"
  },
  {
    "amoeba_dry",
    "amoeba",
    "normal amoeba"
  },
  {
    "nut",
    "nut",
    "nut with emerald"
  },
  {
    "game_of_life",
    "game_of_life",
    "Conway's wall of life"
  },
  {
    "biomaze",
    "biomaze",
    "biomaze"
  },
  {
    "dynamite.active",
    "dynamite",
    "burning dynamite"
  },
  {
    "stoneblock",
    "wall",
    "wall"
  },
  {
    "robot_wheel",
    "robot_wheel",
    "magic wheel"
  },
  {
    "robot_wheel.active",
    "robot_wheel",
    "magic wheel (running)"
  },
  {
    "key_1",
    "key",
    "key 1"
  },
  {
    "key_2",
    "key",
    "key 2"
  },
  {
    "key_3",
    "key",
    "key 3"
  },
  {
    "key_4",
    "key",
    "key 4"
  },
  {
    "gate_1",
    "gate",
    "door 1"
  },
  {
    "gate_2",
    "gate",
    "door 2"
  },
  {
    "gate_3",
    "gate",
    "door 3"
  },
  {
    "gate_4",
    "gate",
    "door 4"
  },
  {
    "gate_1_gray",
    "gate",
    "gray door (opened by key 1)"
  },
  {
    "gate_2_gray",
    "gate",
    "gray door (opened by key 2)"
  },
  {
    "gate_3_gray",
    "gate",
    "gray door (opened by key 3)"
  },
  {
    "gate_4_gray",
    "gate",
    "gray door (opened by key 4)"
  },
  {
    "dynamite",
    "dynamite",
    "dynamite"
  },
  {
    "pacman",
    "pacman",
    "pac man (random start direction)"
  },
  {
    "invisible_wall",
    "wall",
    "invisible normal wall"
  },
  {
    "lamp",
    "lamp",
    "lamp (off)"
  },
  {
    "lamp.active",
    "lamp",
    "lamp (on)"
  },
  {
    "wall_emerald",
    "wall",
    "wall with emerald"
  },
  {
    "wall_diamond",
    "wall",
    "wall with diamond"
  },
  {
    "amoeba_full",
    "amoeba",
    "amoeba with content"
  },
  {
    "bd_amoeba",
    "bd_amoeba",
    "amoeba (BD style)"
  },
  {
    "time_orb_full",
    "time_orb_full",
    "time orb (full)"
  },
  {
    "time_orb_empty",
    "time_orb_empty",
    "time orb (empty)"
  },
  {
    "expandable_wall",
    "wall",
    "growing wall (horizontal, visible)"
  },
  {
    "bd_diamond",
    "bd_diamond",
    "diamond (BD style)"
  },
  {
    "emerald_yellow",
    "emerald",
    "yellow emerald"
  },
  {
    "wall_bd_diamond",
    "wall",
    "wall with BD style diamond"
  },
  {
    "wall_emerald_yellow",
    "wall",
    "wall with yellow emerald"
  },
  {
    "dark_yamyam",
    "dark_yamyam",
    "dark yam yam"
  },
  {
    "bd_magic_wall",
    "bd_magic_wall",
    "magic wall (BD style)"
  },
  {
    "invisible_steelwall",
    "steelwall",
    "invisible steel wall"
  },
  {
    "sokoban_field_player",
    "sokoban",
    "sokoban field with player"
  },
  {
    "dynabomb_increase_number",
    "dynabomb",
    "increases number of bombs"
  },
  {
    "dynabomb_increase_size",
    "dynabomb",
    "increases explosion size"
  },
  {
    "dynabomb_increase_power",
    "dynabomb",
    "increases power of explosion"
  },
  {
    "sokoban_object",
    "sokoban",
    "sokoban object"
  },
  {
    "sokoban_field_empty",
    "sokoban",
    "sokoban empty field"
  },
  {
    "sokoban_field_full",
    "sokoban",
    "sokoban field with object"
  },
  {
    "bd_butterfly.right",
    "bd_butterfly",
    "butterfly (starts moving right)"},
  {
    "bd_butterfly.up",
    "bd_butterfly",
    "butterfly (starts moving up)"
  },
  {
    "bd_butterfly.left",
    "bd_butterfly",
    "butterfly (starts moving left)"},
  {
    "bd_butterfly.down",
    "bd_butterfly",
    "butterfly (starts moving down)"},
  {
    "bd_firefly.right",
    "bd_firefly",
    "firefly (starts moving right)"
  },
  {
    "bd_firefly.up",
    "bd_firefly",
    "firefly (starts moving up)"
  },
  {
    "bd_firefly.left",
    "bd_firefly",
    "firefly (starts moving left)"
  },
  {
    "bd_firefly.down",
    "bd_firefly",
    "firefly (starts moving down)"
  },
  {
    "bd_butterfly",
    "bd_butterfly",
    "butterfly (random start direction)"
  },
  {
    "bd_firefly",
    "bd_firefly",
    "firefly (random start direction)"
  },
  {
    "player_1",
    "player",
    "player 1"
  },
  {
    "player_2",
    "player",
    "player 2"
  },
  {
    "player_3",
    "player",
    "player 3"
  },
  {
    "player_4",
    "player",
    "player 4"
  },
  {
    "bug.right",
    "bug",
    "bug (starts moving right)"
  },
  {
    "bug.up",
    "bug",
    "bug (starts moving up)"
  },
  {
    "bug.left",
    "bug",
    "bug (starts moving left)"
  },
  {
    "bug.down",
    "bug",
    "bug (starts moving down)"
  },
  {
    "spaceship.right",
    "spaceship",
    "spaceship (starts moving right)"},
  {
    "spaceship.up",
    "spaceship",
    "spaceship (starts moving up)"
  },
  {
    "spaceship.left",
    "spaceship",
    "spaceship (starts moving left)"},
  {
    "spaceship.down",
    "spaceship",
    "spaceship (starts moving down)"},
  {
    "pacman.right",
    "pacman",
    "pac man (starts moving right)"
  },
  {
    "pacman.up",
    "pacman",
    "pac man (starts moving up)"
  },
  {
    "pacman.left",
    "pacman",
    "pac man (starts moving left)"
  },
  {
    "pacman.down",
    "pacman",
    "pac man (starts moving down)"
  },
  {
    "emerald_red",
    "emerald",
    "red emerald"
  },
  {
    "emerald_purple",
    "emerald",
    "purple emerald"
  },
  {
    "wall_emerald_red",
    "wall",
    "wall with red emerald"
  },
  {
    "wall_emerald_purple",
    "wall",
    "wall with purple emerald"
  },
  {
    "acid_pool_topleft",
    "wall",
    "acid pool (top left)"
  },
  {
    "acid_pool_topright",
    "wall",
    "acid pool (top right)"
  },
  {
    "acid_pool_bottomleft",
    "wall",
    "acid pool (bottom left)"
  },
  {
    "acid_pool_bottom",
    "wall",
    "acid pool (bottom)"
  },
  {
    "acid_pool_bottomright",
    "wall",
    "acid pool (bottom right)"
  },
  {
    "bd_wall",
    "wall",
    "normal wall (BD style)"
  },
  {
    "bd_rock",
    "bd_rock",
    "rock (BD style)"
  },
  {
    "exit_open",
    "exit",
    "open exit"
  },
  {
    "black_orb",
    "black_orb",
    "black orb bomb"
  },
  {
    "amoeba_to_diamond",
    "amoeba",
    "amoeba"
  },
  {
    "mole",
    "mole",
    "mole (random start direction)"
  },
  {
    "penguin",
    "penguin",
    "penguin"
  },
  {
    "satellite",
    "satellite",
    "satellite"
  },
  {
    "arrow_left",
    "arrow",
    "arrow left"
  },
  {
    "arrow_right",
    "arrow",
    "arrow right"
  },
  {
    "arrow_up",
    "arrow",
    "arrow up"
  },
  {
    "arrow_down",
    "arrow",
    "arrow down"
  },
  {
    "pig",
    "pig",
    "pig"
  },
  {
    "dragon",
    "dragon",
    "fire breathing dragon"
  },
  {
    "em_key_1_file_obsolete",
    "obsolete",
    "key (OBSOLETE)"
  },
  {
    "char_space",
    "char",
    "letter ' '"
  },
  {
    "char_exclam",
    "char",
    "letter '!'"
  },
  {
    "char_quotedbl",
    "char",
    "letter '\"'"
  },
  {
    "char_numbersign",
    "char",
    "letter '#'"
  },
  {
    "char_dollar",
    "char",
    "letter '$'"
  },
  {
    "char_percent",
    "char",
    "letter '%'"
  },
  {
    "char_ampersand",
    "char",
    "letter '&'"
  },
  {
    "char_apostrophe",
    "char",
    "letter '''"
  },
  {
    "char_parenleft",
    "char",
    "letter '('"
  },
  {
    "char_parenright",
    "char",
    "letter ')'"
  },
  {
    "char_asterisk",
    "char",
    "letter '*'"
  },
  {
    "char_plus",
    "char",
    "letter '+'"
  },
  {
    "char_comma",
    "char",
    "letter ','"
  },
  {
    "char_minus",
    "char",
    "letter '-'"
  },
  {
    "char_period",
    "char",
    "letter '.'"
  },
  {
    "char_slash",
    "char",
    "letter '/'"
  },
  {
    "char_0",
    "char",
    "letter '0'"
  },
  {
    "char_1",
    "char",
    "letter '1'"
  },
  {
    "char_2",
    "char",
    "letter '2'"
  },
  {
    "char_3",
    "char",
    "letter '3'"
  },
  {
    "char_4",
    "char",
    "letter '4'"
  },
  {
    "char_5",
    "char",
    "letter '5'"
  },
  {
    "char_6",
    "char",
    "letter '6'"
  },
  {
    "char_7",
    "char",
    "letter '7'"
  },
  {
    "char_8",
    "char",
    "letter '8'"
  },
  {
    "char_9",
    "char",
    "letter '9'"
  },
  {
    "char_colon",
    "char",
    "letter ':'"
  },
  {
    "char_semicolon",
    "char",
    "letter ';'"
  },
  {
    "char_less",
    "char",
    "letter '<'"
  },
  {
    "char_equal",
    "char",
    "letter '='"
  },
  {
    "char_greater",
    "char",
    "letter '>'"
  },
  {
    "char_question",
    "char",
    "letter '?'"
  },
  {
    "char_at",
    "char",
    "letter '@'"
  },
  {
    "char_a",
    "char",
    "letter 'A'"
  },
  {
    "char_b",
    "char",
    "letter 'B'"
  },
  {
    "char_c",
    "char",
    "letter 'C'"
  },
  {
    "char_d",
    "char",
    "letter 'D'"
  },
  {
    "char_e",
    "char",
    "letter 'E'"
  },
  {
    "char_f",
    "char",
    "letter 'F'"
  },
  {
    "char_g",
    "char",
    "letter 'G'"
  },
  {
    "char_h",
    "char",
    "letter 'H'"
  },
  {
    "char_i",
    "char",
    "letter 'I'"
  },
  {
    "char_j",
    "char",
    "letter 'J'"
  },
  {
    "char_k",
    "char",
    "letter 'K'"
  },
  {
    "char_l",
    "char",
    "letter 'L'"
  },
  {
    "char_m",
    "char",
    "letter 'M'"
  },
  {
    "char_n",
    "char",
    "letter 'N'"
  },
  {
    "char_o",
    "char",
    "letter 'O'"
  },
  {
    "char_p",
    "char",
    "letter 'P'"
  },
  {
    "char_q",
    "char",
    "letter 'Q'"
  },
  {
    "char_r",
    "char",
    "letter 'R'"
  },
  {
    "char_s",
    "char",
    "letter 'S'"
  },
  {
    "char_t",
    "char",
    "letter 'T'"
  },
  {
    "char_u",
    "char",
    "letter 'U'"
  },
  {
    "char_v",
    "char",
    "letter 'V'"
  },
  {
    "char_w",
    "char",
    "letter 'W'"
  },
  {
    "char_x",
    "char",
    "letter 'X'"
  },
  {
    "char_y",
    "char",
    "letter 'Y'"
  },
  {
    "char_z",
    "char",
    "letter 'Z'"
  },
  {
    "char_bracketleft",
    "char",
    "letter '['"
  },
  {
    "char_backslash",
    "char",
    "letter '\\'"
  },
  {
    "char_bracketright",
    "char",
    "letter ']'"
  },
  {
    "char_asciicircum",
    "char",
    "letter '^'"
  },
  {
    "char_underscore",
    "char",
    "letter '_'"
  },
  {
    "char_copyright",
    "char",
    "letter '\xa9'"
  },
  {
    "char_aumlaut",
    "char",
    "letter '\xc4'"
  },
  {
    "char_oumlaut",
    "char",
    "letter '\xd6'"
  },
  {
    "char_uumlaut",
    "char",
    "letter '\xdc'"
  },
  {
    "char_degree",
    "char",
    "letter '\xb0'"
  },
  {
    "char_trademark",
    "char",
    "letter '\xae'"
  },
  {
    "char_cursor",
    "char",
    "letter '\xa0'"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter ''"
  },
  {
    "char_unused",
    "char",
    "letter 'button'"
  },
  {
    "char_unused",
    "char",
    "letter 'up'"
  },
  {
    "char_unused",
    "char",
    "letter 'down'"
  },
  {
    "expandable_wall_horizontal",
    "wall",
    "growing wall (horizontal)"
  },
  {
    "expandable_wall_vertical",
    "wall",
    "growing wall (vertical)"
  },
  {
    "expandable_wall_any",
    "wall",
    "growing wall (any direction)"
  },
  {
    "em_gate_1",
    "gate",
    "door 1 (EM style)"
  },
  {
    "em_gate_2",
    "gate",
    "door 2 (EM style)"
  },
  {
    "em_gate_3",
    "gate",
    "door 3 (EM style)"
  },
  {
    "em_gate_4",
    "gate",
    "door 4 (EM style)"
  },
  {
    "em_key_2_file_obsolete",
    "obsolete",
    "key (OBSOLETE)"
  },
  {
    "em_key_3_file_obsolete",
    "obsolete",
    "key (OBSOLETE)"
  },
  {
    "em_key_4_file_obsolete",
    "obsolete",
    "key (OBSOLETE)"
  },
  {
    "sp_empty_space",
    "empty_space",
    "empty space"
  },
  {
    "sp_zonk",
    "sp_zonk",
    "zonk"
  },
  {
    "sp_base",
    "sp_base",
    "base"
  },
  {
    "sp_murphy",
    "player",
    "murphy"
  },
  {
    "sp_infotron",
    "sp_infotron",
    "infotron"
  },
  {
    "sp_chip_single",
    "wall",
    "chip (single)"
  },
  {
    "sp_hardware_gray",
    "wall",
    "hardware"
  },
  {
    "sp_exit_closed",
    "sp_exit",
    "exit"
  },
  {
    "sp_disk_orange",
    "sp_disk_orange",
    "orange disk"
  },
  {
    "sp_port_right",
    "sp_port",
    "port (leading right)"
  },
  {
    "sp_port_down",
    "sp_port",
    "port (leading down)"
  },
  {
    "sp_port_left",
    "sp_port",
    "port (leading left)"
  },
  {
    "sp_port_up",
    "sp_port",
    "port (leading up)"
  },
  {
    "sp_gravity_port_right",
    "sp_gravity_port",
    "gravity-on/off port (leading right)"
  },
  {
    "sp_gravity_port_down",
    "sp_gravity_port",
    "gravity-on/off port (leading down)"
  },
  {
    "sp_gravity_port_left",
    "sp_gravity_port",
    "gravity-on/off port (leading left)"
  },
  {
    "sp_gravity_port_up",
    "sp_gravity_port",
    "gravity-on/off port (leading up)"
  },
  {
    "sp_sniksnak",
    "sp_sniksnak",
    "snik snak"
  },
  {
    "sp_disk_yellow",
    "sp_disk_yellow",
    "yellow disk"
  },
  {
    "sp_terminal",
    "sp_terminal",
    "terminal"
  },
  {
    "sp_disk_red",
    "dynamite",
    "red disk"
  },
  {
    "sp_port_vertical",
    "sp_port",
    "port (vertical)"
  },
  {
    "sp_port_horizontal",
    "sp_port",
    "port (horizontal)"
  },
  {
    "sp_port_any",
    "sp_port",
    "port (any direction)"
  },
  {
    "sp_electron",
    "sp_electron",
    "electron"
  },
  {
    "sp_buggy_base",
    "sp_buggy_base",
    "buggy base"
  },
  {
    "sp_chip_left",
    "wall",
    "chip (left half)"
  },
  {
    "sp_chip_right",
    "wall",
    "chip (right half)"
  },
  {
    "sp_hardware_base_1",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_green",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_blue",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_red",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_yellow",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_2",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_3",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_4",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_5",
    "wall",
    "hardware"
  },
  {
    "sp_hardware_base_6",
    "wall",
    "hardware"
  },
  {
    "sp_chip_top",
    "wall",
    "chip (upper half)"
  },
  {
    "sp_chip_bottom",
    "wall",
    "chip (lower half)"
  },
  {
    "em_gate_1_gray",
    "gate",
    "gray door (EM style, key 1)"
  },
  {
    "em_gate_2_gray",
    "gate",
    "gray door (EM style, key 2)"
  },
  {
    "em_gate_3_gray",
    "gate",
    "gray door (EM style, key 3)"
  },
  {
    "em_gate_4_gray",
    "gate",
    "gray door (EM style, key 4)"
  },
  {
    "em_dynamite",
    "dynamite",
    "dynamite (EM style)"
  },
  {
    "em_dynamite.active",
    "dynamite",
    "burning dynamite (EM style)"
  },
  {
    "pearl",
    "pearl",
    "pearl"
  },
  {
    "crystal",
    "crystal",
    "crystal"
  },
  {
    "wall_pearl",
    "wall",
    "wall with pearl"
  },
  {
    "wall_crystal",
    "wall",
    "wall with crystal"
  },
  {
    "dc_gate_white",
    "gate",
    "white door"
  },
  {
    "dc_gate_white_gray",
    "gate",
    "gray door (opened by white key)"
  },
  {
    "dc_key_white",
    "key",
    "white key"
  },
  {
    "shield_normal",
    "shield_normal",
    "shield (normal)"
  },
  {
    "extra_time",
    "extra_time",
    "extra time"
  },
  {
    "switchgate_open",
    "switchgate",
    "switch gate (open)"
  },
  {
    "switchgate_closed",
    "switchgate",
    "switch gate (closed)"
  },
  {
    "switchgate_switch_up",
    "switchgate_switch",
    "switch for switch gate"
  },
  {
    "switchgate_switch_down",
    "switchgate_switch",
    "switch for switch gate"
  },
  {
    "unused_269",
    "unused",
    "-"
  },
  {
    "unused_270",
    "unused",
    "-"
  },
  {
    "conveyor_belt_1_left",
    "conveyor_belt",
    "conveyor belt 1 (left)"
  },
  {
    "conveyor_belt_1_middle",
    "conveyor_belt",
    "conveyor belt 1 (middle)"
  },
  {
    "conveyor_belt_1_right",
    "conveyor_belt",
    "conveyor belt 1 (right)"
  },
  {
    "conveyor_belt_1_switch_left",
    "conveyor_belt_switch",
    "switch for conveyor belt 1 (left)"
  },
  {
    "conveyor_belt_1_switch_middle",
    "conveyor_belt_switch",
    "switch for conveyor belt 1 (middle)"
  },
  {
    "conveyor_belt_1_switch_right",
    "conveyor_belt_switch",
    "switch for conveyor belt 1 (right)"
  },
  {
    "conveyor_belt_2_left",
    "conveyor_belt",
    "conveyor belt 2 (left)"
  },
  {
    "conveyor_belt_2_middle",
    "conveyor_belt",
    "conveyor belt 2 (middle)"
  },
  {
    "conveyor_belt_2_right",
    "conveyor_belt",
    "conveyor belt 2 (right)"
  },
  {
    "conveyor_belt_2_switch_left",
    "conveyor_belt_switch",
    "switch for conveyor belt 2 (left)"
  },
  {
    "conveyor_belt_2_switch_middle",
    "conveyor_belt_switch",
    "switch for conveyor belt 2 (middle)"
  },
  {
    "conveyor_belt_2_switch_right",
    "conveyor_belt_switch",
    "switch for conveyor belt 2 (right)"
  },
  {
    "conveyor_belt_3_left",
    "conveyor_belt",
    "conveyor belt 3 (left)"
  },
  {
    "conveyor_belt_3_middle",
    "conveyor_belt",
    "conveyor belt 3 (middle)"
  },
  {
    "conveyor_belt_3_right",
    "conveyor_belt",
    "conveyor belt 3 (right)"
  },
  {
    "conveyor_belt_3_switch_left",
    "conveyor_belt_switch",
    "switch for conveyor belt 3 (left)"
  },
  {
    "conveyor_belt_3_switch_middle",
    "conveyor_belt_switch",
    "switch for conveyor belt 3 (middle)"
  },
  {
    "conveyor_belt_3_switch_right",
    "conveyor_belt_switch",
    "switch for conveyor belt 3 (right)"
  },
  {
    "conveyor_belt_4_left",
    "conveyor_belt",
    "conveyor belt 4 (left)"
  },
  {
    "conveyor_belt_4_middle",
    "conveyor_belt",
    "conveyor belt 4 (middle)"
  },
  {
    "conveyor_belt_4_right",
    "conveyor_belt",
    "conveyor belt 4 (right)"
  },
  {
    "conveyor_belt_4_switch_left",
    "conveyor_belt_switch",
    "switch for conveyor belt 4 (left)"
  },
  {
    "conveyor_belt_4_switch_middle",
    "conveyor_belt_switch",
    "switch for conveyor belt 4 (middle)"
  },
  {
    "conveyor_belt_4_switch_right",
    "conveyor_belt_switch",
    "switch for conveyor belt 4 (right)"
  },
  {
    "landmine",
    "landmine",
    "land mine (not removable)"
  },
  {
    "envelope_obsolete",
    "obsolete",
    "envelope (OBSOLETE)"
  },
  {
    "light_switch",
    "light_switch",
    "light switch (off)"
  },
  {
    "light_switch.active",
    "light_switch",
    "light switch (on)"
  },
  {
    "sign_exclamation",
    "sign",
    "sign (exclamation)"
  },
  {
    "sign_radioactivity",
    "sign",
    "sign (radio activity)"
  },
  {
    "sign_stop",
    "sign",
    "sign (stop)"
  },
  {
    "sign_wheelchair",
    "sign",
    "sign (wheel chair)"
  },
  {
    "sign_parking",
    "sign",
    "sign (parking)"
  },
  {
    "sign_no_entry",
    "sign",
    "sign (no entry)"
  },
  {
    "sign_unused_1",
    "sign",
    "sign (unused)"
  },
  {
    "sign_give_way",
    "sign",
    "sign (give way)"
  },
  {
    "sign_entry_forbidden",
    "sign",
    "sign (entry forbidden)"
  },
  {
    "sign_emergency_exit",
    "sign",
    "sign (emergency exit)"
  },
  {
    "sign_yin_yang",
    "sign",
    "sign (yin yang)"
  },
  {
    "sign_unused_2",
    "sign",
    "sign (unused)"
  },
  {
    "mole.left",
    "mole",
    "mole (starts moving left)"
  },
  {
    "mole.right",
    "mole",
    "mole (starts moving right)"
  },
  {
    "mole.up",
    "mole",
    "mole (starts moving up)"
  },
  {
    "mole.down",
    "mole",
    "mole (starts moving down)"
  },
  {
    "steelwall_slippery",
    "steelwall",
    "slippery steel wall"
  },
  {
    "invisible_sand",
    "sand",
    "invisible sand"
  },
  {
    "dx_unknown_15",
    "unknown",
    "dx unknown element 15"
  },
  {
    "dx_unknown_42",
    "unknown",
    "dx unknown element 42"
  },
  {
    "unused_319",
    "unused",
    "(not used)"
  },
  {
    "unused_320",
    "unused",
    "(not used)"
  },
  {
    "shield_deadly",
    "shield_deadly",
    "shield (deadly, kills enemies)"
  },
  {
    "timegate_open",
    "timegate",
    "time gate (open)"
  },
  {
    "timegate_closed",
    "timegate",
    "time gate (closed)"
  },
  {
    "timegate_switch.active",
    "timegate_switch",
    "switch for time gate"
  },
  {
    "timegate_switch",
    "timegate_switch",
    "switch for time gate"
  },
  {
    "balloon",
    "balloon",
    "balloon"
  },
  {
    "balloon_switch_left",
    "balloon_switch",
    "wind switch (left)"
  },
  {
    "balloon_switch_right",
    "balloon_switch",
    "wind switch (right)"
  },
  {
    "balloon_switch_up",
    "balloon_switch",
    "wind switch (up)"
  },
  {
    "balloon_switch_down",
    "balloon_switch",
    "wind switch (down)"
  },
  {
    "balloon_switch_any",
    "balloon_switch",
    "wind switch (any direction)"
  },
  {
    "emc_steelwall_1",
    "steelwall",
    "steel wall"
  },
  {
    "emc_steelwall_2",
    "steelwall",
    "steel wall"
  },
  {
    "emc_steelwall_3",
    "steelwall",
    "steel wall"
  },
  {
    "emc_steelwall_4",
    "steelwall",
    "steel wall"
  },
  {
    "emc_wall_1",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_2",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_3",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_4",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_5",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_6",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_7",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_8",
    "wall",
    "normal wall"
  },
  {
    "tube_any",
    "tube",
    "tube (any direction)"
  },
  {
    "tube_vertical",
    "tube",
    "tube (vertical)"
  },
  {
    "tube_horizontal",
    "tube",
    "tube (horizontal)"
  },
  {
    "tube_vertical_left",
    "tube",
    "tube (vertical & left)"
  },
  {
    "tube_vertical_right",
    "tube",
    "tube (vertical & right)"
  },
  {
    "tube_horizontal_up",
    "tube",
    "tube (horizontal & up)"
  },
  {
    "tube_horizontal_down",
    "tube",
    "tube (horizontal & down)"
  },
  {
    "tube_left_up",
    "tube",
    "tube (left & up)"
  },
  {
    "tube_left_down",
    "tube",
    "tube (left & down)"
  },
  {
    "tube_right_up",
    "tube",
    "tube (right & up)"
  },
  {
    "tube_right_down",
    "tube",
    "tube (right & down)"
  },
  {
    "spring",
    "spring",
    "spring"
  },
  {
    "trap",
    "trap",
    "trap"
  },
  {
    "dx_supabomb",
    "bomb",
    "stable bomb (DX style)"
  },
  {
    "unused_358",
    "unused",
    "-"
  },
  {
    "unused_359",
    "unused",
    "-"
  },
  {
    "custom_1",
    "custom",
    "custom element 1"
  },
  {
    "custom_2",
    "custom",
    "custom element 2"
  },
  {
    "custom_3",
    "custom",
    "custom element 3"
  },
  {
    "custom_4",
    "custom",
    "custom element 4"
  },
  {
    "custom_5",
    "custom",
    "custom element 5"
  },
  {
    "custom_6",
    "custom",
    "custom element 6"
  },
  {
    "custom_7",
    "custom",
    "custom element 7"
  },
  {
    "custom_8",
    "custom",
    "custom element 8"
  },
  {
    "custom_9",
    "custom",
    "custom element 9"
  },
  {
    "custom_10",
    "custom",
    "custom element 10"
  },
  {
    "custom_11",
    "custom",
    "custom element 11"
  },
  {
    "custom_12",
    "custom",
    "custom element 12"
  },
  {
    "custom_13",
    "custom",
    "custom element 13"
  },
  {
    "custom_14",
    "custom",
    "custom element 14"
  },
  {
    "custom_15",
    "custom",
    "custom element 15"
  },
  {
    "custom_16",
    "custom",
    "custom element 16"
  },
  {
    "custom_17",
    "custom",
    "custom element 17"
  },
  {
    "custom_18",
    "custom",
    "custom element 18"
  },
  {
    "custom_19",
    "custom",
    "custom element 19"
  },
  {
    "custom_20",
    "custom",
    "custom element 20"
  },
  {
    "custom_21",
    "custom",
    "custom element 21"
  },
  {
    "custom_22",
    "custom",
    "custom element 22"
  },
  {
    "custom_23",
    "custom",
    "custom element 23"
  },
  {
    "custom_24",
    "custom",
    "custom element 24"
  },
  {
    "custom_25",
    "custom",
    "custom element 25"
  },
  {
    "custom_26",
    "custom",
    "custom element 26"
  },
  {
    "custom_27",
    "custom",
    "custom element 27"
  },
  {
    "custom_28",
    "custom",
    "custom element 28"
  },
  {
    "custom_29",
    "custom",
    "custom element 29"
  },
  {
    "custom_30",
    "custom",
    "custom element 30"
  },
  {
    "custom_31",
    "custom",
    "custom element 31"
  },
  {
    "custom_32",
    "custom",
    "custom element 32"
  },
  {
    "custom_33",
    "custom",
    "custom element 33"
  },
  {
    "custom_34",
    "custom",
    "custom element 34"
  },
  {
    "custom_35",
    "custom",
    "custom element 35"
  },
  {
    "custom_36",
    "custom",
    "custom element 36"
  },
  {
    "custom_37",
    "custom",
    "custom element 37"
  },
  {
    "custom_38",
    "custom",
    "custom element 38"
  },
  {
    "custom_39",
    "custom",
    "custom element 39"
  },
  {
    "custom_40",
    "custom",
    "custom element 40"
  },
  {
    "custom_41",
    "custom",
    "custom element 41"
  },
  {
    "custom_42",
    "custom",
    "custom element 42"
  },
  {
    "custom_43",
    "custom",
    "custom element 43"
  },
  {
    "custom_44",
    "custom",
    "custom element 44"
  },
  {
    "custom_45",
    "custom",
    "custom element 45"
  },
  {
    "custom_46",
    "custom",
    "custom element 46"
  },
  {
    "custom_47",
    "custom",
    "custom element 47"
  },
  {
    "custom_48",
    "custom",
    "custom element 48"
  },
  {
    "custom_49",
    "custom",
    "custom element 49"
  },
  {
    "custom_50",
    "custom",
    "custom element 50"
  },
  {
    "custom_51",
    "custom",
    "custom element 51"
  },
  {
    "custom_52",
    "custom",
    "custom element 52"
  },
  {
    "custom_53",
    "custom",
    "custom element 53"
  },
  {
    "custom_54",
    "custom",
    "custom element 54"
  },
  {
    "custom_55",
    "custom",
    "custom element 55"
  },
  {
    "custom_56",
    "custom",
    "custom element 56"
  },
  {
    "custom_57",
    "custom",
    "custom element 57"
  },
  {
    "custom_58",
    "custom",
    "custom element 58"
  },
  {
    "custom_59",
    "custom",
    "custom element 59"
  },
  {
    "custom_60",
    "custom",
    "custom element 60"
  },
  {
    "custom_61",
    "custom",
    "custom element 61"
  },
  {
    "custom_62",
    "custom",
    "custom element 62"
  },
  {
    "custom_63",
    "custom",
    "custom element 63"
  },
  {
    "custom_64",
    "custom",
    "custom element 64"
  },
  {
    "custom_65",
    "custom",
    "custom element 65"
  },
  {
    "custom_66",
    "custom",
    "custom element 66"
  },
  {
    "custom_67",
    "custom",
    "custom element 67"
  },
  {
    "custom_68",
    "custom",
    "custom element 68"
  },
  {
    "custom_69",
    "custom",
    "custom element 69"
  },
  {
    "custom_70",
    "custom",
    "custom element 70"
  },
  {
    "custom_71",
    "custom",
    "custom element 71"
  },
  {
    "custom_72",
    "custom",
    "custom element 72"
  },
  {
    "custom_73",
    "custom",
    "custom element 73"
  },
  {
    "custom_74",
    "custom",
    "custom element 74"
  },
  {
    "custom_75",
    "custom",
    "custom element 75"
  },
  {
    "custom_76",
    "custom",
    "custom element 76"
  },
  {
    "custom_77",
    "custom",
    "custom element 77"
  },
  {
    "custom_78",
    "custom",
    "custom element 78"
  },
  {
    "custom_79",
    "custom",
    "custom element 79"
  },
  {
    "custom_80",
    "custom",
    "custom element 80"
  },
  {
    "custom_81",
    "custom",
    "custom element 81"
  },
  {
    "custom_82",
    "custom",
    "custom element 82"
  },
  {
    "custom_83",
    "custom",
    "custom element 83"
  },
  {
    "custom_84",
    "custom",
    "custom element 84"
  },
  {
    "custom_85",
    "custom",
    "custom element 85"
  },
  {
    "custom_86",
    "custom",
    "custom element 86"
  },
  {
    "custom_87",
    "custom",
    "custom element 87"
  },
  {
    "custom_88",
    "custom",
    "custom element 88"
  },
  {
    "custom_89",
    "custom",
    "custom element 89"
  },
  {
    "custom_90",
    "custom",
    "custom element 90"
  },
  {
    "custom_91",
    "custom",
    "custom element 91"
  },
  {
    "custom_92",
    "custom",
    "custom element 92"
  },
  {
    "custom_93",
    "custom",
    "custom element 93"
  },
  {
    "custom_94",
    "custom",
    "custom element 94"
  },
  {
    "custom_95",
    "custom",
    "custom element 95"
  },
  {
    "custom_96",
    "custom",
    "custom element 96"
  },
  {
    "custom_97",
    "custom",
    "custom element 97"
  },
  {
    "custom_98",
    "custom",
    "custom element 98"
  },
  {
    "custom_99",
    "custom",
    "custom element 99"
  },
  {
    "custom_100",
    "custom",
    "custom element 100"
  },
  {
    "custom_101",
    "custom",
    "custom element 101"
  },
  {
    "custom_102",
    "custom",
    "custom element 102"
  },
  {
    "custom_103",
    "custom",
    "custom element 103"
  },
  {
    "custom_104",
    "custom",
    "custom element 104"
  },
  {
    "custom_105",
    "custom",
    "custom element 105"
  },
  {
    "custom_106",
    "custom",
    "custom element 106"
  },
  {
    "custom_107",
    "custom",
    "custom element 107"
  },
  {
    "custom_108",
    "custom",
    "custom element 108"
  },
  {
    "custom_109",
    "custom",
    "custom element 109"
  },
  {
    "custom_110",
    "custom",
    "custom element 110"
  },
  {
    "custom_111",
    "custom",
    "custom element 111"
  },
  {
    "custom_112",
    "custom",
    "custom element 112"
  },
  {
    "custom_113",
    "custom",
    "custom element 113"
  },
  {
    "custom_114",
    "custom",
    "custom element 114"
  },
  {
    "custom_115",
    "custom",
    "custom element 115"
  },
  {
    "custom_116",
    "custom",
    "custom element 116"
  },
  {
    "custom_117",
    "custom",
    "custom element 117"
  },
  {
    "custom_118",
    "custom",
    "custom element 118"
  },
  {
    "custom_119",
    "custom",
    "custom element 119"
  },
  {
    "custom_120",
    "custom",
    "custom element 120"
  },
  {
    "custom_121",
    "custom",
    "custom element 121"
  },
  {
    "custom_122",
    "custom",
    "custom element 122"
  },
  {
    "custom_123",
    "custom",
    "custom element 123"
  },
  {
    "custom_124",
    "custom",
    "custom element 124"
  },
  {
    "custom_125",
    "custom",
    "custom element 125"
  },
  {
    "custom_126",
    "custom",
    "custom element 126"
  },
  {
    "custom_127",
    "custom",
    "custom element 127"
  },
  {
    "custom_128",
    "custom",
    "custom element 128"
  },
  {
    "custom_129",
    "custom",
    "custom element 129"
  },
  {
    "custom_130",
    "custom",
    "custom element 130"
  },
  {
    "custom_131",
    "custom",
    "custom element 131"
  },
  {
    "custom_132",
    "custom",
    "custom element 132"
  },
  {
    "custom_133",
    "custom",
    "custom element 133"
  },
  {
    "custom_134",
    "custom",
    "custom element 134"
  },
  {
    "custom_135",
    "custom",
    "custom element 135"
  },
  {
    "custom_136",
    "custom",
    "custom element 136"
  },
  {
    "custom_137",
    "custom",
    "custom element 137"
  },
  {
    "custom_138",
    "custom",
    "custom element 138"
  },
  {
    "custom_139",
    "custom",
    "custom element 139"
  },
  {
    "custom_140",
    "custom",
    "custom element 140"
  },
  {
    "custom_141",
    "custom",
    "custom element 141"
  },
  {
    "custom_142",
    "custom",
    "custom element 142"
  },
  {
    "custom_143",
    "custom",
    "custom element 143"
  },
  {
    "custom_144",
    "custom",
    "custom element 144"
  },
  {
    "custom_145",
    "custom",
    "custom element 145"
  },
  {
    "custom_146",
    "custom",
    "custom element 146"
  },
  {
    "custom_147",
    "custom",
    "custom element 147"
  },
  {
    "custom_148",
    "custom",
    "custom element 148"
  },
  {
    "custom_149",
    "custom",
    "custom element 149"
  },
  {
    "custom_150",
    "custom",
    "custom element 150"
  },
  {
    "custom_151",
    "custom",
    "custom element 151"
  },
  {
    "custom_152",
    "custom",
    "custom element 152"
  },
  {
    "custom_153",
    "custom",
    "custom element 153"
  },
  {
    "custom_154",
    "custom",
    "custom element 154"
  },
  {
    "custom_155",
    "custom",
    "custom element 155"
  },
  {
    "custom_156",
    "custom",
    "custom element 156"
  },
  {
    "custom_157",
    "custom",
    "custom element 157"
  },
  {
    "custom_158",
    "custom",
    "custom element 158"
  },
  {
    "custom_159",
    "custom",
    "custom element 159"
  },
  {
    "custom_160",
    "custom",
    "custom element 160"
  },
  {
    "custom_161",
    "custom",
    "custom element 161"
  },
  {
    "custom_162",
    "custom",
    "custom element 162"
  },
  {
    "custom_163",
    "custom",
    "custom element 163"
  },
  {
    "custom_164",
    "custom",
    "custom element 164"
  },
  {
    "custom_165",
    "custom",
    "custom element 165"
  },
  {
    "custom_166",
    "custom",
    "custom element 166"
  },
  {
    "custom_167",
    "custom",
    "custom element 167"
  },
  {
    "custom_168",
    "custom",
    "custom element 168"
  },
  {
    "custom_169",
    "custom",
    "custom element 169"
  },
  {
    "custom_170",
    "custom",
    "custom element 170"
  },
  {
    "custom_171",
    "custom",
    "custom element 171"
  },
  {
    "custom_172",
    "custom",
    "custom element 172"
  },
  {
    "custom_173",
    "custom",
    "custom element 173"
  },
  {
    "custom_174",
    "custom",
    "custom element 174"
  },
  {
    "custom_175",
    "custom",
    "custom element 175"
  },
  {
    "custom_176",
    "custom",
    "custom element 176"
  },
  {
    "custom_177",
    "custom",
    "custom element 177"
  },
  {
    "custom_178",
    "custom",
    "custom element 178"
  },
  {
    "custom_179",
    "custom",
    "custom element 179"
  },
  {
    "custom_180",
    "custom",
    "custom element 180"
  },
  {
    "custom_181",
    "custom",
    "custom element 181"
  },
  {
    "custom_182",
    "custom",
    "custom element 182"
  },
  {
    "custom_183",
    "custom",
    "custom element 183"
  },
  {
    "custom_184",
    "custom",
    "custom element 184"
  },
  {
    "custom_185",
    "custom",
    "custom element 185"
  },
  {
    "custom_186",
    "custom",
    "custom element 186"
  },
  {
    "custom_187",
    "custom",
    "custom element 187"
  },
  {
    "custom_188",
    "custom",
    "custom element 188"
  },
  {
    "custom_189",
    "custom",
    "custom element 189"
  },
  {
    "custom_190",
    "custom",
    "custom element 190"
  },
  {
    "custom_191",
    "custom",
    "custom element 191"
  },
  {
    "custom_192",
    "custom",
    "custom element 192"
  },
  {
    "custom_193",
    "custom",
    "custom element 193"
  },
  {
    "custom_194",
    "custom",
    "custom element 194"
  },
  {
    "custom_195",
    "custom",
    "custom element 195"
  },
  {
    "custom_196",
    "custom",
    "custom element 196"
  },
  {
    "custom_197",
    "custom",
    "custom element 197"
  },
  {
    "custom_198",
    "custom",
    "custom element 198"
  },
  {
    "custom_199",
    "custom",
    "custom element 199"
  },
  {
    "custom_200",
    "custom",
    "custom element 200"
  },
  {
    "custom_201",
    "custom",
    "custom element 201"
  },
  {
    "custom_202",
    "custom",
    "custom element 202"
  },
  {
    "custom_203",
    "custom",
    "custom element 203"
  },
  {
    "custom_204",
    "custom",
    "custom element 204"
  },
  {
    "custom_205",
    "custom",
    "custom element 205"
  },
  {
    "custom_206",
    "custom",
    "custom element 206"
  },
  {
    "custom_207",
    "custom",
    "custom element 207"
  },
  {
    "custom_208",
    "custom",
    "custom element 208"
  },
  {
    "custom_209",
    "custom",
    "custom element 209"
  },
  {
    "custom_210",
    "custom",
    "custom element 210"
  },
  {
    "custom_211",
    "custom",
    "custom element 211"
  },
  {
    "custom_212",
    "custom",
    "custom element 212"
  },
  {
    "custom_213",
    "custom",
    "custom element 213"
  },
  {
    "custom_214",
    "custom",
    "custom element 214"
  },
  {
    "custom_215",
    "custom",
    "custom element 215"
  },
  {
    "custom_216",
    "custom",
    "custom element 216"
  },
  {
    "custom_217",
    "custom",
    "custom element 217"
  },
  {
    "custom_218",
    "custom",
    "custom element 218"
  },
  {
    "custom_219",
    "custom",
    "custom element 219"
  },
  {
    "custom_220",
    "custom",
    "custom element 220"
  },
  {
    "custom_221",
    "custom",
    "custom element 221"
  },
  {
    "custom_222",
    "custom",
    "custom element 222"
  },
  {
    "custom_223",
    "custom",
    "custom element 223"
  },
  {
    "custom_224",
    "custom",
    "custom element 224"
  },
  {
    "custom_225",
    "custom",
    "custom element 225"
  },
  {
    "custom_226",
    "custom",
    "custom element 226"
  },
  {
    "custom_227",
    "custom",
    "custom element 227"
  },
  {
    "custom_228",
    "custom",
    "custom element 228"
  },
  {
    "custom_229",
    "custom",
    "custom element 229"
  },
  {
    "custom_230",
    "custom",
    "custom element 230"
  },
  {
    "custom_231",
    "custom",
    "custom element 231"
  },
  {
    "custom_232",
    "custom",
    "custom element 232"
  },
  {
    "custom_233",
    "custom",
    "custom element 233"
  },
  {
    "custom_234",
    "custom",
    "custom element 234"
  },
  {
    "custom_235",
    "custom",
    "custom element 235"
  },
  {
    "custom_236",
    "custom",
    "custom element 236"
  },
  {
    "custom_237",
    "custom",
    "custom element 237"
  },
  {
    "custom_238",
    "custom",
    "custom element 238"
  },
  {
    "custom_239",
    "custom",
    "custom element 239"
  },
  {
    "custom_240",
    "custom",
    "custom element 240"
  },
  {
    "custom_241",
    "custom",
    "custom element 241"
  },
  {
    "custom_242",
    "custom",
    "custom element 242"
  },
  {
    "custom_243",
    "custom",
    "custom element 243"
  },
  {
    "custom_244",
    "custom",
    "custom element 244"
  },
  {
    "custom_245",
    "custom",
    "custom element 245"
  },
  {
    "custom_246",
    "custom",
    "custom element 246"
  },
  {
    "custom_247",
    "custom",
    "custom element 247"
  },
  {
    "custom_248",
    "custom",
    "custom element 248"
  },
  {
    "custom_249",
    "custom",
    "custom element 249"
  },
  {
    "custom_250",
    "custom",
    "custom element 250"
  },
  {
    "custom_251",
    "custom",
    "custom element 251"
  },
  {
    "custom_252",
    "custom",
    "custom element 252"
  },
  {
    "custom_253",
    "custom",
    "custom element 253"
  },
  {
    "custom_254",
    "custom",
    "custom element 254"
  },
  {
    "custom_255",
    "custom",
    "custom element 255"
  },
  {
    "custom_256",
    "custom",
    "custom element 256"
  },
  {
    "em_key_1",
    "key",
    "key 1 (EM style)"
    },
  {
    "em_key_2",
    "key",
    "key 2 (EM style)"
    },
  {
    "em_key_3",
    "key",
    "key 3 (EM style)"
  },
  {
    "em_key_4",
    "key",
    "key 4 (EM style)"
  },
  {
    "envelope_1",
    "envelope",
    "mail envelope 1"
  },
  {
    "envelope_2",
    "envelope",
    "mail envelope 2"
  },
  {
    "envelope_3",
    "envelope",
    "mail envelope 3"
  },
  {
    "envelope_4",
    "envelope",
    "mail envelope 4"
  },
  {
    "group_1",
    "group",
    "group element 1"
  },
  {
    "group_2",
    "group",
    "group element 2"
  },
  {
    "group_3",
    "group",
    "group element 3"
  },
  {
    "group_4",
    "group",
    "group element 4"
  },
  {
    "group_5",
    "group",
    "group element 5"
  },
  {
    "group_6",
    "group",
    "group element 6"
  },
  {
    "group_7",
    "group",
    "group element 7"
  },
  {
    "group_8",
    "group",
    "group element 8"
  },
  {
    "group_9",
    "group",
    "group element 9"
  },
  {
    "group_10",
    "group",
    "group element 10"
  },
  {
    "group_11",
    "group",
    "group element 11"
  },
  {
    "group_12",
    "group",
    "group element 12"
  },
  {
    "group_13",
    "group",
    "group element 13"
  },
  {
    "group_14",
    "group",
    "group element 14"
  },
  {
    "group_15",
    "group",
    "group element 15"
  },
  {
    "group_16",
    "group",
    "group element 16"
  },
  {
    "group_17",
    "group",
    "group element 17"
  },
  {
    "group_18",
    "group",
    "group element 18"
  },
  {
    "group_19",
    "group",
    "group element 19"
  },
  {
    "group_20",
    "group",
    "group element 20"
  },
  {
    "group_21",
    "group",
    "group element 21"
  },
  {
    "group_22",
    "group",
    "group element 22"
  },
  {
    "group_23",
    "group",
    "group element 23"
  },
  {
    "group_24",
    "group",
    "group element 24"
  },
  {
    "group_25",
    "group",
    "group element 25"
  },
  {
    "group_26",
    "group",
    "group element 26"
  },
  {
    "group_27",
    "group",
    "group element 27"
  },
  {
    "group_28",
    "group",
    "group element 28"
  },
  {
    "group_29",
    "group",
    "group element 29"
  },
  {
    "group_30",
    "group",
    "group element 30"
  },
  {
    "group_31",
    "group",
    "group element 31"
  },
  {
    "group_32",
    "group",
    "group element 32"
  },
  {
    "unknown",
    "unknown",
    "unknown element"
  },
  {
    "trigger_element",
    "trigger",
    "element triggering change"
  },
  {
    "trigger_player",
    "trigger",
    "player triggering change"
  },
  {
    "sp_gravity_on_port_right",
    "sp_gravity_on_port",
    "gravity-on port (leading right)"
  },
  {
    "sp_gravity_on_port_down",
    "sp_gravity_on_port",
    "gravity-on port (leading down)"
  },
  {
    "sp_gravity_on_port_left",
    "sp_gravity_on_port",
    "gravity-on port (leading left)"
  },
  {
    "sp_gravity_on_port_up",
    "sp_gravity_on_port",
    "gravity-on port (leading up)"
  },
  {
    "sp_gravity_off_port_right",
    "sp_gravity_off_port",
    "gravity-off port (leading right)"
  },
  {
    "sp_gravity_off_port_down",
    "sp_gravity_off_port",
    "gravity-off port (leading down)"
  },
  {
    "sp_gravity_off_port_left",
    "sp_gravity_off_port",
    "gravity-off port (leading left)"
  },
  {
    "sp_gravity_off_port_up",
    "sp_gravity_off_port",
    "gravity-off port (leading up)"
  },
  {
    "balloon_switch_none",
    "balloon_switch",
    "wind switch (off)"
  },
  {
    "emc_gate_5",
    "gate",
    "door 5 (EMC style)",
  },
  {
    "emc_gate_6",
    "gate",
    "door 6 (EMC style)",
  },
  {
    "emc_gate_7",
    "gate",
    "door 7 (EMC style)",
  },
  {
    "emc_gate_8",
    "gate",
    "door 8 (EMC style)",
  },
  {
    "emc_gate_5_gray",
    "gate",
    "gray door (EMC style, key 5)",
  },
  {
    "emc_gate_6_gray",
    "gate",
    "gray door (EMC style, key 6)",
  },
  {
    "emc_gate_7_gray",
    "gate",
    "gray door (EMC style, key 7)",
  },
  {
    "emc_gate_8_gray",
    "gate",
    "gray door (EMC style, key 8)",
  },
  {
    "emc_key_5",
    "key",
    "key 5 (EMC style)",
  },
  {
    "emc_key_6",
    "key",
    "key 6 (EMC style)",
  },
  {
    "emc_key_7",
    "key",
    "key 7 (EMC style)",
  },
  {
    "emc_key_8",
    "key",
    "key 8 (EMC style)",
  },
  {
    "emc_android",
    "emc_android",
    "android",
  },
  {
    "emc_grass",
    "emc_grass",
    "grass",
  },
  {
    "emc_magic_ball",
    "emc_magic_ball",
    "magic ball",
  },
  {
    "emc_magic_ball.active",
    "emc_magic_ball",
    "magic ball (activated)",
  },
  {
    "emc_magic_ball_switch",
    "emc_magic_ball_switch",
    "magic ball switch (off)",
  },
  {
    "emc_magic_ball_switch.active",
    "emc_magic_ball_switch",
    "magic ball switch (on)",
  },
  {
    "emc_spring_bumper",
    "emc_spring_bumper",
    "spring bumper",
  },
  {
    "emc_plant",
    "emc_plant",
    "plant",
  },
  {
    "emc_lenses",
    "emc_lenses",
    "lenses",
  },
  {
    "emc_magnifier",
    "emc_magnifier",
    "magnifier",
  },
  {
    "emc_wall_9",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_10",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_11",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_12",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_13",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_14",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_15",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_16",
    "wall",
    "normal wall"
  },
  {
    "emc_wall_slippery_1",
    "wall",
    "slippery wall"
  },
  {
    "emc_wall_slippery_2",
    "wall",
    "slippery wall"
  },
  {
    "emc_wall_slippery_3",
    "wall",
    "slippery wall"
  },
  {
    "emc_wall_slippery_4",
    "wall",
    "slippery wall"
  },
  {
    "emc_fake_grass",
    "fake_grass",
    "fake grass"
  },
  {
    "emc_fake_acid",
    "fake_acid",
    "fake acid"
  },
  {
    "emc_dripper",
    "dripper",
    "dripper"
  },
  {
    "trigger_ce_value",
    "trigger",
    "CE value of element triggering change"
  },
  {
    "trigger_ce_score",
    "trigger",
    "CE score of element triggering change"
  },
  {
    "current_ce_value",
    "current",
    "CE value of current element"
  },
  {
    "current_ce_score",
    "current",
    "CE score of current element"
  },
  {
    "yamyam.left",
    "yamyam",
    "yam yam (starts moving left)"
  },
  {
    "yamyam.right",
    "yamyam",
    "yam yam (starts moving right)"
  },
  {
    "yamyam.up",
    "yamyam",
    "yam yam (starts moving up)"
  },
  {
    "yamyam.down",
    "yamyam",
    "yam yam (starts moving down)"
  },
  {
    "bd_expandable_wall",
    "wall",
    "growing wall (horizontal, BD style)"
  },
  {
    "prev_ce_8",
    "prev_ce",
    "CE 8 positions earlier in list"
  },
  {
    "prev_ce_7",
    "prev_ce",
    "CE 7 positions earlier in list"
  },
  {
    "prev_ce_6",
    "prev_ce",
    "CE 6 positions earlier in list"
  },
  {
    "prev_ce_5",
    "prev_ce",
    "CE 5 positions earlier in list"
  },
  {
    "prev_ce_4",
    "prev_ce",
    "CE 4 positions earlier in list"
  },
  {
    "prev_ce_3",
    "prev_ce",
    "CE 3 positions earlier in list"
  },
  {
    "prev_ce_2",
    "prev_ce",
    "CE 2 positions earlier in list"
  },
  {
    "prev_ce_1",
    "prev_ce",
    "CE 1 position earlier in list"
  },
  {
    "self",
    "self",
    "the current custom element"
  },
  {
    "next_ce_1",
    "next_ce",
    "CE 1 position later in list"
  },
  {
    "next_ce_2",
    "next_ce",
    "CE 2 positions later in list"
  },
  {
    "next_ce_3",
    "next_ce",
    "CE 3 positions later in list"
  },
  {
    "next_ce_4",
    "next_ce",
    "CE 4 positions later in list"
  },
  {
    "next_ce_5",
    "next_ce",
    "CE 5 positions later in list"
  },
  {
    "next_ce_6",
    "next_ce",
    "CE 6 positions later in list"
  },
  {
    "next_ce_7",
    "next_ce",
    "CE 7 positions later in list"
  },
  {
    "next_ce_8",
    "next_ce",
    "CE 8 positions later in list"
  },
  {
    "any_element",
    "any_element",
    "this element matches any element"
  },
  {
    "steel_char_space",
    "steel_char",
    "steel letter ' '"
  },
  {
    "steel_char_exclam",
    "steel_char",
    "steel letter '!'"
  },
  {
    "steel_char_quotedbl",
    "steel_char",
    "steel letter '\"'"
  },
  {
    "steel_char_numbersign",
    "steel_char",
    "steel letter '#'"
  },
  {
    "steel_char_dollar",
    "steel_char",
    "steel letter '$'"
  },
  {
    "steel_char_percent",
    "steel_char",
    "steel letter '%'"
  },
  {
    "steel_char_ampersand",
    "steel_char",
    "steel letter '&'"
  },
  {
    "steel_char_apostrophe",
    "steel_char",
    "steel letter '''"
  },
  {
    "steel_char_parenleft",
    "steel_char",
    "steel letter '('"
  },
  {
    "steel_char_parenright",
    "steel_char",
    "steel letter ')'"
  },
  {
    "steel_char_asterisk",
    "steel_char",
    "steel letter '*'"
  },
  {
    "steel_char_plus",
    "steel_char",
    "steel letter '+'"
  },
  {
    "steel_char_comma",
    "steel_char",
    "steel letter ','"
  },
  {
    "steel_char_minus",
    "steel_char",
    "steel letter '-'"
  },
  {
    "steel_char_period",
    "steel_char",
    "steel letter '.'"
  },
  {
    "steel_char_slash",
    "steel_char",
    "steel letter '/'"
  },
  {
    "steel_char_0",
    "steel_char",
    "steel letter '0'"
  },
  {
    "steel_char_1",
    "steel_char",
    "steel letter '1'"
  },
  {
    "steel_char_2",
    "steel_char",
    "steel letter '2'"
  },
  {
    "steel_char_3",
    "steel_char",
    "steel letter '3'"
  },
  {
    "steel_char_4",
    "steel_char",
    "steel letter '4'"
  },
  {
    "steel_char_5",
    "steel_char",
    "steel letter '5'"
  },
  {
    "steel_char_6",
    "steel_char",
    "steel letter '6'"
  },
  {
    "steel_char_7",
    "steel_char",
    "steel letter '7'"
  },
  {
    "steel_char_8",
    "steel_char",
    "steel letter '8'"
  },
  {
    "steel_char_9",
    "steel_char",
    "steel letter '9'"
  },
  {
    "steel_char_colon",
    "steel_char",
    "steel letter ':'"
  },
  {
    "steel_char_semicolon",
    "steel_char",
    "steel letter ';'"
  },
  {
    "steel_char_less",
    "steel_char",
    "steel letter '<'"
  },
  {
    "steel_char_equal",
    "steel_char",
    "steel letter '='"
  },
  {
    "steel_char_greater",
    "steel_char",
    "steel letter '>'"
  },
  {
    "steel_char_question",
    "steel_char",
    "steel letter '?'"
  },
  {
    "steel_char_at",
    "steel_char",
    "steel letter '@'"
  },
  {
    "steel_char_a",
    "steel_char",
    "steel letter 'A'"
  },
  {
    "steel_char_b",
    "steel_char",
    "steel letter 'B'"
  },
  {
    "steel_char_c",
    "steel_char",
    "steel letter 'C'"
  },
  {
    "steel_char_d",
    "steel_char",
    "steel letter 'D'"
  },
  {
    "steel_char_e",
    "steel_char",
    "steel letter 'E'"
  },
  {
    "steel_char_f",
    "steel_char",
    "steel letter 'F'"
  },
  {
    "steel_char_g",
    "steel_char",
    "steel letter 'G'"
  },
  {
    "steel_char_h",
    "steel_char",
    "steel letter 'H'"
  },
  {
    "steel_char_i",
    "steel_char",
    "steel letter 'I'"
  },
  {
    "steel_char_j",
    "steel_char",
    "steel letter 'J'"
  },
  {
    "steel_char_k",
    "steel_char",
    "steel letter 'K'"
  },
  {
    "steel_char_l",
    "steel_char",
    "steel letter 'L'"
  },
  {
    "steel_char_m",
    "steel_char",
    "steel letter 'M'"
  },
  {
    "steel_char_n",
    "steel_char",
    "steel letter 'N'"
  },
  {
    "steel_char_o",
    "steel_char",
    "steel letter 'O'"
  },
  {
    "steel_char_p",
    "steel_char",
    "steel letter 'P'"
  },
  {
    "steel_char_q",
    "steel_char",
    "steel letter 'Q'"
  },
  {
    "steel_char_r",
    "steel_char",
    "steel letter 'R'"
  },
  {
    "steel_char_s",
    "steel_char",
    "steel letter 'S'"
  },
  {
    "steel_char_t",
    "steel_char",
    "steel letter 'T'"
  },
  {
    "steel_char_u",
    "steel_char",
    "steel letter 'U'"
  },
  {
    "steel_char_v",
    "steel_char",
    "steel letter 'V'"
  },
  {
    "steel_char_w",
    "steel_char",
    "steel letter 'W'"
  },
  {
    "steel_char_x",
    "steel_char",
    "steel letter 'X'"
  },
  {
    "steel_char_y",
    "steel_char",
    "steel letter 'Y'"
  },
  {
    "steel_char_z",
    "steel_char",
    "steel letter 'Z'"
  },
  {
    "steel_char_bracketleft",
    "steel_char",
    "steel letter '['"
  },
  {
    "steel_char_backslash",
    "steel_char",
    "steel letter '\\'"
  },
  {
    "steel_char_bracketright",
    "steel_char",
    "steel letter ']'"
  },
  {
    "steel_char_asciicircum",
    "steel_char",
    "steel letter '^'"
  },
  {
    "steel_char_underscore",
    "steel_char",
    "steel letter '_'"
  },
  {
    "steel_char_copyright",
    "steel_char",
    "steel letter '\xa9'"
  },
  {
    "steel_char_aumlaut",
    "steel_char",
    "steel letter '\xc4'"
  },
  {
    "steel_char_oumlaut",
    "steel_char",
    "steel letter '\xd6'"
  },
  {
    "steel_char_uumlaut",
    "steel_char",
    "steel letter '\xdc'"
  },
  {
    "steel_char_degree",
    "steel_char",
    "steel letter '\xb0'"
  },
  {
    "steel_char_trademark",
    "steel_char",
    "steel letter '\xae'"
  },
  {
    "steel_char_cursor",
    "steel_char",
    "steel letter '\xa0'"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter ''"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter 'button'"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter 'up'"
  },
  {
    "steel_char_unused",
    "steel_char",
    "steel letter 'down'"
  },
  {
    "sperms",
    "frankie",
    "sperms"
  },
  {
    "bullet",
    "frankie",
    "bullet"
  },
  {
    "heart",
    "frankie",
    "heart"
  },
  {
    "cross",
    "frankie",
    "cross"
  },
  {
    "frankie",
    "frankie",
    "frankie"
  },
  {
    "sign_sperms",
    "sign",
    "sign (sperms)"
  },
  {
    "sign_bullet",
    "sign",
    "sign (bullet)"
  },
  {
    "sign_heart",
    "sign",
    "sign (heart)"
  },
  {
    "sign_cross",
    "sign",
    "sign (cross)"
  },
  {
    "sign_frankie",
    "sign",
    "sign (frankie)"
  },
  {
    "steel_exit_closed",
    "steel_exit",
    "closed steel exit"
  },
  {
    "steel_exit_open",
    "steel_exit",
    "open steel exit"
  },
  {
    "dc_steelwall_1_left",
    "steelwall",
    "steel wall (left)"
  },
  {
    "dc_steelwall_1_right",
    "steelwall",
    "steel wall (right)"
  },
  {
    "dc_steelwall_1_top",
    "steelwall",
    "steel wall (top)"
  },
  {
    "dc_steelwall_1_bottom",
    "steelwall",
    "steel wall (bottom)"
  },
  {
    "dc_steelwall_1_horizontal",
    "steelwall",
    "steel wall (top/bottom)"
  },
  {
    "dc_steelwall_1_vertical",
    "steelwall",
    "steel wall (left/right)"
  },
  {
    "dc_steelwall_1_topleft",
    "steelwall",
    "steel wall (top/left)"
  },
  {
    "dc_steelwall_1_topright",
    "steelwall",
    "steel wall (top/right)"
  },
  {
    "dc_steelwall_1_bottomleft",
    "steelwall",
    "steel wall (bottom/left)"
  },
  {
    "dc_steelwall_1_bottomright",
    "steelwall",
    "steel wall (bottom/right)"
  },
  {
    "dc_steelwall_1_topleft_2",
    "steelwall",
    "steel wall (top/left corner)"
  },
  {
    "dc_steelwall_1_topright_2",
    "steelwall",
    "steel wall (top/right corner)"
  },
  {
    "dc_steelwall_1_bottomleft_2",
    "steelwall",
    "steel wall (bottom/left corner)"
  },
  {
    "dc_steelwall_1_bottomright_2",
    "steelwall",
    "steel wall (bottom/right corner)"
  },
  {
    "dc_steelwall_2_left",
    "steelwall",
    "steel wall (left)"
  },
  {
    "dc_steelwall_2_right",
    "steelwall",
    "steel wall (right)"
  },
  {
    "dc_steelwall_2_top",
    "steelwall",
    "steel wall (top)"
  },
  {
    "dc_steelwall_2_bottom",
    "steelwall",
    "steel wall (bottom)"
  },
  {
    "dc_steelwall_2_horizontal",
    "steelwall",
    "steel wall (horizontal)"
  },
  {
    "dc_steelwall_2_vertical",
    "steelwall",
    "steel wall (vertical)"
  },
  {
    "dc_steelwall_2_middle",
    "steelwall",
    "steel wall (middle)"
  },
  {
    "dc_steelwall_2_single",
    "steelwall",
    "steel wall (single)"
  },
  {
    "dc_switchgate_switch_up",
    "switchgate_switch",
    "switch for switch gate (steel)"
  },
  {
    "dc_switchgate_switch_down",
    "switchgate_switch",
    "switch for switch gate (steel)"
  },
  {
    "dc_timegate_switch",
    "timegate_switch",
    "switch for time gate (steel)"
  },
  {
    "dc_timegate_switch.active",
    "timegate_switch",
    "switch for time gate (steel)"
  },
  {
    "dc_landmine",
    "dc_landmine",
    "land mine (DC style, removable)"
  },
  {
    "expandable_steelwall",
    "steelwall",
    "growing steel wall"
  },
  {
    "expandable_steelwall_horizontal",
    "steelwall",
    "growing steel wall (horizontal)"
  },
  {
    "expandable_steelwall_vertical",
    "steelwall",
    "growing steel wall (vertical)"
  },
  {
    "expandable_steelwall_any",
    "steelwall",
    "growing steel wall (any direction)"
  },
  {
    "em_exit_closed",
    "em_exit",
    "closed exit (EM style)"
  },
  {
    "em_exit_open",
    "em_exit",
    "open exit (EM style)"
  },
  {
    "em_steel_exit_closed",
    "em_steel_exit",
    "closed steel exit (EM style)"
  },
  {
    "em_steel_exit_open",
    "em_steel_exit",
    "open steel exit (EM style)"
  },
  {
    "dc_gate_fake_gray",
    "gate",
    "gray door (opened by no key)"
  },
  {
    "dc_magic_wall",
    "dc_magic_wall",
    "magic wall (DC style)"
  },
  {
    "quicksand_fast_empty",
    "quicksand",
    "fast quicksand (empty)"
  },
  {
    "quicksand_fast_full",
    "quicksand",
    "fast quicksand (with rock)"
  },
  {
    "from_level_template",
    "from_level_template",
    "element taken from level template"
  },
  {
    "mm_empty_space",
    "empty_space",
    "empty space"
  },
  {
    "mm_mirror_1",
    "mm_mirror",
    "mirror (0\xb0)"
  },
  {
    "mm_mirror_2",
    "mm_mirror",
    "mirror (11.25\xb0)"
  },
  {
    "mm_mirror_3",
    "mm_mirror",
    "mirror (22.5\xb0)"
  },
  {
    "mm_mirror_4",
    "mm_mirror",
    "mirror (33.75\xb0)"
  },
  {
    "mm_mirror_5",
    "mm_mirror",
    "mirror (45\xb0)"
  },
  {
    "mm_mirror_6",
    "mm_mirror",
    "mirror (56.25\xb0)"
  },
  {
    "mm_mirror_7",
    "mm_mirror",
    "mirror (67.5\xb0)"
  },
  {
    "mm_mirror_8",
    "mm_mirror",
    "mirror (78.75\xb0)"
  },
  {
    "mm_mirror_9",
    "mm_mirror",
    "mirror (90\xb0)"
  },
  {
    "mm_mirror_10",
    "mm_mirror",
    "mirror (101.25\xb0)"
  },
  {
    "mm_mirror_11",
    "mm_mirror",
    "mirror (112.5\xb0)"
  },
  {
    "mm_mirror_12",
    "mm_mirror",
    "mirror (123.75\xb0)"
  },
  {
    "mm_mirror_13",
    "mm_mirror",
    "mirror (135\xb0)"
  },
  {
    "mm_mirror_14",
    "mm_mirror",
    "mirror (146.25\xb0)"
  },
  {
    "mm_mirror_15",
    "mm_mirror",
    "mirror (157.5\xb0)"
  },
  {
    "mm_mirror_16",
    "mm_mirror",
    "mirror (168.75\xb0)"
  },
  {
    "mm_steel_grid_fixed_1",
    "mm_steel_grid_fixed",
    "fixed steel polarizer (0\xb0)"
  },
  {
    "mm_steel_grid_fixed_2",
    "mm_steel_grid_fixed",
    "fixed steel polarizer (90\xb0)"
  },
  {
    "mm_steel_grid_fixed_3",
    "mm_steel_grid_fixed",
    "fixed steel polarizer (45\xb0)"
  },
  {
    "mm_steel_grid_fixed_4",
    "mm_steel_grid_fixed",
    "fixed steel polarizer (135\xb0)"
  },
  {
    "mm_mcduffin.right",
    "mm_mcduffin",
    "Gregor McDuffin (looking right)"
  },
  {
    "mm_mcduffin.up",
    "mm_mcduffin",
    "Gregor McDuffin (looking up)"
  },
  {
    "mm_mcduffin.left",
    "mm_mcduffin",
    "Gregor McDuffin (looking left)"
  },
  {
    "mm_mcduffin.down",
    "mm_mcduffin",
    "Gregor McDuffin (looking down)"
  },
  {
    "mm_exit_closed",
    "mm_exit",
    "closed exit (MM style)"
  },
  {
    "mm_exit_opening_1",
    "mm_exit",
    "opening exit 1"
  },
  {
    "mm_exit_opening_2",
    "mm_exit",
    "opening exit 2"
  },
  {
    "mm_exit_open",
    "mm_exit",
    "open exit (MM style)"
  },
  {
    "mm_kettle",
    "mm_kettle",
    "magic kettle"
  },
  {
    "mm_bomb",
    "mm_bomb",
    "bomb (MM style)"
  },
  {
    "mm_prism",
    "mm_prism",
    "prism"
  },
  {
    "mm_steel_wall_1",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_2",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_3",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_4",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_5",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_6",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_7",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_8",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_9",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_10",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_11",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_12",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_13",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_14",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_15",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_steel_wall_16",
    "mm_steel_wall",
    "steel wall"
  },
  {
    "mm_wooden_wall_1",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_2",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_3",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_4",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_5",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_6",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_7",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_8",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_9",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_10",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_11",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_12",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_13",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_14",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_15",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_wooden_wall_16",
    "mm_wooden_wall",
    "wooden wall"
  },
  {
    "mm_ice_wall_1",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_2",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_3",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_4",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_5",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_6",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_7",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_8",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_9",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_10",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_11",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_12",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_13",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_14",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_15",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_ice_wall_16",
    "mm_ice_wall",
    "ice wall"
  },
  {
    "mm_amoeba_wall_1",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_2",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_3",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_4",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_5",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_6",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_7",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_8",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_9",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_10",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_11",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_12",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_13",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_14",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_15",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_amoeba_wall_16",
    "mm_amoeba_wall",
    "amoeba wall"
  },
  {
    "mm_wooden_block",
    "mm_wooden_block",
    "wooden block"
  },
  {
    "mm_gray_ball",
    "mm_gray_ball",
    "gray ball"
  },
  {
    "mm_teleporter_1",
    "mm_teleporter",
    "teleporter (0\xb0)"
  },
  {
    "mm_teleporter_2",
    "mm_teleporter",
    "teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_3",
    "mm_teleporter",
    "teleporter (45\xb0)"
  },
  {
    "mm_teleporter_4",
    "mm_teleporter",
    "teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_5",
    "mm_teleporter",
    "teleporter (90\xb0)"
  },
  {
    "mm_teleporter_6",
    "mm_teleporter",
    "teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_7",
    "mm_teleporter",
    "teleporter (135\xb0)"
  },
  {
    "mm_teleporter_8",
    "mm_teleporter",
    "teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_9",
    "mm_teleporter",
    "teleporter (180\xb0)"
  },
  {
    "mm_teleporter_10",
    "mm_teleporter",
    "teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_11",
    "mm_teleporter",
    "teleporter (225\xb0)"
  },
  {
    "mm_teleporter_12",
    "mm_teleporter",
    "teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_13",
    "mm_teleporter",
    "teleporter (270\xb0)"
  },
  {
    "mm_teleporter_14",
    "mm_teleporter",
    "teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_15",
    "mm_teleporter",
    "teleporter (315\xb0)"
  },
  {
    "mm_teleporter_16",
    "mm_teleporter",
    "teleporter (337.5\xb0)"
  },
  {
    "mm_fuse.active",
    "mm_fuse",
    "fuse (on)"
  },
  {
    "mm_pacman.right",
    "mm_pacman",
    "pac man (starts moving right)"
  },
  {
    "mm_pacman.up",
    "mm_pacman",
    "pac man (starts moving up)"
  },
  {
    "mm_pacman.left",
    "mm_pacman",
    "pac man (starts moving left)"
  },
  {
    "mm_pacman.down",
    "mm_pacman",
    "pac man (starts moving down)"
  },
  {
    "mm_polarizer_1",
    "mm_polarizer",
    "polarizer (0\xb0)"
  },
  {
    "mm_polarizer_2",
    "mm_polarizer",
    "polarizer (11.25\xb0)"
  },
  {
    "mm_polarizer_3",
    "mm_polarizer",
    "polarizer (22.5\xb0)"
  },
  {
    "mm_polarizer_4",
    "mm_polarizer",
    "polarizer (33.75\xb0)"
  },
  {
    "mm_polarizer_5",
    "mm_polarizer",
    "polarizer (45\xb0)"
  },
  {
    "mm_polarizer_6",
    "mm_polarizer",
    "polarizer (56.25\xb0)"
  },
  {
    "mm_polarizer_7",
    "mm_polarizer",
    "polarizer (67.5\xb0)"
  },
  {
    "mm_polarizer_8",
    "mm_polarizer",
    "polarizer (78.75\xb0)"
  },
  {
    "mm_polarizer_9",
    "mm_polarizer",
    "polarizer (90\xb0)"
  },
  {
    "mm_polarizer_10",
    "mm_polarizer",
    "polarizer (101.25\xb0)"
  },
  {
    "mm_polarizer_11",
    "mm_polarizer",
    "polarizer (112.5\xb0)"
  },
  {
    "mm_polarizer_12",
    "mm_polarizer",
    "polarizer (123.75\xb0)"
  },
  {
    "mm_polarizer_13",
    "mm_polarizer",
    "polarizer (135\xb0)"
  },
  {
    "mm_polarizer_14",
    "mm_polarizer",
    "polarizer (146.25\xb0)"
  },
  {
    "mm_polarizer_15",
    "mm_polarizer",
    "polarizer (157.5\xb0)"
  },
  {
    "mm_polarizer_16",
    "mm_polarizer",
    "polarizer (168.75\xb0)"
  },
  {
    "mm_polarizer_cross_1",
    "mm_polarizer_cross",
    "two-way polarizer (0\xb0)"
  },
  {
    "mm_polarizer_cross_2",
    "mm_polarizer_cross",
    "two-way polarizer (22.5\xb0)"
  },
  {
    "mm_polarizer_cross_3",
    "mm_polarizer_cross",
    "two-way polarizer (45\xb0)"
  },
  {
    "mm_polarizer_cross_4",
    "mm_polarizer_cross",
    "two-way polarizer (67.5\xb0)"
  },
  {
    "mm_mirror_fixed_1",
    "mm_mirror_fixed",
    "fixed mirror (0\xb0)"
  },
  {
    "mm_mirror_fixed_2",
    "mm_mirror_fixed",
    "fixed mirror (0\xb0)"
  },
  {
    "mm_mirror_fixed_3",
    "mm_mirror_fixed",
    "fixed mirror (0\xb0)"
  },
  {
    "mm_mirror_fixed_4",
    "mm_mirror_fixed",
    "fixed mirror (0\xb0)"
  },
  {
    "mm_steel_lock",
    "mm_steel_lock",
    "steel lock"
  },
  {
    "mm_key",
    "mm_key",
    "key (MM style)"
  },
  {
    "mm_lightbulb",
    "mm_lightbulb",
    "light bulb (off)"
  },
  {
    "mm_lightbulb.active",
    "mm_lightbulb",
    "light bulb (on)"
  },
  {
    "mm_lightball",
    "mm_lightball",
    "bonus ball"
  },
  {
    "mm_steel_block",
    "mm_steel_block",
    "steel block"
  },
  {
    "mm_wooden_lock",
    "mm_wooden_lock",
    "wooden lock"
  },
  {
    "mm_fuel_full",
    "mm_fuel",
    "extra energy ball (full)"
  },
  {
    "mm_wooden_grid_fixed_1",
    "mm_wooden_grid_fixed",
    "fixed wooden polarizer (0\xb0)"
  },
  {
    "mm_wooden_grid_fixed_2",
    "mm_wooden_grid_fixed",
    "fixed wooden polarizer (90\xb0)"
  },
  {
    "mm_wooden_grid_fixed_3",
    "mm_wooden_grid_fixed",
    "fixed wooden polarizer (45\xb0)"
  },
  {
    "mm_wooden_grid_fixed_4",
    "mm_wooden_grid_fixed",
    "fixed wooden polarizer (135\xb0)"
  },
  {
    "mm_fuel_empty",
    "mm_fuel",
    "extra energy ball (empty)"
  },
  {
    "mm_unused_156",
    "unused",
    "(not used)"
  },
  {
    "mm_unused_157",
    "unused",
    "(not used)"
  },
  {
    "mm_unused_158",
    "unused",
    "(not used)"
  },
  {
    "mm_unused_159",
    "unused",
    "(not used)"
  },
  {
    "df_mirror_1",
    "df_mirror",
    "mirror (DF style) (0\xb0)"
  },
  {
    "df_mirror_2",
    "df_mirror",
    "mirror (DF style) (11.25\xb0)"
  },
  {
    "df_mirror_3",
    "df_mirror",
    "mirror (DF style) (22.5\xb0)"
  },
  {
    "df_mirror_4",
    "df_mirror",
    "mirror (DF style) (33.75\xb0)"
  },
  {
    "df_mirror_5",
    "df_mirror",
    "mirror (DF style) (45\xb0)"
  },
  {
    "df_mirror_6",
    "df_mirror",
    "mirror (DF style) (56.25\xb0)"
  },
  {
    "df_mirror_7",
    "df_mirror",
    "mirror (DF style) (67.5\xb0)"
  },
  {
    "df_mirror_8",
    "df_mirror",
    "mirror (DF style) (78.75\xb0)"
  },
  {
    "df_mirror_9",
    "df_mirror",
    "mirror (DF style) (90\xb0)"
  },
  {
    "df_mirror_10",
    "df_mirror",
    "mirror (DF style) (101.25\xb0)"
  },
  {
    "df_mirror_11",
    "df_mirror",
    "mirror (DF style) (112.5\xb0)"
  },
  {
    "df_mirror_12",
    "df_mirror",
    "mirror (DF style) (123.75\xb0)"
  },
  {
    "df_mirror_13",
    "df_mirror",
    "mirror (DF style) (135\xb0)"
  },
  {
    "df_mirror_14",
    "df_mirror",
    "mirror (DF style) (146.25\xb0)"
  },
  {
    "df_mirror_15",
    "df_mirror",
    "mirror (DF style) (157.5\xb0)"
  },
  {
    "df_mirror_16",
    "df_mirror",
    "mirror (DF style) (168.75\xb0)"
  },
  {
    "df_wooden_grid_fixed_1",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (0\xb0)"
  },
  {
    "df_wooden_grid_fixed_2",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (22.5\xb0)"
  },
  {
    "df_wooden_grid_fixed_3",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (45\xb0)"
  },
  {
    "df_wooden_grid_fixed_4",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (67.5\xb0)"
  },
  {
    "df_wooden_grid_fixed_5",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (90\xb0)"
  },
  {
    "df_wooden_grid_fixed_6",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (112.5\xb0)"
  },
  {
    "df_wooden_grid_fixed_7",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (135\xb0)"
  },
  {
    "df_wooden_grid_fixed_8",
    "df_wooden_grid_fixed",
    "fixed wooden polarizer (157.5\xb0)"
  },
  {
    "df_steel_grid_fixed_1",
    "df_steel_grid_fixed",
    "fixed steel polarizer (0\xb0)"
  },
  {
    "df_steel_grid_fixed_2",
    "df_steel_grid_fixed",
    "fixed steel polarizer (22.5\xb0)"
  },
  {
    "df_steel_grid_fixed_3",
    "df_steel_grid_fixed",
    "fixed steel polarizer (45\xb0)"
  },
  {
    "df_steel_grid_fixed_4",
    "df_steel_grid_fixed",
    "fixed steel polarizer (67.5\xb0)"
  },
  {
    "df_steel_grid_fixed_5",
    "df_steel_grid_fixed",
    "fixed steel polarizer (90\xb0)"
  },
  {
    "df_steel_grid_fixed_6",
    "df_steel_grid_fixed",
    "fixed steel polarizer (112.5\xb0)"
  },
  {
    "df_steel_grid_fixed_7",
    "df_steel_grid_fixed",
    "fixed steel polarizer (135\xb0)"
  },
  {
    "df_steel_grid_fixed_8",
    "df_steel_grid_fixed",
    "fixed steel polarizer (157.5\xb0)"
  },
  {
    "df_wooden_wall_1",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_2",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_3",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_4",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_5",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_6",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_7",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_8",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_9",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_10",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_11",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_12",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_13",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_14",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_15",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_wooden_wall_16",
    "df_wooden_wall",
    "wooden wall (DF style)"
  },
  {
    "df_steel_wall_1",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_2",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_3",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_4",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_5",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_6",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_7",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_8",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_9",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_10",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_11",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_12",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_13",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_14",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_15",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_steel_wall_16",
    "df_steel_wall",
    "steel wall (DF style)"
  },
  {
    "df_empty_space",
    "empty_space",
    "empty space"
  },
  {
    "df_cell",
    "df_cell",
    "cell"
  },
  {
    "df_mine",
    "df_mine",
    "mine"
  },
  {
    "df_refractor",
    "df_refractor",
    "refractor"
  },
  {
    "df_laser.right",
    "df_laser",
    "laser cannon (shooting right)"
  },
  {
    "df_laser.up",
    "df_laser",
    "laser cannon (shooting up)"
  },
  {
    "df_laser.left",
    "df_laser",
    "laser cannon (shooting left)"
  },
  {
    "df_laser.down",
    "df_laser",
    "laser cannon (shooting down)"
  },
  {
    "df_receiver.right",
    "df_receiver",
    "laser receiver (directed right)"
  },
  {
    "df_receiver.up",
    "df_receiver",
    "laser receiver (directed up)"
  },
  {
    "df_receiver.left",
    "df_receiver",
    "laser receiver (directed left)"
  },
  {
    "df_receiver.down",
    "df_receiver",
    "laser receiver (directed down)"
  },
  {
    "df_fibre_optic_red_1",
    "df_fibre_optic",
    "red fibre optic (part 1)"
  },
  {
    "df_fibre_optic_red_2",
    "df_fibre_optic",
    "red fibre optic (part 2)"
  },
  {
    "df_fibre_optic_yellow_1",
    "df_fibre_optic",
    "yellow fibre optic (part 1)"
  },
  {
    "df_fibre_optic_yellow_2",
    "df_fibre_optic",
    "yellow fibre optic (part 2)"
  },
  {
    "df_fibre_optic_green_1",
    "df_fibre_optic",
    "green fibre optic (part 1)"
  },
  {
    "df_fibre_optic_green_2",
    "df_fibre_optic",
    "green fibre optic (part 2)"
  },
  {
    "df_fibre_optic_blue_1",
    "df_fibre_optic",
    "blue fibre optic (part 1)"
  },
  {
    "df_fibre_optic_blue_2",
    "df_fibre_optic",
    "blue fibre optic (part 2)"
  },
  {
    "df_mirror_rotating_1",
    "df_mirror_rotating",
    "rotating mirror (0\xb0)"
  },
  {
    "df_mirror_rotating_2",
    "df_mirror_rotating",
    "rotating mirror (11.25\xb0)"
  },
  {
    "df_mirror_rotating_3",
    "df_mirror_rotating",
    "rotating mirror (22.5\xb0)"
  },
  {
    "df_mirror_rotating_4",
    "df_mirror_rotating",
    "rotating mirror (33.75\xb0)"
  },
  {
    "df_mirror_rotating_5",
    "df_mirror_rotating",
    "rotating mirror (45\xb0)"
  },
  {
    "df_mirror_rotating_6",
    "df_mirror_rotating",
    "rotating mirror (56.25\xb0)"
  },
  {
    "df_mirror_rotating_7",
    "df_mirror_rotating",
    "rotating mirror (67.5\xb0)"
  },
  {
    "df_mirror_rotating_8",
    "df_mirror_rotating",
    "rotating mirror (78.75\xb0)"
  },
  {
    "df_mirror_rotating_9",
    "df_mirror_rotating",
    "rotating mirror (90\xb0)"
  },
  {
    "df_mirror_rotating_10",
    "df_mirror_rotating",
    "rotating mirror (101.25\xb0)"
  },
  {
    "df_mirror_rotating_11",
    "df_mirror_rotating",
    "rotating mirror (112.5\xb0)"
  },
  {
    "df_mirror_rotating_12",
    "df_mirror_rotating",
    "rotating mirror (123.75\xb0)"
  },
  {
    "df_mirror_rotating_13",
    "df_mirror_rotating",
    "rotating mirror (135\xb0)"
  },
  {
    "df_mirror_rotating_14",
    "df_mirror_rotating",
    "rotating mirror (146.25\xb0)"
  },
  {
    "df_mirror_rotating_15",
    "df_mirror_rotating",
    "rotating mirror (157.5\xb0)"
  },
  {
    "df_mirror_rotating_16",
    "df_mirror_rotating",
    "rotating mirror (168.75\xb0)"
  },
  {
    "df_wooden_grid_rotating_1",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (0\xb0)"
  },
  {
    "df_wooden_grid_rotating_2",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (22.5\xb0)"
  },
  {
    "df_wooden_grid_rotating_3",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (45\xb0)"
  },
  {
    "df_wooden_grid_rotating_4",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (67.5\xb0)"
  },
  {
    "df_wooden_grid_rotating_5",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (90\xb0)"
  },
  {
    "df_wooden_grid_rotating_6",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (112.5\xb0)"
  },
  {
    "df_wooden_grid_rotating_7",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (135\xb0)"
  },
  {
    "df_wooden_grid_rotating_8",
    "df_wooden_grid_rotating",
    "rotating wooden polarizer (157.5\xb0)"
  },
  {
    "df_steel_grid_rotating_1",
    "df_steel_grid_rotating",
    "rotating steel polarizer (0\xb0)"
  },
  {
    "df_steel_grid_rotating_2",
    "df_steel_grid_rotating",
    "rotating steel polarizer (22.5\xb0)"
  },
  {
    "df_steel_grid_rotating_3",
    "df_steel_grid_rotating",
    "rotating steel polarizer (45\xb0)"
  },
  {
    "df_steel_grid_rotating_4",
    "df_steel_grid_rotating",
    "rotating steel polarizer (67.5\xb0)"
  },
  {
    "df_steel_grid_rotating_5",
    "df_steel_grid_rotating",
    "rotating steel polarizer (90\xb0)"
  },
  {
    "df_steel_grid_rotating_6",
    "df_steel_grid_rotating",
    "rotating steel polarizer (112.5\xb0)"
  },
  {
    "df_steel_grid_rotating_7",
    "df_steel_grid_rotating",
    "rotating steel polarizer (135\xb0)"
  },
  {
    "df_steel_grid_rotating_8",
    "df_steel_grid_rotating",
    "rotating steel polarizer (157.5\xb0)"
  },
  {
    "mm_teleporter_red_1",
    "mm_teleporter",
    "red teleporter (0\xb0)"
  },
  {
    "mm_teleporter_red_2",
    "mm_teleporter",
    "red teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_red_3",
    "mm_teleporter",
    "red teleporter (45\xb0)"
  },
  {
    "mm_teleporter_red_4",
    "mm_teleporter",
    "red teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_red_5",
    "mm_teleporter",
    "red teleporter (90\xb0)"
  },
  {
    "mm_teleporter_red_6",
    "mm_teleporter",
    "red teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_red_7",
    "mm_teleporter",
    "red teleporter (135\xb0)"
  },
  {
    "mm_teleporter_red_8",
    "mm_teleporter",
    "red teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_red_9",
    "mm_teleporter",
    "red teleporter (180\xb0)"
  },
  {
    "mm_teleporter_red_10",
    "mm_teleporter",
    "red teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_red_11",
    "mm_teleporter",
    "red teleporter (225\xb0)"
  },
  {
    "mm_teleporter_red_12",
    "mm_teleporter",
    "red teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_red_13",
    "mm_teleporter",
    "red teleporter (270\xb0)"
  },
  {
    "mm_teleporter_red_14",
    "mm_teleporter",
    "red teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_red_15",
    "mm_teleporter",
    "red teleporter (315\xb0)"
  },
  {
    "mm_teleporter_red_16",
    "mm_teleporter",
    "red teleporter (337.5\xb0)"
  },
  {
    "mm_teleporter_yellow_1",
    "mm_teleporter",
    "yellow teleporter (0\xb0)"
  },
  {
    "mm_teleporter_yellow_2",
    "mm_teleporter",
    "yellow teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_yellow_3",
    "mm_teleporter",
    "yellow teleporter (45\xb0)"
  },
  {
    "mm_teleporter_yellow_4",
    "mm_teleporter",
    "yellow teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_yellow_5",
    "mm_teleporter",
    "yellow teleporter (90\xb0)"
  },
  {
    "mm_teleporter_yellow_6",
    "mm_teleporter",
    "yellow teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_yellow_7",
    "mm_teleporter",
    "yellow teleporter (135\xb0)"
  },
  {
    "mm_teleporter_yellow_8",
    "mm_teleporter",
    "yellow teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_yellow_9",
    "mm_teleporter",
    "yellow teleporter (180\xb0)"
  },
  {
    "mm_teleporter_yellow_10",
    "mm_teleporter",
    "yellow teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_yellow_11",
    "mm_teleporter",
    "yellow teleporter (225\xb0)"
  },
  {
    "mm_teleporter_yellow_12",
    "mm_teleporter",
    "yellow teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_yellow_13",
    "mm_teleporter",
    "yellow teleporter (270\xb0)"
  },
  {
    "mm_teleporter_yellow_14",
    "mm_teleporter",
    "yellow teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_yellow_15",
    "mm_teleporter",
    "yellow teleporter (315\xb0)"
  },
  {
    "mm_teleporter_yellow_16",
    "mm_teleporter",
    "yellow teleporter (337.5\xb0)"
  },
  {
    "mm_teleporter_green_1",
    "mm_teleporter",
    "green teleporter (0\xb0)"
  },
  {
    "mm_teleporter_green_2",
    "mm_teleporter",
    "green teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_green_3",
    "mm_teleporter",
    "green teleporter (45\xb0)"
  },
  {
    "mm_teleporter_green_4",
    "mm_teleporter",
    "green teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_green_5",
    "mm_teleporter",
    "green teleporter (90\xb0)"
  },
  {
    "mm_teleporter_green_6",
    "mm_teleporter",
    "green teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_green_7",
    "mm_teleporter",
    "green teleporter (135\xb0)"
  },
  {
    "mm_teleporter_green_8",
    "mm_teleporter",
    "green teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_green_9",
    "mm_teleporter",
    "green teleporter (180\xb0)"
  },
  {
    "mm_teleporter_green_10",
    "mm_teleporter",
    "green teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_green_11",
    "mm_teleporter",
    "green teleporter (225\xb0)"
  },
  {
    "mm_teleporter_green_12",
    "mm_teleporter",
    "green teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_green_13",
    "mm_teleporter",
    "green teleporter (270\xb0)"
  },
  {
    "mm_teleporter_green_14",
    "mm_teleporter",
    "green teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_green_15",
    "mm_teleporter",
    "green teleporter (315\xb0)"
  },
  {
    "mm_teleporter_green_16",
    "mm_teleporter",
    "green teleporter (337.5\xb0)"
  },
  {
    "mm_teleporter_blue_1",
    "mm_teleporter",
    "blue teleporter (0\xb0)"
  },
  {
    "mm_teleporter_blue_2",
    "mm_teleporter",
    "blue teleporter (22.5\xb0)"
  },
  {
    "mm_teleporter_blue_3",
    "mm_teleporter",
    "blue teleporter (45\xb0)"
  },
  {
    "mm_teleporter_blue_4",
    "mm_teleporter",
    "blue teleporter (67.5\xb0)"
  },
  {
    "mm_teleporter_blue_5",
    "mm_teleporter",
    "blue teleporter (90\xb0)"
  },
  {
    "mm_teleporter_blue_6",
    "mm_teleporter",
    "blue teleporter (112.5\xb0)"
  },
  {
    "mm_teleporter_blue_7",
    "mm_teleporter",
    "blue teleporter (135\xb0)"
  },
  {
    "mm_teleporter_blue_8",
    "mm_teleporter",
    "blue teleporter (157.5\xb0)"
  },
  {
    "mm_teleporter_blue_9",
    "mm_teleporter",
    "blue teleporter (180\xb0)"
  },
  {
    "mm_teleporter_blue_10",
    "mm_teleporter",
    "blue teleporter (202.5\xb0)"
  },
  {
    "mm_teleporter_blue_11",
    "mm_teleporter",
    "blue teleporter (225\xb0)"
  },
  {
    "mm_teleporter_blue_12",
    "mm_teleporter",
    "blue teleporter (247.5\xb0)"
  },
  {
    "mm_teleporter_blue_13",
    "mm_teleporter",
    "blue teleporter (270\xb0)"
  },
  {
    "mm_teleporter_blue_14",
    "mm_teleporter",
    "blue teleporter (292.5\xb0)"
  },
  {
    "mm_teleporter_blue_15",
    "mm_teleporter",
    "blue teleporter (315\xb0)"
  },
  {
    "mm_teleporter_blue_16",
    "mm_teleporter",
    "blue teleporter (337.5\xb0)"
  },
  {
    "mm_mcduffin",
    "mm_mcduffin",
    "Gregor McDuffin"
  },
  {
    "mm_pacman",
    "mm_pacman",
    "pac man (MM style)"
  },
  {
    "mm_fuse",
    "mm_fuse",
    "fuse (off)",
  },
  {
    "mm_steel_wall",
    "mm_steel_wall",
    "steel wall",
  },
  {
    "mm_wooden_wall",
    "mm_wooden_wall",
    "wooden wall",
  },
  {
    "mm_ice_wall",
    "mm_ice_wall",
    "ice wall",
  },
  {
    "mm_amoeba_wall",
    "mm_amoeba_wall",
    "amoeba wall",
  },
  {
    "df_laser",
    "df_laser",
    "laser cannon"
  },
  {
    "df_receiver",
    "df_receiver",
    "laser receiver"
  },
  {
    "df_steel_wall",
    "df_steel_wall",
    "steel wall (DF style)",
  },
  {
    "df_wooden_wall",
    "df_wooden_wall",
    "wooden wall (DF style)",
  },

  /* ----------------------------------------------------------------------- */
  /* "real" (and therefore drawable) runtime elements                        */
  /* ----------------------------------------------------------------------- */

  {
    "dynabomb_player_1.active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_2.active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_3.active",
    "dynabomb",
    "-"
  },
  {
    "dynabomb_player_4.active",
    "dynabomb",
    "-"
  },
  {
    "sp_disk_red.active",
    "dynamite",
    "-"
  },
  {
    "switchgate.opening",
    "switchgate",
    "-"
  },
  {
    "switchgate.closing",
    "switchgate",
    "-"
  },
  {
    "timegate.opening",
    "timegate",
    "-"
  },
  {
    "timegate.closing",
    "timegate",
    "-"
  },
  {
    "pearl.breaking",
    "pearl",
    "-"
  },
  {
    "trap.active",
    "trap",
    "-"
  },
  {
    "invisible_steelwall.active",
    "steelwall",
    "-"
  },
  {
    "invisible_wall.active",
    "wall",
    "-"
  },
  {
    "invisible_sand.active",
    "sand",
    "-"
  },
  {
    "conveyor_belt_1_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_1_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_1_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_2_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_3_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_left.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_middle.active",
    "conveyor_belt",
    "-"
  },
  {
    "conveyor_belt_4_right.active",
    "conveyor_belt",
    "-"
  },
  {
    "exit.opening",
    "exit",
    "-"
  },
  {
    "exit.closing",
    "exit",
    "-"
  },
  {
    "steel_exit.opening",
    "steel_exit",
    "-"
  },
  {
    "steel_exit.closing",
    "steel_exit",
    "-"
  },
  {
    "em_exit.opening",
    "em_exit",
    "-"
  },
  {
    "em_exit.closing",
    "em_exit",
    "-"
  },
  {
    "em_steel_exit.opening",
    "em_steel_exit",
    "-"
  },
  {
    "em_steel_exit.closing",
    "em_steel_exit",
    "-"
  },
  {
    "sp_exit.opening",
    "sp_exit",
    "-"
  },
  {
    "sp_exit.closing",
    "sp_exit",
    "-"
  },
  {
    "sp_exit_open",
    "sp_exit",
    "-"
  },
  {
    "sp_terminal.active",
    "sp_terminal",
    "-"
  },
  {
    "sp_buggy_base.activating",
    "sp_buggy_base",
    "-"
  },
  {
    "sp_buggy_base.active",
    "sp_buggy_base",
    "-"
  },
  {
    "sp_murphy_clone",
    "murphy_clone",
    "-"
  },
  {
    "amoeba.dropping",
    "amoeba",
    "-"
  },
  {
    "quicksand.emptying",
    "quicksand",
    "-"
  },
  {
    "quicksand_fast.emptying",
    "quicksand",
    "-"
  },
  {
    "magic_wall.active",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall.active",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall.active",
    "magic_wall",
    "-"
  },
  {
    "magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall_full",
    "magic_wall",
    "-"
  },
  {
    "magic_wall.emptying",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall.emptying",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall.emptying",
    "magic_wall",
    "-"
  },
  {
    "magic_wall_dead",
    "magic_wall",
    "-"
  },
  {
    "bd_magic_wall_dead",
    "magic_wall",
    "-"
  },
  {
    "dc_magic_wall_dead",
    "magic_wall",
    "-"
  },

  {
    "emc_fake_grass.active",
    "fake_grass",
    "-"
  },
  {
    "gate_1_gray.active",
    "gate",
    ""
  },
  {
    "gate_2_gray.active",
    "gate",
    ""
  },
  {
    "gate_3_gray.active",
    "gate",
    ""
  },
  {
    "gate_4_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_1_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_2_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_3_gray.active",
    "gate",
    ""
  },
  {
    "em_gate_4_gray.active",
    "gate",
    ""
  },
  {
    "emc_gate_5_gray.active",
    "gate",
    "",
  },
  {
    "emc_gate_6_gray.active",
    "gate",
    "",
  },
  {
    "emc_gate_7_gray.active",
    "gate",
    "",
  },
  {
    "emc_gate_8_gray.active",
    "gate",
    "",
  },
  {
    "dc_gate_white_gray.active",
    "gate",
    "",
  },
  {
    "emc_dripper.active",
    "dripper",
    "dripper"
  },
  {
    "emc_spring_bumper.active",
    "emc_spring_bumper",
    "spring bumper",
  },
  {
    "mm_exit.opening",
    "mm_exit",
    "-"
  },
  {
    "mm_exit.closing",
    "mm_exit",
    "-"
  },
  {
    "mm_gray_ball.opening",
    "mm_gray_ball",
    "-",
  },
  {
    "mm_ice_wall.shrinking",
    "mm_ice_wall",
    "-",
  },
  {
    "mm_amoeba_wall.growing",
    "mm_amoeba_wall",
    "-",
  },
  {
    "mm_pacman.eating.right",
    "mm_pacman",
    "pac man (eating right)"
  },
  {
    "mm_pacman.eating.up",
    "mm_pacman",
    "pac man (eating up)"
  },
  {
    "mm_pacman.eating.left",
    "mm_pacman",
    "pac man (eating left)"
  },
  {
    "mm_pacman.eating.down",
    "mm_pacman",
    "pac man (eating down)"
  },

  /* ----------------------------------------------------------------------- */
  /* "unreal" (and therefore not drawable) runtime elements                  */
  /* ----------------------------------------------------------------------- */

  {
    "blocked",
    "-",
    "-"
  },
  {
    "explosion",
    "-",
    "-"
  },
  {
    "nut.breaking",
    "-",
    "-"
  },
  {
    "diamond.breaking",
    "-",
    "-"
  },
  {
    "acid_splash_left",
    "-",
    "-"
  },
  {
    "acid_splash_right",
    "-",
    "-"
  },
  {
    "amoeba.growing",
    "-",
    "-"
  },
  {
    "amoeba.shrinking",
    "-",
    "-"
  },
  {
    "expandable_wall.growing",
    "-",
    "-"
  },
  {
    "expandable_steelwall.growing",
    "-",
    "-"
  },
  {
    "flames",
    "-",
    "-"
  },
  {
    "player_is_leaving",
    "-",
    "-"
  },
  {
    "player_is_exploding_1",
    "-",
    "-"
  },
  {
    "player_is_exploding_2",
    "-",
    "-"
  },
  {
    "player_is_exploding_3",
    "-",
    "-"
  },
  {
    "player_is_exploding_4",
    "-",
    "-"
  },
  {
    "quicksand.filling",
    "quicksand",
    "-"
  },
  {
    "quicksand_fast.filling",
    "quicksand",
    "-"
  },
  {
    "magic_wall.filling",
    "-",
    "-"
  },
  {
    "bd_magic_wall.filling",
    "-",
    "-"
  },
  {
    "dc_magic_wall.filling",
    "-",
    "-"
  },
  {
    "element.snapping",
    "-",
    "-"
  },
  {
    "diagonal.shrinking",
    "-",
    "-"
  },
  {
    "diagonal.growing",
    "-",
    "-"
  },

  /* ----------------------------------------------------------------------- */
  /* dummy elements (never used as game elements, only used as graphics)     */
  /* ----------------------------------------------------------------------- */

  {
    "steelwall_topleft",
    "-",
    "-"
  },
  {
    "steelwall_topright",
    "-",
    "-"
  },
  {
    "steelwall_bottomleft",
    "-",
    "-"
  },
  {
    "steelwall_bottomright",
    "-",
    "-"
  },
  {
    "steelwall_horizontal",
    "-",
    "-"
  },
  {
    "steelwall_vertical",
    "-",
    "-"
  },
  {
    "invisible_steelwall_topleft",
    "-",
    "-"
  },
  {
    "invisible_steelwall_topright",
    "-",
    "-"
  },
  {
    "invisible_steelwall_bottomleft",
    "-",
    "-"
  },
  {
    "invisible_steelwall_bottomright",
    "-",
    "-"
  },
  {
    "invisible_steelwall_horizontal",
    "-",
    "-"
  },
  {
    "invisible_steelwall_vertical",
    "-",
    "-"
  },
  {
    "dynabomb",
    "-",
    "-"
  },
  {
    "dynabomb.active",
    "-",
    "-"
  },
  {
    "dynabomb_player_1",
    "-",
    "-"
  },
  {
    "dynabomb_player_2",
    "-",
    "-"
  },
  {
    "dynabomb_player_3",
    "-",
    "-"
  },
  {
    "dynabomb_player_4",
    "-",
    "-"
  },
  {
    "shield_normal.active",
    "-",
    "-"
  },
  {
    "shield_deadly.active",
    "-",
    "-"
  },
  {
    "amoeba",
    "amoeba",
    "-"
  },
  {
    "mm_lightball_red",
    "-",
    "-"
  },
  {
    "mm_lightball_blue",
    "-",
    "-"
  },
  {
    "mm_lightball_yellow",
    "-",
    "-"
  },
  {
    "mm_mask_mcduffin.right",
    "-",
    "-"
  },
  {
    "mm_mask_mcduffin.up",
    "-",
    "-"
  },
  {
    "mm_mask_mcduffin.left",
    "-",
    "-"
  },
  {
    "mm_mask_mcduffin.down",
    "-",
    "-"
  },
  {
    "mm_mask_grid_1",
    "-",
    "-"
  },
  {
    "mm_mask_grid_2",
    "-",
    "-"
  },
  {
    "mm_mask_grid_3",
    "-",
    "-"
  },
  {
    "mm_mask_grid_4",
    "-",
    "-"
  },
  {
    "mm_mask_rectangle",
    "-",
    "-"
  },
  {
    "mm_mask_circle",
    "-",
    "-"
  },
  {
    "[default]",
    "default",
    "-"
  },
  {
    "[bd_default]",
    "bd_default",
    "-"
  },
  {
    "[sp_default]",
    "sp_default",
    "-"
  },
  {
    "[sb_default]",
    "sb_default",
    "-"
  },
  {
    "[mm_default]",
    "mm_default",
    "-"
  },
  {
    "graphic_1",
    "graphic",
    "-"
  },
  {
    "graphic_2",
    "graphic",
    "-"
  },
  {
    "graphic_3",
    "graphic",
    "-"
  },
  {
    "graphic_4",
    "graphic",
    "-"
  },
  {
    "graphic_5",
    "graphic",
    "-"
  },
  {
    "graphic_6",
    "graphic",
    "-"
  },
  {
    "graphic_7",
    "graphic",
    "-"
  },
  {
    "graphic_8",
    "graphic",
    "-"
  },
  {
    "internal_clipboard_custom",
    "internal",
    "empty custom element"
  },
  {
    "internal_clipboard_change",
    "internal",
    "empty change page"
  },
  {
    "internal_clipboard_group",
    "internal",
    "empty group element"
  },
  {
    "internal_dummy",
    "internal",
    "-"
  },
  {
    "internal_cascade_bd",
    "internal",
    "show Boulder Dash elements"
  },
  {
    "internal_cascade_bd.active",
    "internal",
    "hide Boulder Dash elements"
  },
  {
    "internal_cascade_em",
    "internal",
    "show Emerald Mine elements"
  },
  {
    "internal_cascade_em.active",
    "internal",
    "hide Emerald Mine elements"
  },
  {
    "internal_cascade_emc",
    "internal",
    "show Emerald Mine Club elements"
  },
  {
    "internal_cascade_emc.active",
    "internal",
    "hide Emerald Mine Club elements"
  },
  {
    "internal_cascade_rnd",
    "internal",
    "show Rocks'n'Diamonds elements"
  },
  {
    "internal_cascade_rnd.active",
    "internal",
    "hide Rocks'n'Diamonds elements"
  },
  {
    "internal_cascade_sb",
    "internal",
    "show Sokoban elements"
  },
  {
    "internal_cascade_sb.active",
    "internal",
    "hide Sokoban elements"
  },
  {
    "internal_cascade_sp",
    "internal",
    "show Supaplex elements"
  },
  {
    "internal_cascade_sp.active",
    "internal",
    "hide Supaplex elements"
  },
  {
    "internal_cascade_dc",
    "internal",
    "show Diamond Caves II elements"
  },
  {
    "internal_cascade_dc.active",
    "internal",
    "hide Diamond Caves II elements"
  },
  {
    "internal_cascade_dx",
    "internal",
    "show DX Boulderdash elements"
  },
  {
    "internal_cascade_dx.active",
    "internal",
    "hide DX Boulderdash elements"
  },
  {
    "internal_cascade_mm",
    "internal",
    "show Mirror Magic elements"
  },
  {
    "internal_cascade_mm.active",
    "internal",
    "hide Mirror Magic elements"
  },
  {
    "internal_cascade_df",
    "internal",
    "show Deflektor elements"
  },
  {
    "internal_cascade_df.active",
    "internal",
    "hide Deflektor elements"
  },
  {
    "internal_cascade_chars",
    "internal",
    "show text elements"
  },
  {
    "internal_cascade_chars.active",
    "internal",
    "hide text elements"
  },
  {
    "internal_cascade_steel_chars",
    "internal",
    "show steel text elements"
  },
  {
    "internal_cascade_steel_chars.active",
    "internal",
    "hide steel text elements"
  },
  {
    "internal_cascade_ce",
    "internal",
    "show custom elements"
  },
  {
    "internal_cascade_ce.active",
    "internal",
    "hide custom elements"
  },
  {
    "internal_cascade_ge",
    "internal",
    "show group elements"
  },
  {
    "internal_cascade_ge.active",
    "internal",
    "hide group elements"
  },
  {
    "internal_cascade_ref",
    "internal",
    "show reference elements"
  },
  {
    "internal_cascade_ref.active",
    "internal",
    "hide reference elements"
  },
  {
    "internal_cascade_user",
    "internal",
    "show user defined elements"
  },
  {
    "internal_cascade_user.active",
    "internal",
    "hide user defined elements"
  },
  {
    "internal_cascade_dynamic",
    "internal",
    "show elements used in this level"
  },
  {
    "internal_cascade_dynamic.active",
    "internal",
    "hide elements used in this level"
  },

  /* keyword to stop parser: "ELEMENT_INFO_END" <-- do not change! */

  {
    NULL,
    NULL,
    NULL
  }
};


/* ------------------------------------------------------------------------- */
/* element action and direction definitions                                  */
/* ------------------------------------------------------------------------- */

struct ElementActionInfo element_action_info[NUM_ACTIONS + 1 + 1] =
{
  { ".[DEFAULT]",		ACTION_DEFAULT,			TRUE	},
  { ".waiting",			ACTION_WAITING,			TRUE	},
  { ".falling",			ACTION_FALLING,			TRUE	},
  { ".moving",			ACTION_MOVING,			TRUE	},
  { ".digging",			ACTION_DIGGING,			FALSE	},
  { ".snapping",		ACTION_SNAPPING,		FALSE	},
  { ".collecting",		ACTION_COLLECTING,		FALSE	},
  { ".dropping",		ACTION_DROPPING,		FALSE	},
  { ".pushing",			ACTION_PUSHING,			FALSE	},
  { ".walking",			ACTION_WALKING,			FALSE	},
  { ".passing",			ACTION_PASSING,			FALSE	},
  { ".impact",			ACTION_IMPACT,			FALSE	},
  { ".breaking",		ACTION_BREAKING,		FALSE	},
  { ".activating",		ACTION_ACTIVATING,		FALSE	},
  { ".deactivating",		ACTION_DEACTIVATING,		FALSE	},
  { ".opening",			ACTION_OPENING,			FALSE	},
  { ".closing",			ACTION_CLOSING,			FALSE	},
  { ".attacking",		ACTION_ATTACKING,		TRUE	},
  { ".growing",			ACTION_GROWING,			TRUE	},
  { ".shrinking",		ACTION_SHRINKING,		FALSE	},
  { ".active",			ACTION_ACTIVE,			TRUE	},
  { ".filling",			ACTION_FILLING,			FALSE	},
  { ".emptying",		ACTION_EMPTYING,		FALSE	},
  { ".changing",		ACTION_CHANGING,		FALSE	},
  { ".exploding",		ACTION_EXPLODING,		FALSE	},
  { ".boring",			ACTION_BORING,			FALSE	},
  { ".boring[1]",		ACTION_BORING_1,		FALSE	},
  { ".boring[2]",		ACTION_BORING_2,		FALSE	},
  { ".boring[3]",		ACTION_BORING_3,		FALSE	},
  { ".boring[4]",		ACTION_BORING_4,		FALSE	},
  { ".boring[5]",		ACTION_BORING_5,		FALSE	},
  { ".boring[6]",		ACTION_BORING_6,		FALSE	},
  { ".boring[7]",		ACTION_BORING_7,		FALSE	},
  { ".boring[8]",		ACTION_BORING_8,		FALSE	},
  { ".boring[9]",		ACTION_BORING_9,		FALSE	},
  { ".boring[10]",		ACTION_BORING_10,		FALSE	},
  { ".sleeping",		ACTION_SLEEPING,		FALSE	},
  { ".sleeping[1]",		ACTION_SLEEPING_1,		FALSE	},
  { ".sleeping[2]",		ACTION_SLEEPING_2,		FALSE	},
  { ".sleeping[3]",		ACTION_SLEEPING_3,		FALSE	},
  { ".awakening",		ACTION_AWAKENING,		FALSE	},
  { ".dying",			ACTION_DYING,			FALSE	},
  { ".turning",			ACTION_TURNING,			FALSE	},
  { ".turning_from_left",	ACTION_TURNING_FROM_LEFT,	FALSE	},
  { ".turning_from_right",	ACTION_TURNING_FROM_RIGHT,	FALSE	},
  { ".turning_from_up",		ACTION_TURNING_FROM_UP,		FALSE	},
  { ".turning_from_down",	ACTION_TURNING_FROM_DOWN,	FALSE	},
  { ".smashed_by_rock",		ACTION_SMASHED_BY_ROCK,		FALSE	},
  { ".smashed_by_spring",	ACTION_SMASHED_BY_SPRING,	FALSE	},
  { ".eating",			ACTION_EATING,			FALSE	},
  { ".twinkling",		ACTION_TWINKLING,		FALSE	},
  { ".splashing",		ACTION_SPLASHING,		FALSE	},
  { ".hitting",			ACTION_HITTING,			FALSE	},
  { ".page[1]",			ACTION_PAGE_1,			FALSE	},
  { ".page[2]",			ACTION_PAGE_2,			FALSE	},
  { ".page[3]",			ACTION_PAGE_3,			FALSE	},
  { ".page[4]",			ACTION_PAGE_4,			FALSE	},
  { ".page[5]",			ACTION_PAGE_5,			FALSE	},
  { ".page[6]",			ACTION_PAGE_6,			FALSE	},
  { ".page[7]",			ACTION_PAGE_7,			FALSE	},
  { ".page[8]",			ACTION_PAGE_8,			FALSE	},
  { ".page[9]",			ACTION_PAGE_9,			FALSE	},
  { ".page[10]",		ACTION_PAGE_10,			FALSE	},
  { ".page[11]",		ACTION_PAGE_11,			FALSE	},
  { ".page[12]",		ACTION_PAGE_12,			FALSE	},
  { ".page[13]",		ACTION_PAGE_13,			FALSE	},
  { ".page[14]",		ACTION_PAGE_14,			FALSE	},
  { ".page[15]",		ACTION_PAGE_15,			FALSE	},
  { ".page[16]",		ACTION_PAGE_16,			FALSE	},
  { ".page[17]",		ACTION_PAGE_17,			FALSE	},
  { ".page[18]",		ACTION_PAGE_18,			FALSE	},
  { ".page[19]",		ACTION_PAGE_19,			FALSE	},
  { ".page[20]",		ACTION_PAGE_20,			FALSE	},
  { ".page[21]",		ACTION_PAGE_21,			FALSE	},
  { ".page[22]",		ACTION_PAGE_22,			FALSE	},
  { ".page[23]",		ACTION_PAGE_23,			FALSE	},
  { ".page[24]",		ACTION_PAGE_24,			FALSE	},
  { ".page[25]",		ACTION_PAGE_25,			FALSE	},
  { ".page[26]",		ACTION_PAGE_26,			FALSE	},
  { ".page[27]",		ACTION_PAGE_27,			FALSE	},
  { ".page[28]",		ACTION_PAGE_28,			FALSE	},
  { ".page[29]",		ACTION_PAGE_29,			FALSE	},
  { ".page[30]",		ACTION_PAGE_30,			FALSE	},
  { ".page[31]",		ACTION_PAGE_31,			FALSE	},
  { ".page[32]",		ACTION_PAGE_32,			FALSE	},
  { ".part_1",			ACTION_PART_1,			FALSE	},
  { ".part_2",			ACTION_PART_2,			FALSE	},
  { ".part_3",			ACTION_PART_3,			FALSE	},
  { ".part_4",			ACTION_PART_4,			FALSE	},
  { ".part_5",			ACTION_PART_5,			FALSE	},
  { ".part_6",			ACTION_PART_6,			FALSE	},
  { ".part_7",			ACTION_PART_7,			FALSE	},
  { ".part_8",			ACTION_PART_8,			FALSE	},
  { ".part_9",			ACTION_PART_9,			FALSE	},
  { ".part_10",			ACTION_PART_10,			FALSE	},
  { ".part_11",			ACTION_PART_11,			FALSE	},
  { ".part_12",			ACTION_PART_12,			FALSE	},
  { ".part_13",			ACTION_PART_13,			FALSE	},
  { ".part_14",			ACTION_PART_14,			FALSE	},
  { ".part_15",			ACTION_PART_15,			FALSE	},
  { ".part_16",			ACTION_PART_16,			FALSE	},
  { ".part_17",			ACTION_PART_17,			FALSE	},
  { ".part_18",			ACTION_PART_18,			FALSE	},
  { ".part_19",			ACTION_PART_19,			FALSE	},
  { ".part_20",			ACTION_PART_20,			FALSE	},
  { ".part_21",			ACTION_PART_21,			FALSE	},
  { ".part_22",			ACTION_PART_22,			FALSE	},
  { ".part_23",			ACTION_PART_23,			FALSE	},
  { ".part_24",			ACTION_PART_24,			FALSE	},
  { ".part_25",			ACTION_PART_25,			FALSE	},
  { ".part_26",			ACTION_PART_26,			FALSE	},
  { ".part_27",			ACTION_PART_27,			FALSE	},
  { ".part_28",			ACTION_PART_28,			FALSE	},
  { ".part_29",			ACTION_PART_29,			FALSE	},
  { ".part_30",			ACTION_PART_30,			FALSE	},
  { ".part_31",			ACTION_PART_31,			FALSE	},
  { ".part_32",			ACTION_PART_32,			FALSE	},
  { ".other",			ACTION_OTHER,			FALSE	},

  /* empty suffix always matches -- check as last entry in InitSoundInfo() */
  { "",				ACTION_DEFAULT,			TRUE	},

  { NULL,			0,				0	}
};

struct ElementDirectionInfo element_direction_info[NUM_DIRECTIONS_FULL + 1] =
{
  { ".left",			MV_BIT_LEFT				},
  { ".right",			MV_BIT_RIGHT				},
  { ".up",			MV_BIT_UP				},
  { ".down",			MV_BIT_DOWN				},
  { ".upleft",			MV_BIT_UP				},
  { ".upright",			MV_BIT_RIGHT				},
  { ".downleft",		MV_BIT_LEFT				},
  { ".downright",		MV_BIT_DOWN				},

  { NULL,			0					}
};

struct SpecialSuffixInfo special_suffix_info[NUM_SPECIAL_GFX_ARGS + 1 + 1] =
{
  { ".[DEFAULT]",		GFX_SPECIAL_ARG_DEFAULT,		},
  { ".LOADING",			GFX_SPECIAL_ARG_LOADING,		},
  { ".TITLE_INITIAL",		GFX_SPECIAL_ARG_TITLE_INITIAL,		},
  { ".TITLE_INITIAL_1",		GFX_SPECIAL_ARG_TITLE_INITIAL_1,	},
  { ".TITLE_INITIAL_2",		GFX_SPECIAL_ARG_TITLE_INITIAL_2,	},
  { ".TITLE_INITIAL_3",		GFX_SPECIAL_ARG_TITLE_INITIAL_3,	},
  { ".TITLE_INITIAL_4",		GFX_SPECIAL_ARG_TITLE_INITIAL_4,	},
  { ".TITLE_INITIAL_5",		GFX_SPECIAL_ARG_TITLE_INITIAL_5,	},
  { ".TITLE",			GFX_SPECIAL_ARG_TITLE,			},
  { ".TITLE_1",			GFX_SPECIAL_ARG_TITLE_1,		},
  { ".TITLE_2",			GFX_SPECIAL_ARG_TITLE_2,		},
  { ".TITLE_3",			GFX_SPECIAL_ARG_TITLE_3,		},
  { ".TITLE_4",			GFX_SPECIAL_ARG_TITLE_4,		},
  { ".TITLE_5",			GFX_SPECIAL_ARG_TITLE_5,		},
  { ".MAIN",			GFX_SPECIAL_ARG_MAIN,			},
  { ".LEVELS",			GFX_SPECIAL_ARG_LEVELS			},
  { ".LEVELNR",			GFX_SPECIAL_ARG_LEVELNR			},
  { ".SCORES",			GFX_SPECIAL_ARG_SCORES,			},
  { ".EDITOR",			GFX_SPECIAL_ARG_EDITOR,			},
  { ".INFO",			GFX_SPECIAL_ARG_INFO,			},
  { ".SETUP",			GFX_SPECIAL_ARG_SETUP,			},
  { ".PLAYING",			GFX_SPECIAL_ARG_PLAYING,		},
  { ".DOOR",			GFX_SPECIAL_ARG_DOOR,			},
  { ".TAPE",			GFX_SPECIAL_ARG_TAPE,			},
  { ".PANEL",			GFX_SPECIAL_ARG_PANEL,			},
  { ".PREVIEW",			GFX_SPECIAL_ARG_PREVIEW,		},
  { ".CRUMBLED",		GFX_SPECIAL_ARG_CRUMBLED,		},
  { ".MAINONLY",		GFX_SPECIAL_ARG_MAINONLY,		},
  { ".TYPENAME",		GFX_SPECIAL_ARG_TYPENAME,		},
  { ".SUBMENU",			GFX_SPECIAL_ARG_SUBMENU,		},
  { ".MENU",			GFX_SPECIAL_ARG_MENU,			},
  { ".TOONS",			GFX_SPECIAL_ARG_TOONS,			},
  { ".SCORESOLD",		GFX_SPECIAL_ARG_SCORESOLD,		},
  { ".SCORESNEW",		GFX_SPECIAL_ARG_SCORESNEW,		},
  { ".FADING",			GFX_SPECIAL_ARG_FADING,			},
  { ".QUIT",			GFX_SPECIAL_ARG_QUIT,			},

  /* empty suffix always matches -- check as last entry in InitMusicInfo() */
  { "",				GFX_SPECIAL_ARG_DEFAULT,		},

  { NULL,			0,					}
};

#include "conf_var.c"	/* include auto-generated data structure definitions */


/* ------------------------------------------------------------------------- */
/* font definitions                                                          */
/* ------------------------------------------------------------------------- */

/* Important: When one entry is a prefix of another entry, the longer entry
   must come first, because the dynamic configuration does prefix matching!
   (These definitions must match the corresponding definitions in "main.h"!) */

struct FontInfo font_info[NUM_FONTS + 1] =
{
  { "font.initial_1"		},
  { "font.initial_2"		},
  { "font.initial_3"		},
  { "font.initial_4"		},
  { "font.title_1"		},
  { "font.title_2"		},
  { "font.menu_1.active"	},
  { "font.menu_2.active"	},
  { "font.menu_1"		},
  { "font.menu_2"		},
  { "font.text_1.active"	},
  { "font.text_2.active"	},
  { "font.text_3.active"	},
  { "font.text_4.active"	},
  { "font.text_1"		},
  { "font.text_2"		},
  { "font.text_3"		},
  { "font.text_4"		},
  { "font.envelope_1"		},
  { "font.envelope_2"		},
  { "font.envelope_3"		},
  { "font.envelope_4"		},
  { "font.request"		},
  { "font.input_1.active"	},
  { "font.input_2.active"	},
  { "font.input_1"		},
  { "font.input_2"		},
  { "font.option_off_narrow"	},
  { "font.option_off"		},
  { "font.option_on_narrow"	},
  { "font.option_on"		},
  { "font.value_1"		},
  { "font.value_2"		},
  { "font.value_old"		},
  { "font.value_narrow"		},
  { "font.level_number.active"	},
  { "font.level_number"		},
  { "font.tape_recorder"	},
  { "font.game_info"		},
  { "font.info.elements"	},
  { "font.info.levelset"	},

  { NULL			}
};

struct GlobalAnimInfo global_anim_info[NUM_GLOBAL_ANIM_TOKENS + 1];

/* this contains predefined structure elements to init "global_anim_info" */
struct GlobalAnimNameInfo global_anim_name_info[NUM_GLOBAL_ANIM_TOKENS + 1] =
{
  /* (real) graphic definitions used to define animation graphics */
  { "gfx.global.anim_1",	},
  { "gfx.global.anim_2",	},
  { "gfx.global.anim_3",	},
  { "gfx.global.anim_4",	},
  { "gfx.global.anim_5",	},
  { "gfx.global.anim_6",	},
  { "gfx.global.anim_7",	},
  { "gfx.global.anim_8",	},
  { "gfx.global.anim_9",	},
  { "gfx.global.anim_10",	},
  { "gfx.global.anim_11",	},
  { "gfx.global.anim_12",	},
  { "gfx.global.anim_13",	},
  { "gfx.global.anim_14",	},
  { "gfx.global.anim_15",	},
  { "gfx.global.anim_16",	},
  { "gfx.global.anim_17",	},
  { "gfx.global.anim_18",	},
  { "gfx.global.anim_19",	},
  { "gfx.global.anim_20",	},
  { "gfx.global.anim_21",	},
  { "gfx.global.anim_22",	},
  { "gfx.global.anim_23",	},
  { "gfx.global.anim_24",	},
  { "gfx.global.anim_25",	},
  { "gfx.global.anim_26",	},
  { "gfx.global.anim_27",	},
  { "gfx.global.anim_28",	},
  { "gfx.global.anim_29",	},
  { "gfx.global.anim_30",	},
  { "gfx.global.anim_31",	},
  { "gfx.global.anim_32",	},

  /* (dummy) graphic definitions used to define animation controls */
  { "global.anim_1",		},
  { "global.anim_2",		},
  { "global.anim_3",		},
  { "global.anim_4",		},
  { "global.anim_5",		},
  { "global.anim_6",		},
  { "global.anim_7",		},
  { "global.anim_8",		},
  { "global.anim_9",		},
  { "global.anim_10",		},
  { "global.anim_11",		},
  { "global.anim_12",		},
  { "global.anim_13",		},
  { "global.anim_14",		},
  { "global.anim_15",		},
  { "global.anim_16",		},
  { "global.anim_17",		},
  { "global.anim_18",		},
  { "global.anim_19",		},
  { "global.anim_20",		},
  { "global.anim_21",		},
  { "global.anim_22",		},
  { "global.anim_23",		},
  { "global.anim_24",		},
  { "global.anim_25",		},
  { "global.anim_26",		},
  { "global.anim_27",		},
  { "global.anim_28",		},
  { "global.anim_29",		},
  { "global.anim_30",		},
  { "global.anim_31",		},
  { "global.anim_32",		},

  { NULL			}
};


/* ------------------------------------------------------------------------- */
/* music token prefix definitions                                            */
/* ------------------------------------------------------------------------- */

struct MusicPrefixInfo music_prefix_info[NUM_MUSIC_PREFIXES + 1] =
{
  { "background",		TRUE	},

  { NULL,			0	}
};


/* ========================================================================= */
/* main()                                                                    */
/* ========================================================================= */

static void print_usage()
{
  Print("\n"
	"Usage: %s [OPTION]... [HOSTNAME [PORT]]\n"
	"\n"
	"Options:\n"
	"  -b, --basepath DIRECTORY         alternative base DIRECTORY\n"
	"  -l, --level DIRECTORY            alternative level DIRECTORY\n"
	"  -g, --graphics DIRECTORY         alternative graphics DIRECTORY\n"
	"  -s, --sounds DIRECTORY           alternative sounds DIRECTORY\n"
	"  -m, --music DIRECTORY            alternative music DIRECTORY\n"
	"      --mytapes                    use private tapes for tape tests\n"
	"  -n, --network                    network multiplayer game\n"
	"      --serveronly                 only start network server\n"
	"  -v, --verbose                    verbose mode\n"
	"  -V, --version                    show program version\n"
	"      --debug                      display debugging information\n"
	"  -e, --execute COMMAND            execute batch COMMAND\n"
	"\n"
	"Valid commands for '--execute' option:\n"
	"  \"print graphicsinfo.conf\"        print default graphics config\n"
	"  \"print soundsinfo.conf\"          print default sounds config\n"
	"  \"print musicinfo.conf\"           print default music config\n"
	"  \"print editorsetup.conf\"         print default editor config\n"
	"  \"print helpanim.conf\"            print default helpanim config\n"
	"  \"print helptext.conf\"            print default helptext config\n"
	"  \"dump level FILE\"                dump level data from FILE\n"
	"  \"dump tape FILE\"                 dump tape data from FILE\n"
	"  \"autotest LEVELDIR [NR ...]\"     test level tapes for LEVELDIR\n"
	"  \"autoplay LEVELDIR [NR ...]\"     play level tapes for LEVELDIR\n"
	"  \"autoffwd LEVELDIR [NR ...]\"     ffwd level tapes for LEVELDIR\n"
	"  \"autowarp LEVELDIR [NR ...]\"     warp level tapes for LEVELDIR\n"
	"  \"convert LEVELDIR [NR]\"          convert levels in LEVELDIR\n"
	"  \"create images DIRECTORY\"        write BMP images to DIRECTORY\n"
	"  \"create CE image DIRECTORY\"      write BMP image to DIRECTORY\n"
	"\n",
	program.command_basename);
}

static void print_version()
{
  Print("%s", getProgramInitString());

  if (!strEqual(getProgramVersionString(), getProgramRealVersionString()))
  {
    Print(" (%s %d.%d.%d.%d%s)",
	  PROGRAM_TITLE_STRING,
	  PROGRAM_VERSION_MAJOR,
	  PROGRAM_VERSION_MINOR,
	  PROGRAM_VERSION_PATCH,
	  PROGRAM_VERSION_BUILD,
	  PROGRAM_VERSION_EXTRA);
  }

  Print("\n");

  if (options.debug)
  {
    SDL_version sdl_version;

    SDL_VERSION(&sdl_version);
    Print("- SDL %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);

    SDL_IMAGE_VERSION(&sdl_version);
    Print("- SDL_image %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);

    SDL_MIXER_VERSION(&sdl_version);
    Print("- SDL_mixer %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);

    SDL_NET_VERSION(&sdl_version);
    Print("- SDL_net %d.%d.%d\n",
	  sdl_version.major,
	  sdl_version.minor,
	  sdl_version.patch);
  }
}

static void InitProgramConfig(char *command_filename)
{
  char *program_title = PROGRAM_TITLE_STRING;
  char *program_icon_file = PROGRAM_ICON_FILENAME;
  char *program_version = getProgramRealVersionString();
  char *config_filename = getProgramConfigFilename(command_filename);
  char *userdata_basename = getBaseNameNoSuffix(command_filename);
  char *userdata_subdir;
  char *userdata_subdir_unix;

  // read default program config, if existing
  if (fileExists(config_filename))
  {
    // if program config file exists, derive Unix user data directory from it
    // (but only if the program config file is not generic "setup.conf" file)
    if (!strEqual(getBaseNamePtr(config_filename), SETUP_FILENAME))
    {
      userdata_basename = getBaseName(config_filename);

      if (strSuffix(userdata_basename, ".conf"))
	userdata_basename[strlen(userdata_basename) - 5] = '\0';
    }

    LoadSetupFromFilename(config_filename);
  }

  // set user data directory for Linux/Unix (but not Mac OS X)
  userdata_subdir_unix = getStringCat2(".", userdata_basename);

  // set program title from potentially redefined program title
  if (setup.internal.program_title != NULL &&
      strlen(setup.internal.program_title) > 0)
    program_title = getStringCopy(setup.internal.program_title);

  // set program version from potentially redefined program version
  if (setup.internal.program_version != NULL &&
      strlen(setup.internal.program_version) > 0)
    program_version = getStringCopy(setup.internal.program_version);

  // set program icon file from potentially redefined program icon file
  if (setup.internal.program_icon_file != NULL &&
      strlen(setup.internal.program_icon_file) > 0)
    program_icon_file = getStringCopy(setup.internal.program_icon_file);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_MACOSX)
  userdata_subdir = program_title;
#elif defined(PLATFORM_UNIX)
  userdata_subdir = userdata_subdir_unix;
#else
  userdata_subdir = USERDATA_DIRECTORY_OTHER;
#endif

  // set default window size (only relevant on program startup)
  if (setup.internal.default_window_width  != 0 &&
      setup.internal.default_window_height != 0)
  {
    WIN_XSIZE = setup.internal.default_window_width;
    WIN_YSIZE = setup.internal.default_window_height;
  }

  InitProgramInfo(command_filename,
		  config_filename,
		  userdata_subdir,
		  program_title,
		  program_title,
		  program_icon_file,
		  COOKIE_PREFIX,
		  program_version,
		  GAME_VERSION_ACTUAL);
}

int main(int argc, char *argv[])
{
  InitProgramConfig(argv[0]);

  InitWindowTitleFunction(getWindowTitleString);
  InitExitMessageFunction(DisplayExitMessage);
  InitExitFunction(CloseAllAndExit);
  InitPlatformDependentStuff();

  GetOptions(argc, argv, print_usage, print_version);
  OpenAll();

  EventLoop();
  CloseAllAndExit(0);

  return 0;	/* to keep compilers happy */
}
