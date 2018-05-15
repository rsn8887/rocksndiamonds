#ifndef MAIN_EM_H
#define MAIN_EM_H

/* ========================================================================= */
/* external functions and definitions imported from main program to game_em  */
/* ========================================================================= */

#include "../engines.h"


/* ========================================================================= */
/* functions and definitions that are exported from game_em to main program  */
/* ========================================================================= */

#include "export.h"


/* ========================================================================= */
/* internal functions and definitions that are not exported to main program  */
/* ========================================================================= */

#include "global.h"
#include "sample.h"


/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */

/* values for native Emerald Mine game version */
#define FILE_VERSION_EM_UNKNOWN	0
#define FILE_VERSION_EM_V1	1
#define FILE_VERSION_EM_V2	2
#define FILE_VERSION_EM_V3	3
#define FILE_VERSION_EM_V4	4	/* (there really was no version 4) */
#define FILE_VERSION_EM_V5	5
#define FILE_VERSION_EM_V6	6

#define FILE_VERSION_EM_ACTUAL	FILE_VERSION_EM_V6

/* level sizes and positions for EM engine */

#define WIDTH			EM_MAX_CAVE_WIDTH
#define HEIGHT			EM_MAX_CAVE_HEIGHT

/* screen sizes and positions for EM engine */

#define TILESIZE		32

extern int			TILESIZE_VAR;

#define TILEX			TILESIZE_VAR
#define TILEY			TILESIZE_VAR

extern int			SCR_FIELDX, SCR_FIELDY;

#define MAX_BUF_XSIZE		(SCR_FIELDX + 2)
#define MAX_BUF_YSIZE		(SCR_FIELDY + 2)

/* often used screen positions */

extern int			SX, SY;

#define SXSIZE			(SCR_FIELDX * TILEX)
#define SYSIZE			(SCR_FIELDY * TILEY)

/* other settings */
#define PLAY_ELEMENT_SOUND	FALSE


/* ------------------------------------------------------------------------- */
/* macro function definitions                                                */
/* ------------------------------------------------------------------------- */

#define ROUNDED_DIVIDE(x, y)	(((x) + (y) - 1) / (y))

#define SECONDS_TO_FRAMES(x)	((x) * FRAMES_PER_SECOND)
#define FRAMES_TO_SECONDS(x)	((x) / FRAMES_PER_SECOND)

#define SECONDS_TO_CYCLES(x)	ROUNDED_DIVIDE((x) * FRAMES_PER_SECOND, 8)
#define CYCLES_TO_SECONDS(x)	ROUNDED_DIVIDE((x) * 8, FRAMES_PER_SECOND)

#define DISPLAY_TIME(x)		ROUNDED_DIVIDE(x, FRAMES_PER_SECOND)


/* ------------------------------------------------------------------------- */
/* data structure definitions                                                */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* exported variables                                                        */
/* ------------------------------------------------------------------------- */

extern unsigned int RandomEM;

extern struct LEVEL lev;
extern struct PLAYER ply[MAX_PLAYERS];

extern struct LevelInfo_EM native_em_level;
extern struct GraphicInfo_EM graphic_info_em_object[TILE_MAX][8];
extern struct GraphicInfo_EM graphic_info_em_player[MAX_PLAYERS][SPR_MAX][8];

extern short **Boom;
extern short **Cave;
extern short **Next;
extern short **Draw;

extern struct GameInfo_EM game_em;

extern unsigned char tab_blank[TILE_MAX];
extern unsigned char tab_acid[TILE_MAX];
extern unsigned char tab_amoeba[TILE_MAX];
extern unsigned char tab_android_move[TILE_MAX];
extern unsigned short tab_explode_normal[TILE_MAX];
extern unsigned short tab_explode_dynamite[TILE_MAX];

extern unsigned short map_obj[8][TILE_MAX];
extern unsigned short map_spr[2][8][SPR_MAX];
extern unsigned short map_ttl[128];

extern Bitmap *screenBitmap;
extern Bitmap *scoreBitmap;

extern Bitmap *objBitmap;
extern Bitmap *sprBitmap;
extern Bitmap *ttlBitmap;
extern Bitmap *botBitmap;


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

#endif	/* MAIN_EM_H */
