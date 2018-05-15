// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// game.h
// ============================================================================

#ifndef GAME_H
#define GAME_H

/* (not included here due to collisions with Emerald Mine engine definitions) */
/* #include "main.h" */

#define MAX_INVENTORY_SIZE		1000

#define MAX_HEALTH			100

#define STD_NUM_KEYS			4
#define MAX_NUM_KEYS			8

#define NUM_BELTS			4
#define NUM_BELT_PARTS			3

#define NUM_PANEL_INVENTORY		8
#define NUM_PANEL_GRAPHICS		8
#define NUM_PANEL_ELEMENTS		8
#define NUM_PANEL_CE_SCORE		8

#define STR_SNAPSHOT_MODE_OFF		"off"
#define STR_SNAPSHOT_MODE_EVERY_STEP	"every_step"
#define STR_SNAPSHOT_MODE_EVERY_MOVE	"every_move"
#define STR_SNAPSHOT_MODE_EVERY_COLLECT	"every_collect"
#define STR_SNAPSHOT_MODE_DEFAULT	STR_SNAPSHOT_MODE_OFF

#define SNAPSHOT_MODE_OFF		0
#define SNAPSHOT_MODE_EVERY_STEP	1
#define SNAPSHOT_MODE_EVERY_MOVE	2
#define SNAPSHOT_MODE_EVERY_COLLECT	3
#define SNAPSHOT_MODE_DEFAULT		SNAPSHOT_MODE_OFF


struct GamePanelInfo
{
  struct TextPosInfo level_number;
  struct TextPosInfo gems;
  struct TextPosInfo inventory_count;
  struct TextPosInfo inventory_first[NUM_PANEL_INVENTORY];
  struct TextPosInfo inventory_last[NUM_PANEL_INVENTORY];
  struct TextPosInfo key[MAX_NUM_KEYS];
  struct TextPosInfo key_white;
  struct TextPosInfo key_white_count;
  struct TextPosInfo score;
  struct TextPosInfo highscore;
  struct TextPosInfo time;
  struct TextPosInfo time_hh;
  struct TextPosInfo time_mm;
  struct TextPosInfo time_ss;
  struct TextPosInfo time_anim;
  struct TextPosInfo health;
  struct TextPosInfo health_anim;
  struct TextPosInfo frame;
  struct TextPosInfo shield_normal;
  struct TextPosInfo shield_normal_time;
  struct TextPosInfo shield_deadly;
  struct TextPosInfo shield_deadly_time;
  struct TextPosInfo exit;
  struct TextPosInfo emc_magic_ball;
  struct TextPosInfo emc_magic_ball_switch;
  struct TextPosInfo light_switch;
  struct TextPosInfo light_switch_time;
  struct TextPosInfo timegate_switch;
  struct TextPosInfo timegate_switch_time;
  struct TextPosInfo switchgate_switch;
  struct TextPosInfo emc_lenses;
  struct TextPosInfo emc_lenses_time;
  struct TextPosInfo emc_magnifier;
  struct TextPosInfo emc_magnifier_time;
  struct TextPosInfo balloon_switch;
  struct TextPosInfo dynabomb_number;
  struct TextPosInfo dynabomb_size;
  struct TextPosInfo dynabomb_power;
  struct TextPosInfo penguins;
  struct TextPosInfo sokoban_objects;
  struct TextPosInfo sokoban_fields;
  struct TextPosInfo robot_wheel;
  struct TextPosInfo conveyor_belt[NUM_BELTS];
  struct TextPosInfo conveyor_belt_switch[NUM_BELTS];
  struct TextPosInfo magic_wall;
  struct TextPosInfo magic_wall_time;
  struct TextPosInfo gravity_state;
  struct TextPosInfo graphic[NUM_PANEL_GRAPHICS];
  struct TextPosInfo element[NUM_PANEL_ELEMENTS];
  struct TextPosInfo element_count[NUM_PANEL_ELEMENTS];
  struct TextPosInfo ce_score[NUM_PANEL_CE_SCORE];
  struct TextPosInfo ce_score_element[NUM_PANEL_CE_SCORE];
  struct TextPosInfo player_name;
  struct TextPosInfo level_name;
  struct TextPosInfo level_author;
};

struct GameButtonInfo
{
  struct XY stop;
  struct XY pause;
  struct XY play;

  struct XY undo;
  struct XY redo;

  struct XY save;
  struct XY pause2;
  struct XY load;

  struct XY sound_music;
  struct XY sound_loops;
  struct XY sound_simple;

