/* 2000-08-13T14:36:17Z
 *
 * graphics manipulation crap
 */

#include "main_em.h"

#define MIN_SCREEN_XPOS		1
#define MIN_SCREEN_YPOS		1
#define MAX_SCREEN_XPOS		MAX(1, lev.width  - (SCR_FIELDX - 1))
#define MAX_SCREEN_YPOS		MAX(1, lev.height - (SCR_FIELDY - 1))

#define MIN_SCREEN_X		(MIN_SCREEN_XPOS * TILEX)
#define MIN_SCREEN_Y		(MIN_SCREEN_YPOS * TILEY)
#define MAX_SCREEN_X		(MAX_SCREEN_XPOS * TILEX)
#define MAX_SCREEN_Y		(MAX_SCREEN_YPOS * TILEY)

#define VALID_SCREEN_X(x)	((x) < MIN_SCREEN_X ? MIN_SCREEN_X :	\
				 (x) > MAX_SCREEN_X ? MAX_SCREEN_X : (x))
#define VALID_SCREEN_Y(y)	((y) < MIN_SCREEN_Y ? MIN_SCREEN_Y :	\
				 (y) > MAX_SCREEN_Y ? MAX_SCREEN_Y : (y))

#define PLAYER_SCREEN_X(p)	(((    frame) * ply[p].oldx +		\
				  (8 - frame) * ply[p].x) * TILEX / 8	\
				 - ((SCR_FIELDX - 1) * TILEX) / 2)
#define PLAYER_SCREEN_Y(p)	(((    frame) * ply[p].oldy +		\
				  (8 - frame) * ply[p].y) * TILEY / 8	\
				 - ((SCR_FIELDY - 1) * TILEY) / 2)

#define USE_EXTENDED_GRAPHICS_ENGINE		1


int frame;				/* current screen frame */
int screen_x, screen_y;			/* current scroll position */

/* tiles currently on screen */
static int screentiles[MAX_PLAYFIELD_HEIGHT + 2][MAX_PLAYFIELD_WIDTH + 2];
static int crumbled_state[MAX_PLAYFIELD_HEIGHT + 2][MAX_PLAYFIELD_WIDTH + 2];

int getFieldbufferOffsetX_EM()
{
  return screen_x % TILEX;
}

int getFieldbufferOffsetY_EM()
{
  return screen_y % TILEY;
}

void BlitScreenToBitmap_EM(Bitmap *target_bitmap)
{
  /* blit all (up to four) parts of the scroll buffer to the target bitmap */

  int x = screen_x % (MAX_BUF_XSIZE * TILEX);
  int y = screen_y % (MAX_BUF_YSIZE * TILEY);
  int xsize = SXSIZE;
  int ysize = SYSIZE;
  int full_xsize = lev.width  * TILEX;
  int full_ysize = lev.height * TILEY;
  int sx = SX + (full_xsize < xsize ? (xsize - full_xsize) / 2 : 0);
  int sy = SY + (full_ysize < ysize ? (ysize - full_ysize) / 2 : 0);
  int sxsize = (full_xsize < xsize ? full_xsize : xsize);
  int sysize = (full_ysize < ysize ? full_ysize : ysize);

  if (x < 2 * TILEX && y < 2 * TILEY)
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       sxsize, sysize, sx, sy);
  }
  else if (x < 2 * TILEX && y >= 2 * TILEY)
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       sxsize, MAX_BUF_YSIZE * TILEY - y,
	       sx, sy);
    BlitBitmap(screenBitmap, target_bitmap, x, 0,
	       sxsize, y - 2 * TILEY,
	       sx, sy + MAX_BUF_YSIZE * TILEY - y);
  }
  else if (x >= 2 * TILEX && y < 2 * TILEY)
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       MAX_BUF_XSIZE * TILEX - x, sysize,
	       sx, sy);
    BlitBitmap(screenBitmap, target_bitmap, 0, y,
	       x - 2 * TILEX, sysize,
	       sx + MAX_BUF_XSIZE * TILEX - x, sy);
  }
  else
  {
    BlitBitmap(screenBitmap, target_bitmap, x, y,
	       MAX_BUF_XSIZE * TILEX - x, MAX_BUF_YSIZE * TILEY - y,
	       sx, sy);
    BlitBitmap(screenBitmap, target_bitmap, 0, y,
	       x - 2 * TILEX, MAX_BUF_YSIZE * TILEY - y,
	       sx + MAX_BUF_XSIZE * TILEX - x, sy);
    BlitBitmap(screenBitmap, target_bitmap, x, 0,
	       MAX_BUF_XSIZE * TILEX - x, y - 2 * TILEY,
	       sx, sy + MAX_BUF_YSIZE * TILEY - y);
    BlitBitmap(screenBitmap, target_bitmap, 0, 0,
	       x - 2 * TILEX, y - 2 * TILEY,
	       sx + MAX_BUF_XSIZE * TILEX - x, sy + MAX_BUF_YSIZE * TILEY - y);
  }
}

