#ifndef EXPORT_H
#define EXPORT_H

/* ========================================================================= */
/* functions and definitions exported from game_em to main program           */
/* ========================================================================= */

/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */

/* define these for backwards compatibility */
#define EM_ENGINE_BAD_ROLL
#define EM_ENGINE_BAD_SPRING

/* define these to use additional elements */
#define EM_ENGINE_USE_ADDITIONAL_ELEMENTS

/* internal definitions for EM engine */
#ifdef EM_ENGINE_BAD_ROLL
#define BAD_ROLL
#endif

#ifdef EM_ENGINE_BAD_SPRING
#define BAD_SPRING
#endif

/* one border for ZBORDER elements, one border for steelwall, if needed */
#define EM_MAX_CAVE_WIDTH		(MAX_PLAYFIELD_WIDTH  + 2 + 2)
#define EM_MAX_CAVE_HEIGHT		(MAX_PLAYFIELD_HEIGHT + 2 + 2)

/*
  -----------------------------------------------------------------------------
  definition of elements used in the Emerald Mine Club engine;
  the element names have the following properties:
  - elements that start with 'X' can be stored in a level file
  - elements that start with 'Y' indicate moving elements
  - elements that end with 'B' are the "backside" of moving elements
  -----------------------------------------------------------------------------
*/

enum
{
  Xblank = 0,		/* still */
  Yacid_splash_eB,	/* hmm */
  Yacid_splash_wB,	/* hmm */

#ifdef EM_ENGINE_BAD_ROLL
  Xstone_force_e,	/* only use these in eater */
  Xstone_force_w,
  Xnut_force_e,
  Xnut_force_w,
  Xspring_force_e,
  Xspring_force_w,
  Xemerald_force_e,
  Xemerald_force_w,
  Xdiamond_force_e,
  Xdiamond_force_w,
  Xbomb_force_e,
  Xbomb_force_w,
#endif