  struct XY panel_stop;
  struct XY panel_pause;
  struct XY panel_play;

  struct XY panel_sound_music;
  struct XY panel_sound_loops;
  struct XY panel_sound_simple;
};

struct GameSnapshotInfo
{
  int mode;

  byte last_action[MAX_PLAYERS];
  boolean changed_action;
  boolean collected_item;

  boolean save_snapshot;
};

struct GameInfo
{
  /* values for control panel */
  struct GamePanelInfo panel;
  struct GameButtonInfo button;

  /* values for graphics engine customization */
  int graphics_engine_version;
  boolean use_native_emc_graphics_engine;
  boolean use_native_sp_graphics_engine;
  boolean use_masked_pushing;
  int forced_scroll_delay_value;
  int scroll_delay_value;
  int tile_size;

  /* values for engine initialization */
  int default_push_delay_fixed;
  int default_push_delay_random;

  /* constant within running game */
  int engine_version;
  int emulation;
  int initial_move_delay[MAX_PLAYERS];
  int initial_move_delay_value[MAX_PLAYERS];
  int initial_push_delay_value;

  /* flag for single or multi-player mode (needed for playing tapes) */
  /* (when playing/recording games, this is identical to "setup.team_mode" */
  boolean team_mode;

  /* flags to handle bugs in and changes between different engine versions */
  /* (for the latest engine version, these flags should always be "FALSE") */
  boolean use_change_when_pushing_bug;
  boolean use_block_last_field_bug;
  boolean max_num_changes_per_frame;
  boolean use_reverse_scan_direction;

  /* variable within running game */
  int yamyam_content_nr;
  boolean robot_wheel_active;
  boolean magic_wall_active;
  int magic_wall_time_left;
  int light_time_left;
  int timegate_time_left;
  int belt_dir[4];
  int belt_dir_nr[4];
  int switchgate_pos;
  int wind_direction;
  boolean gravity;
  boolean explosions_delayed;
  boolean envelope_active;
  boolean no_time_limit;	/* (variable only in very special case) */

  /* values for the new EMC elements */
  int lenses_time_left;
  int magnify_time_left;
  boolean ball_state;
  int ball_content_nr;

  /* values for player idle animation (no effect on engine) */
  int player_boring_delay_fixed;
  int player_boring_delay_random;
  int player_sleeping_delay_fixed;
  int player_sleeping_delay_random;

  /* values for special game initialization control */
  boolean restart_level;

  /* trigger message to ask for restarting the game */
  char *restart_game_message;

  /* values for special game control */
  int centered_player_nr;
  int centered_player_nr_next;
  boolean set_centered_player;

  /* values for random number generator initialization after snapshot */
  unsigned int num_random_calls;

  /* values for game engine snapshot control */
  struct GameSnapshotInfo snapshot;
};

struct PlayerInfo
{
  boolean present;		/* player present in level playfield */
  boolean connected;		/* player connected (locally or via network) */
  boolean active;		/* player present and connected */
  boolean mapped;		/* player already mapped to input device */

  boolean killed;		/* player maybe present/active, but killed */
  boolean reanimated;		/* player maybe killed, but reanimated */

  int index_nr;			/* player number (0 to 3) */
  int index_bit;		/* player number bit (1 << 0 to 1 << 3) */
  int element_nr;		/* element (EL_PLAYER_1 to EL_PLAYER_4) */
  int client_nr;		/* network client identifier */

  byte action;			/* action from local input device */
  byte mapped_action;		/* action mapped from device to player */
  byte effective_action;	/* action acknowledged from network server
				   or summarized over all configured input
				   devices when in single player mode */
  byte programmed_action;	/* action forced by game itself (like moving
				   through doors); overrides other actions */

  struct MouseActionInfo mouse_action;		 /* (used by MM engine only) */
  struct MouseActionInfo effective_mouse_action; /* (used by MM engine only) */

  int jx, jy, last_jx, last_jy;
  int MovDir, MovPos, GfxDir, GfxPos;
  int Frame, StepFrame;

  int GfxAction;

  int initial_element;		/* EL_PLAYER_1 to EL_PLAYER_4 or EL_SP_MURPHY */
  int artwork_element;
  boolean use_murphy;

  boolean block_last_field;
  int block_delay_adjustment;	/* needed for different engine versions */

  boolean can_fall_into_acid;

  boolean gravity;

  boolean LevelSolved, GameOver;

