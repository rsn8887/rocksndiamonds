/***********************************************************
* Mirror Magic -- McDuffin's Revenge                       *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* tools.c                                                  *
***********************************************************/

#include "main_mm.h"

#include "mm_main.h"
#include "mm_tools.h"


void SetDrawtoField_MM(int mode)
{
  int full_xsize = lev_fieldx * TILESIZE_VAR;
  int full_ysize = lev_fieldy * TILESIZE_VAR;

  // distance (delta) from screen border (SX/SY) to centered level playfield
  dSX = (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);
  dSY = (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  // for convenience, absolute screen position to centered level playfield
  cSX = SX + dSX;
  cSY = SY + dSY;
  cSX2 = SX + dSX + 2;	// including playfield border
  cSY2 = SY + dSY + 2;	// including playfield border

  if (mode == DRAW_TO_BACKBUFFER)
  {
    cFX = FX + dSX;
    cFY = FY + dSY;
  }

  SetTileCursorSXSY(cSX, cSY);
}

void ClearWindow()
{
  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  SetDrawtoField(DRAW_TO_BACKBUFFER);
  SetDrawtoField_MM(DRAW_TO_BACKBUFFER);

  redraw_mask |= REDRAW_FIELD;
}

void DrawGraphicAnimation_MM(int x, int y, int graphic, int frame)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &bitmap, &src_x, &src_y);

  BlitBitmap(bitmap, drawto_field, src_x, src_y, TILEX, TILEY,
	     cFX + x * TILEX, cFY + y * TILEY);
}

void DrawGraphic_MM(int x, int y, int graphic)
{
#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphic_MM(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphic_MM(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicExt_MM(drawto_field, cFX + x * TILEX, cFY + y * TILEY, graphic);

  MarkTileDirty(x, y);
}

void DrawGraphicExt_MM(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, 0, &bitmap, &src_x, &src_y);

  BlitBitmap(bitmap, d, src_x, src_y, TILEX, TILEY, x, y);
}

void DrawGraphicThruMask_MM(int x, int y, int graphic)
{
#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicThruMask_MM(): x = %d,y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicThruMask_MM(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicThruMaskExt_MM(drawto_field, cFX + x * TILEX, cFY + y * TILEY,
			    graphic);

  MarkTileDirty(x,y);
}

void DrawGraphicThruMaskExt_MM(DrawBuffer *d, int dest_x, int dest_y,
			       int graphic)
{
  int src_x, src_y;
  Bitmap *src_bitmap;

  if (graphic == IMG_EMPTY)
    return;

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX, TILEY, dest_x, dest_y);
}

void DrawMiniGraphic_MM(int x, int y, int graphic)
{
  DrawMiniGraphicExt_MM(drawto, cSX + x * MINI_TILEX, cSY + y * MINI_TILEY,
			graphic);

  MarkTileDirty(x / 2, y / 2);
}

void getMicroGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSource(graphic, 0, TILESIZE / 4, bitmap, x, y);
}

void DrawMiniGraphicExt_MM(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *bitmap;
  int src_x, src_y;

  getMiniGraphicSource(graphic, &bitmap, &src_x, &src_y);

  BlitBitmap(bitmap, d, src_x, src_y, MINI_TILEX, MINI_TILEY, x, y);
}

void DrawGraphicShifted_MM(int x,int y, int dx,int dy, int graphic,
			int cut_mode, int mask_mode)
{
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;
  int src_x, src_y, dest_x, dest_y;
  Bitmap *src_bitmap;

  if (graphic < 0)
  {
    DrawGraphic_MM(x, y, graphic);

    return;
  }

  if (dx || dy)			/* Verschiebung der Grafik? */
  {
    if (x < BX1)		/* Element kommt von links ins Bild */
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		/* Element kommt von rechts ins Bild */
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x==BX1 && dx < 0)	/* Element verläßt links das Bild */
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x==BX2 && dx > 0)	/* Element verläßt rechts das Bild */
      width -= dx;
    else if (dx)		/* allg. Bewegung in x-Richtung */
      MarkTileDirty(x + SIGN(dx), y);

    if (y < BY1)		/* Element kommt von oben ins Bild */
    {
      if (cut_mode==CUT_BELOW)	/* Element oberhalb des Bildes */
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		/* Element kommt von unten ins Bild */
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y==BY1 && dy < 0)	/* Element verläßt oben das Bild */
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode == CUT_ABOVE)
    {
      if (y == BY2)		/* Element unterhalb des Bildes */
	return;

      height = dy;
      cy = TILEY - dy;
      dy = TILEY;
      MarkTileDirty(x, y + 1);
    }				/* Element verläßt unten das Bild */
    else if (dy > 0 && (y == BY2 || cut_mode == CUT_BELOW))
    {
      height -= dy;
    }
    else if (dy)		/* allg. Bewegung in y-Richtung */
    {
      MarkTileDirty(x, y + SIGN(dy));
    }
  }

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  src_x += cx;
  src_y += cy;

  dest_x = cFX + x * TILEX + dx;
  dest_y = cFY + y * TILEY + dy;

