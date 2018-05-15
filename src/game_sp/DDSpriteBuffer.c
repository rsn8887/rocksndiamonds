// ----------------------------------------------------------------------------
// DDSpriteBuffer.c
// ----------------------------------------------------------------------------

#include "DDSpriteBuffer.h"


static void Blt(int pX, int pY, Bitmap *bitmap, int SpriteX, int SpriteY)
{
  int scx = (mScrollX_last < 0 ? 0 : mScrollX_last);
  int scy = (mScrollY_last < 0 ? 0 : mScrollY_last);
  int sx1 = scx - 2 * TILEX;
  int sy1 = scy - 2 * TILEY;
  int sx2 = scx + (SCR_FIELDX + 1) * TILEX;
  int sy2 = scy + (SCR_FIELDY + 1) * TILEY;
  int sx = pX - sx1;
  int sy = pY - sy1;

  if (NoDisplayFlag)
    return;

  /* do not draw fields that are outside the visible screen area */
  if (pX < sx1 || pX > sx2 || pY < sy1 || pY > sy2)
    return;

  sx = sx * TILESIZE_VAR / TILESIZE;
  sy = sy * TILESIZE_VAR / TILESIZE;

  BlitBitmap(bitmap, bitmap_db_field_sp, SpriteX, SpriteY,
	     TILEX_VAR, TILEY_VAR, sx, sy);
}

void DDSpriteBuffer_BltImg(int pX, int pY, int graphic, int sync_frame)
{
  struct GraphicInfo_SP g;

  if (NoDisplayFlag)
    return;

  getGraphicSource_SP(&g, graphic, sync_frame, -1, -1);

  Blt(pX, pY, g.bitmap, g.src_x, g.src_y);
}