  Xstone,
  Xstone_pause,
  Xstone_fall,
  Ystone_s,
  Ystone_sB,
  Ystone_e,
  Ystone_eB,
  Ystone_w,
  Ystone_wB,
  Xnut,
  Xnut_pause,
  Xnut_fall,
  Ynut_s,
  Ynut_sB,
  Ynut_e,
  Ynut_eB,
  Ynut_w,
  Ynut_wB,
  Xbug_n,
  Xbug_e,
  Xbug_s,
  Xbug_w,
  Xbug_gon,
  Xbug_goe,
  Xbug_gos,
  Xbug_gow,
  Ybug_n,
  Ybug_nB,
  Ybug_e,
  Ybug_eB,
  Ybug_s,
  Ybug_sB,
  Ybug_w,
  Ybug_wB,
  Ybug_w_n,
  Ybug_n_e,
  Ybug_e_s,
  Ybug_s_w,
  Ybug_e_n,
  Ybug_s_e,
  Ybug_w_s,
  Ybug_n_w,
  Ybug_stone,
  Ybug_spring,
  Xtank_n,
  Xtank_e,
  Xtank_s,
  Xtank_w,
  Xtank_gon,
  Xtank_goe,
  Xtank_gos,
  Xtank_gow,
  Ytank_n,
  Ytank_nB,
  Ytank_e,
  Ytank_eB,
  Ytank_s,
  Ytank_sB,
  Ytank_w,
  Ytank_wB,
  Ytank_w_n,
  Ytank_n_e,
  Ytank_e_s,
  Ytank_s_w,
  Ytank_e_n,
  Ytank_s_e,
  Ytank_w_s,
  Ytank_n_w,
  Ytank_stone,
  Ytank_spring,
  Xandroid,
  Xandroid_1_n,
  Xandroid_2_n,
  Xandroid_1_e,
  Xandroid_2_e,
  Xandroid_1_w,
  Xandroid_2_w,
  Xandroid_1_s,
  Xandroid_2_s,
  Yandroid_n,
  Yandroid_nB,
  Yandroid_ne,
  Yandroid_neB,
  Yandroid_e,
  Yandroid_eB,
  Yandroid_se,
  Yandroid_seB,
  Yandroid_s,
  Yandroid_sB,
  Yandroid_sw,
  Yandroid_swB,
  Yandroid_w,
  Yandroid_wB,
  Yandroid_nw,
  Yandroid_nwB,
  Xspring,
  Xspring_pause,
  Xspring_e,
  Xspring_w,
  Xspring_fall,
  Yspring_s,
  Yspring_sB,
  Yspring_e,
  Yspring_eB,
  Yspring_w,
  Yspring_wB,
  Yspring_kill_e,
  Yspring_kill_eB,
  Yspring_kill_w,
  Yspring_kill_wB,
  Xeater_n,
  Xeater_e,
  Xeater_w,
  Xeater_s,
  Yeater_n,
  Yeater_nB,
  Yeater_e,
  Yeater_eB,
  Yeater_s,
  Yeater_sB,
  Yeater_w,
  Yeater_wB,
  Yeater_stone,
  Yeater_spring,
  Xalien,
  Xalien_pause,
  Yalien_n,
  Yalien_nB,
  Yalien_e,
  Yalien_eB,
  Yalien_s,
  Yalien_sB,
  Yalien_w,
  Yalien_wB,
  Yalien_stone,
  Yalien_spring,
  Xemerald,
  Xemerald_pause,
  Xemerald_fall,
  Xemerald_shine,
  Yemerald_s,
  Yemerald_sB,
  Yemerald_e,
  Yemerald_eB,
  Yemerald_w,
  Yemerald_wB,
  Yemerald_eat,
  Yemerald_stone,
  Xdiamond,
  Xdiamond_pause,
  Xdiamond_fall,
  Xdiamond_shine,
  Ydiamond_s,
  Ydiamond_sB,
  Ydiamond_e,
  Ydiamond_eB,
  Ydiamond_w,
  Ydiamond_wB,
  Ydiamond_eat,
  Ydiamond_stone,
  Xdrip_fall,
  Xdrip_stretch,
  Xdrip_stretchB,
  Xdrip_eat,
  Ydrip_s1,
  Ydrip_s1B,
  Ydrip_s2,
  Ydrip_s2B,
  Xbomb,
  Xbomb_pause,
  Xbomb_fall,
  Ybomb_s,
  Ybomb_sB,
  Ybomb_e,
  Ybomb_eB,
  Ybomb_w,
  Ybomb_wB,
  Ybomb_eat,
  Xballoon,
  Yballoon_n,
  Yballoon_nB,
  Yballoon_e,
  Yballoon_eB,
  Yballoon_s,
  Yballoon_sB,
  Yballoon_w,
  Yballoon_wB,
  Xgrass,
  Ygrass_nB,
  Ygrass_eB,
  Ygrass_sB,
  Ygrass_wB,
  Xdirt,
  Ydirt_nB,
  Ydirt_eB,
  Ydirt_sB,
  Ydirt_wB,
  Xacid_ne,
  Xacid_se,
  Xacid_s,
  Xacid_sw,
  Xacid_nw,
  Xacid_1,
  Xacid_2,
  Xacid_3,
  Xacid_4,
  Xacid_5,
  Xacid_6,
  Xacid_7,
  Xacid_8,
  Xball_1,
  Xball_1B,
  Xball_2,
  Xball_2B,
  Yball_eat,

#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
  Ykey_1_eat,
  Ykey_2_eat,
  Ykey_3_eat,
  Ykey_4_eat,
  Ykey_5_eat,
  Ykey_6_eat,
  Ykey_7_eat,
  Ykey_8_eat,
  Ylenses_eat,
  Ymagnify_eat,
  Ygrass_eat,
  Ydirt_eat,
#endif

  Xgrow_ns,
  Ygrow_ns_eat,
  Xgrow_ew,
  Ygrow_ew_eat,
  Xwonderwall,
  XwonderwallB,
  Xamoeba_1,
  Xamoeba_2,
  Xamoeba_3,
  Xamoeba_4,
  Xamoeba_5,
  Xamoeba_6,
  Xamoeba_7,
  Xamoeba_8,
  Xdoor_1,
  Xdoor_2,
  Xdoor_3,
  Xdoor_4,
  Xdoor_5,
  Xdoor_6,
  Xdoor_7,
  Xdoor_8,
  Xkey_1,
  Xkey_2,
  Xkey_3,
  Xkey_4,
  Xkey_5,
  Xkey_6,
  Xkey_7,
  Xkey_8,
  Xwind_n,
  Xwind_e,
  Xwind_s,
  Xwind_w,
  Xwind_nesw,
  Xwind_stop,
  Xexit,
  Xexit_1,
  Xexit_2,
  Xexit_3,
  Xdynamite,
  Ydynamite_eat,
  Xdynamite_1,
  Xdynamite_2,
  Xdynamite_3,
  Xdynamite_4,
  Xbumper,
  XbumperB,
  Xwheel,
  XwheelB,
  Xswitch,
  XswitchB,
  Xsand,
  Xsand_stone,
  Xsand_stonein_1,
  Xsand_stonein_2,
  Xsand_stonein_3,
  Xsand_stonein_4,
  Xsand_stonesand_1,
  Xsand_stonesand_2,
  Xsand_stonesand_3,
  Xsand_stonesand_4,

#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
  Xsand_stonesand_quickout_1,
  Xsand_stonesand_quickout_2,
#endif