#if DEBUG
  if (!IN_SCR_FIELD(x,y))
  {
    printf("DrawGraphicShifted_MM(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicShifted_MM(): This should never happen!\n");
    return;
  }
#endif

  if (mask_mode == USE_MASKING)
    BlitBitmapMasked(src_bitmap, drawto_field,
		     src_x, src_y, TILEX, TILEY, dest_x, dest_y);
  else
    BlitBitmap(src_bitmap, drawto_field,
	       src_x, src_y, width, height, dest_x, dest_y);

  MarkTileDirty(x,y);
}

void DrawGraphicShiftedThruMask_MM(int x,int y, int dx,int dy, int graphic,
				int cut_mode)
{
  DrawGraphicShifted_MM(x, y, dx, dy, graphic, cut_mode, USE_MASKING);
}

void DrawScreenElementExt_MM(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int ux = LEVELX(x), uy = LEVELY(y);
  int graphic = el2gfx(element);
  int phase8 = ABS(MovPos[ux][uy]) / (TILEX / 8);
  int phase2  = phase8 / 4;
  int dir = MovDir[ux][uy];

  if (element == EL_PACMAN)
  {
    graphic = (phase2 ? IMG_MM_PACMAN_RIGHT : IMG_MM_PACMAN_EATING_RIGHT);

    if (dir == MV_UP)
      graphic += 1;
    else if (dir == MV_LEFT)
      graphic += 2;
    else if (dir == MV_DOWN)
      graphic += 3;
  }

  if (dx || dy)
    DrawGraphicShifted_MM(x, y, dx, dy, graphic, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask_MM(x, y, graphic);
  else
    DrawGraphic_MM(x, y, graphic);
}

void DrawLevelElementExt_MM(int x, int y, int dx, int dy, int element,
			 int cut_mode, int mask_mode)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElementExt_MM(SCREENX(x), SCREENY(y), dx, dy, element,
			 cut_mode, mask_mode);
}