void BackToFront_EM(void)
{
  BlitBitmap(backbuffer, window, SX, SY, SXSIZE, SYSIZE, SX, SY);
}

static struct GraphicInfo_EM *getObjectGraphic(int x, int y)
{
  int tile = Draw[y][x];
  struct GraphicInfo_EM *g = &graphic_info_em_object[tile][frame];

  if (!game.use_native_emc_graphics_engine)
    getGraphicSourceObjectExt_EM(g, tile, 7 - frame, x - 2, y - 2);

  return g;
}

static struct GraphicInfo_EM *getPlayerGraphic(int player_nr, int anim)
{
  struct GraphicInfo_EM *g = &graphic_info_em_player[player_nr][anim][frame];

  if (!game.use_native_emc_graphics_engine)
    getGraphicSourcePlayerExt_EM(g, player_nr, anim, 7 - frame);

  return g;
}

static void DrawLevelField_EM(int x, int y, int sx, int sy,
			      boolean draw_masked)
{
  struct GraphicInfo_EM *g = getObjectGraphic(x, y);
  int src_x = g->src_x + g->src_offset_x * TILESIZE_VAR / TILESIZE;
  int src_y = g->src_y + g->src_offset_y * TILESIZE_VAR / TILESIZE;
  int dst_x = sx * TILEX + g->dst_offset_x * TILESIZE_VAR / TILESIZE;
  int dst_y = sy * TILEY + g->dst_offset_y * TILESIZE_VAR / TILESIZE;
  int width = g->width * TILESIZE_VAR / TILESIZE;
  int height = g->height * TILESIZE_VAR / TILESIZE;
  int left = screen_x / TILEX;
  int top  = screen_y / TILEY;

  /* do not draw fields that are outside the visible screen area */
  if (x < left || x >= left + MAX_BUF_XSIZE ||
      y < top  || y >= top  + MAX_BUF_YSIZE)
    return;

  if (draw_masked)
  {
    if (width > 0 && height > 0)
      BlitBitmapMasked(g->bitmap, screenBitmap,
		       src_x, src_y, width, height, dst_x, dst_y);
  }
  else
  {
    if ((width != TILEX || height != TILEY) && !g->preserve_background)
      ClearRectangle(screenBitmap, sx * TILEX, sy * TILEY, TILEX, TILEY);

    if (width > 0 && height > 0)
      BlitBitmap(g->bitmap, screenBitmap,
		 src_x, src_y, width, height, dst_x, dst_y);
  }
}

