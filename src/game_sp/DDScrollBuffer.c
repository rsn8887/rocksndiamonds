// ----------------------------------------------------------------------------
// DDScrollBuffer.c
// ----------------------------------------------------------------------------

#include "DDScrollBuffer.h"

#include <math.h>


int mScrollX, mScrollY;
int mScrollX_last, mScrollY_last;

int ScreenBuffer[2 + MAX_PLAYFIELD_WIDTH + 2][2 + MAX_PLAYFIELD_HEIGHT + 2];


int getFieldbufferOffsetX_SP()
{
  int px = 2 * TILEX + (mScrollX - mScrollX_last) % TILEX;

  /* scroll correction for even number of visible tiles (half tile shifted) */
  px += game_sp.scroll_xoffset;

  if (ExplosionShakeMurphy != 0)
    px += TILEX / 2 - GetSimpleRandom(TILEX + 1);

  px = px * TILESIZE_VAR / TILESIZE;

  return px;
}

int getFieldbufferOffsetY_SP()
{
  int py = 2 * TILEY + (mScrollY - mScrollY_last) % TILEY;

  /* scroll correction for even number of visible tiles (half tile shifted) */
  py += game_sp.scroll_yoffset;

  if (ExplosionShakeMurphy != 0)
    py += TILEY / 2 - GetSimpleRandom(TILEX + 1);

  py = py * TILESIZE_VAR / TILESIZE;

  return py;
}

void RestorePlayfield()
{
  int x1 = mScrollX / TILEX - 2;
  int y1 = mScrollY / TILEY - 2;
  int x2 = mScrollX / TILEX + (SCR_FIELDX - 1) + 2;
  int y2 = mScrollY / TILEY + (SCR_FIELDY - 1) + 2;
  int x, y;

  DrawFrameIfNeeded();

  for (y = DisplayMinY; y <= DisplayMaxY; y++)
  {
    for (x = DisplayMinX; x <= DisplayMaxX; x++)
    {
      if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
      {
	DrawFieldNoAnimated(x, y);
	DrawFieldAnimated(x, y);
      }
    }
  }
}

static void ScrollPlayfield(int dx, int dy)
{
  int x1 = mScrollX_last / TILEX - 2;
  int y1 = mScrollY_last / TILEY - 2;
  int x2 = mScrollX_last / TILEX + (SCR_FIELDX - 1) + 2;
  int y2 = mScrollY_last / TILEY + (SCR_FIELDY - 1) + 2;
  int x, y;

  BlitBitmap(bitmap_db_field_sp, bitmap_db_field_sp,
             TILEX_VAR * (dx == -1),
             TILEY_VAR * (dy == -1),
             (MAX_BUF_XSIZE * TILEX_VAR) - TILEX_VAR * (dx != 0),
             (MAX_BUF_YSIZE * TILEY_VAR) - TILEY_VAR * (dy != 0),
             TILEX_VAR * (dx == 1),
             TILEY_VAR * (dy == 1));

  DrawFrameIfNeeded();

  for (y = DisplayMinY; y <= DisplayMaxY; y++)
  {
    for (x = DisplayMinX; x <= DisplayMaxX; x++)
    {
      if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
      {
	int sx = x - x1;
	int sy = y - y1;
	int tsi = GetSI(x, y);
	int id = ((PlayField16[tsi]) |
		  (PlayField8[tsi] << 16) |
		  (DisPlayField[tsi] << 24));

	if ((dx == -1 && x == x2) ||
	    (dx == +1 && x == x1) ||
	    (dy == -1 && y == y2) ||
	    (dy == +1 && y == y1))
	{
	  DrawFieldNoAnimated(x, y);
	  DrawFieldAnimated(x, y);
	}

	ScreenBuffer[sx][sy] = id;
      }
    }
  }
}

static void ScrollPlayfieldIfNeededExt(boolean reset)
{
  if (reset)
  {
    mScrollX_last = -1;
    mScrollY_last = -1;

    return;
  }

  if (mScrollX_last == -1 || mScrollY_last == -1)
  {
    mScrollX_last = (mScrollX / TILESIZE) * TILESIZE;
    mScrollY_last = (mScrollY / TILESIZE) * TILESIZE;

    return;
  }

  /* check if scrolling the playfield requires redrawing the viewport bitmap */
  if ((mScrollX != mScrollX_last ||
       mScrollY != mScrollY_last) &&
      (ABS(mScrollX - mScrollX_last) >= TILEX ||
       ABS(mScrollY - mScrollY_last) >= TILEY))
  {
    int dx = (ABS(mScrollX - mScrollX_last) < TILEX ? 0 :
	      mScrollX < mScrollX_last ? 1 : mScrollX > mScrollX_last ? -1 : 0);
    int dy = (ABS(mScrollY - mScrollY_last) < TILEY ? 0 :
	      mScrollY < mScrollY_last ? 1 : mScrollY > mScrollY_last ? -1 : 0);

    mScrollX_last -= dx * TILEX;
    mScrollY_last -= dy * TILEY;

    ScrollPlayfield(dx, dy);
  }
}

static void ScrollPlayfieldIfNeeded()
{
  ScrollPlayfieldIfNeededExt(FALSE);
}

