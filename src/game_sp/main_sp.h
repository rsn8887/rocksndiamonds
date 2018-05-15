#ifndef MAIN_SP_H
#define MAIN_SP_H

/* ========================================================================= */
/* external functions and definitions imported from main program to game_sp  */
/* ========================================================================= */

#include "../engines.h"
#include "../conf_gfx.h"


/* ========================================================================= */
/* functions and definitions that are exported from game_sp to main program  */
/* ========================================================================= */

#include "export.h"


/* ========================================================================= */
/* internal functions and definitions that are not exported to main program  */
/* ========================================================================= */


/* ------------------------------------------------------------------------- */
/* constant definitions                                                      */
/* ------------------------------------------------------------------------- */

/* screen sizes and positions for SP engine */

extern int			TILESIZE_VAR;

#define TILESIZE		32
#define TILEX			TILESIZE
#define TILEY			TILESIZE
#define TILEX_VAR		TILESIZE_VAR
#define TILEY_VAR		TILESIZE_VAR

extern int			SCR_FIELDX, SCR_FIELDY;

#define MAX_BUF_XSIZE		(2 + SCR_FIELDX + 2)
#define MAX_BUF_YSIZE		(2 + SCR_FIELDY + 2)

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

extern struct LevelInfo_SP native_sp_level;

extern Bitmap *bitmap_db_field_sp;

extern int GfxElementLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
extern int GfxGraphicLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
extern int GfxGraphic[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
extern int GfxFrame[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];


/* ------------------------------------------------------------------------- */
/* exported functions                                                        */
/* ------------------------------------------------------------------------- */

#endif	/* MAIN_SP_H */