  Xsand_stoneout_1,
  Xsand_stoneout_2,
  Xsand_sandstone_1,
  Xsand_sandstone_2,
  Xsand_sandstone_3,
  Xsand_sandstone_4,
  Xplant,
  Yplant,
  Xlenses,
  Xmagnify,
  Xdripper,
  XdripperB,
  Xfake_blank,
  Xfake_blankB,
  Xfake_grass,
  Xfake_grassB,
  Xfake_door_1,
  Xfake_door_2,
  Xfake_door_3,
  Xfake_door_4,
  Xfake_door_5,
  Xfake_door_6,
  Xfake_door_7,
  Xfake_door_8,

#ifdef EM_ENGINE_USE_ADDITIONAL_ELEMENTS
  Xfake_acid_1,
  Xfake_acid_2,
  Xfake_acid_3,
  Xfake_acid_4,
  Xfake_acid_5,
  Xfake_acid_6,
  Xfake_acid_7,
  Xfake_acid_8,
#endif

  Xsteel_1,
  Xsteel_2,
  Xsteel_3,
  Xsteel_4,
  Xwall_1,
  Xwall_2,
  Xwall_3,
  Xwall_4,
  Xround_wall_1,
  Xround_wall_2,
  Xround_wall_3,
  Xround_wall_4,
  Xdecor_1,
  Xdecor_2,
  Xdecor_3,
  Xdecor_4,
  Xdecor_5,
  Xdecor_6,
  Xdecor_7,
  Xdecor_8,
  Xdecor_9,
  Xdecor_10,
  Xdecor_11,
  Xdecor_12,
  Xalpha_0,
  Xalpha_1,
  Xalpha_2,
  Xalpha_3,
  Xalpha_4,
  Xalpha_5,
  Xalpha_6,
  Xalpha_7,
  Xalpha_8,
  Xalpha_9,
  Xalpha_excla,
  Xalpha_quote,
  Xalpha_comma,
  Xalpha_minus,
  Xalpha_perio,
  Xalpha_colon,
  Xalpha_quest,
  Xalpha_a,
  Xalpha_b,
  Xalpha_c,
  Xalpha_d,
  Xalpha_e,
  Xalpha_f,
  Xalpha_g,
  Xalpha_h,
  Xalpha_i,
  Xalpha_j,
  Xalpha_k,
  Xalpha_l,
  Xalpha_m,
  Xalpha_n,
  Xalpha_o,
  Xalpha_p,
  Xalpha_q,
  Xalpha_r,
  Xalpha_s,
  Xalpha_t,
  Xalpha_u,
  Xalpha_v,
  Xalpha_w,
  Xalpha_x,
  Xalpha_y,
  Xalpha_z,
  Xalpha_arrow_e,
  Xalpha_arrow_w,
  Xalpha_copyr,

  Xboom_bug,		/* passed from explode to synchro (linked explosion);
			   transition to explode_normal */
  Xboom_bomb,		/* passed from explode to synchro (linked explosion);
			   transition to explode_normal */
  Xboom_android,	/* passed from explode to synchro;
			   transition to boom_2 */
  Xboom_1,		/* passed from explode to synchro;
			   transition to boom_2 */
  Xboom_2,		/* transition to boom[] */

  Znormal,		/* passed from synchro to explode, only in next[];
			   no picture */
  Zdynamite,		/* passed from synchro to explode, only in next[];
			   no picture */
  Zplayer,		/* special code to indicate player;
			   no picture */
  ZBORDER,		/* special code to indicate border;
			   no picture */

  TILE_MAX
};

/* other definitions */

enum
{
  SPR_still = 0,
  SPR_walk  = 1,
  SPR_push  = 5,
  SPR_spray = 9,

  SPR_MAX   = 13
};