static void DrawLevelFieldCrumbled_EM(int x, int y, int sx, int sy,
				      int crm, boolean draw_masked)
{
  struct GraphicInfo_EM *g;
  int crumbled_border_size;
  int left = screen_x / TILEX;
  int top  = screen_y / TILEY;
  int i;

  /* do not draw fields that are outside the visible screen area */
  if (x < left || x >= left + MAX_BUF_XSIZE ||
      y < top  || y >= top  + MAX_BUF_YSIZE)
    return;

  if (crm == 0)		/* no crumbled edges for this tile */
    return;

  g = getObjectGraphic(x, y);

  crumbled_border_size =
    g->crumbled_border_size * TILESIZE_VAR / g->crumbled_tile_size;

  for (i = 0; i < 4; i++)
  {
    if (crm & (1 << i))
    {
      int width, height, cx, cy;

      if (i == 1 || i == 2)
      {
	width = crumbled_border_size;
	height = TILEY;
	cx = (i == 2 ? TILEX - crumbled_border_size : 0);
	cy = 0;
      }
      else
      {
	width = TILEX;
	height = crumbled_border_size;
	cx = 0;
	cy = (i == 3 ? TILEY - crumbled_border_size : 0);
      }

      if (width > 0 && height > 0)
      {
	int src_x = g->crumbled_src_x + cx;
	int src_y = g->crumbled_src_y + cy;
	int dst_x = sx * TILEX + cx;
	int dst_y = sy * TILEY + cy;

	if (draw_masked)
	  BlitBitmapMasked(g->crumbled_bitmap, screenBitmap,
			   src_x, src_y, width, height, dst_x, dst_y);
	else
	  BlitBitmap(g->crumbled_bitmap, screenBitmap,
		     src_x, src_y, width, height, dst_x, dst_y);
      }
    }
  }
}

static void DrawLevelPlayer_EM(int x1, int y1, int player_nr, int anim,
			       boolean draw_masked)
{
  struct GraphicInfo_EM *g = getPlayerGraphic(player_nr, anim);
  int src_x = g->src_x, src_y = g->src_y;
  int dst_x, dst_y;

  /* do not draw fields that are outside the visible screen area */
  if (x1 < screen_x - TILEX || x1 >= screen_x + MAX_BUF_XSIZE * TILEX ||
      y1 < screen_y - TILEY || y1 >= screen_y + MAX_BUF_YSIZE * TILEY)
    return;

  x1 %= MAX_BUF_XSIZE * TILEX;
  y1 %= MAX_BUF_YSIZE * TILEY;

  if (draw_masked)
  {
    /* draw the player to current location */
    dst_x = x1;
    dst_y = y1;
    BlitBitmapMasked(g->bitmap, screenBitmap,
		     src_x, src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around column */
    dst_x = x1 - MAX_BUF_XSIZE * TILEX;
    dst_y = y1;
    BlitBitmapMasked(g->bitmap, screenBitmap,
		     g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around row */
    dst_x = x1;
    dst_y = y1 - MAX_BUF_YSIZE * TILEY;
    BlitBitmapMasked(g->bitmap, screenBitmap,
		     g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);
  }
  else
  {
    /* draw the player to current location */
    dst_x = x1;
    dst_y = y1;
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around column */
    dst_x = x1 - MAX_BUF_XSIZE * TILEX;
    dst_y = y1;
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);

    /* draw the player to opposite wrap-around row */
    dst_x = x1;
    dst_y = y1 - MAX_BUF_YSIZE * TILEY;
    BlitBitmap(g->bitmap, screenBitmap,
	       g->src_x, g->src_y, TILEX, TILEY, dst_x, dst_y);
  }
}

/* draw differences between game tiles and screen tiles
 *
 * implicitly handles scrolling and restoring background under the sprites
 */

static void animscreen(void)
{
  int x, y, i;
  int left = screen_x / TILEX;
  int top  = screen_y / TILEY;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!game.use_native_emc_graphics_engine)
    for (y = 2; y < EM_MAX_CAVE_HEIGHT - 2; y++)
      for (x = 2; x < EM_MAX_CAVE_WIDTH - 2; x++)
	SetGfxAnimation_EM(&graphic_info_em_object[Draw[y][x]][frame],
			   Draw[y][x], 7 - frame, x - 2, y - 2);

  for (y = top; y < top + MAX_BUF_YSIZE; y++)
  {
    for (x = left; x < left + MAX_BUF_XSIZE; x++)
    {
      int sx = x % MAX_BUF_XSIZE;
      int sy = y % MAX_BUF_YSIZE;    
      int tile = Draw[y][x];
      struct GraphicInfo_EM *g = &graphic_info_em_object[tile][frame];
      int obj = g->unique_identifier;
      int crm = 0;
      boolean redraw_screen_tile = FALSE;

      /* re-calculate crumbled state of this tile */
      if (g->has_crumbled_graphics)
      {
	for (i = 0; i < 4; i++)
	{
	  int xx = x + xy[i][0];
	  int yy = y + xy[i][1];
	  int tile_next;

	  if (xx < 0 || xx >= EM_MAX_CAVE_WIDTH ||
	      yy < 0 || yy >= EM_MAX_CAVE_HEIGHT)
	    continue;

	  tile_next = Draw[yy][xx];

	  if (!graphic_info_em_object[tile_next][frame].has_crumbled_graphics)
	    crm |= (1 << i);
	}
      }

      redraw_screen_tile = (screentiles[sy][sx]    != obj ||
			    crumbled_state[sy][sx] != crm);

      /* only redraw screen tiles if they (or their crumbled state) changed */
      if (redraw_screen_tile)
      {
	DrawLevelField_EM(x, y, sx, sy, FALSE);
	DrawLevelFieldCrumbled_EM(x, y, sx, sy, crm, FALSE);

	screentiles[sy][sx] = obj;
	crumbled_state[sy][sx] = crm;
      }
    }
  }
}


