#ifndef MAIN_MM_H
#define MAIN_MM_H

/* ========================================================================= */
/* external functions and definitions imported from main program to game_mm  */
/* ========================================================================= */

#include "../engines.h"
#include "../conf_gfx.h"


/* ========================================================================= */
/* functions and definitions that are exported from game_mm to main program  */
/* ========================================================================= */

#include "export.h"


/* ========================================================================= */
/* internal functions and definitions that are not exported to main program  */
/* ========================================================================= */


/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */

/* screen sizes and positions for MM engine */

extern int			TILESIZE_VAR;

#define TILESIZE		32
#define TILEX			TILESIZE
#define TILEY			TILESIZE
#define TILEX_VAR		TILESIZE_VAR
#define TILEY_VAR		TILESIZE_VAR

extern int			SCR_FIELDX, SCR_FIELDY;

#define MAX_BUF_XSIZE		SCR_FIELDX
#define MAX_BUF_YSIZE		SCR_FIELDY

/* often used screen positions */

extern int			SX, SY;

#define SXSIZE			(SCR_FIELDX * TILEX_VAR)
#define SYSIZE			(SCR_FIELDY * TILEY_VAR)
#define FXSIZE			(MAX_BUF_XSIZE * TILEX_VAR)
#define FYSIZE			(MAX_BUF_YSIZE * TILEY_VAR)

extern int			REAL_SX, REAL_SY;

#define FULL_SXSIZE		(2 + SXSIZE + 2)
#define FULL_SYSIZE		(2 + SYSIZE + 2)


/* ------------------------------------------------------------------------- */
/* data structure definitions                                                */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* exported variables                                                        */
/* ------------------------------------------------------------------------- */

extern struct LevelInfo_MM native_mm_level;

extern Bitmap *bitmap_db_field_mm;

extern int GfxElementLast[MM_MAX_PLAYFIELD_WIDTH][MM_MAX_PLAYFIELD_HEIGHT];
extern int GfxGraphicLast[MM_MAX_PLAYFIELD_WIDTH][MM_MAX_PLAYFIELD_HEIGHT];
extern int GfxGraphic[MM_MAX_PLAYFIELD_WIDTH][MM_MAX_PLAYFIELD_HEIGHT];
extern int GfxFrame[MM_MAX_PLAYFIELD_WIDTH][MM_MAX_PLAYFIELD_HEIGHT];


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

#endif	/* MAIN_MM_H */