enum
{
  SAMPLE_blank = 0,	/* player walks on blank */
  SAMPLE_roll,		/* player pushes stone/bomb/nut/spring */
  SAMPLE_stone,		/* stone hits ground */
  SAMPLE_nut,		/* nut hits ground */
  SAMPLE_crack,		/* stone hits nut */
  SAMPLE_bug,		/* bug moves */
  SAMPLE_tank,		/* tank moves */
  SAMPLE_android_clone,	/* android places something */
  SAMPLE_android_move,	/* android moves */
  SAMPLE_spring,	/* spring hits ground/wall/bumper, stone hits spring */
  SAMPLE_slurp,		/* spring kills alien */
  SAMPLE_eater,		/* eater sits */
  SAMPLE_eater_eat,	/* eater eats diamond */
  SAMPLE_alien,		/* alien moves */
  SAMPLE_collect,	/* player collects object */
  SAMPLE_diamond,	/* diamond/emerald hits ground */
  SAMPLE_squash,	/* stone squashes diamond */
  SAMPLE_wonderfall,	/* object falls thru wonderwall */
  SAMPLE_drip,		/* drip hits ground */
  SAMPLE_push,		/* player pushes balloon/android */
  SAMPLE_dirt,		/* player walks on dirt */
  SAMPLE_acid,		/* acid splashes */
  SAMPLE_ball,		/* ball places something */
  SAMPLE_grow,		/* growing wall grows */
  SAMPLE_wonder,	/* wonderwall is active */
  SAMPLE_door,		/* player goes thru door (gate) */
  SAMPLE_exit_open,	/* exit opens */
  SAMPLE_exit_leave,	/* player goes into exit */
  SAMPLE_dynamite,	/* player places dynamite */
  SAMPLE_tick,		/* dynamite ticks */
  SAMPLE_press,		/* player presses wheel/wind/switch */
  SAMPLE_wheel,		/* wheel moves */
  SAMPLE_boom,		/* explosion */
  SAMPLE_time,		/* time runs out */
  SAMPLE_die,		/* player dies */

  SAMPLE_MAX
};


/* ------------------------------------------------------------------------- */
/* data structure definitions                                                */
/* ------------------------------------------------------------------------- */

struct LEVEL
{
  int home_initial;		/* number of players (initial) */
  int home;			/* number of players not yet at home */
				/* 0 == all players at home */

  int width;			/* playfield width */
  int height;			/* playfield height */

  int time_seconds;		/* available time (seconds) */
  int time_initial;		/* available time (initial) */
  int time;			/* time remaining (runtime) */

  boolean killed_out_of_time;	/* kill player due to time out */

  int required_initial;		/* emeralds needed (initial) */
  int required;			/* emeralds needed (runtime) */

  int score;			/* score */

  /* all below entries must be filled every time a level is read */

  int alien_score;		/* score for killing alien */
  int amoeba_time;		/* amoeba speed */
  int android_move_cnt_initial;	/* android move counter (initial) */
  int android_move_cnt;		/* android move counter */
  int android_move_time;	/* android move reset time */
  int android_clone_cnt_initial;/* android clone counter (initial) */
  int android_clone_cnt;	/* android clone counter */
  int android_clone_time;	/* android clone reset time */
  int ball_cnt;			/* ball counter */
  int ball_pos;			/* ball array pos counter */
  int ball_random;		/* ball is random flag */
  int ball_state_initial;	/* ball active flag (initial) */
  int ball_state;		/* ball active flag */
  int ball_time;		/* ball reset time */
  int bug_score;		/* score for killing bug */
  int diamond_score;		/* score for collecting diamond */
  int dynamite_score;		/* score for collecting dynamite */
  int eater_pos;		/* eater array pos */
  int eater_score;		/* score for killing eater */
  int emerald_score;		/* score for collecting emerald */
  int exit_score;		/* score for entering exit */
  int key_score;		/* score for colleting key */
  int lenses_cnt_initial;	/* lenses counter (initial) */
  int lenses_cnt;		/* lenses counter */
  int lenses_score;		/* score for collecting lenses */
  int lenses_time;		/* lenses reset time */
  int magnify_cnt_initial;	/* magnify counter (initial) */
  int magnify_cnt;		/* magnify counter */
  int magnify_score;		/* score for collecting magnifier */
  int magnify_time;		/* magnify reset time */
  int nut_score;		/* score for cracking nut */
  int shine_cnt;		/* shine counter for emerald/diamond */
  int slurp_score;		/* score for slurping alien */
  int tank_score;		/* score for killing tank */
  int wheel_cnt_initial;	/* wheel counter (initial) */
  int wheel_cnt;		/* wheel counter */
  int wheel_x_initial;		/* wheel x pos (initial) */
  int wheel_x;			/* wheel x pos */
  int wheel_y_initial;		/* wheel y pos (initial) */
  int wheel_y;			/* wheel y pos */
  int wheel_time;		/* wheel reset time */
  int wind_cnt_initial;		/* wind counter (initial) */
  int wind_cnt;			/* wind time counter */
  int wind_direction_initial;	/* wind direction (initial) */
  int wind_direction;		/* wind direction */
  int wind_time;		/* wind reset time */
  int wonderwall_state_initial;	/* wonderwall active flag (initial) */
  int wonderwall_state;		/* wonderwall active flag */
  int wonderwall_time_initial;	/* wonderwall time (initial) */
  int wonderwall_time;		/* wonderwall time */
  short eater_array[8][9];	/* eater data */
  short ball_array[8][8];	/* ball data */
  short android_array[TILE_MAX];/* android clone table */
  int num_ball_arrays;		/* number of ball data arrays used */