void InitScrollPlayfield()
{
  ScrollPlayfieldIfNeededExt(TRUE);
}

#define DEBUG_REDRAW	0

void UpdatePlayfield(boolean force_redraw)
{
  int x, y;

#if DEBUG_REDRAW
  int num_redrawn = 0;
#endif

  if (force_redraw)
  {
    // force re-initialization of graphics status variables
    for (y = DisplayMinY; y <= DisplayMaxY; y++)
      for (x = DisplayMinX; x <= DisplayMaxX; x++)
	GfxGraphic[x][y] = -1;

    // force complete playfield redraw
    DisplayLevel();
  }

  for (y = DisplayMinY; y <= DisplayMaxY; y++)
  {
    for (x = DisplayMinX; x <= DisplayMaxX; x++)
    {
      int element = LowByte(PlayField16[GetSI(x, y)]);
      int graphic = GfxGraphic[x][y];
      int sync_frame = GfxFrame[x][y];
      boolean redraw = force_redraw;

      if (graphic < 0)
      {
	GfxGraphicLast[x][y] = GfxGraphic[x][y];

	continue;
      }

      if (element != GfxElementLast[x][y] &&
	  graphic == GfxGraphicLast[x][y])
      {
	/* element changed, but not graphic => disable updating graphic */

	GfxElementLast[x][y] = element;
	GfxGraphicLast[x][y] = GfxGraphic[x][y] = -1;

	continue;
      }

      if (graphic != GfxGraphicLast[x][y])			// new graphic
      {
	redraw = TRUE;

	GfxElementLast[x][y] = element;
	GfxGraphicLast[x][y] = GfxGraphic[x][y];
	sync_frame = GfxFrame[x][y] = 0;
      }
      else if (isNextAnimationFrame_SP(graphic, sync_frame))	// new frame
      {
	redraw = TRUE;
      }

      if (redraw)
      {
	int sx = x * StretchWidth;
	int sy = y * StretchWidth;

	DDSpriteBuffer_BltImg(sx, sy, graphic, sync_frame);

#if DEBUG_REDRAW
	num_redrawn++;
#endif
      }
    }
  }

#if DEBUG_REDRAW
  printf("::: FRAME %d: %d redrawn\n", FrameCounter, num_redrawn);
#endif
}

void BlitScreenToBitmap_SP(Bitmap *target_bitmap)
{
  /* copy playfield buffer to target bitmap at scroll position */

  int px = getFieldbufferOffsetX_SP();
  int py = getFieldbufferOffsetY_SP();
  int xsize = SXSIZE;
  int ysize = SYSIZE;
  int full_xsize = (FieldWidth  - (menBorder ? 0 : 1)) * TILEX_VAR;
  int full_ysize = (FieldHeight - (menBorder ? 0 : 1)) * TILEY_VAR;
  int sx = SX + (full_xsize < xsize ? (xsize - full_xsize) / 2 : 0);
  int sy = SY + (full_ysize < ysize ? (ysize - full_ysize) / 2 : 0);
  int sxsize = (full_xsize < xsize ? full_xsize : xsize);
  int sysize = (full_ysize < ysize ? full_ysize : ysize);

  BlitBitmap(bitmap_db_field_sp, target_bitmap, px, py, sxsize, sysize, sx, sy);
}

void DDScrollBuffer_ScrollTo(int X, int Y)
{
  if (NoDisplayFlag)
    return;

  ScrollX = mScrollX = X;
  ScrollY = mScrollY = Y;

  ScrollPlayfieldIfNeeded();
}

void DDScrollBuffer_ScrollTowards(int X, int Y, double Step)
{
  double dx, dY, r;

  if (NoDisplayFlag)
    return;

  dx = X - mScrollX;
  dY = Y - mScrollY;

  r = Sqr(dx * dx + dY * dY);
  if (r == 0)	// we are there already
    return;

  if (Step < r)
    r = Step / r;
  else
    r = 1;

  ScrollX = mScrollX = mScrollX + dx * r;
  ScrollY = mScrollY = mScrollY + dY * r;

  ScrollPlayfieldIfNeeded();
}

void DDScrollBuffer_SoftScrollTo(int X, int Y, int TimeMS, int FPS)
{
  double dx, dY;
  int StepCount;
  double T, tStep;
  int oldX, oldY, maxD;
  static boolean AlreadyRunning = False;

  if (NoDisplayFlag)
    return;

  if (AlreadyRunning)
    return;

  AlreadyRunning = True;

  dx = X - mScrollX;
  dY = Y - mScrollY;
  maxD = (Abs(dx) < Abs(dY) ? Abs(dY) : Abs(dx));

  StepCount = FPS * (TimeMS / (double)1000);
  if (StepCount > maxD)
    StepCount = maxD;

  if (StepCount == 0)
    StepCount = 1;

  tStep = (double)1 / StepCount;
  oldX = mScrollX;
  oldY = mScrollY;

  for (T = (double)tStep; T <= (double)1; T += tStep)
  {
    ScrollX = mScrollX = oldX + T * dx;
    ScrollY = mScrollY = oldY + T * dY;
  }

  ScrollX = mScrollX = X;
  ScrollY = mScrollY = Y;

  AlreadyRunning = False;

  ScrollPlayfieldIfNeeded();
}