/* blit players to the screen
 *
 * handles transparency and movement
 */

static void blitplayer(struct PLAYER *ply)
{
  int x1, y1, x2, y2;

  if (!ply->alive)
    return;

  /* x1/y1 are left/top and x2/y2 are right/down part of the player movement */
  x1 = (frame * ply->oldx + (8 - frame) * ply->x) * TILEX / 8;
  y1 = (frame * ply->oldy + (8 - frame) * ply->y) * TILEY / 8;
  x2 = x1 + TILEX - 1;
  y2 = y1 + TILEY - 1;

  if ((int)(x2 - screen_x) < ((MAX_BUF_XSIZE - 1) * TILEX - 1) &&
      (int)(y2 - screen_y) < ((MAX_BUF_YSIZE - 1) * TILEY - 1))
  {
    /* some casts to "int" are needed because of negative calculation values */
    int dx = (int)ply->x - (int)ply->oldx;
    int dy = (int)ply->y - (int)ply->oldy;
    int old_x = (int)ply->oldx + (7 - (int)frame) * dx / 8;
    int old_y = (int)ply->oldy + (7 - (int)frame) * dy / 8;
    int new_x = old_x + SIGN(dx);
    int new_y = old_y + SIGN(dy);
    int old_sx = old_x % MAX_BUF_XSIZE;
    int old_sy = old_y % MAX_BUF_YSIZE;
    int new_sx = new_x % MAX_BUF_XSIZE;
    int new_sy = new_y % MAX_BUF_YSIZE;
    int new_crm = crumbled_state[new_sy][new_sx];

    /* only diggable elements can be crumbled in the classic EM engine */
    boolean player_is_digging = (new_crm != 0);

    if (player_is_digging)
    {
      /* draw the field the player is moving to (under the player) */
      DrawLevelField_EM(new_x, new_y, new_sx, new_sy, FALSE);
      DrawLevelFieldCrumbled_EM(new_x, new_y, new_sx, new_sy, new_crm, FALSE);

      /* draw the player (masked) over the element he is just digging away */
      DrawLevelPlayer_EM(x1, y1, ply->num, ply->anim, TRUE);

      /* draw the field the player is moving from (masked over the player) */
      DrawLevelField_EM(old_x, old_y, old_sx, old_sy, TRUE);
    }
    else
    {
      /* draw the player under the element which is on the same field */
      DrawLevelPlayer_EM(x1, y1, ply->num, ply->anim, FALSE);

      /* draw the field the player is moving from (masked over the player) */
      DrawLevelField_EM(old_x, old_y, old_sx, old_sy, TRUE);

      /* draw the field the player is moving to (masked over the player) */
      DrawLevelField_EM(new_x, new_y, new_sx, new_sy, TRUE);
    }

    /* redraw screen tiles in the next frame (player may have left the tiles) */
    screentiles[old_sy][old_sx] = -1;
    screentiles[new_sy][new_sx] = -1;
  }
}