  int exit_x, exit_y;		/* kludge for playing player exit sound */
};

struct PLAYER
{
  int num;
  int exists;
  int alive_initial;
  int alive;

  int dynamite;
  int dynamite_cnt;
  int keys;
  int anim;

  int x_initial;
  int y_initial;
  int x;
  int y;
  int oldx;
  int oldy;

  int last_move_dir;

  int joy_n:1;
  int joy_e:1;
  int joy_s:1;
  int joy_w:1;
  int joy_snap:1;
  int joy_drop:1;
  int joy_stick:1;
  int joy_spin:1;
};

struct GlobalInfo_EM
{
  Bitmap *screenbuffer;
};

struct GameInfo_EM
{
  boolean any_player_moving;
  boolean any_player_snapping;

  boolean use_single_button;
  boolean use_snap_key_bug;

  int last_moving_player;
  int last_player_direction[MAX_PLAYERS];
};

struct LevelInfo_EM
{
  int file_version;

  short cave[EM_MAX_CAVE_WIDTH][EM_MAX_CAVE_HEIGHT];

  struct LEVEL *lev;
  struct PLAYER *ply[MAX_PLAYERS];

  /* used for runtime values */
  struct GameInfo_EM *game_em;
};

struct GraphicInfo_EM
{
  Bitmap *bitmap;
  int src_x, src_y;
  int src_offset_x, src_offset_y;
  int dst_offset_x, dst_offset_y;
  int width, height;

  Bitmap *crumbled_bitmap;
  int crumbled_src_x, crumbled_src_y;
  int crumbled_border_size;
  int crumbled_tile_size;

  boolean has_crumbled_graphics;
  boolean preserve_background;

  int unique_identifier;	/* used to identify needed screen updates */
};

struct EngineSnapshotInfo_EM
{
  struct GameInfo_EM game_em;
  unsigned int RandomEM;
  struct LEVEL lev;
  struct PLAYER ply[MAX_PLAYERS];
  short Array[4][EM_MAX_CAVE_HEIGHT][EM_MAX_CAVE_WIDTH];
  int screen_x;
  int screen_y;
  int frame;
  short **Boom;
  short **Cave;
  short **Next;
  short **Draw;
};


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

extern struct GlobalInfo_EM global_em_info;
extern struct GameInfo_EM game_em;
extern struct LevelInfo_EM native_em_level;
extern struct GraphicInfo_EM graphic_info_em_object[TILE_MAX][8];
extern struct GraphicInfo_EM graphic_info_em_player[MAX_PLAYERS][SPR_MAX][8];
extern struct EngineSnapshotInfo_EM engine_snapshot_em;

extern void em_open_all();
extern void em_close_all();

extern void InitGfxBuffers_EM();

extern void InitGameEngine_EM();
extern void GameActions_EM(byte *, boolean);

extern unsigned int InitEngineRandom_EM(int);

extern void setLevelInfoToDefaults_EM();
extern boolean LoadNativeLevel_EM(char *, boolean);

extern int getFieldbufferOffsetX_EM();
extern int getFieldbufferOffsetY_EM();

extern void BackToFront_EM(void);
extern void BlitScreenToBitmap_EM(Bitmap *);
extern void RedrawPlayfield_EM(boolean);

extern void LoadEngineSnapshotValues_EM();
extern void SaveEngineSnapshotValues_EM();

#endif	/* EXPORT_H */