void DrawScreenElementShifted_MM(int x, int y, int dx, int dy, int element,
			      int cut_mode)
{
  DrawScreenElementExt_MM(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementShifted_MM(int x, int y, int dx, int dy, int element,
			     int cut_mode)
{
  DrawLevelElementExt_MM(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawScreenElementThruMask_MM(int x, int y, int element)
{
  DrawScreenElementExt_MM(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelElementThruMask_MM(int x, int y, int element)
{
  DrawLevelElementExt_MM(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelFieldThruMask_MM(int x, int y)
{
  DrawLevelElementExt_MM(x, y, 0, 0, Feld[x][y], NO_CUTTING, USE_MASKING);
}

void DrawScreenElement_MM(int x, int y, int element)
{
  DrawScreenElementExt_MM(x, y, 0, 0, element, NO_CUTTING, NO_MASKING);
}

void DrawLevelElement_MM(int x, int y, int element)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElement_MM(SCREENX(x), SCREENY(y), element);
}

void DrawScreenField_MM(int x, int y)
{
  int element = Feld[x][y];

  if (!IN_LEV_FIELD(x, y))
    return;

  if (IS_MOVING(x, y))
  {
    int horiz_move = (MovDir[x][y] == MV_LEFT || MovDir[x][y] == MV_RIGHT);

    DrawScreenElement_MM(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted_MM(x, y, MovPos[x][y], 0, element, NO_CUTTING);
    else
      DrawScreenElementShifted_MM(x, y, 0, MovPos[x][y], element, NO_CUTTING);
  }
  else if (IS_BLOCKED(x, y))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;

    Blocked2Moving(x, y, &oldx, &oldy);

    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    DrawScreenElement_MM(x, y, EL_EMPTY);

    element = Feld[oldx][oldy];

    if (horiz_move)
      DrawScreenElementShifted_MM(sx, sy, MovPos[oldx][oldy], 0, element,
				  NO_CUTTING);
    else
      DrawScreenElementShifted_MM(sx, sy, 0, MovPos[oldx][oldy], element,
				  NO_CUTTING);
  }
  else if (IS_DRAWABLE(element))
  {
    DrawScreenElement_MM(x, y, element);
  }
  else
  {
    DrawScreenElement_MM(x, y, EL_EMPTY);
  }
}

void DrawLevelField_MM(int x, int y)
{
  DrawScreenField_MM(x, y);
}

void DrawMiniElement_MM(int x, int y, int element)
{
  int graphic;

  if (!element)
  {
    DrawMiniGraphic_MM(x, y, IMG_EMPTY);

    return;
  }

  graphic = el2gfx(element);

  DrawMiniGraphic_MM(x, y, graphic);
}

void DrawMiniElementOrWall_MM(int sx, int sy, int scroll_x, int scroll_y)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawMiniElement_MM(sx, sy, EL_EMPTY);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawMiniElement_MM(sx, sy, Feld[x][y]);
}

void DrawField_MM(int x, int y)
{
  int element = Feld[x][y];

  DrawElement_MM(x, y, element);
}

void DrawLevel_MM()
{
  int x,y;

  ClearWindow();

  for (x = 0; x < lev_fieldx; x++)
    for (y = 0; y < lev_fieldy; y++)
      DrawField_MM(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawWallsExt_MM(int x, int y, int element, int draw_mask)
{
  Bitmap *bitmap;
  int graphic = el2gfx(WALL_BASE(element));
  int gx, gy;
  int i;

  getMiniGraphicSource(graphic, &bitmap, &gx, &gy);

  DrawGraphic_MM(x, y, IMG_EMPTY);

  /*
  if (IS_WALL_WOOD(element) || IS_WALL_AMOEBA(element) ||
      IS_DF_WALL_WOOD(element))
    gx += MINI_TILEX;
  if (IS_WALL_ICE(element) || IS_WALL_AMOEBA(element))
    gy += MINI_TILEY;
  */

  for (i = 0; i < 4; i++)
  {
    int dest_x = cSX + x * TILEX + MINI_TILEX * (i % 2);
    int dest_y = cSY + y * TILEY + MINI_TILEY * (i / 2);

    if (!((1 << i) & draw_mask))
      continue;

    if (element & (1 << i))
      BlitBitmap(bitmap, drawto, gx, gy, MINI_TILEX, MINI_TILEY,
		 dest_x, dest_y);
    else
      ClearRectangle(drawto, dest_x, dest_y, MINI_TILEX, MINI_TILEY);
  }

  MarkTileDirty(x, y);
}

void DrawWalls_MM(int x, int y, int element)
{
  DrawWallsExt_MM(x, y, element, HIT_MASK_ALL);
}

void DrawWallsAnimation_MM(int x, int y, int element, int phase, int bit_mask)
{
  int i;

  if (phase == 0)
  {
    DrawWalls_MM(x, y, element);

    return;
  }

  for (i = 0; i < 4; i++)
  {
    if (element & (1 << i))
    {
      int graphic;
      int frame;
      Bitmap *bitmap;
      int src_x, src_y;
      int dst_x = cSX + x * TILEX + (i % 2) * MINI_TILEX;
      int dst_y = cSY + y * TILEY + (i / 2) * MINI_TILEY;

      if (bit_mask & (1 << i))
      {
	graphic = (IS_WALL_AMOEBA(element) ?
		   IMG_MM_AMOEBA_WALL_GROWING :
		   IMG_MM_ICE_WALL_SHRINKING);
	frame = phase;
      }
      else
      {
	graphic = (IS_WALL_AMOEBA(element) ?
		   IMG_MM_AMOEBA_WALL :
		   IMG_MM_ICE_WALL);
	frame = 0;
      }

      getSizedGraphicSource(graphic, frame, MINI_TILESIZE, &bitmap,
			    &src_x, &src_y);

      BlitBitmap(bitmap, drawto, src_x, src_y, MINI_TILEX, MINI_TILEY,
		 dst_x, dst_y);
    }
  }

  MarkTileDirty(x, y);
}

void DrawElement_MM(int x, int y, int element)
{
  if (element == EL_EMPTY)
    DrawGraphic_MM(x, y, IMG_EMPTY);
  else if (IS_WALL(element))
    DrawWalls_MM(x, y, element);
#if 0
  else if (IS_WALL_CHANGING(element) && IS_WALL_CHANGING(Feld[x][y]))
  {
    int wall_element = Feld[x][y] - EL_WALL_CHANGING + Store[x][y];

    DrawWalls_MM(x, y, wall_element);
  }
#endif
  else if (element == EL_PACMAN)
    DrawLevelField_MM(x, y);
  else if (element == EL_FUSE_ON &&
	   laser.fuse_off &&
	   laser.fuse_x == x &&
	   laser.fuse_y == y)
    DrawGraphic_MM(x, y, IMG_MM_FUSE);
  else
    DrawGraphic_MM(x, y, el2gfx(element));
}

void DrawMicroWalls_MM(int x, int y, int element)
{
  Bitmap *bitmap;
  int graphic = el2gfx(WALL_BASE(element));
  int gx, gy;
  int i;

  getMicroGraphicSource(graphic, &bitmap, &gx, &gy);

  for (i = 0; i < 4; i++)
  {
    int xpos = MICROLEV_XPOS + x * MICRO_TILEX + MICRO_WALLX * (i % 2);
    int ypos = MICROLEV_YPOS + y * MICRO_TILEY + MICRO_WALLY * (i / 2);

    if (element & (1 << i))
      BlitBitmap(bitmap, drawto, gx, gy, MICRO_WALLX, MICRO_WALLY, xpos, ypos);
    else
      ClearRectangle(drawto, xpos, ypos, MICRO_WALLX, MICRO_WALLY);
  }
}

void DrawMicroElement_MM(int x, int y, int element)
{
  Bitmap *bitmap;
  int graphic = el2gfx(element);
  int gx, gy;

  if (element == EL_EMPTY)
    return;

  if (IS_WALL(element))
  {
    DrawMicroWalls_MM(x, y, element);

    return;
  }

  getMicroGraphicSource(graphic, &bitmap, &gx, &gy);

  BlitBitmap(bitmap, drawto, gx, gy, MICRO_TILEX, MICRO_TILEY,
	     MICROLEV_XPOS + x * MICRO_TILEX, MICROLEV_YPOS + y * MICRO_TILEY);
}

void DrawMicroLevelExt_MM(int xpos, int ypos)
{
  int x, y;

  ClearRectangle(drawto, xpos, ypos, MICROLEV_XSIZE, MICROLEV_YSIZE);

  for (x = 0; x < STD_LEV_FIELDX; x++)
    for (y = 0; y < STD_LEV_FIELDY; y++)
      DrawMicroElement_MM(x, y, Ur[x][y]);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMiniLevel_MM(int size_x, int size_y, int scroll_x, int scroll_y)
{
  int x, y;

  for(x = 0; x < size_x; x++)
    for(y = 0; y < size_y; y++)
      DrawMiniElementOrWall_MM(x, y, scroll_x, scroll_y);

  redraw_mask |= REDRAW_FIELD;
}

int REQ_in_range(int x, int y)
{
  if (y > DY + 249 && y < DY + 278)
  {
    if (x > DX + 1 && x < DX + 48)
      return 1;
    else if (x > DX + 51 && x < DX + 98)
      return 2;
  }

  return 0;
}

Pixel ReadPixel(DrawBuffer *bitmap, int x, int y)
{
  return GetPixel(bitmap, x, y);
}

void SetRGB(unsigned int pixel,
	    unsigned short red, unsigned short green, unsigned short blue)
{
}

int get_base_element(int element)
{
  if (IS_MIRROR(element))
    return EL_MIRROR_START;
  else if (IS_MIRROR_FIXED(element))
    return EL_MIRROR_FIXED_START;
  else if (IS_POLAR(element))
    return EL_POLAR_START;
  else if (IS_POLAR_CROSS(element))
    return EL_POLAR_CROSS_START;
  else if (IS_BEAMER(element))
    return EL_BEAMER_RED_START + BEAMER_NR(element) * 16;
  else if (IS_FIBRE_OPTIC(element))
    return EL_FIBRE_OPTIC_START + FIBRE_OPTIC_NR(element) * 2;
  else if (IS_MCDUFFIN(element))
    return EL_MCDUFFIN_START;
  else if (IS_LASER(element))
    return EL_LASER_START;
  else if (IS_RECEIVER(element))
    return EL_RECEIVER_START;
  else if (IS_DF_MIRROR(element))
    return EL_DF_MIRROR_START;
  else if (IS_DF_MIRROR_AUTO(element))
    return EL_DF_MIRROR_AUTO_START;
  else if (IS_PACMAN(element))
    return EL_PACMAN_START;
  else if (IS_GRID_STEEL(element))
    return EL_GRID_STEEL_START;
  else if (IS_GRID_WOOD(element))
    return EL_GRID_WOOD_START;
  else if (IS_GRID_STEEL_FIXED(element))
    return EL_GRID_STEEL_FIXED_START;
  else if (IS_GRID_WOOD_FIXED(element))
    return EL_GRID_WOOD_FIXED_START;
  else if (IS_GRID_STEEL_AUTO(element))
    return EL_GRID_STEEL_AUTO_START;
  else if (IS_GRID_WOOD_AUTO(element))
    return EL_GRID_WOOD_AUTO_START;
  else if (IS_WALL_STEEL(element))
    return EL_WALL_STEEL_START;
  else if (IS_WALL_WOOD(element))
    return EL_WALL_WOOD_START;
  else if (IS_WALL_ICE(element))
    return EL_WALL_ICE_START;
  else if (IS_WALL_AMOEBA(element))
    return EL_WALL_AMOEBA_START;
  else if (IS_DF_WALL_STEEL(element))
    return EL_DF_WALL_STEEL_START;
  else if (IS_DF_WALL_WOOD(element))
    return EL_DF_WALL_WOOD_START;
  else if (IS_CHAR(element))
    return EL_CHAR_START;
  else
    return element;
}

int get_element_phase(int element)
{
  return element - get_base_element(element);
}

int get_num_elements(int element)
{
  if (IS_MIRROR(element) ||
      IS_POLAR(element) ||
      IS_BEAMER(element) ||
      IS_DF_MIRROR(element) ||
      IS_DF_MIRROR_AUTO(element))
    return 16;
  else if (IS_GRID_STEEL_FIXED(element) ||
	   IS_GRID_WOOD_FIXED(element) ||
	   IS_GRID_STEEL_AUTO(element) ||
	   IS_GRID_WOOD_AUTO(element))
    return 8;
  else if (IS_MIRROR_FIXED(element) ||
	   IS_POLAR_CROSS(element) ||
	   IS_MCDUFFIN(element) ||
	   IS_LASER(element) ||
	   IS_RECEIVER(element) ||
	   IS_PACMAN(element) ||
	   IS_GRID_STEEL(element) ||
	   IS_GRID_WOOD(element))
    return 4;
  else
    return 1;
}

int get_rotated_element(int element, int step)
{
  int base_element = get_base_element(element);
  int num_elements = get_num_elements(element);
  int element_phase = element - base_element;

  return base_element + (element_phase + step + num_elements) % num_elements;
}

static int map_element(int element)
{
  switch (element)
  {
    case EL_WALL_STEEL:		return EL_STEEL_WALL;
    case EL_WALL_WOOD:		return EL_WOODEN_WALL;
    case EL_WALL_ICE:		return EL_ICE_WALL;
    case EL_WALL_AMOEBA:	return EL_AMOEBA_WALL;
    case EL_DF_WALL_STEEL:	return EL_DF_STEEL_WALL;
    case EL_DF_WALL_WOOD:	return EL_DF_WOODEN_WALL;

    default:			return element;
  }
}

int el2gfx(int element)
{
  element = map_element(element);

  switch (element)
  {
    case EL_LIGHTBALL:
      return IMG_MM_LIGHTBALL_RED + RND(3);

    default:
      return el2img_mm(element);
  }
}

void RedrawPlayfield_MM()
{
  DrawLevel_MM();
  DrawLaser_MM();
}

void BlitScreenToBitmap_MM(Bitmap *target_bitmap)
{
  BlitBitmap(drawto_field, target_bitmap,
	     REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE, REAL_SX, REAL_SY);
}