void game_initscreen(void)
{
  int player_nr;
  int x,y;

  frame = 6;

  player_nr = (game.centered_player_nr != -1 ? game.centered_player_nr : 0);

  screen_x = VALID_SCREEN_X(PLAYER_SCREEN_X(player_nr));
  screen_y = VALID_SCREEN_Y(PLAYER_SCREEN_Y(player_nr));

  for (y = 0; y < MAX_BUF_YSIZE; y++)
  {
    for (x = 0; x < MAX_BUF_XSIZE; x++)
    {
      screentiles[y][x] = -1;
      crumbled_state[y][x] = 0;
    }
  }
}

static int getMaxCenterDistancePlayerNr(int center_x, int center_y)
{
  int max_dx = 0, max_dy = 0;
  int player_nr = game_em.last_moving_player;
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (ply[i].alive)
    {
      int sx = PLAYER_SCREEN_X(i);
      int sy = PLAYER_SCREEN_Y(i);

      if (game_em.last_player_direction[i] != MV_NONE &&
	  (ABS(sx - center_x) > max_dx ||
	   ABS(sy - center_y) > max_dy))
      {
	max_dx = MAX(max_dx, ABS(sx - center_x));
	max_dy = MAX(max_dy, ABS(sy - center_y));

	player_nr = i;
      }
    }
  }

  return player_nr;
}

static void setMinimalPlayerBoundaries(int *sx1, int *sy1, int *sx2, int *sy2)
{
  boolean num_checked_players = 0;
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (ply[i].alive)
    {
      int sx = PLAYER_SCREEN_X(i);
      int sy = PLAYER_SCREEN_Y(i);

      if (num_checked_players == 0)
      {
	*sx1 = *sx2 = sx;
	*sy1 = *sy2 = sy;
      }
      else
      {
	*sx1 = MIN(*sx1, sx);
	*sy1 = MIN(*sy1, sy);
	*sx2 = MAX(*sx2, sx);
	*sy2 = MAX(*sy2, sy);
      }

      num_checked_players++;
    }
  }
}

boolean checkIfAllPlayersFitToScreen()
{
  int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;

  setMinimalPlayerBoundaries(&sx1, &sy1, &sx2, &sy2);

  return (sx2 - sx1 <= SCR_FIELDX * TILEX &&
	  sy2 - sy1 <= SCR_FIELDY * TILEY);
}

static void setScreenCenteredToAllPlayers(int *sx, int *sy)
{
  int sx1 = screen_x, sy1 = screen_y, sx2 = screen_x, sy2 = screen_y;

  setMinimalPlayerBoundaries(&sx1, &sy1, &sx2, &sy2);

  *sx = (sx1 + sx2) / 2;
  *sy = (sy1 + sy2) / 2;
}

static void setMaxCenterDistanceForAllPlayers(int *max_dx, int *max_dy,
					      int center_x, int center_y)
{
  int sx1 = center_x, sy1 = center_y, sx2 = center_x, sy2 = center_y;

  setMinimalPlayerBoundaries(&sx1, &sy1, &sx2, &sy2);

  *max_dx = MAX(ABS(sx1 - center_x), ABS(sx2 - center_x));
  *max_dy = MAX(ABS(sy1 - center_y), ABS(sy2 - center_y));
}

