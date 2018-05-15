#ifndef GAME_MM_EXPORT_H
#define GAME_MM_EXPORT_H

/* ========================================================================= */
/* functions and definitions exported from game_mm to main program           */
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */

#define MM_MAX_PLAYFIELD_WIDTH		MAX_PLAYFIELD_WIDTH
#define MM_MAX_PLAYFIELD_HEIGHT		MAX_PLAYFIELD_HEIGHT

#define MM_STD_PLAYFIELD_WIDTH		16
#define MM_STD_PLAYFIELD_HEIGHT		12

#define MM_MAX_PLAYFIELD_SIZE		(MM_MAX_PLAYFIELD_WIDTH *	\
					 MM_MAX_PLAYFIELD_HEIGHT)

#define MAX_NUM_AMOEBA			100
#define MAX_NUM_BEAMERS			8

#define MAX_LASER_LEN			256
#define MAX_LASER_ENERGY		100
#define MAX_LASER_OVERLOAD		100

#define MM_LEVEL_SCORE_ELEMENTS		16

#define MM_MAX_LEVEL_NAME_LEN		32
#define MM_MAX_LEVEL_AUTHOR_LEN		32


#define EL_MM_START_1_NATIVE		0
#define EL_MM_END_1_NATIVE		155

#define EL_MM_CHAR_START_NATIVE		160
#define EL_MM_CHAR_END_NATIVE		239

#define EL_MM_START_2_NATIVE		240
#define EL_MM_END_2_NATIVE		430

#define EL_MM_RUNTIME_START_NATIVE	500
#define EL_MM_RUNTIME_END_NATIVE	504

#define EL_MM_DUMMY_START_NATIVE	700
#define EL_MM_DUMMY_END_NATIVE		709

/* elements to be specially mapped */
#define EL_MM_EMPTY_NATIVE		0
#define EL_DF_EMPTY_NATIVE		304

/* sounds */
#define SND_MM_GAME_LEVELTIME_CHARGING	0
#define SND_MM_GAME_HEALTH_CHARGING	1


/* ------------------------------------------------------------------------- */
/* data structure definitions                                                */
/* ------------------------------------------------------------------------- */

struct GlobalInfo_MM
{
};

struct CycleList
{
  int x, y;
  int steps;
};

struct MovingList
{
  int x, y;
  int dir;
};

struct DamageList
{
  int x, y;
  int edge, angle;
  boolean is_mirror;
};

struct BeamerInfo
{
  int x, y;
  int num;
};

struct PacMan
{
  int XP, YP;
  int Dr;
};

struct LaserInfo
{
  struct XY start_edge;
  int start_angle;

  int current_angle;

  struct DamageList damage[MAX_LASER_LEN + 10];
  int num_damages;

  struct XY edge[MAX_LASER_LEN + 10];
  int num_edges;

  struct BeamerInfo beamer[MAX_NUM_BEAMERS][2];
  int beamer_edge[MAX_NUM_BEAMERS];
  int beamer_nr[MAX_NUM_BEAMERS];
  int num_beamers;

  boolean overloaded;
  int overload_value;

  boolean fuse_off;
  int fuse_x, fuse_y;

  int dest_element;
  boolean stops_inside_element;

  boolean redraw;

  int wall_mask;
};

struct GameInfo_MM
{
  boolean LevelSolved;
  boolean GameOver;

  struct CycleList cycle[MM_MAX_PLAYFIELD_SIZE];
  int num_cycle;

  struct MovingList pacman[MM_MAX_PLAYFIELD_SIZE];
  int num_pacman;

  int score;
  int energy_left;
  int kettles_still_needed;
  int lights_still_needed;
  int num_keys;

  boolean level_solved;
  boolean game_over;
  int game_over_cause;

  boolean cheat_no_overload;
  boolean cheat_no_explosion;

  int laser_overload_value;
  boolean laser_enabled;
};

struct LevelInfo_MM
{
  int file_version;		/* version of file the level was stored with */
  int game_version;		/* version of game engine to play this level */
  boolean encoding_16bit_field;		/* level contains 16-bit elements */

  int fieldx;
  int fieldy;
  int time;
  int kettles_needed;
  boolean auto_count_kettles;
  boolean laser_red, laser_green, laser_blue;
  char name[MM_MAX_LEVEL_NAME_LEN + 1];
  char author[MM_MAX_LEVEL_AUTHOR_LEN + 1];
  int score[MM_LEVEL_SCORE_ELEMENTS];
  int amoeba_speed;
  int time_fuse;
  int time_bomb;
  int time_ball;
  int time_block;

  short field[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
};

struct GraphicInfo_MM
{
};

struct EngineSnapshotInfo_MM
{
  struct GameInfo_MM game_mm;

  struct LaserInfo laser;

  short Ur[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short Hit[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short Box[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short Angle[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];
  short Frame[MAX_PLAYFIELD_WIDTH][MAX_PLAYFIELD_HEIGHT];

  short LX,LY, XS,YS, ELX,ELY;
  short CT,Ct;

  int last_LX, last_LY, last_hit_mask;
  int hold_x, hold_y;
  int pacman_nr;

  unsigned int rotate_delay;
  unsigned int pacman_delay;
  unsigned int energy_delay;
  unsigned int overload_delay;
};


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

extern struct GlobalInfo_MM global_mm_info;
extern struct GameInfo_MM game_mm;
extern struct LevelInfo_MM native_mm_level;
extern struct EngineSnapshotInfo_MM engine_snapshot_mm;

extern short Ur[MM_MAX_PLAYFIELD_WIDTH][MM_MAX_PLAYFIELD_HEIGHT];

extern void mm_open_all();
extern void mm_close_all();

void InitElementProperties_MM();

extern void InitGfxBuffers_MM();

extern void InitGameEngine_MM();
extern void InitGameActions_MM();
extern void GameActions_MM(struct MouseActionInfo, boolean);

extern void DrawLaser_MM();

extern boolean ClickElement(int, int, int);

extern unsigned int InitEngineRandom_MM(int);

extern void setLevelInfoToDefaults_MM();
extern void copyInternalEngineVars_MM();
extern boolean LoadNativeLevel_MM(char *, boolean);
extern void SaveNativeLevel_MM(char *);

extern int getFieldbufferOffsetX_MM();
extern int getFieldbufferOffsetY_MM();

extern void BlitScreenToBitmap_MM(Bitmap *);
extern void RedrawPlayfield_MM();

extern void LoadEngineSnapshotValues_MM();
extern void SaveEngineSnapshotValues_MM(ListNode **);

extern int getButtonFromTouchPosition(int, int, int, int);

#endif	/* GAME_MM_EXPORT_H */