  boolean LevelSolved_GameWon;
  boolean LevelSolved_GameEnd;
  boolean LevelSolved_PanelOff;
  boolean LevelSolved_SaveTape;
  boolean LevelSolved_SaveScore;

  int LevelSolved_CountingTime;
  int LevelSolved_CountingScore;
  int LevelSolved_CountingHealth;

  int last_move_dir;

  boolean is_active;

  boolean is_waiting;
  boolean is_moving;
  boolean is_auto_moving;
  boolean is_digging;
  boolean is_snapping;
  boolean is_collecting;
  boolean is_pushing;
  boolean is_switching;
  boolean is_dropping;
  boolean is_dropping_pressed;

  boolean is_bored;
  boolean is_sleeping;

  boolean was_waiting;
  boolean was_moving;
  boolean was_snapping;
  boolean was_dropping;

  boolean cannot_move;

  boolean force_dropping;	/* needed for single step mode */

  int frame_counter_bored;
  int frame_counter_sleeping;

  int anim_delay_counter;
  int post_delay_counter;

  int dir_waiting;
  int action_waiting, last_action_waiting;
  int special_action_bored;
  int special_action_sleeping;

  int num_special_action_bored;
  int num_special_action_sleeping;

  int switch_x, switch_y;
  int drop_x, drop_y;

  int show_envelope;

  int move_delay;
  int move_delay_value;
  int move_delay_value_next;
  int move_delay_reset_counter;

  int push_delay;
  int push_delay_value;

  unsigned int actual_frame_counter;

  int drop_delay;
  int drop_pressed_delay;

  int step_counter;

  int score;
  int score_final;

  int health;
  int health_final;

  int gems_still_needed;
  int sokobanfields_still_needed;
  int lights_still_needed;
  int friends_still_needed;
  int key[MAX_NUM_KEYS];
  int num_white_keys;
  int dynabomb_count, dynabomb_size, dynabombs_left, dynabomb_xl;
  int shield_normal_time_left;
  int shield_deadly_time_left;

  int inventory_element[MAX_INVENTORY_SIZE];
  int inventory_infinite_element;
  int inventory_size;
};

extern struct GameInfo		game;
extern struct PlayerInfo	stored_player[MAX_PLAYERS], *local_player;


#ifdef DEBUG
void DEBUG_SetMaximumDynamite();
#endif

void GetPlayerConfig(void);
int GetElementFromGroupElement(int);

int getPlayerInventorySize(int);

void DrawGameValue_Time(int);
void DrawGameDoorValues(void);

void UpdateAndDisplayGameControlValues();

void InitGameSound();
void InitGame();

void UpdateEngineValues(int, int, int, int);
void GameWon(void);
void GameEnd(void);

void InitPlayerGfxAnimation(struct PlayerInfo *, int, int);
void Moving2Blocked(int, int, int *, int *);
void Blocked2Moving(int, int, int *, int *);
void DrawDynamite(int, int);

void StartGameActions(boolean, boolean, int);

void GameActions(void);
void GameActions_EM_Main();
void GameActions_SP_Main();
void GameActions_MM_Main();
void GameActions_RND_Main();
void GameActions_RND();

void ScrollLevel(int, int);

void InitPlayLevelSound();
void PlayLevelSound_EM(int, int, int, int);
void PlayLevelSound_SP(int, int, int, int);
void PlayLevelSound_MM(int, int, int, int);
void PlaySound_MM(int);
void PlaySoundLoop_MM(int);
void StopSound_MM(int);

void RaiseScore(int);
void RaiseScoreElement(int);

void RequestQuitGameExt(boolean, boolean, char *);
void RequestQuitGame(boolean);
void RequestRestartGame(char *);

unsigned int InitEngineRandom_RND(int);
unsigned int RND(int);

void FreeEngineSnapshotSingle();
void FreeEngineSnapshotList();
void LoadEngineSnapshotSingle();
void SaveEngineSnapshotSingle();
boolean CheckSaveEngineSnapshotToList();
void SaveEngineSnapshotToList();
void SaveEngineSnapshotToListInitial();
boolean CheckEngineSnapshotSingle();
boolean CheckEngineSnapshotList();

void CreateGameButtons();
void FreeGameButtons();
void MapUndoRedoButtons();
void UnmapUndoRedoButtons();
void MapGameButtons();
void UnmapGameButtons();
void RedrawGameButtons();
void MapGameButtonsOnTape();
void UnmapGameButtonsOnTape();
void RedrawGameButtonsOnTape();

void HandleSoundButtonKeys(Key);

#endif