static boolean checkIfAllPlayersAreVisible(int center_x, int center_y)
{
  int max_dx, max_dy;

  setMaxCenterDistanceForAllPlayers(&max_dx, &max_dy, center_x, center_y);

  return (max_dx <= SCR_FIELDX * TILEX / 2 &&
	  max_dy <= SCR_FIELDY * TILEY / 2);
}

void RedrawPlayfield_EM(boolean force_redraw)
{
  boolean draw_new_player_location = FALSE;
  boolean quick_relocation = setup.quick_switch;
  int max_center_distance_player_nr =
    getMaxCenterDistancePlayerNr(screen_x, screen_y);
  int stepsize = TILEX / 8;
  int offset = game.scroll_delay_value * TILEX;
  int offset_x = offset;
  int offset_y = offset;
  int screen_x_old = screen_x;
  int screen_y_old = screen_y;
  int x, y, sx, sy;
  int i;

  if (game.set_centered_player)
  {
    boolean all_players_fit_to_screen = checkIfAllPlayersFitToScreen();

    /* switching to "all players" only possible if all players fit to screen */
    if (game.centered_player_nr_next == -1 && !all_players_fit_to_screen)
    {
      game.centered_player_nr_next = game.centered_player_nr;
      game.set_centered_player = FALSE;
    }

    /* do not switch focus to non-existing (or non-active) player */
    if (game.centered_player_nr_next >= 0 &&
	!ply[game.centered_player_nr_next].alive)
    {
      game.centered_player_nr_next = game.centered_player_nr;
      game.set_centered_player = FALSE;
    }
  }

  /* also allow focus switching when screen is scrolled to half tile */
  if (game.set_centered_player)
  {
    game.centered_player_nr = game.centered_player_nr_next;

    draw_new_player_location = TRUE;
    force_redraw = TRUE;

    game.set_centered_player = FALSE;
  }

  if (game.centered_player_nr == -1)
  {
    if (draw_new_player_location || offset == 0)
    {
      setScreenCenteredToAllPlayers(&sx, &sy);
    }
    else
    {
      sx = PLAYER_SCREEN_X(max_center_distance_player_nr);
      sy = PLAYER_SCREEN_Y(max_center_distance_player_nr);
    }
  }
  else
  {
    sx = PLAYER_SCREEN_X(game.centered_player_nr);
    sy = PLAYER_SCREEN_Y(game.centered_player_nr);
  }

  if (draw_new_player_location && quick_relocation)
  {
    screen_x = VALID_SCREEN_X(sx);
    screen_y = VALID_SCREEN_Y(sy);
    screen_x_old = screen_x;
    screen_y_old = screen_y;
  }

  if (draw_new_player_location && !quick_relocation)
  {
    unsigned int game_frame_delay_value = getGameFrameDelay_EM(20);
    int wait_delay_value = game_frame_delay_value;
    int screen_xx = VALID_SCREEN_X(sx);
    int screen_yy = VALID_SCREEN_Y(sy);

    while (screen_x != screen_xx || screen_y != screen_yy)
    {
      int dx = (screen_xx < screen_x ? +1 : screen_xx > screen_x ? -1 : 0);
      int dy = (screen_yy < screen_y ? +1 : screen_yy > screen_y ? -1 : 0);
      int dxx = 0, dyy = 0;

      if (dx == 0 && dy == 0)		/* no scrolling needed at all */
	break;

      if (ABS(screen_xx - screen_x) >= TILEX)
      {
	screen_x -= dx * TILEX;
	dxx = dx * TILEX / 2;
      }
      else
      {
	screen_x = screen_xx;
	dxx = 0;
      }

      if (ABS(screen_yy - screen_y) >= TILEY)
      {
	screen_y -= dy * TILEY;
	dyy = dy * TILEY / 2;
      }
      else
      {
	screen_y = screen_yy;
	dyy = 0;
      }

      /* scroll in two steps of half tile size to make things smoother */
      screen_x += dxx;
      screen_y += dyy;

      animscreen();

      for (i = 0; i < MAX_PLAYERS; i++)
	blitplayer(&ply[i]);

      BlitScreenToBitmap_EM(backbuffer);
      BackToFront_EM();

      Delay(wait_delay_value);

      /* scroll second step to align at full tile size */
      screen_x -= dxx;
      screen_y -= dyy;

      animscreen();

      for (i = 0; i < MAX_PLAYERS; i++)
	blitplayer(&ply[i]);

      BlitScreenToBitmap_EM(backbuffer);
      BackToFront_EM();

      Delay(wait_delay_value);
    }

    screen_x_old = screen_x;
    screen_y_old = screen_y;
  }

  if (force_redraw)
  {
    for (y = 0; y < MAX_BUF_YSIZE; y++)
    {
      for (x = 0; x < MAX_BUF_XSIZE; x++)
      {
	screentiles[y][x] = -1;
	crumbled_state[y][x] = 0;
      }
    }
  }

  /* calculate new screen scrolling position, with regard to scroll delay */
  screen_x = VALID_SCREEN_X(sx + offset_x < screen_x ? sx + offset_x :
			    sx - offset_x > screen_x ? sx - offset_x :
			    screen_x);
  screen_y = VALID_SCREEN_Y(sy + offset_y < screen_y ? sy + offset_y :
			    sy - offset_y > screen_y ? sy - offset_y :
			    screen_y);

  /* prevent scrolling further than double player step size when scrolling */
  if (ABS(screen_x - screen_x_old) > 2 * stepsize)
  {
    int dx = SIGN(screen_x - screen_x_old);

    screen_x = screen_x_old + dx * 2 * stepsize;
  }
  if (ABS(screen_y - screen_y_old) > 2 * stepsize)
  {
    int dy = SIGN(screen_y - screen_y_old);

    screen_y = screen_y_old + dy * 2 * stepsize;
  }

  /* prevent scrolling away from the other players when focus on all players */
  if (game.centered_player_nr == -1)
  {
    /* check if all players are still visible with new scrolling position */
    if (checkIfAllPlayersAreVisible(screen_x_old, screen_y_old) &&
	!checkIfAllPlayersAreVisible(screen_x, screen_y))
    {
      /* reset horizontal scroll position to last value, if needed */
      if (!checkIfAllPlayersAreVisible(screen_x, screen_y_old))
	screen_x = screen_x_old;

      /* reset vertical scroll position to last value, if needed */
      if (!checkIfAllPlayersAreVisible(screen_x_old, screen_y))
	screen_y = screen_y_old;
    }
  }

  /* prevent scrolling (for screen correcting) if no player is moving */
  if (!game_em.any_player_moving)
  {
    screen_x = screen_x_old;
    screen_y = screen_y_old;
  }
  else
  {
    /* prevent scrolling against the players move direction */
    int player_nr = (game.centered_player_nr == -1 ?
		     max_center_distance_player_nr : game.centered_player_nr);
    int player_move_dir = game_em.last_player_direction[player_nr];
    int dx = SIGN(screen_x - screen_x_old);
    int dy = SIGN(screen_y - screen_y_old);

    if ((dx < 0 && player_move_dir != MV_LEFT) ||
	(dx > 0 && player_move_dir != MV_RIGHT))
      screen_x = screen_x_old;

    if ((dy < 0 && player_move_dir != MV_UP) ||
	(dy > 0 && player_move_dir != MV_DOWN))
      screen_y = screen_y_old;
  }

  // skip redrawing playfield in warp mode or when testing tapes with "autotest"
  if (DrawingDeactivatedField())
    return;

  animscreen();

  for (i = 0; i < MAX_PLAYERS; i++)
    blitplayer(&ply[i]);
}
