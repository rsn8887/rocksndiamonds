// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// tools.c
// ============================================================================

#include <math.h>

#include "libgame/libgame.h"

#include "tools.h"
#include "init.h"
#include "game.h"
#include "events.h"
#include "anim.h"
#include "network.h"
#include "tape.h"
#include "screens.h"


/* select level set with EMC X11 graphics before activating EM GFX debugging */
#define DEBUG_EM_GFX		FALSE
#define DEBUG_FRAME_TIME	FALSE

/* tool button identifiers */
#define TOOL_CTRL_ID_YES	0
#define TOOL_CTRL_ID_NO		1
#define TOOL_CTRL_ID_CONFIRM	2
#define TOOL_CTRL_ID_PLAYER_1	3
#define TOOL_CTRL_ID_PLAYER_2	4
#define TOOL_CTRL_ID_PLAYER_3	5
#define TOOL_CTRL_ID_PLAYER_4	6

#define NUM_TOOL_BUTTONS	7

/* constants for number of doors and door parts */
#define NUM_DOORS		2
#define NUM_PANELS		NUM_DOORS
// #define NUM_PANELS		0
#define MAX_PARTS_PER_DOOR	8
#define MAX_DOOR_PARTS		(NUM_DOORS * MAX_PARTS_PER_DOOR + NUM_PANELS)
#define DOOR_PART_IS_PANEL(i)	((i) >= NUM_DOORS * MAX_PARTS_PER_DOOR)


struct DoorPartOrderInfo
{
  int nr;
  int sort_priority;
};

static struct DoorPartOrderInfo door_part_order[MAX_DOOR_PARTS];

struct DoorPartControlInfo
{
  int door_token;
  int graphic;
  struct DoorPartPosInfo *pos;
};

static struct DoorPartControlInfo door_part_controls[] =
{
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_1,
    &door_1.part_1
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_2,
    &door_1.part_2
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_3,
    &door_1.part_3
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_4,
    &door_1.part_4
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_5,
    &door_1.part_5
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_6,
    &door_1.part_6
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_7,
    &door_1.part_7
  },
  {
    DOOR_1,
    IMG_GFX_DOOR_1_PART_8,
    &door_1.part_8
  },

  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_1,
    &door_2.part_1
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_2,
    &door_2.part_2
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_3,
    &door_2.part_3
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_4,
    &door_2.part_4
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_5,
    &door_2.part_5
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_6,
    &door_2.part_6
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_7,
    &door_2.part_7
  },
  {
    DOOR_2,
    IMG_GFX_DOOR_2_PART_8,
    &door_2.part_8
  },

  {
    DOOR_1,
    IMG_BACKGROUND_PANEL,
    &door_1.panel
  },
  {
    DOOR_2,
    IMG_BACKGROUND_TAPE,
    &door_2.panel
  },

  {
    -1,
    -1,
    NULL
  }
};


/* forward declaration for internal use */
static void UnmapToolButtons();
static void HandleToolButtons(struct GadgetInfo *);
static int el_act_dir2crm(int, int, int);
static int el_act2crm(int, int);

static struct GadgetInfo *tool_gadget[NUM_TOOL_BUTTONS];
static int request_gadget_id = -1;

static char *print_if_not_empty(int element)
{
  static char *s = NULL;
  char *token_name = element_info[element].token_name;

  if (s != NULL)
    free(s);

  s = checked_malloc(strlen(token_name) + 10 + 1);

  if (element != EL_EMPTY)
    sprintf(s, "%d\t['%s']", element, token_name);
  else
    sprintf(s, "%d", element);

  return s;
}

int correctLevelPosX_EM(int lx)
{
  lx -= 1;
  lx -= (BorderElement != EL_EMPTY ? 1 : 0);

  return lx;
}

int correctLevelPosY_EM(int ly)
{
  ly -= 1;
  ly -= (BorderElement != EL_EMPTY ? 1 : 0);

  return ly;
}

static int getFieldbufferOffsetX_RND()
{
  int full_lev_fieldx = lev_fieldx + (BorderElement != EL_EMPTY ? 2 : 0);
  int dx = (ScreenMovDir & (MV_LEFT | MV_RIGHT) ? ScreenGfxPos : 0);
  int dx_var = dx * TILESIZE_VAR / TILESIZE;
  int fx = FX;

  if (EVEN(SCR_FIELDX))
  {
    int ffx = (scroll_x - SBX_Left)  * TILEX_VAR + dx_var;

    if (ffx < SBX_Right * TILEX_VAR + TILEX_VAR / 2 + TILEX_VAR)
      fx += dx_var - MIN(ffx, TILEX_VAR / 2) + TILEX_VAR;
    else
      fx += (dx_var > 0 ? TILEX_VAR : 0);
  }
  else
  {
    fx += dx_var;
  }

  if (full_lev_fieldx <= SCR_FIELDX)
  {
    if (EVEN(SCR_FIELDX))
      fx = 2 * TILEX_VAR - (ODD(lev_fieldx)  ? TILEX_VAR / 2 : 0);
    else
      fx = 2 * TILEX_VAR - (EVEN(lev_fieldx) ? TILEX_VAR / 2 : 0);
  }

  return fx;
}

static int getFieldbufferOffsetY_RND()
{
  int full_lev_fieldy = lev_fieldy + (BorderElement != EL_EMPTY ? 2 : 0);
  int dy = (ScreenMovDir & (MV_UP | MV_DOWN)    ? ScreenGfxPos : 0);
  int dy_var = dy * TILESIZE_VAR / TILESIZE;
  int fy = FY;

  if (EVEN(SCR_FIELDY))
  {
    int ffy = (scroll_y - SBY_Upper) * TILEY_VAR + dy_var;

    if (ffy < SBY_Lower * TILEY_VAR + TILEY_VAR / 2 + TILEY_VAR)
      fy += dy_var - MIN(ffy, TILEY_VAR / 2) + TILEY_VAR;
    else
      fy += (dy_var > 0 ? TILEY_VAR : 0);
  }
  else
  {
    fy += dy_var;
  }

  if (full_lev_fieldy <= SCR_FIELDY)
  {
    if (EVEN(SCR_FIELDY))
      fy = 2 * TILEY_VAR - (ODD(lev_fieldy)  ? TILEY_VAR / 2 : 0);
    else
      fy = 2 * TILEY_VAR - (EVEN(lev_fieldy) ? TILEY_VAR / 2 : 0);
  }

  return fy;
}

static int getLevelFromScreenX_RND(int sx)
{
  int fx = getFieldbufferOffsetX_RND();
  int dx = fx - FX;
  int px = sx - SX;
  int lx = LEVELX((px + dx) / TILESIZE_VAR);

  return lx;
}

static int getLevelFromScreenY_RND(int sy)
{
  int fy = getFieldbufferOffsetY_RND();
  int dy = fy - FY;
  int py = sy - SY;
  int ly = LEVELY((py + dy) / TILESIZE_VAR);

  return ly;
}

static int getLevelFromScreenX_EM(int sx)
{
  int level_xsize = level.native_em_level->lev->width;
  int full_xsize = level_xsize * TILESIZE_VAR;

  sx -= (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);

  int fx = getFieldbufferOffsetX_EM();
  int dx = fx;
  int px = sx - SX;
  int lx = LEVELX((px + dx) / TILESIZE_VAR);

  lx = correctLevelPosX_EM(lx);

  return lx;
}

static int getLevelFromScreenY_EM(int sy)
{
  int level_ysize = level.native_em_level->lev->height;
  int full_ysize = level_ysize * TILESIZE_VAR;

  sy -= (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  int fy = getFieldbufferOffsetY_EM();
  int dy = fy;
  int py = sy - SY;
  int ly = LEVELY((py + dy) / TILESIZE_VAR);

  ly = correctLevelPosY_EM(ly);

  return ly;
}

static int getLevelFromScreenX_SP(int sx)
{
  int menBorder = setup.sp_show_border_elements;
  int level_xsize = level.native_sp_level->width;
  int full_xsize = (level_xsize - (menBorder ? 0 : 1)) * TILESIZE_VAR;

  sx += (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);

  int fx = getFieldbufferOffsetX_SP();
  int dx = fx - FX;
  int px = sx - SX;
  int lx = LEVELX((px + dx) / TILESIZE_VAR);

  return lx;
}

static int getLevelFromScreenY_SP(int sy)
{
  int menBorder = setup.sp_show_border_elements;
  int level_ysize = level.native_sp_level->height;
  int full_ysize = (level_ysize - (menBorder ? 0 : 1)) * TILESIZE_VAR;

  sy += (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  int fy = getFieldbufferOffsetY_SP();
  int dy = fy - FY;
  int py = sy - SY;
  int ly = LEVELY((py + dy) / TILESIZE_VAR);

  return ly;
}

static int getLevelFromScreenX_MM(int sx)
{
  int level_xsize = level.native_mm_level->fieldx;
  int full_xsize = level_xsize * TILESIZE_VAR;

  sx -= (full_xsize < SXSIZE ? (SXSIZE - full_xsize) / 2 : 0);

  int px = sx - SX;
  int lx = (px + TILESIZE_VAR) / TILESIZE_VAR - 1;

  return lx;
}

static int getLevelFromScreenY_MM(int sy)
{
  int level_ysize = level.native_mm_level->fieldy;
  int full_ysize = level_ysize * TILESIZE_VAR;

  sy -= (full_ysize < SYSIZE ? (SYSIZE - full_ysize) / 2 : 0);

  int py = sy - SY;
  int ly = (py + TILESIZE_VAR) / TILESIZE_VAR - 1;

  return ly;
}

int getLevelFromScreenX(int x)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return getLevelFromScreenX_EM(x);
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return getLevelFromScreenX_SP(x);
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return getLevelFromScreenX_MM(x);
  else
    return getLevelFromScreenX_RND(x);
}

int getLevelFromScreenY(int y)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return getLevelFromScreenY_EM(y);
  if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return getLevelFromScreenY_SP(y);
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return getLevelFromScreenY_MM(y);
  else
    return getLevelFromScreenY_RND(y);
}

void DumpTile(int x, int y)
{
  int sx = SCREENX(x);
  int sy = SCREENY(y);
  char *token_name;

  printf_line("-", 79);
  printf("Field Info: SCREEN(%d, %d), LEVEL(%d, %d)\n", sx, sy, x, y);
  printf_line("-", 79);

  if (!IN_LEV_FIELD(x, y))
  {
    printf("(not in level field)\n");
    printf("\n");

    return;
  }

  token_name = element_info[Feld[x][y]].token_name;

  printf("  Feld:        %d\t['%s']\n", Feld[x][y], token_name);
  printf("  Back:        %s\n", print_if_not_empty(Back[x][y]));
  printf("  Store:       %s\n", print_if_not_empty(Store[x][y]));
  printf("  Store2:      %s\n", print_if_not_empty(Store2[x][y]));
  printf("  StorePlayer: %s\n", print_if_not_empty(StorePlayer[x][y]));
  printf("  MovPos:      %d\n", MovPos[x][y]);
  printf("  MovDir:      %d\n", MovDir[x][y]);
  printf("  MovDelay:    %d\n", MovDelay[x][y]);
  printf("  ChangeDelay: %d\n", ChangeDelay[x][y]);
  printf("  CustomValue: %d\n", CustomValue[x][y]);
  printf("  GfxElement:  %d\n", GfxElement[x][y]);
  printf("  GfxAction:   %d\n", GfxAction[x][y]);
  printf("  GfxFrame:    %d [%d]\n", GfxFrame[x][y], FrameCounter);
  printf("  Player x/y:  %d, %d\n", local_player->jx, local_player->jy);
  printf("\n");
}

void DumpTileFromScreen(int sx, int sy)
{
  int lx = getLevelFromScreenX(sx);
  int ly = getLevelFromScreenY(sy);

  DumpTile(lx, ly);
}

void SetDrawtoField(int mode)
{
  if (mode == DRAW_TO_FIELDBUFFER)
  {
    FX = 2 * TILEX_VAR;
    FY = 2 * TILEY_VAR;
    BX1 = -2;
    BY1 = -2;
    BX2 = SCR_FIELDX + 1;
    BY2 = SCR_FIELDY + 1;

    drawto_field = fieldbuffer;
  }
  else	/* DRAW_TO_BACKBUFFER */
  {
    FX = SX;
    FY = SY;
    BX1 = 0;
    BY1 = 0;
    BX2 = SCR_FIELDX - 1;
    BY2 = SCR_FIELDY - 1;

    drawto_field = backbuffer;
  }
}

static void RedrawPlayfield_RND()
{
  if (game.envelope_active)
    return;

  DrawLevel(REDRAW_ALL);
  DrawAllPlayers();
}

void RedrawPlayfield()
{
  if (game_status != GAME_MODE_PLAYING)
    return;

  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    RedrawPlayfield_EM(TRUE);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    RedrawPlayfield_SP(TRUE);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    RedrawPlayfield_MM();
  else if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    RedrawPlayfield_RND();

  BlitScreenToBitmap(backbuffer);

  BlitBitmap(drawto, window, gfx.sx, gfx.sy, gfx.sxsize, gfx.sysize,
	     gfx.sx, gfx.sy);
}

static void DrawMaskedBorderExt_Rect(int x, int y, int width, int height,
				     int draw_target)
{
  Bitmap *src_bitmap = getGlobalBorderBitmapFromStatus(global.border_status);
  Bitmap *dst_bitmap = gfx.masked_border_bitmap_ptr;

  if (x == -1 && y == -1)
    return;

  if (draw_target == DRAW_TO_SCREEN)
    BlitToScreenMasked(src_bitmap, x, y, width, height, x, y);
  else
    BlitBitmapMasked(src_bitmap, dst_bitmap, x, y, width, height, x, y);
}

static void DrawMaskedBorderExt_FIELD(int draw_target)
{
  if (global.border_status >= GAME_MODE_MAIN &&
      global.border_status <= GAME_MODE_PLAYING &&
      border.draw_masked[global.border_status])
    DrawMaskedBorderExt_Rect(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
			     draw_target);
}

static void DrawMaskedBorderExt_DOOR_1(int draw_target)
{
  // when drawing to backbuffer, never draw border over open doors
  if (draw_target == DRAW_TO_BACKBUFFER &&
      (GetDoorState() & DOOR_OPEN_1))
    return;

  if (border.draw_masked[GFX_SPECIAL_ARG_DOOR] &&
      (global.border_status != GAME_MODE_EDITOR ||
       border.draw_masked[GFX_SPECIAL_ARG_EDITOR]))
    DrawMaskedBorderExt_Rect(DX, DY, DXSIZE, DYSIZE, draw_target);
}

static void DrawMaskedBorderExt_DOOR_2(int draw_target)
{
  // when drawing to backbuffer, never draw border over open doors
  if (draw_target == DRAW_TO_BACKBUFFER &&
      (GetDoorState() & DOOR_OPEN_2))
    return;

  if (border.draw_masked[GFX_SPECIAL_ARG_DOOR] &&
      global.border_status != GAME_MODE_EDITOR)
    DrawMaskedBorderExt_Rect(VX, VY, VXSIZE, VYSIZE, draw_target);
}

static void DrawMaskedBorderExt_DOOR_3(int draw_target)
{
  /* currently not available */
}

static void DrawMaskedBorderExt_ALL(int draw_target)
{
  DrawMaskedBorderExt_FIELD(draw_target);
  DrawMaskedBorderExt_DOOR_1(draw_target);
  DrawMaskedBorderExt_DOOR_2(draw_target);
  DrawMaskedBorderExt_DOOR_3(draw_target);
}

static void DrawMaskedBorderExt(int redraw_mask, int draw_target)
{
  /* never draw masked screen borders on borderless screens */
  if (global.border_status == GAME_MODE_LOADING ||
      global.border_status == GAME_MODE_TITLE)
    return;

  if (redraw_mask & REDRAW_ALL)
    DrawMaskedBorderExt_ALL(draw_target);
  else
  {
    if (redraw_mask & REDRAW_FIELD)
      DrawMaskedBorderExt_FIELD(draw_target);
    if (redraw_mask & REDRAW_DOOR_1)
      DrawMaskedBorderExt_DOOR_1(draw_target);
    if (redraw_mask & REDRAW_DOOR_2)
      DrawMaskedBorderExt_DOOR_2(draw_target);
    if (redraw_mask & REDRAW_DOOR_3)
      DrawMaskedBorderExt_DOOR_3(draw_target);
  }
}

void DrawMaskedBorder_FIELD()
{
  DrawMaskedBorderExt_FIELD(DRAW_TO_BACKBUFFER);
}

void DrawMaskedBorder(int redraw_mask)
{
  DrawMaskedBorderExt(redraw_mask, DRAW_TO_BACKBUFFER);
}

void DrawMaskedBorderToTarget(int draw_target)
{
  if (draw_target == DRAW_TO_BACKBUFFER ||
      draw_target == DRAW_TO_SCREEN)
  {
    DrawMaskedBorderExt(REDRAW_ALL, draw_target);
  }
  else
  {
    int last_border_status = global.border_status;

    if (draw_target == DRAW_TO_FADE_SOURCE)
    {
      global.border_status = gfx.fade_border_source_status;
      gfx.masked_border_bitmap_ptr = gfx.fade_bitmap_source;
    }
    else if (draw_target == DRAW_TO_FADE_TARGET)
    {
      global.border_status = gfx.fade_border_target_status;
      gfx.masked_border_bitmap_ptr = gfx.fade_bitmap_target;
    }

    DrawMaskedBorderExt(REDRAW_ALL, draw_target);

    global.border_status = last_border_status;
    gfx.masked_border_bitmap_ptr = backbuffer;
  }
}

void DrawTileCursor(int draw_target)
{
  Bitmap *fade_bitmap;
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int graphic = IMG_GLOBAL_TILE_CURSOR;
  int frame = 0;
  int tilesize = TILESIZE_VAR;
  int width = tilesize;
  int height = tilesize;

  if (game_status != GAME_MODE_PLAYING)
    return;

  if (!tile_cursor.enabled ||
      !tile_cursor.active)
    return;

  if (tile_cursor.moving)
  {
    int step = TILESIZE_VAR / 4;
    int dx = tile_cursor.target_x - tile_cursor.x;
    int dy = tile_cursor.target_y - tile_cursor.y;

    if (ABS(dx) < step)
      tile_cursor.x = tile_cursor.target_x;
    else
      tile_cursor.x += SIGN(dx) * step;

    if (ABS(dy) < step)
      tile_cursor.y = tile_cursor.target_y;
    else
      tile_cursor.y += SIGN(dy) * step;

    if (tile_cursor.x == tile_cursor.target_x &&
	tile_cursor.y == tile_cursor.target_y)
      tile_cursor.moving = FALSE;
  }

  dst_x = tile_cursor.x;
  dst_y = tile_cursor.y;

  frame = getGraphicAnimationFrame(graphic, -1);

  getSizedGraphicSource(graphic, frame, tilesize, &src_bitmap, &src_x, &src_y);

  fade_bitmap =
    (draw_target == DRAW_TO_FADE_SOURCE ? gfx.fade_bitmap_source :
     draw_target == DRAW_TO_FADE_TARGET ? gfx.fade_bitmap_target : NULL);

  if (draw_target == DRAW_TO_SCREEN)
    BlitToScreenMasked(src_bitmap, src_x, src_y, width, height, dst_x, dst_y);
  else
    BlitBitmapMasked(src_bitmap, fade_bitmap, src_x, src_y, width, height,
		     dst_x, dst_y);
}

void BlitScreenToBitmap_RND(Bitmap *target_bitmap)
{
  int fx = getFieldbufferOffsetX_RND();
  int fy = getFieldbufferOffsetY_RND();

  BlitBitmap(drawto_field, target_bitmap, fx, fy, SXSIZE, SYSIZE, SX, SY);
}

void BlitScreenToBitmap(Bitmap *target_bitmap)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    BlitScreenToBitmap_EM(target_bitmap);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    BlitScreenToBitmap_SP(target_bitmap);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    BlitScreenToBitmap_MM(target_bitmap);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_RND)
    BlitScreenToBitmap_RND(target_bitmap);

  redraw_mask |= REDRAW_FIELD;
}

void DrawFramesPerSecond()
{
  char text[100];
  int font_nr = FONT_TEXT_2;
  int font_width = getFontWidth(font_nr);
  int draw_deactivation_mask = GetDrawDeactivationMask();
  boolean draw_masked = (draw_deactivation_mask == REDRAW_NONE);

  /* draw FPS with leading space (needed if field buffer deactivated) */
  sprintf(text, " %04.1f fps", global.frames_per_second);

  /* override draw deactivation mask (required for invisible warp mode) */
  SetDrawDeactivationMask(REDRAW_NONE);

  /* draw opaque FPS if field buffer deactivated, else draw masked FPS */
  DrawTextExt(backbuffer, SX + SXSIZE - font_width * strlen(text), SY, text,
	      font_nr, (draw_masked ? BLIT_MASKED : BLIT_OPAQUE));

  /* set draw deactivation mask to previous value */
  SetDrawDeactivationMask(draw_deactivation_mask);

  /* force full-screen redraw in this frame */
  redraw_mask = REDRAW_ALL;
}

#if DEBUG_FRAME_TIME
static void PrintFrameTimeDebugging()
{
  static unsigned int last_counter = 0;
  unsigned int counter = Counter();
  int diff_1 = counter - last_counter;
  int diff_2 = diff_1 - GAME_FRAME_DELAY;
  int diff_2_max = 20;
  int diff_2_cut = MIN(ABS(diff_2), diff_2_max);
  char diff_bar[2 * diff_2_max + 5];
  int pos = 0;
  int i;

  diff_bar[pos++] = (diff_2 < -diff_2_max ? '<' : ' ');

  for (i = 0; i < diff_2_max; i++)
    diff_bar[pos++] = (diff_2 >= 0 ? ' ' :
		       i >= diff_2_max - diff_2_cut ? '-' : ' ');

  diff_bar[pos++] = '|';

  for (i = 0; i < diff_2_max; i++)
    diff_bar[pos++] = (diff_2 <= 0 ? ' ' : i < diff_2_cut ? '+' : ' ');

  diff_bar[pos++] = (diff_2 > diff_2_max ? '>' : ' ');

  diff_bar[pos++] = '\0';

  Error(ERR_INFO, "%06d [%02d] [%c%02d] %s",
	counter,
	diff_1,
	(diff_2 < 0 ? '-' : diff_2 > 0 ? '+' : ' '), ABS(diff_2),
	diff_bar);

  last_counter = counter;
}
#endif

static int unifiedRedrawMask(int mask)
{
  if (mask & REDRAW_ALL)
    return REDRAW_ALL;

  if (mask & REDRAW_FIELD && mask & REDRAW_DOORS)
    return REDRAW_ALL;

  return mask;
}

static boolean equalRedrawMasks(int mask_1, int mask_2)
{
  return unifiedRedrawMask(mask_1) == unifiedRedrawMask(mask_2);
}

void BackToFront()
{
  static int last_redraw_mask = REDRAW_NONE;

  // force screen redraw in every frame to continue drawing global animations
  // (but always use the last redraw mask to prevent unwanted side effects)
  if (redraw_mask == REDRAW_NONE)
    redraw_mask = last_redraw_mask;

  last_redraw_mask = redraw_mask;

#if 1
  // masked border now drawn immediately when blitting backbuffer to window
#else
  // draw masked border to all viewports, if defined
  DrawMaskedBorder(redraw_mask);
#endif

  // draw frames per second (only if debug mode is enabled)
  if (redraw_mask & REDRAW_FPS)
    DrawFramesPerSecond();

  // remove playfield redraw before potentially merging with doors redraw
  if (DrawingDeactivated(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE))
    redraw_mask &= ~REDRAW_FIELD;

  // redraw complete window if both playfield and (some) doors need redraw
  if (redraw_mask & REDRAW_FIELD && redraw_mask & REDRAW_DOORS)
    redraw_mask = REDRAW_ALL;

  /* although redrawing the whole window would be fine for normal gameplay,
     being able to only redraw the playfield is required for deactivating
     certain drawing areas (mainly playfield) to work, which is needed for
     warp-forward to be fast enough (by skipping redraw of most frames) */

  if (redraw_mask & REDRAW_ALL)
  {
    BlitBitmap(backbuffer, window, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  }
  else if (redraw_mask & REDRAW_FIELD)
  {
    BlitBitmap(backbuffer, window,
	       REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE, REAL_SX, REAL_SY);
  }
  else if (redraw_mask & REDRAW_DOORS)
  {
    // merge door areas to prevent calling screen redraw more than once
    int x1 = WIN_XSIZE;
    int y1 = WIN_YSIZE;
    int x2 = 0;
    int y2 = 0;

    if (redraw_mask & REDRAW_DOOR_1)
    {
      x1 = MIN(x1, DX);
      y1 = MIN(y1, DY);
      x2 = MAX(x2, DX + DXSIZE);
      y2 = MAX(y2, DY + DYSIZE);
    }

    if (redraw_mask & REDRAW_DOOR_2)
    {
      x1 = MIN(x1, VX);
      y1 = MIN(y1, VY);
      x2 = MAX(x2, VX + VXSIZE);
      y2 = MAX(y2, VY + VYSIZE);
    }

    if (redraw_mask & REDRAW_DOOR_3)
    {
      x1 = MIN(x1, EX);
      y1 = MIN(y1, EY);
      x2 = MAX(x2, EX + EXSIZE);
      y2 = MAX(y2, EY + EYSIZE);
    }

    // make sure that at least one pixel is blitted, and inside the screen
    // (else nothing is blitted, causing the animations not to be updated)
    x1 = MIN(MAX(0, x1), WIN_XSIZE - 1);
    y1 = MIN(MAX(0, y1), WIN_YSIZE - 1);
    x2 = MIN(MAX(1, x2), WIN_XSIZE);
    y2 = MIN(MAX(1, y2), WIN_YSIZE);

    BlitBitmap(backbuffer, window, x1, y1, x2 - x1, y2 - y1, x1, y1);
  }

  redraw_mask = REDRAW_NONE;

#if DEBUG_FRAME_TIME
  PrintFrameTimeDebugging();
#endif
}

void BackToFront_WithFrameDelay(unsigned int frame_delay_value)
{
  unsigned int frame_delay_value_old = GetVideoFrameDelay();

  SetVideoFrameDelay(frame_delay_value);

  BackToFront();

  SetVideoFrameDelay(frame_delay_value_old);
}

static int fade_type_skip = FADE_TYPE_NONE;

static void FadeExt(int fade_mask, int fade_mode, int fade_type)
{
  void (*draw_border_function)(void) = NULL;
  int x, y, width, height;
  int fade_delay, post_delay;

  if (fade_type == FADE_TYPE_FADE_OUT)
  {
    if (fade_type_skip != FADE_TYPE_NONE)
    {
      /* skip all fade operations until specified fade operation */
      if (fade_type & fade_type_skip)
	fade_type_skip = FADE_TYPE_NONE;

      return;
    }

    if (fading.fade_mode & FADE_TYPE_TRANSFORM)
      return;
  }

  redraw_mask |= fade_mask;

  if (fade_type == FADE_TYPE_SKIP)
  {
    fade_type_skip = fade_mode;

    return;
  }

  fade_delay = fading.fade_delay;
  post_delay = (fade_mode == FADE_MODE_FADE_OUT ? fading.post_delay : 0);

  if (fade_type_skip != FADE_TYPE_NONE)
  {
    /* skip all fade operations until specified fade operation */
    if (fade_type & fade_type_skip)
      fade_type_skip = FADE_TYPE_NONE;

    fade_delay = 0;
  }

  if (global.autoplay_leveldir)
  {
    return;
  }

  if (fade_mask == REDRAW_FIELD)
  {
    x = FADE_SX;
    y = FADE_SY;
    width  = FADE_SXSIZE;
    height = FADE_SYSIZE;

    if (border.draw_masked_when_fading)
      draw_border_function = DrawMaskedBorder_FIELD;	/* update when fading */
    else
      DrawMaskedBorder_FIELD();				/* draw once */
  }
  else		/* REDRAW_ALL */
  {
    x = 0;
    y = 0;
    width  = WIN_XSIZE;
    height = WIN_YSIZE;
  }

  if (!setup.fade_screens ||
      fade_delay == 0 ||
      fading.fade_mode == FADE_MODE_NONE)
  {
    if (fade_mode == FADE_MODE_FADE_OUT)
      return;

    BlitBitmap(backbuffer, window, x, y, width, height, x, y);

    redraw_mask &= ~fade_mask;

    return;
  }

  FadeRectangle(x, y, width, height, fade_mode, fade_delay, post_delay,
		draw_border_function);

  redraw_mask &= ~fade_mask;
}

static void SetScreenStates_BeforeFadingIn()
{
  // temporarily set screen mode for animations to screen after fading in
  global.anim_status = global.anim_status_next;

  // store backbuffer with all animations that will be started after fading in
  if (fade_type_skip != FADE_MODE_SKIP_FADE_IN)
    PrepareFadeBitmap(DRAW_TO_FADE_TARGET);

  // set screen mode for animations back to fading
  global.anim_status = GAME_MODE_PSEUDO_FADING;
}

static void SetScreenStates_AfterFadingIn()
{
  // store new source screen (to use correct masked border for fading)
  gfx.fade_border_source_status = global.border_status;

  global.anim_status = global.anim_status_next;
}

static void SetScreenStates_BeforeFadingOut()
{
  // store new target screen (to use correct masked border for fading)
  gfx.fade_border_target_status = game_status;

  // set screen mode for animations to fading
  global.anim_status = GAME_MODE_PSEUDO_FADING;

  // store backbuffer with all animations that will be stopped for fading out
  if (fade_type_skip != FADE_MODE_SKIP_FADE_OUT)
    PrepareFadeBitmap(DRAW_TO_FADE_SOURCE);
}

static void SetScreenStates_AfterFadingOut()
{
  global.border_status = game_status;
}

void FadeIn(int fade_mask)
{
  SetScreenStates_BeforeFadingIn();

#if 1
  DrawMaskedBorder(REDRAW_ALL);
#endif

  if (fading.fade_mode & FADE_TYPE_TRANSFORM)
    FadeExt(fade_mask, fading.fade_mode, FADE_TYPE_FADE_IN);
  else
    FadeExt(fade_mask, FADE_MODE_FADE_IN, FADE_TYPE_FADE_IN);

  FADE_SX = REAL_SX;
  FADE_SY = REAL_SY;
  FADE_SXSIZE = FULL_SXSIZE;
  FADE_SYSIZE = FULL_SYSIZE;

  if (game_status == GAME_MODE_PLAYING &&
      strEqual(setup.touch.control_type, TOUCH_CONTROL_VIRTUAL_BUTTONS))
    SetOverlayActive(TRUE);

  SetScreenStates_AfterFadingIn();

  // force update of global animation status in case of rapid screen changes
  redraw_mask = REDRAW_ALL;
  BackToFront();
}

void FadeOut(int fade_mask)
{
  // update screen if areas covered by "fade_mask" and "redraw_mask" differ
  if (!equalRedrawMasks(fade_mask, redraw_mask))
    BackToFront();

  SetScreenStates_BeforeFadingOut();

  SetTileCursorActive(FALSE);
  SetOverlayActive(FALSE);

#if 0
  DrawMaskedBorder(REDRAW_ALL);
#endif

  if (fading.fade_mode & FADE_TYPE_TRANSFORM)
    FadeExt(fade_mask, fading.fade_mode, FADE_TYPE_FADE_OUT);
  else
    FadeExt(fade_mask, FADE_MODE_FADE_OUT, FADE_TYPE_FADE_OUT);

  SetScreenStates_AfterFadingOut();
}

static void FadeSetLeaveNext(struct TitleFadingInfo fading_leave, boolean set)
{
  static struct TitleFadingInfo fading_leave_stored;

  if (set)
    fading_leave_stored = fading_leave;
  else
    fading = fading_leave_stored;
}

void FadeSetEnterMenu()
{
  fading = menu.enter_menu;

  FadeSetLeaveNext(fading, TRUE);	/* (keep same fade mode) */
}

void FadeSetLeaveMenu()
{
  fading = menu.leave_menu;

  FadeSetLeaveNext(fading, TRUE);	/* (keep same fade mode) */
}

void FadeSetEnterScreen()
{
  fading = menu.enter_screen[game_status];

  FadeSetLeaveNext(menu.leave_screen[game_status], TRUE);	/* store */
}

void FadeSetNextScreen()
{
  fading = menu.next_screen[game_status];

  // (do not overwrite fade mode set by FadeSetEnterScreen)
  // FadeSetLeaveNext(fading, TRUE);	/* (keep same fade mode) */
}

void FadeSetLeaveScreen()
{
  FadeSetLeaveNext(menu.leave_screen[game_status], FALSE);	/* recall */
}

void FadeSetFromType(int type)
{
  if (type & TYPE_ENTER_SCREEN)
    FadeSetEnterScreen();
  else if (type & TYPE_ENTER)
    FadeSetEnterMenu();
  else if (type & TYPE_LEAVE)
    FadeSetLeaveMenu();
}

void FadeSetDisabled()
{
  static struct TitleFadingInfo fading_none = { FADE_MODE_NONE, -1, -1, -1 };

  fading = fading_none;
}

void FadeSkipNextFadeIn()
{
  FadeExt(0, FADE_MODE_SKIP_FADE_IN, FADE_TYPE_SKIP);
}

void FadeSkipNextFadeOut()
{
  FadeExt(0, FADE_MODE_SKIP_FADE_OUT, FADE_TYPE_SKIP);
}

Bitmap *getBitmapFromGraphicOrDefault(int graphic, int default_graphic)
{
  boolean redefined = getImageListEntryFromImageID(graphic)->redefined;

  return (graphic == IMG_UNDEFINED ? NULL :
	  graphic_info[graphic].bitmap != NULL || redefined ?
	  graphic_info[graphic].bitmap :
	  graphic_info[default_graphic].bitmap);
}

Bitmap *getBackgroundBitmap(int graphic)
{
  return getBitmapFromGraphicOrDefault(graphic, IMG_BACKGROUND);
}

Bitmap *getGlobalBorderBitmap(int graphic)
{
  return getBitmapFromGraphicOrDefault(graphic, IMG_GLOBAL_BORDER);
}

Bitmap *getGlobalBorderBitmapFromStatus(int status)
{
  int graphic =
    (status == GAME_MODE_MAIN ||
     status == GAME_MODE_PSEUDO_TYPENAME	? IMG_GLOBAL_BORDER_MAIN :
     status == GAME_MODE_SCORES			? IMG_GLOBAL_BORDER_SCORES :
     status == GAME_MODE_EDITOR			? IMG_GLOBAL_BORDER_EDITOR :
     status == GAME_MODE_PLAYING		? IMG_GLOBAL_BORDER_PLAYING :
     IMG_GLOBAL_BORDER);

  return getGlobalBorderBitmap(graphic);
}

void SetWindowBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetWindowBackgroundBitmap(graphic_info[graphic].bitmap);
}

void SetMainBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetMainBackgroundBitmap(graphic_info[graphic].bitmap);
}

void SetDoorBackgroundImageIfDefined(int graphic)
{
  if (graphic_info[graphic].bitmap)
    SetDoorBackgroundBitmap(graphic_info[graphic].bitmap);
}

void SetWindowBackgroundImage(int graphic)
{
  SetWindowBackgroundBitmap(getBackgroundBitmap(graphic));
}

void SetMainBackgroundImage(int graphic)
{
  SetMainBackgroundBitmap(getBackgroundBitmap(graphic));
}

void SetDoorBackgroundImage(int graphic)
{
  SetDoorBackgroundBitmap(getBackgroundBitmap(graphic));
}

void SetPanelBackground()
{
  struct GraphicInfo *gfx = &graphic_info[IMG_BACKGROUND_PANEL];

  BlitBitmapTiled(gfx->bitmap, bitmap_db_panel, gfx->src_x, gfx->src_y,
		  gfx->width, gfx->height, 0, 0, DXSIZE, DYSIZE);

  SetDoorBackgroundBitmap(bitmap_db_panel);
}

void DrawBackground(int x, int y, int width, int height)
{
  /* "drawto" might still point to playfield buffer here (hall of fame) */
  ClearRectangleOnBackground(backbuffer, x, y, width, height);

  if (IN_GFX_FIELD_FULL(x, y))
    redraw_mask |= REDRAW_FIELD;
  else if (IN_GFX_DOOR_1(x, y))
    redraw_mask |= REDRAW_DOOR_1;
  else if (IN_GFX_DOOR_2(x, y))
    redraw_mask |= REDRAW_DOOR_2;
  else if (IN_GFX_DOOR_3(x, y))
    redraw_mask |= REDRAW_DOOR_3;
}

void DrawBackgroundForFont(int x, int y, int width, int height, int font_nr)
{
  struct FontBitmapInfo *font = getFontBitmapInfo(font_nr);

  if (font->bitmap == NULL)
    return;

  DrawBackground(x, y, width, height);
}

void DrawBackgroundForGraphic(int x, int y, int width, int height, int graphic)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  if (g->bitmap == NULL)
    return;

  DrawBackground(x, y, width, height);
}

static int game_status_last = -1;
static Bitmap *global_border_bitmap_last = NULL;
static Bitmap *global_border_bitmap = NULL;
static int real_sx_last = -1, real_sy_last = -1;
static int full_sxsize_last = -1, full_sysize_last = -1;
static int dx_last = -1, dy_last = -1;
static int dxsize_last = -1, dysize_last = -1;
static int vx_last = -1, vy_last = -1;
static int vxsize_last = -1, vysize_last = -1;
static int ex_last = -1, ey_last = -1;
static int exsize_last = -1, eysize_last = -1;

boolean CheckIfGlobalBorderHasChanged()
{
  // if game status has not changed, global border has not changed either
  if (game_status == game_status_last)
    return FALSE;

  // determine and store new global border bitmap for current game status
  global_border_bitmap = getGlobalBorderBitmapFromStatus(game_status);

  return (global_border_bitmap_last != global_border_bitmap);
}

boolean CheckIfGlobalBorderRedrawIsNeeded()
{
  // if game status has not changed, nothing has to be redrawn
  if (game_status == game_status_last)
    return FALSE;

  // redraw if last screen was title screen
  if (game_status_last == GAME_MODE_TITLE)
    return TRUE;

  // redraw if global screen border has changed
  if (CheckIfGlobalBorderHasChanged())
    return TRUE;

  // redraw if position or size of playfield area has changed
  if (real_sx_last != REAL_SX || real_sy_last != REAL_SY ||
      full_sxsize_last != FULL_SXSIZE || full_sysize_last != FULL_SYSIZE)
    return TRUE;

  // redraw if position or size of door area has changed
  if (dx_last != DX || dy_last != DY ||
      dxsize_last != DXSIZE || dysize_last != DYSIZE)
    return TRUE;

  // redraw if position or size of tape area has changed
  if (vx_last != VX || vy_last != VY ||
      vxsize_last != VXSIZE || vysize_last != VYSIZE)
    return TRUE;

  // redraw if position or size of editor area has changed
  if (ex_last != EX || ey_last != EY ||
      exsize_last != EXSIZE || eysize_last != EYSIZE)
    return TRUE;

  return FALSE;
}

void RedrawGlobalBorderFromBitmap(Bitmap *bitmap)
{
  if (bitmap)
    BlitBitmap(bitmap, backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  else
    ClearRectangle(backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE);
}

void RedrawGlobalBorder()
{
  Bitmap *bitmap = getGlobalBorderBitmapFromStatus(game_status);

  RedrawGlobalBorderFromBitmap(bitmap);

  redraw_mask = REDRAW_ALL;
}

#define ONLY_REDRAW_GLOBAL_BORDER_IF_NEEDED		0

static void RedrawGlobalBorderIfNeeded()
{
#if ONLY_REDRAW_GLOBAL_BORDER_IF_NEEDED
  if (game_status == game_status_last)
    return;
#endif

  // copy current draw buffer to later copy back areas that have not changed
  if (game_status_last != GAME_MODE_TITLE)
    BlitBitmap(backbuffer, bitmap_db_store_1, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

#if ONLY_REDRAW_GLOBAL_BORDER_IF_NEEDED
  if (CheckIfGlobalBorderRedrawIsNeeded())
#endif
  {
    // redraw global screen border (or clear, if defined to be empty)
    RedrawGlobalBorderFromBitmap(global_border_bitmap);

    if (game_status == GAME_MODE_EDITOR)
      DrawSpecialEditorDoor();

    // copy previous playfield and door areas, if they are defined on both
    // previous and current screen and if they still have the same size

    if (real_sx_last != -1 && real_sy_last != -1 &&
	REAL_SX != -1 && REAL_SY != -1 &&
	full_sxsize_last == FULL_SXSIZE && full_sysize_last == FULL_SYSIZE)
      BlitBitmap(bitmap_db_store_1, backbuffer,
		 real_sx_last, real_sy_last, FULL_SXSIZE, FULL_SYSIZE,
		 REAL_SX, REAL_SY);

    if (dx_last != -1 && dy_last != -1 &&
	DX != -1 && DY != -1 &&
	dxsize_last == DXSIZE && dysize_last == DYSIZE)
      BlitBitmap(bitmap_db_store_1, backbuffer,
		 dx_last, dy_last, DXSIZE, DYSIZE, DX, DY);

    if (game_status != GAME_MODE_EDITOR)
    {
      if (vx_last != -1 && vy_last != -1 &&
	  VX != -1 && VY != -1 &&
	  vxsize_last == VXSIZE && vysize_last == VYSIZE)
	BlitBitmap(bitmap_db_store_1, backbuffer,
		   vx_last, vy_last, VXSIZE, VYSIZE, VX, VY);
    }
    else
    {
      if (ex_last != -1 && ey_last != -1 &&
	  EX != -1 && EY != -1 &&
	  exsize_last == EXSIZE && eysize_last == EYSIZE)
	BlitBitmap(bitmap_db_store_1, backbuffer,
		   ex_last, ey_last, EXSIZE, EYSIZE, EX, EY);
    }

    redraw_mask = REDRAW_ALL;
  }

  game_status_last = game_status;

  global_border_bitmap_last = global_border_bitmap;

  real_sx_last = REAL_SX;
  real_sy_last = REAL_SY;
  full_sxsize_last = FULL_SXSIZE;
  full_sysize_last = FULL_SYSIZE;
  dx_last = DX;
  dy_last = DY;
  dxsize_last = DXSIZE;
  dysize_last = DYSIZE;
  vx_last = VX;
  vy_last = VY;
  vxsize_last = VXSIZE;
  vysize_last = VYSIZE;
  ex_last = EX;
  ey_last = EY;
  exsize_last = EXSIZE;
  eysize_last = EYSIZE;
}

void ClearField()
{
  RedrawGlobalBorderIfNeeded();

  /* !!! "drawto" might still point to playfield buffer here (see above) !!! */
  /* (when entering hall of fame after playing) */
  DrawBackground(REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);

  /* !!! maybe this should be done before clearing the background !!! */
  if (game_status == GAME_MODE_PLAYING)
  {
    ClearRectangle(fieldbuffer, 0, 0, FXSIZE, FYSIZE);
    SetDrawtoField(DRAW_TO_FIELDBUFFER);
  }
  else
  {
    SetDrawtoField(DRAW_TO_BACKBUFFER);
  }
}

void MarkTileDirty(int x, int y)
{
  redraw_mask |= REDRAW_FIELD;
}

void SetBorderElement()
{
  int x, y;

  BorderElement = EL_EMPTY;

  /* the MM game engine does not use a visible border element */
  if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return;

  for (y = 0; y < lev_fieldy && BorderElement == EL_EMPTY; y++)
  {
    for (x = 0; x < lev_fieldx; x++)
    {
      if (!IS_INDESTRUCTIBLE(Feld[x][y]))
	BorderElement = EL_STEELWALL;

      if (y != 0 && y != lev_fieldy - 1 && x != lev_fieldx - 1)
	x = lev_fieldx - 2;
    }
  }
}

void FloodFillLevelExt(int from_x, int from_y, int fill_element,
		       int max_array_fieldx, int max_array_fieldy,
		       short field[max_array_fieldx][max_array_fieldy],
		       int max_fieldx, int max_fieldy)
{
  int i,x,y;
  int old_element;
  static int check[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
  static int safety = 0;

  /* check if starting field still has the desired content */
  if (field[from_x][from_y] == fill_element)
    return;

  safety++;

  if (safety > max_fieldx * max_fieldy)
    Error(ERR_EXIT, "Something went wrong in 'FloodFill()'. Please debug.");

  old_element = field[from_x][from_y];
  field[from_x][from_y] = fill_element;

  for (i = 0; i < 4; i++)
  {
    x = from_x + check[i][0];
    y = from_y + check[i][1];

    if (IN_FIELD(x, y, max_fieldx, max_fieldy) && field[x][y] == old_element)
      FloodFillLevelExt(x, y, fill_element, max_array_fieldx, max_array_fieldy,
			field, max_fieldx, max_fieldy);
  }

  safety--;
}

void FloodFillLevel(int from_x, int from_y, int fill_element,
		    short field[MAX_LEV_FIELDX][MAX_LEV_FIELDY],
		    int max_fieldx, int max_fieldy)
{
  FloodFillLevelExt(from_x, from_y, fill_element,
		    MAX_LEV_FIELDX, MAX_LEV_FIELDY, field,
		    max_fieldx, max_fieldy);
}

void SetRandomAnimationValue(int x, int y)
{
  gfx.anim_random_frame = GfxRandom[x][y];
}

int getGraphicAnimationFrame(int graphic, int sync_frame)
{
  /* animation synchronized with global frame counter, not move position */
  if (graphic_info[graphic].anim_global_sync || sync_frame < 0)
    sync_frame = FrameCounter;

  return getAnimationFrame(graphic_info[graphic].anim_frames,
			   graphic_info[graphic].anim_delay,
			   graphic_info[graphic].anim_mode,
			   graphic_info[graphic].anim_start_frame,
			   sync_frame);
}

void getGraphicSourceBitmap(int graphic, int tilesize, Bitmap **bitmap)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  int tilesize_capped = MIN(MAX(1, tilesize), TILESIZE);

  if (tilesize == gfx.standard_tile_size)
    *bitmap = g->bitmaps[IMG_BITMAP_STANDARD];
  else if (tilesize == game.tile_size)
    *bitmap = g->bitmaps[IMG_BITMAP_GAME];
  else
    *bitmap = g->bitmaps[IMG_BITMAP_1x1 - log_2(tilesize_capped)];
}

void getGraphicSourceXY(int graphic, int frame, int *x, int *y,
			boolean get_backside)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  int src_x = g->src_x + (get_backside ? g->offset2_x : 0);
  int src_y = g->src_y + (get_backside ? g->offset2_y : 0);

  if (g->offset_y == 0)		/* frames are ordered horizontally */
  {
    int max_width = g->anim_frames_per_line * g->width;
    int pos = (src_y / g->height) * max_width + src_x + frame * g->offset_x;

    *x = pos % max_width;
    *y = src_y % g->height + pos / max_width * g->height;
  }
  else if (g->offset_x == 0)	/* frames are ordered vertically */
  {
    int max_height = g->anim_frames_per_line * g->height;
    int pos = (src_x / g->width) * max_height + src_y + frame * g->offset_y;

    *x = src_x % g->width + pos / max_height * g->width;
    *y = pos % max_height;
  }
  else				/* frames are ordered diagonally */
  {
    *x = src_x + frame * g->offset_x;
    *y = src_y + frame * g->offset_y;
  }
}

void getSizedGraphicSourceExt(int graphic, int frame, int tilesize,
			      Bitmap **bitmap, int *x, int *y,
			      boolean get_backside)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  // if no in-game graphics defined, always use standard graphic size
  if (g->bitmaps[IMG_BITMAP_GAME] == NULL)
    tilesize = TILESIZE;

  getGraphicSourceBitmap(graphic, tilesize, bitmap);
  getGraphicSourceXY(graphic, frame, x, y, get_backside);

  *x = *x * tilesize / g->tile_size;
  *y = *y * tilesize / g->tile_size;
}

void getSizedGraphicSource(int graphic, int frame, int tilesize,
			   Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSourceExt(graphic, frame, tilesize, bitmap, x, y, FALSE);
}

void getFixedGraphicSource(int graphic, int frame,
			   Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSourceExt(graphic, frame, TILESIZE, bitmap, x, y, FALSE);
}

void getMiniGraphicSource(int graphic, Bitmap **bitmap, int *x, int *y)
{
  getSizedGraphicSource(graphic, 0, MINI_TILESIZE, bitmap, x, y);
}

inline static void getGraphicSourceExt(int graphic, int frame, Bitmap **bitmap,
				       int *x, int *y, boolean get_backside)
{
  getSizedGraphicSourceExt(graphic, frame, TILESIZE_VAR, bitmap, x, y,
			   get_backside);
}

void getGraphicSource(int graphic, int frame, Bitmap **bitmap, int *x, int *y)
{
  getGraphicSourceExt(graphic, frame, bitmap, x, y, FALSE);
}

void DrawGraphic(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphic(): x = %d, y = %d, graphic = %d\n", x, y, graphic);
    printf("DrawGraphic(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicExt(drawto_field, FX + x * TILEX_VAR, FY + y * TILEY_VAR, graphic,
		 frame);

  MarkTileDirty(x, y);
}

void DrawFixedGraphic(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphic(): x = %d, y = %d, graphic = %d\n", x, y, graphic);
    printf("DrawGraphic(): This should never happen!\n");
    return;
  }
#endif

  DrawFixedGraphicExt(drawto_field, FX + x * TILEX, FY + y * TILEY, graphic,
		      frame);
  MarkTileDirty(x, y);
}

void DrawGraphicExt(DrawBuffer *dst_bitmap, int x, int y, int graphic,
		    int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, TILEX_VAR, TILEY_VAR, x, y);
}

void DrawFixedGraphicExt(DrawBuffer *dst_bitmap, int x, int y, int graphic,
			 int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getFixedGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, dst_bitmap, src_x, src_y, TILEX, TILEY, x, y);
}

void DrawGraphicThruMask(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphicThruMask(): x = %d,y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicThruMask(): This should never happen!\n");
    return;
  }
#endif

  DrawGraphicThruMaskExt(drawto_field, FX + x * TILEX_VAR, FY + y * TILEY_VAR,
			 graphic, frame);

  MarkTileDirty(x, y);
}

void DrawFixedGraphicThruMask(int x, int y, int graphic, int frame)
{
#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphicThruMask(): x = %d,y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicThruMask(): This should never happen!\n");
    return;
  }
#endif

  DrawFixedGraphicThruMaskExt(drawto_field, FX + x * TILEX, FY + y * TILEY,
			      graphic, frame);
  MarkTileDirty(x, y);
}

void DrawGraphicThruMaskExt(DrawBuffer *d, int dst_x, int dst_y, int graphic,
			    int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX_VAR, TILEY_VAR,
		   dst_x, dst_y);
}

void DrawFixedGraphicThruMaskExt(DrawBuffer *d, int dst_x, int dst_y,
				 int graphic, int frame)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getFixedGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  BlitBitmapMasked(src_bitmap, d, src_x, src_y, TILEX, TILEY,
		   dst_x, dst_y);
}

void DrawSizedGraphic(int x, int y, int graphic, int frame, int tilesize)
{
  DrawSizedGraphicExt(drawto, SX + x * tilesize, SY + y * tilesize, graphic,
		      frame, tilesize);
  MarkTileDirty(x / tilesize, y / tilesize);
}

void DrawSizedGraphicThruMask(int x, int y, int graphic, int frame,
			      int tilesize)
{
  DrawSizedGraphicThruMaskExt(drawto, SX + x * tilesize, SY + y * tilesize,
			      graphic, frame, tilesize);
  MarkTileDirty(x / tilesize, y / tilesize);
}

void DrawSizedGraphicExt(DrawBuffer *d, int x, int y, int graphic, int frame,
			 int tilesize)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getSizedGraphicSource(graphic, frame, tilesize, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, d, src_x, src_y, tilesize, tilesize, x, y);
}

void DrawSizedGraphicThruMaskExt(DrawBuffer *d, int x, int y, int graphic,
				 int frame, int tilesize)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getSizedGraphicSource(graphic, frame, tilesize, &src_bitmap, &src_x, &src_y);
  BlitBitmapMasked(src_bitmap, d, src_x, src_y, tilesize, tilesize, x, y);
}

void DrawMiniGraphic(int x, int y, int graphic)
{
  DrawMiniGraphicExt(drawto, SX + x * MINI_TILEX,SY + y * MINI_TILEY, graphic);
  MarkTileDirty(x / 2, y / 2);
}

void DrawMiniGraphicExt(DrawBuffer *d, int x, int y, int graphic)
{
  Bitmap *src_bitmap;
  int src_x, src_y;

  getMiniGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
  BlitBitmap(src_bitmap, d, src_x, src_y, MINI_TILEX, MINI_TILEY, x, y);
}

inline static void DrawGraphicShiftedNormal(int x, int y, int dx, int dy,
					    int graphic, int frame,
					    int cut_mode, int mask_mode)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int width = TILEX, height = TILEY;
  int cx = 0, cy = 0;

  if (dx || dy)			/* shifted graphic */
  {
    if (x < BX1)		/* object enters playfield from the left */
    {
      x = BX1;
      width = dx;
      cx = TILEX - dx;
      dx = 0;
    }
    else if (x > BX2)		/* object enters playfield from the right */
    {
      x = BX2;
      width = -dx;
      dx = TILEX + dx;
    }
    else if (x == BX1 && dx < 0) /* object leaves playfield to the left */
    {
      width += dx;
      cx = -dx;
      dx = 0;
    }
    else if (x == BX2 && dx > 0) /* object leaves playfield to the right */
      width -= dx;
    else if (dx)		/* general horizontal movement */
      MarkTileDirty(x + SIGN(dx), y);

    if (y < BY1)		/* object enters playfield from the top */
    {
      if (cut_mode == CUT_BELOW) /* object completely above top border */
	return;

      y = BY1;
      height = dy;
      cy = TILEY - dy;
      dy = 0;
    }
    else if (y > BY2)		/* object enters playfield from the bottom */
    {
      y = BY2;
      height = -dy;
      dy = TILEY + dy;
    }
    else if (y == BY1 && dy < 0) /* object leaves playfield to the top */
    {
      height += dy;
      cy = -dy;
      dy = 0;
    }
    else if (dy > 0 && cut_mode == CUT_ABOVE)
    {
      if (y == BY2)		/* object completely above bottom border */
	return;

      height = dy;
      cy = TILEY - dy;
      dy = TILEY;
      MarkTileDirty(x, y + 1);
    }				/* object leaves playfield to the bottom */
    else if (dy > 0 && (y == BY2 || cut_mode == CUT_BELOW))
      height -= dy;
    else if (dy)		/* general vertical movement */
      MarkTileDirty(x, y + SIGN(dy));
  }

#if DEBUG
  if (!IN_SCR_FIELD(x, y))
  {
    printf("DrawGraphicShifted(): x = %d, y = %d, graphic = %d\n",x,y,graphic);
    printf("DrawGraphicShifted(): This should never happen!\n");
    return;
  }
#endif

  width = width * TILESIZE_VAR / TILESIZE;
  height = height * TILESIZE_VAR / TILESIZE;
  cx = cx * TILESIZE_VAR / TILESIZE;
  cy = cy * TILESIZE_VAR / TILESIZE;
  dx = dx * TILESIZE_VAR / TILESIZE;
  dy = dy * TILESIZE_VAR / TILESIZE;

  if (width > 0 && height > 0)
  {
    getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

    src_x += cx;
    src_y += cy;

    dst_x = FX + x * TILEX_VAR + dx;
    dst_y = FY + y * TILEY_VAR + dy;

    if (mask_mode == USE_MASKING)
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x, y);
  }
}

inline static void DrawGraphicShiftedDouble(int x, int y, int dx, int dy,
					    int graphic, int frame,
					    int cut_mode, int mask_mode)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x, dst_y;
  int width = TILEX_VAR, height = TILEY_VAR;
  int x1 = x;
  int y1 = y;
  int x2 = x + SIGN(dx);
  int y2 = y + SIGN(dy);

  /* movement with two-tile animations must be sync'ed with movement position,
     not with current GfxFrame (which can be higher when using slow movement) */
  int anim_pos = (dx ? ABS(dx) : ABS(dy));
  int anim_frames = graphic_info[graphic].anim_frames;

  /* (we also need anim_delay here for movement animations with less frames) */
  int anim_delay = graphic_info[graphic].anim_delay;
  int sync_frame = anim_pos * anim_frames * anim_delay / TILESIZE;

  boolean draw_start_tile = (cut_mode != CUT_ABOVE);	/* only for falling! */
  boolean draw_end_tile   = (cut_mode != CUT_BELOW);	/* only for falling! */

  /* re-calculate animation frame for two-tile movement animation */
  frame = getGraphicAnimationFrame(graphic, sync_frame);

  /* check if movement start graphic inside screen area and should be drawn */
  if (draw_start_tile && IN_SCR_FIELD(x1, y1))
  {
    getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, TRUE);

    dst_x = FX + x1 * TILEX_VAR;
    dst_y = FY + y1 * TILEY_VAR;

    if (mask_mode == USE_MASKING)
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x1, y1);
  }

  /* check if movement end graphic inside screen area and should be drawn */
  if (draw_end_tile && IN_SCR_FIELD(x2, y2))
  {
    getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, FALSE);

    dst_x = FX + x2 * TILEX_VAR;
    dst_y = FY + y2 * TILEY_VAR;

    if (mask_mode == USE_MASKING)
      BlitBitmapMasked(src_bitmap, drawto_field, src_x, src_y, width, height,
		       dst_x, dst_y);
    else
      BlitBitmap(src_bitmap, drawto_field, src_x, src_y, width, height,
		 dst_x, dst_y);

    MarkTileDirty(x2, y2);
  }
}

static void DrawGraphicShifted(int x, int y, int dx, int dy,
			       int graphic, int frame,
			       int cut_mode, int mask_mode)
{
  if (graphic < 0)
  {
    DrawGraphic(x, y, graphic, frame);

    return;
  }

  if (graphic_info[graphic].double_movement)	/* EM style movement images */
    DrawGraphicShiftedDouble(x, y, dx, dy, graphic, frame, cut_mode,mask_mode);
  else
    DrawGraphicShiftedNormal(x, y, dx, dy, graphic, frame, cut_mode,mask_mode);
}

void DrawGraphicShiftedThruMask(int x, int y, int dx, int dy, int graphic,
				int frame, int cut_mode)
{
  DrawGraphicShifted(x, y, dx, dy, graphic, frame, cut_mode, USE_MASKING);
}

void DrawScreenElementExt(int x, int y, int dx, int dy, int element,
			  int cut_mode, int mask_mode)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int graphic;
  int frame;

  if (IN_LEV_FIELD(lx, ly))
  {
    SetRandomAnimationValue(lx, ly);

    graphic = el_act_dir2img(element, GfxAction[lx][ly], GfxDir[lx][ly]);
    frame = getGraphicAnimationFrame(graphic, GfxFrame[lx][ly]);

    /* do not use double (EM style) movement graphic when not moving */
    if (graphic_info[graphic].double_movement && !dx && !dy)
    {
      graphic = el_act_dir2img(element, ACTION_DEFAULT, GfxDir[lx][ly]);
      frame = getGraphicAnimationFrame(graphic, GfxFrame[lx][ly]);
    }
  }
  else	/* border element */
  {
    graphic = el2img(element);
    frame = getGraphicAnimationFrame(graphic, -1);
  }

  if (element == EL_EXPANDABLE_WALL)
  {
    boolean left_stopped = FALSE, right_stopped = FALSE;

    if (!IN_LEV_FIELD(lx - 1, ly) || IS_WALL(Feld[lx - 1][ly]))
      left_stopped = TRUE;
    if (!IN_LEV_FIELD(lx + 1, ly) || IS_WALL(Feld[lx + 1][ly]))
      right_stopped = TRUE;

    if (left_stopped && right_stopped)
      graphic = IMG_WALL;
    else if (left_stopped)
    {
      graphic = IMG_EXPANDABLE_WALL_GROWING_RIGHT;
      frame = graphic_info[graphic].anim_frames - 1;
    }
    else if (right_stopped)
    {
      graphic = IMG_EXPANDABLE_WALL_GROWING_LEFT;
      frame = graphic_info[graphic].anim_frames - 1;
    }
  }

  if (dx || dy)
    DrawGraphicShifted(x, y, dx, dy, graphic, frame, cut_mode, mask_mode);
  else if (mask_mode == USE_MASKING)
    DrawGraphicThruMask(x, y, graphic, frame);
  else
    DrawGraphic(x, y, graphic, frame);
}

void DrawLevelElementExt(int x, int y, int dx, int dy, int element,
			 int cut_mode, int mask_mode)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElementExt(SCREENX(x), SCREENY(y), dx, dy, element,
			 cut_mode, mask_mode);
}

void DrawScreenElementShifted(int x, int y, int dx, int dy, int element,
			      int cut_mode)
{
  DrawScreenElementExt(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementShifted(int x, int y, int dx, int dy, int element,
			     int cut_mode)
{
  DrawLevelElementExt(x, y, dx, dy, element, cut_mode, NO_MASKING);
}

void DrawLevelElementThruMask(int x, int y, int element)
{
  DrawLevelElementExt(x, y, 0, 0, element, NO_CUTTING, USE_MASKING);
}

void DrawLevelFieldThruMask(int x, int y)
{
  DrawLevelElementExt(x, y, 0, 0, Feld[x][y], NO_CUTTING, USE_MASKING);
}

/* !!! implementation of quicksand is totally broken !!! */
#define IS_CRUMBLED_TILE(x, y, e)					\
	(GFX_CRUMBLED(e) && (!IN_LEV_FIELD(x, y) ||			\
			     !IS_MOVING(x, y) ||			\
			     (e) == EL_QUICKSAND_EMPTYING ||		\
			     (e) == EL_QUICKSAND_FAST_EMPTYING))

static void DrawLevelFieldCrumbledInnerCorners(int x, int y, int dx, int dy,
					       int graphic)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int width, height, cx, cy;
  int sx = SCREENX(x), sy = SCREENY(y);
  int crumbled_border_size = graphic_info[graphic].border_size;
  int crumbled_tile_size = graphic_info[graphic].tile_size;
  int crumbled_border_size_var =
    crumbled_border_size * TILESIZE_VAR / crumbled_tile_size;
  int i;

  getGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  for (i = 1; i < 4; i++)
  {
    int dxx = (i & 1 ? dx : 0);
    int dyy = (i & 2 ? dy : 0);
    int xx = x + dxx;
    int yy = y + dyy;
    int element = (IN_LEV_FIELD(xx, yy) ? TILE_GFX_ELEMENT(xx, yy) :
		   BorderElement);

    /* check if neighbour field is of same crumble type */
    boolean same = (IS_CRUMBLED_TILE(xx, yy, element) &&
		    graphic_info[graphic].class ==
		    graphic_info[el_act2crm(element, ACTION_DEFAULT)].class);

    /* return if check prevents inner corner */
    if (same == (dxx == dx && dyy == dy))
      return;
  }

  /* if we reach this point, we have an inner corner */

  getGraphicSource(graphic, 1, &src_bitmap, &src_x, &src_y);

  width  = crumbled_border_size_var;
  height = crumbled_border_size_var;
  cx = (dx > 0 ? TILESIZE_VAR - width  : 0);
  cy = (dy > 0 ? TILESIZE_VAR - height : 0);

  BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy,
	     width, height, FX + sx * TILEX_VAR + cx, FY + sy * TILEY_VAR + cy);
}

static void DrawLevelFieldCrumbledBorders(int x, int y, int graphic, int frame,
					  int dir)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int width, height, bx, by, cx, cy;
  int sx = SCREENX(x), sy = SCREENY(y);
  int crumbled_border_size = graphic_info[graphic].border_size;
  int crumbled_tile_size = graphic_info[graphic].tile_size;
  int crumbled_border_size_var =
    crumbled_border_size * TILESIZE_VAR / crumbled_tile_size;
  int crumbled_border_pos_var = TILESIZE_VAR - crumbled_border_size_var;
  int i;

  getGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);

  /* draw simple, sloppy, non-corner-accurate crumbled border */

  width  = (dir == 1 || dir == 2 ? crumbled_border_size_var : TILESIZE_VAR);
  height = (dir == 0 || dir == 3 ? crumbled_border_size_var : TILESIZE_VAR);
  cx = (dir == 2 ? crumbled_border_pos_var : 0);
  cy = (dir == 3 ? crumbled_border_pos_var : 0);

  BlitBitmap(src_bitmap, drawto_field, src_x + cx, src_y + cy, width, height,
	     FX + sx * TILEX_VAR + cx,
	     FY + sy * TILEY_VAR + cy);

  /* (remaining middle border part must be at least as big as corner part) */
  if (!(graphic_info[graphic].style & STYLE_ACCURATE_BORDERS) ||
      crumbled_border_size_var >= TILESIZE_VAR / 3)
    return;

  /* correct corners of crumbled border, if needed */

  for (i = -1; i <= 1; i += 2)
  {
    int xx = x + (dir == 0 || dir == 3 ? i : 0);
    int yy = y + (dir == 1 || dir == 2 ? i : 0);
    int element = (IN_LEV_FIELD(xx, yy) ? TILE_GFX_ELEMENT(xx, yy) :
		   BorderElement);

    /* check if neighbour field is of same crumble type */
    if (IS_CRUMBLED_TILE(xx, yy, element) &&
	graphic_info[graphic].class ==
	graphic_info[el_act2crm(element, ACTION_DEFAULT)].class)
    {
      /* no crumbled corner, but continued crumbled border */

      int c1 = (dir == 2 || dir == 3 ? crumbled_border_pos_var : 0);
      int c2 = (i == 1 ? crumbled_border_pos_var : 0);
      int b1 = (i == 1 ? crumbled_border_size_var :
		TILESIZE_VAR - 2 * crumbled_border_size_var);

      width  = crumbled_border_size_var;
      height = crumbled_border_size_var;

      if (dir == 1 || dir == 2)
      {
	cx = c1;
	cy = c2;
	bx = cx;
	by = b1;
      }
      else
      {
	cx = c2;
	cy = c1;
	bx = b1;
	by = cy;
      }

      BlitBitmap(src_bitmap, drawto_field, src_x + bx, src_y + by,
		 width, height,
		 FX + sx * TILEX_VAR + cx,
		 FY + sy * TILEY_VAR + cy);
    }
  }
}

static void DrawLevelFieldCrumbledExt(int x, int y, int graphic, int frame)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int element;
  int i;
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };

  if (!IN_LEV_FIELD(x, y))
    return;

  element = TILE_GFX_ELEMENT(x, y);

  if (IS_CRUMBLED_TILE(x, y, element))		/* crumble field itself */
  {
    if (!IN_SCR_FIELD(sx, sy))
      return;

    /* crumble field borders towards direct neighbour fields */
    for (i = 0; i < 4; i++)
    {
      int xx = x + xy[i][0];
      int yy = y + xy[i][1];

      element = (IN_LEV_FIELD(xx, yy) ? TILE_GFX_ELEMENT(xx, yy) :
		 BorderElement);

      /* check if neighbour field is of same crumble type */
      if (IS_CRUMBLED_TILE(xx, yy, element) &&
	  graphic_info[graphic].class ==
	  graphic_info[el_act2crm(element, ACTION_DEFAULT)].class)
	continue;

      DrawLevelFieldCrumbledBorders(x, y, graphic, frame, i);
    }

    /* crumble inner field corners towards corner neighbour fields */
    if ((graphic_info[graphic].style & STYLE_INNER_CORNERS) &&
	graphic_info[graphic].anim_frames == 2)
    {
      for (i = 0; i < 4; i++)
      {
	int dx = (i & 1 ? +1 : -1);
	int dy = (i & 2 ? +1 : -1);

	DrawLevelFieldCrumbledInnerCorners(x, y, dx, dy, graphic);
      }
    }

    MarkTileDirty(sx, sy);
  }
  else		/* center field is not crumbled -- crumble neighbour fields */
  {
    /* crumble field borders of direct neighbour fields */
    for (i = 0; i < 4; i++)
    {
      int xx = x + xy[i][0];
      int yy = y + xy[i][1];
      int sxx = sx + xy[i][0];
      int syy = sy + xy[i][1];

      if (!IN_LEV_FIELD(xx, yy) ||
	  !IN_SCR_FIELD(sxx, syy))
	continue;

      if (Feld[xx][yy] == EL_ELEMENT_SNAPPING)
	continue;

      element = TILE_GFX_ELEMENT(xx, yy);

      if (!IS_CRUMBLED_TILE(xx, yy, element))
	continue;

      graphic = el_act2crm(element, ACTION_DEFAULT);

      DrawLevelFieldCrumbledBorders(xx, yy, graphic, 0, 3 - i);

      MarkTileDirty(sxx, syy);
    }

    /* crumble inner field corners of corner neighbour fields */
    for (i = 0; i < 4; i++)
    {
      int dx = (i & 1 ? +1 : -1);
      int dy = (i & 2 ? +1 : -1);
      int xx = x + dx;
      int yy = y + dy;
      int sxx = sx + dx;
      int syy = sy + dy;

      if (!IN_LEV_FIELD(xx, yy) ||
	  !IN_SCR_FIELD(sxx, syy))
	continue;

      if (Feld[xx][yy] == EL_ELEMENT_SNAPPING)
	continue;

      element = TILE_GFX_ELEMENT(xx, yy);

      if (!IS_CRUMBLED_TILE(xx, yy, element))
	continue;

      graphic = el_act2crm(element, ACTION_DEFAULT);

      if ((graphic_info[graphic].style & STYLE_INNER_CORNERS) &&
	  graphic_info[graphic].anim_frames == 2)
	DrawLevelFieldCrumbledInnerCorners(xx, yy, -dx, -dy, graphic);

      MarkTileDirty(sxx, syy);
    }
  }
}

void DrawLevelFieldCrumbled(int x, int y)
{
  int graphic;

  if (!IN_LEV_FIELD(x, y))
    return;

  if (Feld[x][y] == EL_ELEMENT_SNAPPING &&
      GfxElement[x][y] != EL_UNDEFINED &&
      GFX_CRUMBLED(GfxElement[x][y]))
  {
    DrawLevelFieldCrumbledDigging(x, y, GfxDir[x][y], GfxFrame[x][y]);

    return;
  }

  graphic = el_act2crm(TILE_GFX_ELEMENT(x, y), ACTION_DEFAULT);

  DrawLevelFieldCrumbledExt(x, y, graphic, 0);
}

void DrawLevelFieldCrumbledDigging(int x, int y, int direction,
				   int step_frame)
{
  int graphic1 = el_act_dir2img(GfxElement[x][y], ACTION_DIGGING, direction);
  int graphic2 = el_act_dir2crm(GfxElement[x][y], ACTION_DIGGING, direction);
  int frame1 = getGraphicAnimationFrame(graphic1, step_frame);
  int frame2 = getGraphicAnimationFrame(graphic2, step_frame);
  int sx = SCREENX(x), sy = SCREENY(y);

  DrawGraphic(sx, sy, graphic1, frame1);
  DrawLevelFieldCrumbledExt(x, y, graphic2, frame2);
}

void DrawLevelFieldCrumbledNeighbours(int x, int y)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  static int xy[4][2] =
  {
    { 0, -1 },
    { -1, 0 },
    { +1, 0 },
    { 0, +1 }
  };
  int i;

  /* crumble direct neighbour fields (required for field borders) */
  for (i = 0; i < 4; i++)
  {
    int xx = x + xy[i][0];
    int yy = y + xy[i][1];
    int sxx = sx + xy[i][0];
    int syy = sy + xy[i][1];

    if (!IN_LEV_FIELD(xx, yy) ||
	!IN_SCR_FIELD(sxx, syy) ||
	!GFX_CRUMBLED(Feld[xx][yy]) ||
	IS_MOVING(xx, yy))
      continue;

    DrawLevelField(xx, yy);
  }

  /* crumble corner neighbour fields (required for inner field corners) */
  for (i = 0; i < 4; i++)
  {
    int dx = (i & 1 ? +1 : -1);
    int dy = (i & 2 ? +1 : -1);
    int xx = x + dx;
    int yy = y + dy;
    int sxx = sx + dx;
    int syy = sy + dy;

    if (!IN_LEV_FIELD(xx, yy) ||
	!IN_SCR_FIELD(sxx, syy) ||
	!GFX_CRUMBLED(Feld[xx][yy]) ||
	IS_MOVING(xx, yy))
      continue;

    int element = TILE_GFX_ELEMENT(xx, yy);
    int graphic = el_act2crm(element, ACTION_DEFAULT);

    if ((graphic_info[graphic].style & STYLE_INNER_CORNERS) &&
	graphic_info[graphic].anim_frames == 2)
      DrawLevelField(xx, yy);
  }
}

static int getBorderElement(int x, int y)
{
  int border[7][2] =
  {
    { EL_STEELWALL_TOPLEFT,		EL_INVISIBLE_STEELWALL_TOPLEFT     },
    { EL_STEELWALL_TOPRIGHT,		EL_INVISIBLE_STEELWALL_TOPRIGHT    },
    { EL_STEELWALL_BOTTOMLEFT,		EL_INVISIBLE_STEELWALL_BOTTOMLEFT  },
    { EL_STEELWALL_BOTTOMRIGHT,		EL_INVISIBLE_STEELWALL_BOTTOMRIGHT },
    { EL_STEELWALL_VERTICAL,		EL_INVISIBLE_STEELWALL_VERTICAL    },
    { EL_STEELWALL_HORIZONTAL,		EL_INVISIBLE_STEELWALL_HORIZONTAL  },
    { EL_STEELWALL,			EL_INVISIBLE_STEELWALL		   }
  };
  int steel_type = (BorderElement == EL_STEELWALL ? 0 : 1);
  int steel_position = (x == -1		&& y == -1		? 0 :
			x == lev_fieldx	&& y == -1		? 1 :
			x == -1		&& y == lev_fieldy	? 2 :
			x == lev_fieldx	&& y == lev_fieldy	? 3 :
			x == -1		|| x == lev_fieldx	? 4 :
			y == -1		|| y == lev_fieldy	? 5 : 6);

  return border[steel_position][steel_type];
}

void DrawScreenElement(int x, int y, int element)
{
  DrawScreenElementExt(x, y, 0, 0, element, NO_CUTTING, NO_MASKING);
  DrawLevelFieldCrumbled(LEVELX(x), LEVELY(y));
}

void DrawLevelElement(int x, int y, int element)
{
  if (IN_LEV_FIELD(x, y) && IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenElement(SCREENX(x), SCREENY(y), element);
}

void DrawScreenField(int x, int y)
{
  int lx = LEVELX(x), ly = LEVELY(y);
  int element, content;

  if (!IN_LEV_FIELD(lx, ly))
  {
    if (lx < -1 || lx > lev_fieldx || ly < -1 || ly > lev_fieldy)
      element = EL_EMPTY;
    else
      element = getBorderElement(lx, ly);

    DrawScreenElement(x, y, element);

    return;
  }

  element = Feld[lx][ly];
  content = Store[lx][ly];

  if (IS_MOVING(lx, ly))
  {
    int horiz_move = (MovDir[lx][ly] == MV_LEFT || MovDir[lx][ly] == MV_RIGHT);
    boolean cut_mode = NO_CUTTING;

    if (element == EL_QUICKSAND_EMPTYING ||
	element == EL_QUICKSAND_FAST_EMPTYING ||
	element == EL_MAGIC_WALL_EMPTYING ||
	element == EL_BD_MAGIC_WALL_EMPTYING ||
	element == EL_DC_MAGIC_WALL_EMPTYING ||
	element == EL_AMOEBA_DROPPING)
      cut_mode = CUT_ABOVE;
    else if (element == EL_QUICKSAND_FILLING ||
	     element == EL_QUICKSAND_FAST_FILLING ||
	     element == EL_MAGIC_WALL_FILLING ||
	     element == EL_BD_MAGIC_WALL_FILLING ||
	     element == EL_DC_MAGIC_WALL_FILLING)
      cut_mode = CUT_BELOW;

    if (cut_mode == CUT_ABOVE)
      DrawScreenElement(x, y, element);
    else
      DrawScreenElement(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted(x, y, MovPos[lx][ly], 0, element, NO_CUTTING);
    else if (cut_mode == NO_CUTTING)
      DrawScreenElementShifted(x, y, 0, MovPos[lx][ly], element, cut_mode);
    else
    {
      DrawScreenElementShifted(x, y, 0, MovPos[lx][ly], content, cut_mode);

      if (cut_mode == CUT_BELOW &&
	  IN_LEV_FIELD(lx, ly + 1) && IN_SCR_FIELD(x, y + 1))
	DrawLevelElement(lx, ly + 1, element);
    }

    if (content == EL_ACID)
    {
      int dir = MovDir[lx][ly];
      int newlx = lx + (dir == MV_LEFT ? -1 : dir == MV_RIGHT ? +1 : 0);
      int newly = ly + (dir == MV_UP   ? -1 : dir == MV_DOWN  ? +1 : 0);

      DrawLevelElementThruMask(newlx, newly, EL_ACID);

      // prevent target field from being drawn again (but without masking)
      // (this would happen if target field is scanned after moving element)
      Stop[newlx][newly] = TRUE;
    }
  }
  else if (IS_BLOCKED(lx, ly))
  {
    int oldx, oldy;
    int sx, sy;
    int horiz_move;
    boolean cut_mode = NO_CUTTING;
    int element_old, content_old;

    Blocked2Moving(lx, ly, &oldx, &oldy);
    sx = SCREENX(oldx);
    sy = SCREENY(oldy);
    horiz_move = (MovDir[oldx][oldy] == MV_LEFT ||
		  MovDir[oldx][oldy] == MV_RIGHT);

    element_old = Feld[oldx][oldy];
    content_old = Store[oldx][oldy];

    if (element_old == EL_QUICKSAND_EMPTYING ||
	element_old == EL_QUICKSAND_FAST_EMPTYING ||
	element_old == EL_MAGIC_WALL_EMPTYING ||
	element_old == EL_BD_MAGIC_WALL_EMPTYING ||
	element_old == EL_DC_MAGIC_WALL_EMPTYING ||
	element_old == EL_AMOEBA_DROPPING)
      cut_mode = CUT_ABOVE;

    DrawScreenElement(x, y, EL_EMPTY);

    if (horiz_move)
      DrawScreenElementShifted(sx, sy, MovPos[oldx][oldy], 0, element_old,
			       NO_CUTTING);
    else if (cut_mode == NO_CUTTING)
      DrawScreenElementShifted(sx, sy, 0, MovPos[oldx][oldy], element_old,
			       cut_mode);
    else
      DrawScreenElementShifted(sx, sy, 0, MovPos[oldx][oldy], content_old,
			       cut_mode);
  }
  else if (IS_DRAWABLE(element))
    DrawScreenElement(x, y, element);
  else
    DrawScreenElement(x, y, EL_EMPTY);
}

void DrawLevelField(int x, int y)
{
  if (IN_SCR_FIELD(SCREENX(x), SCREENY(y)))
    DrawScreenField(SCREENX(x), SCREENY(y));
  else if (IS_MOVING(x, y))
  {
    int newx,newy;

    Moving2Blocked(x, y, &newx, &newy);
    if (IN_SCR_FIELD(SCREENX(newx), SCREENY(newy)))
      DrawScreenField(SCREENX(newx), SCREENY(newy));
  }
  else if (IS_BLOCKED(x, y))
  {
    int oldx, oldy;

    Blocked2Moving(x, y, &oldx, &oldy);
    if (IN_SCR_FIELD(SCREENX(oldx), SCREENY(oldy)))
      DrawScreenField(SCREENX(oldx), SCREENY(oldy));
  }
}

static void DrawSizedWallExt_MM(int dst_x, int dst_y, int element, int tilesize,
				int (*el2img_function)(int), boolean masked,
				int element_bits_draw)
{
  int element_base = map_mm_wall_element(element);
  int element_bits = (IS_DF_WALL(element) ?
		      element - EL_DF_WALL_START :
		      IS_MM_WALL(element) ?
		      element - EL_MM_WALL_START : EL_EMPTY) & 0x000f;
  int graphic = el2img_function(element_base);
  int tilesize_draw = tilesize / 2;
  Bitmap *src_bitmap;
  int src_x, src_y;
  int i;

  getSizedGraphicSource(graphic, 0, tilesize_draw, &src_bitmap, &src_x, &src_y);

  for (i = 0; i < 4; i++)
  {
    int dst_draw_x = dst_x + (i % 2) * tilesize_draw;
    int dst_draw_y = dst_y + (i / 2) * tilesize_draw;

    if (!(element_bits_draw & (1 << i)))
      continue;

    if (element_bits & (1 << i))
    {
      if (masked)
	BlitBitmapMasked(src_bitmap, drawto, src_x, src_y,
			 tilesize_draw, tilesize_draw, dst_draw_x, dst_draw_y);
      else
	BlitBitmap(src_bitmap, drawto, src_x, src_y,
		   tilesize_draw, tilesize_draw, dst_draw_x, dst_draw_y);
    }
    else
    {
      if (!masked)
	ClearRectangle(drawto, dst_draw_x, dst_draw_y,
		       tilesize_draw, tilesize_draw);
    }
  }
}

void DrawSizedWallParts_MM(int x, int y, int element, int tilesize,
			   boolean masked, int element_bits_draw)
{
  DrawSizedWallExt_MM(SX + x * tilesize, SY + y * tilesize,
		      element, tilesize, el2edimg, masked, element_bits_draw);
}

void DrawSizedWall_MM(int dst_x, int dst_y, int element, int tilesize,
		      int (*el2img_function)(int))
{
  DrawSizedWallExt_MM(dst_x, dst_y, element, tilesize, el2img_function, FALSE,
		      0x000f);
}

void DrawSizedElementExt(int x, int y, int element, int tilesize,
			 boolean masked)
{
  if (IS_MM_WALL(element))
  {
    DrawSizedWallExt_MM(SX + x * tilesize, SY + y * tilesize,
			element, tilesize, el2edimg, masked, 0x000f);
  }
  else
  {
    int graphic = el2edimg(element);

    if (masked)
      DrawSizedGraphicThruMask(x, y, graphic, 0, tilesize);
    else
      DrawSizedGraphic(x, y, graphic, 0, tilesize);
  }
}

void DrawSizedElement(int x, int y, int element, int tilesize)
{
  DrawSizedElementExt(x, y, element, tilesize, FALSE);
}

void DrawSizedElementThruMask(int x, int y, int element, int tilesize)
{
  DrawSizedElementExt(x, y, element, tilesize, TRUE);
}

void DrawMiniElement(int x, int y, int element)
{
  int graphic;

  graphic = el2edimg(element);
  DrawMiniGraphic(x, y, graphic);
}

void DrawSizedElementOrWall(int sx, int sy, int scroll_x, int scroll_y,
			    int tilesize)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawSizedElement(sx, sy, EL_EMPTY, tilesize);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawSizedElement(sx, sy, Feld[x][y], tilesize);
  else
    DrawSizedGraphic(sx, sy, el2edimg(getBorderElement(x, y)), 0, tilesize);
}

void DrawMiniElementOrWall(int sx, int sy, int scroll_x, int scroll_y)
{
  int x = sx + scroll_x, y = sy + scroll_y;

  if (x < -1 || x > lev_fieldx || y < -1 || y > lev_fieldy)
    DrawMiniElement(sx, sy, EL_EMPTY);
  else if (x > -1 && x < lev_fieldx && y > -1 && y < lev_fieldy)
    DrawMiniElement(sx, sy, Feld[x][y]);
  else
    DrawMiniGraphic(sx, sy, el2edimg(getBorderElement(x, y)));
}

void DrawEnvelopeBackgroundTiles(int graphic, int startx, int starty,
				 int x, int y, int xsize, int ysize,
				 int tile_width, int tile_height)
{
  Bitmap *src_bitmap;
  int src_x, src_y;
  int dst_x = startx + x * tile_width;
  int dst_y = starty + y * tile_height;
  int width  = graphic_info[graphic].width;
  int height = graphic_info[graphic].height;
  int inner_width_raw  = MAX(width  - 2 * tile_width,  tile_width);
  int inner_height_raw = MAX(height - 2 * tile_height, tile_height);
  int inner_width  = inner_width_raw  - (inner_width_raw  % tile_width);
  int inner_height = inner_height_raw - (inner_height_raw % tile_height);
  int inner_sx = (width  >= 3 * tile_width  ? tile_width  : 0);
  int inner_sy = (height >= 3 * tile_height ? tile_height : 0);
  boolean draw_masked = graphic_info[graphic].draw_masked;

  getFixedGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);

  if (src_bitmap == NULL || width < tile_width || height < tile_height)
  {
    ClearRectangle(drawto, dst_x, dst_y, tile_width, tile_height);
    return;
  }

  src_x += (x == 0 ? 0 : x == xsize - 1 ? width  - tile_width  :
	    inner_sx + (x - 1) * tile_width  % inner_width);
  src_y += (y == 0 ? 0 : y == ysize - 1 ? height - tile_height :
	    inner_sy + (y - 1) * tile_height % inner_height);

  if (draw_masked)
    BlitBitmapMasked(src_bitmap, drawto, src_x, src_y, tile_width, tile_height,
		     dst_x, dst_y);
  else
    BlitBitmap(src_bitmap, drawto, src_x, src_y, tile_width, tile_height,
	       dst_x, dst_y);
}

void DrawEnvelopeBackground(int graphic, int startx, int starty,
			    int x, int y, int xsize, int ysize, int font_nr)
{
  int font_width  = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);

  DrawEnvelopeBackgroundTiles(graphic, startx, starty, x, y, xsize, ysize,
			      font_width, font_height);
}

void AnimateEnvelope(int envelope_nr, int anim_mode, int action)
{
  int graphic = IMG_BACKGROUND_ENVELOPE_1 + envelope_nr;
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int mask_mode = (src_bitmap != NULL ? BLIT_MASKED : BLIT_ON_BACKGROUND);
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  unsigned int anim_delay = 0;
  int frame_delay_value = (ffwd_delay ? FfwdFrameDelay : GameFrameDelay);
  int anim_delay_value = MAX(1, (no_delay ? 0 : frame_delay_value) / 2);
  int font_nr = FONT_ENVELOPE_1 + envelope_nr;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int max_xsize = level.envelope[envelope_nr].xsize;
  int max_ysize = level.envelope[envelope_nr].ysize;
  int xstart = (anim_mode & ANIM_VERTICAL ? max_xsize : 0);
  int ystart = (anim_mode & ANIM_HORIZONTAL ? max_ysize : 0);
  int xend = max_xsize;
  int yend = (anim_mode != ANIM_DEFAULT ? max_ysize : 0);
  int xstep = (xstart < xend ? 1 : 0);
  int ystep = (ystart < yend || xstep == 0 ? 1 : 0);
  int start = 0;
  int end = MAX(xend - xstart, yend - ystart);
  int i;

  for (i = start; i <= end; i++)
  {
    int last_frame = end;	// last frame of this "for" loop
    int x = xstart + i * xstep;
    int y = ystart + i * ystep;
    int xsize = (action == ACTION_CLOSING ? xend - (x - xstart) : x) + 2;
    int ysize = (action == ACTION_CLOSING ? yend - (y - ystart) : y) + 2;
    int sx = SX + (SXSIZE - xsize * font_width)  / 2;
    int sy = SY + (SYSIZE - ysize * font_height) / 2;
    int xx, yy;

    SetDrawtoField(DRAW_TO_FIELDBUFFER);

    BlitScreenToBitmap(backbuffer);

    SetDrawtoField(DRAW_TO_BACKBUFFER);

    for (yy = 0; yy < ysize; yy++)
      for (xx = 0; xx < xsize; xx++)
	DrawEnvelopeBackground(graphic, sx, sy, xx, yy, xsize, ysize, font_nr);

    DrawTextBuffer(sx + font_width, sy + font_height,
		   level.envelope[envelope_nr].text, font_nr, max_xsize,
		   xsize - 2, ysize - 2, 0, mask_mode,
		   level.envelope[envelope_nr].autowrap,
		   level.envelope[envelope_nr].centered, FALSE);

    redraw_mask |= REDRAW_FIELD;
    BackToFront();

    SkipUntilDelayReached(&anim_delay, anim_delay_value, &i, last_frame);
  }
}

void ShowEnvelope(int envelope_nr)
{
  int element = EL_ENVELOPE_1 + envelope_nr;
  int graphic = IMG_BACKGROUND_ENVELOPE_1 + envelope_nr;
  int sound_opening = element_info[element].sound[ACTION_OPENING];
  int sound_closing = element_info[element].sound[ACTION_CLOSING];
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int normal_delay_value = ONE_SECOND_DELAY / (ffwd_delay ? 2 : 1);
  int wait_delay_value = (no_delay ? 0 : normal_delay_value);
  int anim_mode = graphic_info[graphic].anim_mode;
  int main_anim_mode = (anim_mode == ANIM_NONE ? ANIM_VERTICAL|ANIM_HORIZONTAL:
			anim_mode == ANIM_DEFAULT ? ANIM_VERTICAL : anim_mode);

  game.envelope_active = TRUE;	/* needed for RedrawPlayfield() events */

  PlayMenuSoundStereo(sound_opening, SOUND_MIDDLE);

  if (anim_mode == ANIM_DEFAULT)
    AnimateEnvelope(envelope_nr, ANIM_DEFAULT, ACTION_OPENING);

  AnimateEnvelope(envelope_nr, main_anim_mode, ACTION_OPENING);

  if (tape.playing)
    Delay(wait_delay_value);
  else
    WaitForEventToContinue();

  PlayMenuSoundStereo(sound_closing, SOUND_MIDDLE);

  if (anim_mode != ANIM_NONE)
    AnimateEnvelope(envelope_nr, main_anim_mode, ACTION_CLOSING);

  if (anim_mode == ANIM_DEFAULT)
    AnimateEnvelope(envelope_nr, ANIM_DEFAULT, ACTION_CLOSING);

  game.envelope_active = FALSE;

  SetDrawtoField(DRAW_TO_FIELDBUFFER);

  redraw_mask |= REDRAW_FIELD;
  BackToFront();
}

static void setRequestBasePosition(int *x, int *y)
{
  int sx_base, sy_base;

  if (request.x != -1)
    sx_base = request.x;
  else if (request.align == ALIGN_LEFT)
    sx_base = SX;
  else if (request.align == ALIGN_RIGHT)
    sx_base = SX + SXSIZE;
  else
    sx_base = SX + SXSIZE / 2;

  if (request.y != -1)
    sy_base = request.y;
  else if (request.valign == VALIGN_TOP)
    sy_base = SY;
  else if (request.valign == VALIGN_BOTTOM)
    sy_base = SY + SYSIZE;
  else
    sy_base = SY + SYSIZE / 2;

  *x = sx_base;
  *y = sy_base;
}

static void setRequestPositionExt(int *x, int *y, int width, int height,
				  boolean add_border_size)
{
  int border_size = request.border_size;
  int sx_base, sy_base;
  int sx, sy;

  setRequestBasePosition(&sx_base, &sy_base);

  if (request.align == ALIGN_LEFT)
    sx = sx_base;
  else if (request.align == ALIGN_RIGHT)
    sx = sx_base - width;
  else
    sx = sx_base - width  / 2;

  if (request.valign == VALIGN_TOP)
    sy = sy_base;
  else if (request.valign == VALIGN_BOTTOM)
    sy = sy_base - height;
  else
    sy = sy_base - height / 2;

  sx = MAX(0, MIN(sx, WIN_XSIZE - width));
  sy = MAX(0, MIN(sy, WIN_YSIZE - height));

  if (add_border_size)
  {
    sx += border_size;
    sy += border_size;
  }

  *x = sx;
  *y = sy;
}

static void setRequestPosition(int *x, int *y, boolean add_border_size)
{
  setRequestPositionExt(x, y, request.width, request.height, add_border_size);
}

void DrawEnvelopeRequest(char *text)
{
  char *text_final = text;
  char *text_door_style = NULL;
  int graphic = IMG_BACKGROUND_REQUEST;
  Bitmap *src_bitmap = graphic_info[graphic].bitmap;
  int mask_mode = (src_bitmap != NULL ? BLIT_MASKED : BLIT_ON_BACKGROUND);
  int font_nr = FONT_REQUEST;
  int font_width = getFontWidth(font_nr);
  int font_height = getFontHeight(font_nr);
  int border_size = request.border_size;
  int line_spacing = request.line_spacing;
  int line_height = font_height + line_spacing;
  int max_text_width  = request.width  - 2 * border_size;
  int max_text_height = request.height - 2 * border_size;
  int line_length = max_text_width  / font_width;
  int max_lines   = max_text_height / line_height;
  int text_width = line_length * font_width;
  int width = request.width;
  int height = request.height;
  int tile_size = MAX(request.step_offset, 1);
  int x_steps = width  / tile_size;
  int y_steps = height / tile_size;
  int sx_offset = border_size;
  int sy_offset = border_size;
  int sx, sy;
  int i, x, y;

  if (request.centered)
    sx_offset = (request.width - text_width) / 2;

  if (request.wrap_single_words && !request.autowrap)
  {
    char *src_text_ptr, *dst_text_ptr;

    text_door_style = checked_malloc(2 * strlen(text) + 1);

    src_text_ptr = text;
    dst_text_ptr = text_door_style;

    while (*src_text_ptr)
    {
      if (*src_text_ptr == ' ' ||
	  *src_text_ptr == '?' ||
	  *src_text_ptr == '!')
	*dst_text_ptr++ = '\n';

      if (*src_text_ptr != ' ')
	*dst_text_ptr++ = *src_text_ptr;

      src_text_ptr++;
    }

    *dst_text_ptr = '\0';

    text_final = text_door_style;
  }

  setRequestPosition(&sx, &sy, FALSE);

  ClearRectangle(backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE);

  for (y = 0; y < y_steps; y++)
    for (x = 0; x < x_steps; x++)
      DrawEnvelopeBackgroundTiles(graphic, sx, sy,
				  x, y, x_steps, y_steps,
				  tile_size, tile_size);

  /* force DOOR font inside door area */
  SetFontStatus(GAME_MODE_PSEUDO_DOOR);

  DrawTextBuffer(sx + sx_offset, sy + sy_offset, text_final, font_nr,
		 line_length, -1, max_lines, line_spacing, mask_mode,
		 request.autowrap, request.centered, FALSE);

  ResetFontStatus();

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
    RedrawGadget(tool_gadget[i]);

  // store readily prepared envelope request for later use when animating
  BlitBitmap(backbuffer, bitmap_db_store_2, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

  if (text_door_style)
    free(text_door_style);
}

void AnimateEnvelopeRequest(int anim_mode, int action)
{
  int graphic = IMG_BACKGROUND_REQUEST;
  boolean draw_masked = graphic_info[graphic].draw_masked;
  int delay_value_normal = request.step_delay;
  int delay_value_fast = delay_value_normal / 2;
  boolean ffwd_delay = (tape.playing && tape.fast_forward);
  boolean no_delay = (tape.warp_forward);
  int delay_value = (ffwd_delay ? delay_value_fast : delay_value_normal);
  int anim_delay_value = MAX(1, (no_delay ? 0 : delay_value + 500 * 0) / 2);
  unsigned int anim_delay = 0;

  int tile_size = MAX(request.step_offset, 1);
  int max_xsize = request.width  / tile_size;
  int max_ysize = request.height / tile_size;
  int max_xsize_inner = max_xsize - 2;
  int max_ysize_inner = max_ysize - 2;

  int xstart = (anim_mode & ANIM_VERTICAL ? max_xsize_inner : 0);
  int ystart = (anim_mode & ANIM_HORIZONTAL ? max_ysize_inner : 0);
  int xend = max_xsize_inner;
  int yend = (anim_mode != ANIM_DEFAULT ? max_ysize_inner : 0);
  int xstep = (xstart < xend ? 1 : 0);
  int ystep = (ystart < yend || xstep == 0 ? 1 : 0);
  int start = 0;
  int end = MAX(xend - xstart, yend - ystart);
  int i;

  if (setup.quick_doors)
  {
    xstart = xend;
    ystart = yend;
    end = 0;
  }

  for (i = start; i <= end; i++)
  {
    int last_frame = end;	// last frame of this "for" loop
    int x = xstart + i * xstep;
    int y = ystart + i * ystep;
    int xsize = (action == ACTION_CLOSING ? xend - (x - xstart) : x) + 2;
    int ysize = (action == ACTION_CLOSING ? yend - (y - ystart) : y) + 2;
    int xsize_size_left = (xsize - 1) * tile_size;
    int ysize_size_top  = (ysize - 1) * tile_size;
    int max_xsize_pos = (max_xsize - 1) * tile_size;
    int max_ysize_pos = (max_ysize - 1) * tile_size;
    int width  = xsize * tile_size;
    int height = ysize * tile_size;
    int src_x, src_y;
    int dst_x, dst_y;
    int xx, yy;

    setRequestPosition(&src_x, &src_y, FALSE);
    setRequestPositionExt(&dst_x, &dst_y, width, height, FALSE);

    BlitBitmap(bitmap_db_store_1, backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    for (yy = 0; yy < 2; yy++)
    {
      for (xx = 0; xx < 2; xx++)
      {
	int src_xx = src_x + xx * max_xsize_pos;
	int src_yy = src_y + yy * max_ysize_pos;
	int dst_xx = dst_x + xx * xsize_size_left;
	int dst_yy = dst_y + yy * ysize_size_top;
	int xx_size = (xx ? tile_size : xsize_size_left);
	int yy_size = (yy ? tile_size : ysize_size_top);

	if (draw_masked)
	  BlitBitmapMasked(bitmap_db_store_2, backbuffer,
			   src_xx, src_yy, xx_size, yy_size, dst_xx, dst_yy);
	else
	  BlitBitmap(bitmap_db_store_2, backbuffer,
		     src_xx, src_yy, xx_size, yy_size, dst_xx, dst_yy);
      }
    }

    redraw_mask |= REDRAW_FIELD;

    BackToFront();

    SkipUntilDelayReached(&anim_delay, anim_delay_value, &i, last_frame);
  }
}

void ShowEnvelopeRequest(char *text, unsigned int req_state, int action)
{
  int graphic = IMG_BACKGROUND_REQUEST;
  int sound_opening = SND_REQUEST_OPENING;
  int sound_closing = SND_REQUEST_CLOSING;
  int anim_mode_1 = request.anim_mode;			/* (higher priority) */
  int anim_mode_2 = graphic_info[graphic].anim_mode;	/* (lower priority) */
  int anim_mode = (anim_mode_1 != ANIM_DEFAULT ? anim_mode_1 : anim_mode_2);
  int main_anim_mode = (anim_mode == ANIM_NONE ? ANIM_VERTICAL|ANIM_HORIZONTAL:
			anim_mode == ANIM_DEFAULT ? ANIM_VERTICAL : anim_mode);

  if (game_status == GAME_MODE_PLAYING)
    BlitScreenToBitmap(backbuffer);

  SetDrawtoField(DRAW_TO_BACKBUFFER);

  // SetDrawBackgroundMask(REDRAW_NONE);

  if (action == ACTION_OPENING)
  {
    BlitBitmap(backbuffer, bitmap_db_store_1, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    if (req_state & REQ_ASK)
    {
      MapGadget(tool_gadget[TOOL_CTRL_ID_YES]);
      MapGadget(tool_gadget[TOOL_CTRL_ID_NO]);
    }
    else if (req_state & REQ_CONFIRM)
    {
      MapGadget(tool_gadget[TOOL_CTRL_ID_CONFIRM]);
    }
    else if (req_state & REQ_PLAYER)
    {
      MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_1]);
      MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_2]);
      MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_3]);
      MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_4]);
    }

    DrawEnvelopeRequest(text);
  }

  game.envelope_active = TRUE;	/* needed for RedrawPlayfield() events */

  if (action == ACTION_OPENING)
  {
    PlayMenuSoundStereo(sound_opening, SOUND_MIDDLE);

    if (anim_mode == ANIM_DEFAULT)
      AnimateEnvelopeRequest(ANIM_DEFAULT, ACTION_OPENING);

    AnimateEnvelopeRequest(main_anim_mode, ACTION_OPENING);
  }
  else
  {
    PlayMenuSoundStereo(sound_closing, SOUND_MIDDLE);

    if (anim_mode != ANIM_NONE)
      AnimateEnvelopeRequest(main_anim_mode, ACTION_CLOSING);

    if (anim_mode == ANIM_DEFAULT)
      AnimateEnvelopeRequest(ANIM_DEFAULT, ACTION_CLOSING);
  }

  game.envelope_active = FALSE;

  if (action == ACTION_CLOSING)
    BlitBitmap(bitmap_db_store_1, backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

  // SetDrawBackgroundMask(last_draw_background_mask);

  redraw_mask |= REDRAW_FIELD;

  BackToFront();

  if (action == ACTION_CLOSING &&
      game_status == GAME_MODE_PLAYING &&
      level.game_engine_type == GAME_ENGINE_TYPE_RND)
    SetDrawtoField(DRAW_TO_FIELDBUFFER);
}

void DrawPreviewElement(int dst_x, int dst_y, int element, int tilesize)
{
  if (IS_MM_WALL(element))
  {
    DrawSizedWall_MM(dst_x, dst_y, element, tilesize, el2preimg);
  }
  else
  {
    Bitmap *src_bitmap;
    int src_x, src_y;
    int graphic = el2preimg(element);

    getSizedGraphicSource(graphic, 0, tilesize, &src_bitmap, &src_x, &src_y);
    BlitBitmap(src_bitmap, drawto, src_x, src_y, tilesize, tilesize,
	       dst_x, dst_y);
  }
}

void DrawLevel(int draw_background_mask)
{
  int x,y;

  SetMainBackgroundImage(IMG_BACKGROUND_PLAYING);
  SetDrawBackgroundMask(draw_background_mask);

  ClearField();

  for (x = BX1; x <= BX2; x++)
    for (y = BY1; y <= BY2; y++)
      DrawScreenField(x, y);

  redraw_mask |= REDRAW_FIELD;
}

void DrawSizedLevel(int size_x, int size_y, int scroll_x, int scroll_y,
		    int tilesize)
{
  int x,y;

  for (x = 0; x < size_x; x++)
    for (y = 0; y < size_y; y++)
      DrawSizedElementOrWall(x, y, scroll_x, scroll_y, tilesize);

  redraw_mask |= REDRAW_FIELD;
}

void DrawMiniLevel(int size_x, int size_y, int scroll_x, int scroll_y)
{
  int x,y;

  for (x = 0; x < size_x; x++)
    for (y = 0; y < size_y; y++)
      DrawMiniElementOrWall(x, y, scroll_x, scroll_y);

  redraw_mask |= REDRAW_FIELD;
}

static void DrawPreviewLevelPlayfield(int from_x, int from_y)
{
  boolean show_level_border = (BorderElement != EL_EMPTY);
  int level_xsize = lev_fieldx + (show_level_border ? 2 : 0);
  int level_ysize = lev_fieldy + (show_level_border ? 2 : 0);
  int tile_size = preview.tile_size;
  int preview_width  = preview.xsize * tile_size;
  int preview_height = preview.ysize * tile_size;
  int real_preview_xsize = MIN(level_xsize, preview.xsize);
  int real_preview_ysize = MIN(level_ysize, preview.ysize);
  int real_preview_width  = real_preview_xsize * tile_size;
  int real_preview_height = real_preview_ysize * tile_size;
  int dst_x = SX + ALIGNED_XPOS(preview.x, preview_width, preview.align);
  int dst_y = SY + ALIGNED_YPOS(preview.y, preview_height, preview.valign);
  int x, y;

  if (!IN_GFX_FIELD_FULL(dst_x, dst_y + preview_height - 1))
    return;

  DrawBackground(dst_x, dst_y, preview_width, preview_height);

  dst_x += (preview_width  - real_preview_width)  / 2;
  dst_y += (preview_height - real_preview_height) / 2;

  for (x = 0; x < real_preview_xsize; x++)
  {
    for (y = 0; y < real_preview_ysize; y++)
    {
      int lx = from_x + x + (show_level_border ? -1 : 0);
      int ly = from_y + y + (show_level_border ? -1 : 0);
      int element = (IN_LEV_FIELD(lx, ly) ? level.field[lx][ly] :
		     getBorderElement(lx, ly));

      DrawPreviewElement(dst_x + x * tile_size, dst_y + y * tile_size,
			 element, tile_size);
    }
  }

  redraw_mask |= REDRAW_FIELD;
}

#define MICROLABEL_EMPTY		0
#define MICROLABEL_LEVEL_NAME		1
#define MICROLABEL_LEVEL_AUTHOR_HEAD	2
#define MICROLABEL_LEVEL_AUTHOR		3
#define MICROLABEL_IMPORTED_FROM_HEAD	4
#define MICROLABEL_IMPORTED_FROM	5
#define MICROLABEL_IMPORTED_BY_HEAD	6
#define MICROLABEL_IMPORTED_BY		7

static int getMaxTextLength(struct TextPosInfo *pos, int font_nr)
{
  int max_text_width = SXSIZE;
  int font_width = getFontWidth(font_nr);

  if (pos->align == ALIGN_CENTER)
    max_text_width = (pos->x < SXSIZE / 2 ? pos->x * 2 : (SXSIZE - pos->x) * 2);
  else if (pos->align == ALIGN_RIGHT)
    max_text_width = pos->x;
  else
    max_text_width = SXSIZE - pos->x;

  return max_text_width / font_width;
}

static void DrawPreviewLevelLabelExt(int mode, struct TextPosInfo *pos)
{
  char label_text[MAX_OUTPUT_LINESIZE + 1];
  int max_len_label_text;
  int font_nr = pos->font;
  int i;

  if (!IN_GFX_FIELD_FULL(pos->x, pos->y + getFontHeight(pos->font)))
    return;

  if (mode == MICROLABEL_LEVEL_AUTHOR_HEAD ||
      mode == MICROLABEL_IMPORTED_FROM_HEAD ||
      mode == MICROLABEL_IMPORTED_BY_HEAD)
    font_nr = pos->font_alt;

  max_len_label_text = getMaxTextLength(pos, font_nr);

  if (pos->size != -1)
    max_len_label_text = pos->size;

  for (i = 0; i < max_len_label_text; i++)
    label_text[i] = ' ';
  label_text[max_len_label_text] = '\0';

  if (strlen(label_text) > 0)
    DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);

  strncpy(label_text,
	  (mode == MICROLABEL_LEVEL_NAME ? level.name :
	   mode == MICROLABEL_LEVEL_AUTHOR_HEAD ? "created by" :
	   mode == MICROLABEL_LEVEL_AUTHOR ? level.author :
	   mode == MICROLABEL_IMPORTED_FROM_HEAD ? "imported from" :
	   mode == MICROLABEL_IMPORTED_FROM ? leveldir_current->imported_from :
	   mode == MICROLABEL_IMPORTED_BY_HEAD ? "imported by" :
	   mode == MICROLABEL_IMPORTED_BY ? leveldir_current->imported_by :""),
	  max_len_label_text);
  label_text[max_len_label_text] = '\0';

  if (strlen(label_text) > 0)
    DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);

  redraw_mask |= REDRAW_FIELD;
}

static void DrawPreviewLevelLabel(int mode)
{
  DrawPreviewLevelLabelExt(mode, &menu.main.text.level_info_2);
}

static void DrawPreviewLevelInfo(int mode)
{
  if (mode == MICROLABEL_LEVEL_NAME)
    DrawPreviewLevelLabelExt(mode, &menu.main.text.level_name);
  else if (mode == MICROLABEL_LEVEL_AUTHOR)
    DrawPreviewLevelLabelExt(mode, &menu.main.text.level_author);
}

static void DrawPreviewLevelExt(boolean restart)
{
  static unsigned int scroll_delay = 0;
  static unsigned int label_delay = 0;
  static int from_x, from_y, scroll_direction;
  static int label_state, label_counter;
  unsigned int scroll_delay_value = preview.step_delay;
  boolean show_level_border = (BorderElement != EL_EMPTY);
  int level_xsize = lev_fieldx + (show_level_border ? 2 : 0);
  int level_ysize = lev_fieldy + (show_level_border ? 2 : 0);

  if (restart)
  {
    from_x = 0;
    from_y = 0;

    if (preview.anim_mode == ANIM_CENTERED)
    {
      if (level_xsize > preview.xsize)
	from_x = (level_xsize - preview.xsize) / 2;
      if (level_ysize > preview.ysize)
	from_y = (level_ysize - preview.ysize) / 2;
    }

    from_x += preview.xoffset;
    from_y += preview.yoffset;

    scroll_direction = MV_RIGHT;
    label_state = 1;
    label_counter = 0;

    DrawPreviewLevelPlayfield(from_x, from_y);
    DrawPreviewLevelLabel(label_state);

    DrawPreviewLevelInfo(MICROLABEL_LEVEL_NAME);
    DrawPreviewLevelInfo(MICROLABEL_LEVEL_AUTHOR);

    /* initialize delay counters */
    DelayReached(&scroll_delay, 0);
    DelayReached(&label_delay, 0);

    if (leveldir_current->name)
    {
      struct TextPosInfo *pos = &menu.main.text.level_info_1;
      char label_text[MAX_OUTPUT_LINESIZE + 1];
      int font_nr = pos->font;
      int max_len_label_text = getMaxTextLength(pos, font_nr);

      if (pos->size != -1)
	max_len_label_text = pos->size;

      strncpy(label_text, leveldir_current->name, max_len_label_text);
      label_text[max_len_label_text] = '\0';

      if (IN_GFX_FIELD_FULL(pos->x, pos->y + getFontHeight(pos->font)))
	DrawTextSAligned(pos->x, pos->y, label_text, font_nr, pos->align);
    }

    return;
  }

  /* scroll preview level, if needed */
  if (preview.anim_mode != ANIM_NONE &&
      (level_xsize > preview.xsize || level_ysize > preview.ysize) &&
      DelayReached(&scroll_delay, scroll_delay_value))
  {
    switch (scroll_direction)
    {
      case MV_LEFT:
	if (from_x > 0)
	{
	  from_x -= preview.step_offset;
	  from_x = (from_x < 0 ? 0 : from_x);
	}
	else
	  scroll_direction = MV_UP;
	break;

      case MV_RIGHT:
	if (from_x < level_xsize - preview.xsize)
	{
	  from_x += preview.step_offset;
	  from_x = (from_x > level_xsize - preview.xsize ?
		    level_xsize - preview.xsize : from_x);
	}
	else
	  scroll_direction = MV_DOWN;
	break;

      case MV_UP:
	if (from_y > 0)
	{
	  from_y -= preview.step_offset;
	  from_y = (from_y < 0 ? 0 : from_y);
	}
	else
	  scroll_direction = MV_RIGHT;
	break;

      case MV_DOWN:
	if (from_y < level_ysize - preview.ysize)
	{
	  from_y += preview.step_offset;
	  from_y = (from_y > level_ysize - preview.ysize ?
		    level_ysize - preview.ysize : from_y);
	}
	else
	  scroll_direction = MV_LEFT;
	break;

      default:
	break;
    }

    DrawPreviewLevelPlayfield(from_x, from_y);
  }

  /* !!! THIS ALL SUCKS -- SHOULD BE CLEANLY REWRITTEN !!! */
  /* redraw micro level label, if needed */
  if (!strEqual(level.name, NAMELESS_LEVEL_NAME) &&
      !strEqual(level.author, ANONYMOUS_NAME) &&
      !strEqual(level.author, leveldir_current->name) &&
      DelayReached(&label_delay, MICROLEVEL_LABEL_DELAY))
  {
    int max_label_counter = 23;

    if (leveldir_current->imported_from != NULL &&
	strlen(leveldir_current->imported_from) > 0)
      max_label_counter += 14;
    if (leveldir_current->imported_by != NULL &&
	strlen(leveldir_current->imported_by) > 0)
      max_label_counter += 14;

    label_counter = (label_counter + 1) % max_label_counter;
    label_state = (label_counter >= 0 && label_counter <= 7 ?
		   MICROLABEL_LEVEL_NAME :
		   label_counter >= 9 && label_counter <= 12 ?
		   MICROLABEL_LEVEL_AUTHOR_HEAD :
		   label_counter >= 14 && label_counter <= 21 ?
		   MICROLABEL_LEVEL_AUTHOR :
		   label_counter >= 23 && label_counter <= 26 ?
		   MICROLABEL_IMPORTED_FROM_HEAD :
		   label_counter >= 28 && label_counter <= 35 ?
		   MICROLABEL_IMPORTED_FROM :
		   label_counter >= 37 && label_counter <= 40 ?
		   MICROLABEL_IMPORTED_BY_HEAD :
		   label_counter >= 42 && label_counter <= 49 ?
		   MICROLABEL_IMPORTED_BY : MICROLABEL_EMPTY);

    if (leveldir_current->imported_from == NULL &&
	(label_state == MICROLABEL_IMPORTED_FROM_HEAD ||
	 label_state == MICROLABEL_IMPORTED_FROM))
      label_state = (label_state == MICROLABEL_IMPORTED_FROM_HEAD ?
		     MICROLABEL_IMPORTED_BY_HEAD : MICROLABEL_IMPORTED_BY);

    DrawPreviewLevelLabel(label_state);
  }
}

void DrawPreviewLevelInitial()
{
  DrawPreviewLevelExt(TRUE);
}

void DrawPreviewLevelAnimation()
{
  DrawPreviewLevelExt(FALSE);
}

inline static void DrawGraphicAnimationExt(DrawBuffer *dst_bitmap, int x, int y,
					   int graphic, int sync_frame,
					   int mask_mode)
{
  int frame = getGraphicAnimationFrame(graphic, sync_frame);

  if (mask_mode == USE_MASKING)
    DrawGraphicThruMaskExt(dst_bitmap, x, y, graphic, frame);
  else
    DrawGraphicExt(dst_bitmap, x, y, graphic, frame);
}

void DrawFixedGraphicAnimationExt(DrawBuffer *dst_bitmap, int x, int y,
				  int graphic, int sync_frame, int mask_mode)
{
  int frame = getGraphicAnimationFrame(graphic, sync_frame);

  if (mask_mode == USE_MASKING)
    DrawFixedGraphicThruMaskExt(dst_bitmap, x, y, graphic, frame);
  else
    DrawFixedGraphicExt(dst_bitmap, x, y, graphic, frame);
}

inline static void DrawGraphicAnimation(int x, int y, int graphic)
{
  int lx = LEVELX(x), ly = LEVELY(y);

  if (!IN_SCR_FIELD(x, y))
    return;

  DrawGraphicAnimationExt(drawto_field, FX + x * TILEX_VAR, FY + y * TILEY_VAR,
			  graphic, GfxFrame[lx][ly], NO_MASKING);

  MarkTileDirty(x, y);
}

void DrawFixedGraphicAnimation(int x, int y, int graphic)
{
  int lx = LEVELX(x), ly = LEVELY(y);

  if (!IN_SCR_FIELD(x, y))
    return;

  DrawGraphicAnimationExt(drawto_field, FX + x * TILEX, FY + y * TILEY,
			  graphic, GfxFrame[lx][ly], NO_MASKING);
  MarkTileDirty(x, y);
}

void DrawLevelGraphicAnimation(int x, int y, int graphic)
{
  DrawGraphicAnimation(SCREENX(x), SCREENY(y), graphic);
}

void DrawLevelElementAnimation(int x, int y, int element)
{
  int graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

  DrawGraphicAnimation(SCREENX(x), SCREENY(y), graphic);
}

void DrawLevelGraphicAnimationIfNeeded(int x, int y, int graphic)
{
  int sx = SCREENX(x), sy = SCREENY(y);

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

#if 1
  if (GFX_CRUMBLED(TILE_GFX_ELEMENT(x, y)))
    DrawLevelFieldCrumbled(x, y);
#else
  if (GFX_CRUMBLED(Feld[x][y]))
    DrawLevelFieldCrumbled(x, y);
#endif
}

void DrawLevelElementAnimationIfNeeded(int x, int y, int element)
{
  int sx = SCREENX(x), sy = SCREENY(y);
  int graphic;

  if (!IN_LEV_FIELD(x, y) || !IN_SCR_FIELD(sx, sy))
    return;

  graphic = el_act_dir2img(element, GfxAction[x][y], GfxDir[x][y]);

  if (!IS_NEW_FRAME(GfxFrame[x][y], graphic))
    return;

  DrawGraphicAnimation(sx, sy, graphic);

  if (GFX_CRUMBLED(element))
    DrawLevelFieldCrumbled(x, y);
}

static int getPlayerGraphic(struct PlayerInfo *player, int move_dir)
{
  if (player->use_murphy)
  {
    /* this works only because currently only one player can be "murphy" ... */
    static int last_horizontal_dir = MV_LEFT;
    int graphic = el_act_dir2img(EL_SP_MURPHY, player->GfxAction, move_dir);

    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      last_horizontal_dir = move_dir;

    if (graphic == IMG_SP_MURPHY)	/* undefined => use special graphic */
    {
      int direction = (player->is_snapping ? move_dir : last_horizontal_dir);

      graphic = el_act_dir2img(EL_SP_MURPHY, player->GfxAction, direction);
    }

    return graphic;
  }
  else
    return el_act_dir2img(player->artwork_element, player->GfxAction,move_dir);
}

static boolean equalGraphics(int graphic1, int graphic2)
{
  struct GraphicInfo *g1 = &graphic_info[graphic1];
  struct GraphicInfo *g2 = &graphic_info[graphic2];

  return (g1->bitmap      == g2->bitmap &&
	  g1->src_x       == g2->src_x &&
	  g1->src_y       == g2->src_y &&
	  g1->anim_frames == g2->anim_frames &&
	  g1->anim_delay  == g2->anim_delay &&
	  g1->anim_mode   == g2->anim_mode);
}

void DrawAllPlayers()
{
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].active)
      DrawPlayer(&stored_player[i]);
}

void DrawPlayerField(int x, int y)
{
  if (!IS_PLAYER(x, y))
    return;

  DrawPlayer(PLAYERINFO(x, y));
}

#define DRAW_PLAYER_OVER_PUSHED_ELEMENT	1

void DrawPlayer(struct PlayerInfo *player)
{
  int jx = player->jx;
  int jy = player->jy;
  int move_dir = player->MovDir;
  int dx = (move_dir == MV_LEFT ? -1 : move_dir == MV_RIGHT ? +1 : 0);
  int dy = (move_dir == MV_UP   ? -1 : move_dir == MV_DOWN  ? +1 : 0);
  int last_jx = (player->is_moving ? jx - dx : jx);
  int last_jy = (player->is_moving ? jy - dy : jy);
  int next_jx = jx + dx;
  int next_jy = jy + dy;
  boolean player_is_moving = (player->MovPos ? TRUE : FALSE);
  boolean player_is_opaque = FALSE;
  int sx = SCREENX(jx), sy = SCREENY(jy);
  int sxx = 0, syy = 0;
  int element = Feld[jx][jy], last_element = Feld[last_jx][last_jy];
  int graphic;
  int action = ACTION_DEFAULT;
  int last_player_graphic = getPlayerGraphic(player, move_dir);
  int last_player_frame = player->Frame;
  int frame = 0;

  /* GfxElement[][] is set to the element the player is digging or collecting;
     remove also for off-screen player if the player is not moving anymore */
  if (IN_LEV_FIELD(jx, jy) && !player_is_moving)
    GfxElement[jx][jy] = EL_UNDEFINED;

  if (!player->active || !IN_SCR_FIELD(SCREENX(last_jx), SCREENY(last_jy)))
    return;

#if DEBUG
  if (!IN_LEV_FIELD(jx, jy))
  {
    printf("DrawPlayerField(): x = %d, y = %d\n",jx,jy);
    printf("DrawPlayerField(): sx = %d, sy = %d\n",sx,sy);
    printf("DrawPlayerField(): This should never happen!\n");
    return;
  }
#endif

  if (element == EL_EXPLOSION)
    return;

  action = (player->is_pushing    ? ACTION_PUSHING         :
	    player->is_digging    ? ACTION_DIGGING         :
	    player->is_collecting ? ACTION_COLLECTING      :
	    player->is_moving     ? ACTION_MOVING          :
	    player->is_snapping   ? ACTION_SNAPPING        :
	    player->is_dropping   ? ACTION_DROPPING        :
	    player->is_waiting    ? player->action_waiting : ACTION_DEFAULT);

  if (player->is_waiting)
    move_dir = player->dir_waiting;

  InitPlayerGfxAnimation(player, action, move_dir);

  /* ----------------------------------------------------------------------- */
  /* draw things in the field the player is leaving, if needed               */
  /* ----------------------------------------------------------------------- */

  if (player->is_moving)
  {
    if (Back[last_jx][last_jy] && IS_DRAWABLE(last_element))
    {
      DrawLevelElement(last_jx, last_jy, Back[last_jx][last_jy]);

      if (last_element == EL_DYNAMITE_ACTIVE ||
	  last_element == EL_EM_DYNAMITE_ACTIVE ||
	  last_element == EL_SP_DISK_RED_ACTIVE)
	DrawDynamite(last_jx, last_jy);
      else
	DrawLevelFieldThruMask(last_jx, last_jy);
    }
    else if (last_element == EL_DYNAMITE_ACTIVE ||
	     last_element == EL_EM_DYNAMITE_ACTIVE ||
	     last_element == EL_SP_DISK_RED_ACTIVE)
      DrawDynamite(last_jx, last_jy);
#if 0
    /* !!! this is not enough to prevent flickering of players which are
       moving next to each others without a free tile between them -- this
       can only be solved by drawing all players layer by layer (first the
       background, then the foreground etc.) !!! => TODO */
    else if (!IS_PLAYER(last_jx, last_jy))
      DrawLevelField(last_jx, last_jy);
#else
    else
      DrawLevelField(last_jx, last_jy);
#endif

    if (player->is_pushing && IN_SCR_FIELD(SCREENX(next_jx), SCREENY(next_jy)))
      DrawLevelElement(next_jx, next_jy, EL_EMPTY);
  }

  if (!IN_SCR_FIELD(sx, sy))
    return;

  /* ----------------------------------------------------------------------- */
  /* draw things behind the player, if needed                                */
  /* ----------------------------------------------------------------------- */

  if (Back[jx][jy])
    DrawLevelElement(jx, jy, Back[jx][jy]);
  else if (IS_ACTIVE_BOMB(element))
    DrawLevelElement(jx, jy, EL_EMPTY);
  else
  {
    if (player_is_moving && GfxElement[jx][jy] != EL_UNDEFINED)
    {
      int old_element = GfxElement[jx][jy];
      int old_graphic = el_act_dir2img(old_element, action, move_dir);
      int frame = getGraphicAnimationFrame(old_graphic, player->StepFrame);

      if (GFX_CRUMBLED(old_element))
	DrawLevelFieldCrumbledDigging(jx, jy, move_dir, player->StepFrame);
      else
	DrawGraphic(sx, sy, old_graphic, frame);

      if (graphic_info[old_graphic].anim_mode & ANIM_OPAQUE_PLAYER)
	player_is_opaque = TRUE;
    }
    else
    {
      GfxElement[jx][jy] = EL_UNDEFINED;

      /* make sure that pushed elements are drawn with correct frame rate */
      graphic = el_act_dir2img(element, ACTION_PUSHING, move_dir);

      if (player->is_pushing && player->is_moving && !IS_ANIM_MODE_CE(graphic))
	GfxFrame[jx][jy] = player->StepFrame;

      DrawLevelField(jx, jy);
    }
  }

#if !DRAW_PLAYER_OVER_PUSHED_ELEMENT
  /* ----------------------------------------------------------------------- */
  /* draw player himself                                                     */
  /* ----------------------------------------------------------------------- */

  graphic = getPlayerGraphic(player, move_dir);

  /* in the case of changed player action or direction, prevent the current
     animation frame from being restarted for identical animations */
  if (player->Frame == 0 && equalGraphics(graphic, last_player_graphic))
    player->Frame = last_player_frame;

  frame = getGraphicAnimationFrame(graphic, player->Frame);

  if (player->GfxPos)
  {
    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      sxx = player->GfxPos;
    else
      syy = player->GfxPos;
  }

  if (player_is_opaque)
    DrawGraphicShifted(sx, sy, sxx, syy, graphic, frame,NO_CUTTING,NO_MASKING);
  else
    DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);

  if (SHIELD_ON(player))
  {
    int graphic = (player->shield_deadly_time_left ? IMG_SHIELD_DEADLY_ACTIVE :
		   IMG_SHIELD_NORMAL_ACTIVE);
    int frame = getGraphicAnimationFrame(graphic, -1);

    DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);
  }
#endif

#if DRAW_PLAYER_OVER_PUSHED_ELEMENT
  if (player->GfxPos)
  {
    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      sxx = player->GfxPos;
    else
      syy = player->GfxPos;
  }
#endif

  /* ----------------------------------------------------------------------- */
  /* draw things the player is pushing, if needed                            */
  /* ----------------------------------------------------------------------- */

  if (player->is_pushing && player->is_moving)
  {
    int px = SCREENX(jx), py = SCREENY(jy);
    int pxx = (TILEX - ABS(sxx)) * dx;
    int pyy = (TILEY - ABS(syy)) * dy;
    int gfx_frame = GfxFrame[jx][jy];

    int graphic;
    int sync_frame;
    int frame;

    if (!IS_MOVING(jx, jy))		/* push movement already finished */
    {
      element = Feld[next_jx][next_jy];
      gfx_frame = GfxFrame[next_jx][next_jy];
    }

    graphic = el_act_dir2img(element, ACTION_PUSHING, move_dir);

    sync_frame = (IS_ANIM_MODE_CE(graphic) ? gfx_frame : player->StepFrame);
    frame = getGraphicAnimationFrame(graphic, sync_frame);

    /* draw background element under pushed element (like the Sokoban field) */
    if (game.use_masked_pushing && IS_MOVING(jx, jy))
    {
      /* this allows transparent pushing animation over non-black background */

      if (Back[jx][jy])
	DrawLevelElement(jx, jy, Back[jx][jy]);
      else
	DrawLevelElement(jx, jy, EL_EMPTY);

      if (Back[next_jx][next_jy])
	DrawLevelElement(next_jx, next_jy, Back[next_jx][next_jy]);
      else
	DrawLevelElement(next_jx, next_jy, EL_EMPTY);
    }
    else if (Back[next_jx][next_jy])
      DrawLevelElement(next_jx, next_jy, Back[next_jx][next_jy]);

#if 1
    /* do not draw (EM style) pushing animation when pushing is finished */
    /* (two-tile animations usually do not contain start and end frame) */
    if (graphic_info[graphic].double_movement && !IS_MOVING(jx, jy))
      DrawLevelElement(next_jx, next_jy, Feld[next_jx][next_jy]);
    else
      DrawGraphicShiftedThruMask(px, py, pxx, pyy, graphic, frame, NO_CUTTING);
#else
    /* masked drawing is needed for EMC style (double) movement graphics */
    /* !!! (ONLY WHEN DRAWING PUSHED ELEMENT OVER THE PLAYER) !!! */
    DrawGraphicShiftedThruMask(px, py, pxx, pyy, graphic, frame, NO_CUTTING);
#endif
  }

#if DRAW_PLAYER_OVER_PUSHED_ELEMENT
  /* ----------------------------------------------------------------------- */
  /* draw player himself                                                     */
  /* ----------------------------------------------------------------------- */

  graphic = getPlayerGraphic(player, move_dir);

  /* in the case of changed player action or direction, prevent the current
     animation frame from being restarted for identical animations */
  if (player->Frame == 0 && equalGraphics(graphic, last_player_graphic))
    player->Frame = last_player_frame;

  frame = getGraphicAnimationFrame(graphic, player->Frame);

  if (player->GfxPos)
  {
    if (move_dir == MV_LEFT || move_dir == MV_RIGHT)
      sxx = player->GfxPos;
    else
      syy = player->GfxPos;
  }

  if (player_is_opaque)
    DrawGraphicShifted(sx, sy, sxx, syy, graphic, frame,NO_CUTTING,NO_MASKING);
  else
    DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);

  if (SHIELD_ON(player))
  {
    int graphic = (player->shield_deadly_time_left ? IMG_SHIELD_DEADLY_ACTIVE :
		   IMG_SHIELD_NORMAL_ACTIVE);
    int frame = getGraphicAnimationFrame(graphic, -1);

    DrawGraphicShiftedThruMask(sx, sy, sxx, syy, graphic, frame, NO_CUTTING);
  }
#endif

  /* ----------------------------------------------------------------------- */
  /* draw things in front of player (active dynamite or dynabombs)           */
  /* ----------------------------------------------------------------------- */

  if (IS_ACTIVE_BOMB(element))
  {
    graphic = el2img(element);
    frame = getGraphicAnimationFrame(graphic, GfxFrame[jx][jy]);

    if (game.emulation == EMU_SUPAPLEX)
      DrawGraphic(sx, sy, IMG_SP_DISK_RED, frame);
    else
      DrawGraphicThruMask(sx, sy, graphic, frame);
  }

  if (player_is_moving && last_element == EL_EXPLOSION)
  {
    int element = (GfxElement[last_jx][last_jy] != EL_UNDEFINED ?
		   GfxElement[last_jx][last_jy] :  EL_EMPTY);
    int graphic = el_act2img(element, ACTION_EXPLODING);
    int delay = (game.emulation == EMU_SUPAPLEX ? 3 : 2);
    int phase = ExplodePhase[last_jx][last_jy] - 1;
    int frame = getGraphicAnimationFrame(graphic, phase - delay);

    if (phase >= delay)
      DrawGraphicThruMask(SCREENX(last_jx), SCREENY(last_jy), graphic, frame);
  }

  /* ----------------------------------------------------------------------- */
  /* draw elements the player is just walking/passing through/under          */
  /* ----------------------------------------------------------------------- */

  if (player_is_moving)
  {
    /* handle the field the player is leaving ... */
    if (IS_ACCESSIBLE_INSIDE(last_element))
      DrawLevelField(last_jx, last_jy);
    else if (IS_ACCESSIBLE_UNDER(last_element))
      DrawLevelFieldThruMask(last_jx, last_jy);
  }

  /* do not redraw accessible elements if the player is just pushing them */
  if (!player_is_moving || !player->is_pushing)
  {
    /* ... and the field the player is entering */
    if (IS_ACCESSIBLE_INSIDE(element))
      DrawLevelField(jx, jy);
    else if (IS_ACCESSIBLE_UNDER(element))
      DrawLevelFieldThruMask(jx, jy);
  }

  MarkTileDirty(sx, sy);
}

/* ------------------------------------------------------------------------- */

void WaitForEventToContinue()
{
  boolean still_wait = TRUE;

  if (program.headless)
    return;

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  button_status = MB_RELEASED;

  ClearEventQueue();

  while (still_wait)
  {
    Event event;

    if (NextValidEvent(&event))
    {
      switch (event.type)
      {
	case EVENT_BUTTONPRESS:
	case EVENT_KEYPRESS:
#if defined(TARGET_SDL2)
        case SDL_CONTROLLERBUTTONDOWN:
#endif
        case SDL_JOYBUTTONDOWN:
	  still_wait = FALSE;
	  break;

	case EVENT_KEYRELEASE:
	  ClearPlayerAction();
	  break;

	default:
	  HandleOtherEvents(&event);
	  break;
      }
    }
    else if (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED)
    {
      still_wait = FALSE;
    }

    BackToFront();
  }
}

#define MAX_REQUEST_LINES		13
#define MAX_REQUEST_LINE_FONT1_LEN	7
#define MAX_REQUEST_LINE_FONT2_LEN	10

static int RequestHandleEvents(unsigned int req_state)
{
  boolean level_solved = (game_status == GAME_MODE_PLAYING &&
			  local_player->LevelSolved_GameEnd);
  int width  = request.width;
  int height = request.height;
  int sx, sy;
  int result;

  setRequestPosition(&sx, &sy, FALSE);

  button_status = MB_RELEASED;

  request_gadget_id = -1;
  result = -1;

  while (result < 0)
  {
    if (level_solved)
    {
      /* the MM game engine does not use a special (scrollable) field buffer */
      if (level.game_engine_type != GAME_ENGINE_TYPE_MM)
	SetDrawtoField(DRAW_TO_FIELDBUFFER);

      HandleGameActions();

      SetDrawtoField(DRAW_TO_BACKBUFFER);

      if (global.use_envelope_request)
      {
	/* copy current state of request area to middle of playfield area */
	BlitBitmap(bitmap_db_store_2, drawto, sx, sy, width, height, sx, sy);
      }
    }

    if (PendingEvent())
    {
      Event event;

      while (NextValidEvent(&event))
      {
	switch (event.type)
	{
	  case EVENT_BUTTONPRESS:
	  case EVENT_BUTTONRELEASE:
	  case EVENT_MOTIONNOTIFY:
	  {
	    int mx, my;

	    if (event.type == EVENT_MOTIONNOTIFY)
	    {
	      if (!button_status)
		continue;

	      motion_status = TRUE;
	      mx = ((MotionEvent *) &event)->x;
	      my = ((MotionEvent *) &event)->y;
	    }
	    else
	    {
	      motion_status = FALSE;
	      mx = ((ButtonEvent *) &event)->x;
	      my = ((ButtonEvent *) &event)->y;
	      if (event.type == EVENT_BUTTONPRESS)
		button_status = ((ButtonEvent *) &event)->button;
	      else
		button_status = MB_RELEASED;
	    }

	    /* this sets 'request_gadget_id' */
	    HandleGadgets(mx, my, button_status);

	    switch (request_gadget_id)
	    {
	      case TOOL_CTRL_ID_YES:
		result = TRUE;
		break;
	      case TOOL_CTRL_ID_NO:
		result = FALSE;
		break;
	      case TOOL_CTRL_ID_CONFIRM:
		result = TRUE | FALSE;
		break;

	      case TOOL_CTRL_ID_PLAYER_1:
		result = 1;
		break;
	      case TOOL_CTRL_ID_PLAYER_2:
		result = 2;
		break;
	      case TOOL_CTRL_ID_PLAYER_3:
		result = 3;
		break;
	      case TOOL_CTRL_ID_PLAYER_4:
		result = 4;
		break;

	      default:
		break;
	    }

	    break;
	  }

#if defined(TARGET_SDL2)
	  case SDL_WINDOWEVENT:
	    HandleWindowEvent((WindowEvent *) &event);
	    break;

	  case SDL_APP_WILLENTERBACKGROUND:
	  case SDL_APP_DIDENTERBACKGROUND:
	  case SDL_APP_WILLENTERFOREGROUND:
	  case SDL_APP_DIDENTERFOREGROUND:
	    HandlePauseResumeEvent((PauseResumeEvent *) &event);
	    break;
#endif

	  case EVENT_KEYPRESS:
	  {
	    Key key = GetEventKey((KeyEvent *)&event, TRUE);

	    switch (key)
	    {
	      case KSYM_space:
		if (req_state & REQ_CONFIRM)
		  result = 1;
		break;

	      case KSYM_Return:
#if defined(TARGET_SDL2)
	      case KSYM_Select:
	      case KSYM_Menu:
#if defined(KSYM_Rewind)
	      case KSYM_Rewind:		/* for Amazon Fire TV remote */
#endif
#endif
		result = 1;
		break;

	      case KSYM_Escape:
#if defined(TARGET_SDL2)
	      case KSYM_Back:
#if defined(KSYM_FastForward)
	      case KSYM_FastForward:	/* for Amazon Fire TV remote */
#endif
#endif
		result = 0;
		break;

	      default:
		HandleKeysDebug(key);
		break;
	    }

	    if (req_state & REQ_PLAYER)
	      result = 0;

	    break;
	  }

	  case EVENT_KEYRELEASE:
	    ClearPlayerAction();
	    break;

#if defined(TARGET_SDL2)
	  case SDL_CONTROLLERBUTTONDOWN:
	    switch (event.cbutton.button)
	    {
	      case SDL_CONTROLLER_BUTTON_A:
	      case SDL_CONTROLLER_BUTTON_X:
	      case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		result = 1;
		break;

	      case SDL_CONTROLLER_BUTTON_B:
	      case SDL_CONTROLLER_BUTTON_Y:
	      case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
	      case SDL_CONTROLLER_BUTTON_BACK:
		result = 0;
		break;
	    }

	    if (req_state & REQ_PLAYER)
	      result = 0;

	    break;

	  case SDL_CONTROLLERBUTTONUP:
	    HandleJoystickEvent(&event);
	    ClearPlayerAction();
	    break;
#endif

	  default:
	    HandleOtherEvents(&event);
	    break;
	}
      }
    }
    else if (AnyJoystickButton() == JOY_BUTTON_NEW_PRESSED)
    {
      int joy = AnyJoystick();

      if (joy & JOY_BUTTON_1)
	result = 1;
      else if (joy & JOY_BUTTON_2)
	result = 0;
    }

    if (level_solved)
    {
      if (global.use_envelope_request)
      {
	/* copy back current state of pressed buttons inside request area */
	BlitBitmap(drawto, bitmap_db_store_2, sx, sy, width, height, sx, sy);
      }
    }

    BackToFront();
  }

  return result;
}

static boolean RequestDoor(char *text, unsigned int req_state)
{
  unsigned int old_door_state;
  int max_request_line_len = MAX_REQUEST_LINE_FONT1_LEN;
  int font_nr = FONT_TEXT_2;
  char *text_ptr;
  int result;
  int ty;

  if (maxWordLengthInString(text) > MAX_REQUEST_LINE_FONT1_LEN)
  {
    max_request_line_len = MAX_REQUEST_LINE_FONT2_LEN;
    font_nr = FONT_TEXT_1;
  }

  if (game_status == GAME_MODE_PLAYING)
    BlitScreenToBitmap(backbuffer);

  /* disable deactivated drawing when quick-loading level tape recording */
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOff(TRUE);

  SetMouseCursor(CURSOR_DEFAULT);

#if defined(NETWORK_AVALIABLE)
  /* pause network game while waiting for request to answer */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR_INPUT)
    SendToServer_PausePlaying();
#endif

  old_door_state = GetDoorState();

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  UnmapAllGadgets();

  /* draw released gadget before proceeding */
  // BackToFront();

  if (old_door_state & DOOR_OPEN_1)
  {
    CloseDoor(DOOR_CLOSE_1);

    /* save old door content */
    BlitBitmap(bitmap_db_door_1, bitmap_db_door_1,
	       0 * DXSIZE, 0, DXSIZE, DYSIZE, 1 * DXSIZE, 0);
  }

  SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);
  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  /* clear door drawing field */
  DrawBackground(DX, DY, DXSIZE, DYSIZE);

  /* force DOOR font inside door area */
  SetFontStatus(GAME_MODE_PSEUDO_DOOR);

  /* write text for request */
  for (text_ptr = text, ty = 0; ty < MAX_REQUEST_LINES; ty++)
  {
    char text_line[max_request_line_len + 1];
    int tx, tl, tc = 0;

    if (!*text_ptr)
      break;

    for (tl = 0, tx = 0; tx < max_request_line_len; tl++, tx++)
    {
      tc = *(text_ptr + tx);
      // if (!tc || tc == ' ')
      if (!tc || tc == ' ' || tc == '?' || tc == '!')
	break;
    }

    if ((tc == '?' || tc == '!') && tl == 0)
      tl = 1;

    if (!tl)
    { 
      text_ptr++; 
      ty--; 
      continue; 
    }

    strncpy(text_line, text_ptr, tl);
    text_line[tl] = 0;

    DrawText(DX + (DXSIZE - tl * getFontWidth(font_nr)) / 2,
	     DY + 8 + ty * (getFontHeight(font_nr) + 2),
	     text_line, font_nr);

    text_ptr += tl + (tc == ' ' ? 1 : 0);
    // text_ptr += tl + (tc == ' ' || tc == '?' || tc == '!' ? 1 : 0);
  }

  ResetFontStatus();

  if (req_state & REQ_ASK)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_YES]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_NO]);
  }
  else if (req_state & REQ_CONFIRM)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_CONFIRM]);
  }
  else if (req_state & REQ_PLAYER)
  {
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_1]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_2]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_3]);
    MapGadget(tool_gadget[TOOL_CTRL_ID_PLAYER_4]);
  }

  /* copy request gadgets to door backbuffer */
  BlitBitmap(drawto, bitmap_db_door_1, DX, DY, DXSIZE, DYSIZE, 0, 0);

  OpenDoor(DOOR_OPEN_1);

  if (!(req_state & REQUEST_WAIT_FOR_INPUT))
  {
    if (game_status == GAME_MODE_PLAYING)
    {
      SetPanelBackground();
      SetDrawBackgroundMask(REDRAW_DOOR_1);
    }
    else
    {
      SetDrawBackgroundMask(REDRAW_FIELD);
    }

    return FALSE;
  }

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  // ---------- handle request buttons ----------
  result = RequestHandleEvents(req_state);

  UnmapToolButtons();

  if (!(req_state & REQ_STAY_OPEN))
  {
    CloseDoor(DOOR_CLOSE_1);

    if (((old_door_state & DOOR_OPEN_1) && !(req_state & REQ_STAY_CLOSED)) ||
	(req_state & REQ_REOPEN))
      OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
  }

  RemapAllGadgets();

  if (game_status == GAME_MODE_PLAYING)
  {
    SetPanelBackground();
    SetDrawBackgroundMask(REDRAW_DOOR_1);
  }
  else
  {
    SetDrawBackgroundMask(REDRAW_FIELD);
  }

#if defined(NETWORK_AVALIABLE)
  /* continue network game after request */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR_INPUT)
    SendToServer_ContinuePlaying();
#endif

  /* restore deactivated drawing when quick-loading level tape recording */
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOn();

  return result;
}

static boolean RequestEnvelope(char *text, unsigned int req_state)
{
  int result;

  if (game_status == GAME_MODE_PLAYING)
    BlitScreenToBitmap(backbuffer);

  /* disable deactivated drawing when quick-loading level tape recording */
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOff(TRUE);

  SetMouseCursor(CURSOR_DEFAULT);

#if defined(NETWORK_AVALIABLE)
  /* pause network game while waiting for request to answer */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR_INPUT)
    SendToServer_PausePlaying();
#endif

  /* simulate releasing mouse button over last gadget, if still pressed */
  if (button_status)
    HandleGadgets(-1, -1, 0);

  UnmapAllGadgets();

  // (replace with setting corresponding request background)
  // SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);
  // SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  /* clear door drawing field */
  // DrawBackground(DX, DY, DXSIZE, DYSIZE);

  ShowEnvelopeRequest(text, req_state, ACTION_OPENING);

  if (!(req_state & REQUEST_WAIT_FOR_INPUT))
  {
    if (game_status == GAME_MODE_PLAYING)
    {
      SetPanelBackground();
      SetDrawBackgroundMask(REDRAW_DOOR_1);
    }
    else
    {
      SetDrawBackgroundMask(REDRAW_FIELD);
    }

    return FALSE;
  }

  SetDrawBackgroundMask(REDRAW_FIELD | REDRAW_DOOR_1);

  // ---------- handle request buttons ----------
  result = RequestHandleEvents(req_state);

  UnmapToolButtons();

  ShowEnvelopeRequest(text, req_state, ACTION_CLOSING);

  RemapAllGadgets();

  if (game_status == GAME_MODE_PLAYING)
  {
    SetPanelBackground();
    SetDrawBackgroundMask(REDRAW_DOOR_1);
  }
  else
  {
    SetDrawBackgroundMask(REDRAW_FIELD);
  }

#if defined(NETWORK_AVALIABLE)
  /* continue network game after request */
  if (options.network &&
      game_status == GAME_MODE_PLAYING &&
      req_state & REQUEST_WAIT_FOR_INPUT)
    SendToServer_ContinuePlaying();
#endif

  /* restore deactivated drawing when quick-loading level tape recording */
  if (tape.playing && tape.deactivate_display)
    TapeDeactivateDisplayOn();

  return result;
}

boolean Request(char *text, unsigned int req_state)
{
  boolean overlay_active = GetOverlayActive();
  boolean result;

  SetOverlayActive(FALSE);

  if (global.use_envelope_request)
    result = RequestEnvelope(text, req_state);
  else
    result = RequestDoor(text, req_state);

  SetOverlayActive(overlay_active);

  return result;
}

static int compareDoorPartOrderInfo(const void *object1, const void *object2)
{
  const struct DoorPartOrderInfo *dpo1 = (struct DoorPartOrderInfo *)object1;
  const struct DoorPartOrderInfo *dpo2 = (struct DoorPartOrderInfo *)object2;
  int compare_result;

  if (dpo1->sort_priority != dpo2->sort_priority)
    compare_result = dpo1->sort_priority - dpo2->sort_priority;
  else
    compare_result = dpo1->nr - dpo2->nr;

  return compare_result;
}

void InitGraphicCompatibilityInfo_Doors()
{
  struct
  {
    int door_token;
    int part_1, part_8;
    struct DoorInfo *door;
  }
  doors[] =
  {
    { DOOR_1,	IMG_GFX_DOOR_1_PART_1,	IMG_GFX_DOOR_1_PART_8,	&door_1	},
    { DOOR_2,	IMG_GFX_DOOR_2_PART_1,	IMG_GFX_DOOR_2_PART_8,	&door_2	},

    { -1,	-1,			-1,			NULL	}
  };
  struct Rect door_rect_list[] =
  {
    { DX, DY, DXSIZE, DYSIZE },
    { VX, VY, VXSIZE, VYSIZE }
  };
  int i, j;

  for (i = 0; doors[i].door_token != -1; i++)
  {
    int door_token = doors[i].door_token;
    int door_index = DOOR_INDEX_FROM_TOKEN(door_token);
    int part_1 = doors[i].part_1;
    int part_8 = doors[i].part_8;
    int part_2 = part_1 + 1;
    int part_3 = part_1 + 2;
    struct DoorInfo *door = doors[i].door;
    struct Rect *door_rect = &door_rect_list[door_index];
    boolean door_gfx_redefined = FALSE;

    /* check if any door part graphic definitions have been redefined */

    for (j = 0; door_part_controls[j].door_token != -1; j++)
    {
      struct DoorPartControlInfo *dpc = &door_part_controls[j];
      struct FileInfo *fi = getImageListEntryFromImageID(dpc->graphic);

      if (dpc->door_token == door_token && fi->redefined)
	door_gfx_redefined = TRUE;
    }

    /* check for old-style door graphic/animation modifications */

    if (!door_gfx_redefined)
    {
      if (door->anim_mode & ANIM_STATIC_PANEL)
      {
	door->panel.step_xoffset = 0;
	door->panel.step_yoffset = 0;
      }

      if (door->anim_mode & (ANIM_HORIZONTAL | ANIM_VERTICAL))
      {
	struct GraphicInfo *g_part_1 = &graphic_info[part_1];
	struct GraphicInfo *g_part_2 = &graphic_info[part_2];
	int num_door_steps, num_panel_steps;

	/* remove door part graphics other than the two default wings */

	for (j = 0; door_part_controls[j].door_token != -1; j++)
	{
	  struct DoorPartControlInfo *dpc = &door_part_controls[j];
	  struct GraphicInfo *g = &graphic_info[dpc->graphic];

	  if (dpc->graphic >= part_3 &&
	      dpc->graphic <= part_8)
	    g->bitmap = NULL;
	}

	/* set graphics and screen positions of the default wings */

	g_part_1->width  = door_rect->width;
	g_part_1->height = door_rect->height;
	g_part_2->width  = door_rect->width;
	g_part_2->height = door_rect->height;
	g_part_2->src_x = door_rect->width;
	g_part_2->src_y = g_part_1->src_y;

	door->part_2.x = door->part_1.x;
	door->part_2.y = door->part_1.y;

	if (door->width != -1)
	{
	  g_part_1->width = door->width;
	  g_part_2->width = door->width;

	  // special treatment for graphics and screen position of right wing
	  g_part_2->src_x += door_rect->width - door->width;
	  door->part_2.x  += door_rect->width - door->width;
	}

	if (door->height != -1)
	{
	  g_part_1->height = door->height;
	  g_part_2->height = door->height;

	  // special treatment for graphics and screen position of bottom wing
	  g_part_2->src_y += door_rect->height - door->height;
	  door->part_2.y  += door_rect->height - door->height;
	}

	/* set animation delays for the default wings and panels */

	door->part_1.step_delay = door->step_delay;
	door->part_2.step_delay = door->step_delay;
	door->panel.step_delay  = door->step_delay;

	/* set animation draw order for the default wings */

	door->part_1.sort_priority = 2;	/* draw left wing over ... */
	door->part_2.sort_priority = 1;	/*          ... right wing */

	/* set animation draw offset for the default wings */

	if (door->anim_mode & ANIM_HORIZONTAL)
	{
	  door->part_1.step_xoffset = door->step_offset;
	  door->part_1.step_yoffset = 0;
	  door->part_2.step_xoffset = door->step_offset * -1;
	  door->part_2.step_yoffset = 0;

	  num_door_steps = g_part_1->width / door->step_offset;
	}
	else	// ANIM_VERTICAL
	{
	  door->part_1.step_xoffset = 0;
	  door->part_1.step_yoffset = door->step_offset;
	  door->part_2.step_xoffset = 0;
	  door->part_2.step_yoffset = door->step_offset * -1;

	  num_door_steps = g_part_1->height / door->step_offset;
	}

	/* set animation draw offset for the default panels */

	if (door->step_offset > 1)
	{
	  num_panel_steps = 2 * door_rect->height / door->step_offset;
	  door->panel.start_step = num_panel_steps - num_door_steps;
	  door->panel.start_step_closing = door->panel.start_step;
	}
	else
	{
	  num_panel_steps = door_rect->height / door->step_offset;
	  door->panel.start_step = num_panel_steps - num_door_steps / 2;
	  door->panel.start_step_closing = door->panel.start_step;
	  door->panel.step_delay *= 2;
	}
      }
    }
  }
}

void InitDoors()
{
  int i;

  for (i = 0; door_part_controls[i].door_token != -1; i++)
  {
    struct DoorPartControlInfo *dpc = &door_part_controls[i];
    struct DoorPartOrderInfo *dpo = &door_part_order[i];

    /* initialize "start_step_opening" and "start_step_closing", if needed */
    if (dpc->pos->start_step_opening == 0 &&
	dpc->pos->start_step_closing == 0)
    {
      // dpc->pos->start_step_opening = dpc->pos->start_step;
      dpc->pos->start_step_closing = dpc->pos->start_step;
    }

    /* fill structure for door part draw order (sorted below) */
    dpo->nr = i;
    dpo->sort_priority = dpc->pos->sort_priority;
  }

  /* sort door part controls according to sort_priority and graphic number */
  qsort(door_part_order, MAX_DOOR_PARTS,
        sizeof(struct DoorPartOrderInfo), compareDoorPartOrderInfo);
}

unsigned int OpenDoor(unsigned int door_state)
{
  if (door_state & DOOR_COPY_BACK)
  {
    if (door_state & DOOR_OPEN_1)
      BlitBitmap(bitmap_db_door_1, bitmap_db_door_1,
		 1 * DXSIZE, 0, DXSIZE, DYSIZE, 0 * DXSIZE, 0);

    if (door_state & DOOR_OPEN_2)
      BlitBitmap(bitmap_db_door_2, bitmap_db_door_2,
		 1 * VXSIZE, 0, VXSIZE, VYSIZE, 0 * VXSIZE, 0);

    door_state &= ~DOOR_COPY_BACK;
  }

  return MoveDoor(door_state);
}

unsigned int CloseDoor(unsigned int door_state)
{
  unsigned int old_door_state = GetDoorState();

  if (!(door_state & DOOR_NO_COPY_BACK))
  {
    if (old_door_state & DOOR_OPEN_1)
      BlitBitmap(backbuffer, bitmap_db_door_1,
		 DX, DY, DXSIZE, DYSIZE, 0, 0);

    if (old_door_state & DOOR_OPEN_2)
      BlitBitmap(backbuffer, bitmap_db_door_2,
		 VX, VY, VXSIZE, VYSIZE, 0, 0);

    door_state &= ~DOOR_NO_COPY_BACK;
  }

  return MoveDoor(door_state);
}

unsigned int GetDoorState()
{
  return MoveDoor(DOOR_GET_STATE);
}

unsigned int SetDoorState(unsigned int door_state)
{
  return MoveDoor(door_state | DOOR_SET_STATE);
}

int euclid(int a, int b)
{
  return (b ? euclid(b, a % b) : a);
}

unsigned int MoveDoor(unsigned int door_state)
{
  struct Rect door_rect_list[] =
  {
    { DX, DY, DXSIZE, DYSIZE },
    { VX, VY, VXSIZE, VYSIZE }
  };
  static int door1 = DOOR_CLOSE_1;
  static int door2 = DOOR_CLOSE_2;
  unsigned int door_delay = 0;
  unsigned int door_delay_value;
  int i;

  if (door_state == DOOR_GET_STATE)
    return (door1 | door2);

  if (door_state & DOOR_SET_STATE)
  {
    if (door_state & DOOR_ACTION_1)
      door1 = door_state & DOOR_ACTION_1;
    if (door_state & DOOR_ACTION_2)
      door2 = door_state & DOOR_ACTION_2;

    return (door1 | door2);
  }

  if (!(door_state & DOOR_FORCE_REDRAW))
  {
    if (door1 == DOOR_OPEN_1 && door_state & DOOR_OPEN_1)
      door_state &= ~DOOR_OPEN_1;
    else if (door1 == DOOR_CLOSE_1 && door_state & DOOR_CLOSE_1)
      door_state &= ~DOOR_CLOSE_1;
    if (door2 == DOOR_OPEN_2 && door_state & DOOR_OPEN_2)
      door_state &= ~DOOR_OPEN_2;
    else if (door2 == DOOR_CLOSE_2 && door_state & DOOR_CLOSE_2)
      door_state &= ~DOOR_CLOSE_2;
  }

  if (global.autoplay_leveldir)
  {
    door_state |= DOOR_NO_DELAY;
    door_state &= ~DOOR_CLOSE_ALL;
  }

  if (game_status == GAME_MODE_EDITOR && !(door_state & DOOR_FORCE_ANIM))
    door_state |= DOOR_NO_DELAY;

  if (door_state & DOOR_ACTION)
  {
    boolean door_panel_drawn[NUM_DOORS];
    boolean panel_has_doors[NUM_DOORS];
    boolean door_part_skip[MAX_DOOR_PARTS];
    boolean door_part_done[MAX_DOOR_PARTS];
    boolean door_part_done_all;
    int num_steps[MAX_DOOR_PARTS];
    int max_move_delay = 0;	// delay for complete animations of all doors
    int max_step_delay = 0;	// delay (ms) between two animation frames
    int num_move_steps = 0;	// number of animation steps for all doors
    int max_move_delay_doors_only = 0;	// delay for doors only (no panel)
    int num_move_steps_doors_only = 0;	// steps for doors only (no panel)
    int current_move_delay = 0;
    int start = 0;
    int k;

    for (i = 0; i < NUM_DOORS; i++)
      panel_has_doors[i] = FALSE;

    for (i = 0; i < MAX_DOOR_PARTS; i++)
    {
      struct DoorPartControlInfo *dpc = &door_part_controls[i];
      struct GraphicInfo *g = &graphic_info[dpc->graphic];
      int door_token = dpc->door_token;

      door_part_done[i] = FALSE;
      door_part_skip[i] = (!(door_state & door_token) ||
			   !g->bitmap);
    }

    for (i = 0; i < MAX_DOOR_PARTS; i++)
    {
      int nr = door_part_order[i].nr;
      struct DoorPartControlInfo *dpc = &door_part_controls[nr];
      struct DoorPartPosInfo *pos = dpc->pos;
      struct GraphicInfo *g = &graphic_info[dpc->graphic];
      int door_token = dpc->door_token;
      int door_index = DOOR_INDEX_FROM_TOKEN(door_token);
      boolean is_panel = DOOR_PART_IS_PANEL(nr);
      int step_xoffset = ABS(pos->step_xoffset);
      int step_yoffset = ABS(pos->step_yoffset);
      int step_delay = pos->step_delay;
      int current_door_state = door_state & door_token;
      boolean door_opening = ((current_door_state & DOOR_OPEN)  != 0);
      boolean door_closing = ((current_door_state & DOOR_CLOSE) != 0);
      boolean part_opening = (is_panel ? door_closing : door_opening);
      int start_step = (part_opening ? pos->start_step_opening :
			pos->start_step_closing);
      float move_xsize = (step_xoffset ? g->width  : 0);
      float move_ysize = (step_yoffset ? g->height : 0);
      int move_xsteps = (step_xoffset ? ceil(move_xsize / step_xoffset) : 0);
      int move_ysteps = (step_yoffset ? ceil(move_ysize / step_yoffset) : 0);
      int move_steps = (move_xsteps && move_ysteps ?
			MIN(move_xsteps, move_ysteps) :
			move_xsteps ? move_xsteps : move_ysteps) - start_step;
      int move_delay = move_steps * step_delay;

      if (door_part_skip[nr])
	continue;

      max_move_delay = MAX(max_move_delay, move_delay);
      max_step_delay = (max_step_delay == 0 ? step_delay :
			euclid(max_step_delay, step_delay));
      num_steps[nr] = move_steps;

      if (!is_panel)
      {
	max_move_delay_doors_only = MAX(max_move_delay_doors_only, move_delay);

	panel_has_doors[door_index] = TRUE;
      }
    }

    max_step_delay = MAX(1, max_step_delay);	// prevent division by zero

    num_move_steps = max_move_delay / max_step_delay;
    num_move_steps_doors_only = max_move_delay_doors_only / max_step_delay;

    door_delay_value = max_step_delay;

    if ((door_state & DOOR_NO_DELAY) || setup.quick_doors)
    {
      start = num_move_steps - 1;
    }
    else
    {
      /* opening door sound has priority over simultaneously closing door */
      if (door_state & (DOOR_OPEN_1 | DOOR_OPEN_2))
      {
        PlayMenuSoundStereo(SND_DOOR_OPENING, SOUND_MIDDLE);

	if (door_state & DOOR_OPEN_1)
	  PlayMenuSoundStereo(SND_DOOR_1_OPENING, SOUND_MIDDLE);
	if (door_state & DOOR_OPEN_2)
	  PlayMenuSoundStereo(SND_DOOR_2_OPENING, SOUND_MIDDLE);
      }
      else if (door_state & (DOOR_CLOSE_1 | DOOR_CLOSE_2))
      {
        PlayMenuSoundStereo(SND_DOOR_CLOSING, SOUND_MIDDLE);

	if (door_state & DOOR_CLOSE_1)
	  PlayMenuSoundStereo(SND_DOOR_1_CLOSING, SOUND_MIDDLE);
	if (door_state & DOOR_CLOSE_2)
	  PlayMenuSoundStereo(SND_DOOR_2_CLOSING, SOUND_MIDDLE);
      }
    }

    for (k = start; k < num_move_steps; k++)
    {
      int last_frame = num_move_steps - 1;	// last frame of this "for" loop

      door_part_done_all = TRUE;

      for (i = 0; i < NUM_DOORS; i++)
	door_panel_drawn[i] = FALSE;

      for (i = 0; i < MAX_DOOR_PARTS; i++)
      {
	int nr = door_part_order[i].nr;
	struct DoorPartControlInfo *dpc = &door_part_controls[nr];
	struct DoorPartPosInfo *pos = dpc->pos;
	struct GraphicInfo *g = &graphic_info[dpc->graphic];
	int door_token = dpc->door_token;
	int door_index = DOOR_INDEX_FROM_TOKEN(door_token);
	boolean is_panel = DOOR_PART_IS_PANEL(nr);
	boolean is_panel_and_door_has_closed = FALSE;
	struct Rect *door_rect = &door_rect_list[door_index];
	Bitmap *bitmap_db_door = (door_token == DOOR_1 ? bitmap_db_door_1 :
				  bitmap_db_door_2);
	Bitmap *bitmap = (is_panel ? bitmap_db_door : g->bitmap);
	int current_door_state = door_state & door_token;
	boolean door_opening = ((current_door_state & DOOR_OPEN)  != 0);
	boolean door_closing = !door_opening;
	boolean part_opening = (is_panel ? door_closing : door_opening);
	boolean part_closing = !part_opening;
	int start_step = (part_opening ? pos->start_step_opening :
			  pos->start_step_closing);
	int step_delay = pos->step_delay;
	int step_factor = step_delay / max_step_delay;
	int k1 = (step_factor ? k / step_factor + 1 : k);
	int k2 = (part_opening ? k1 + start_step : num_steps[nr] - k1);
	int kk = MAX(0, k2);
	int g_src_x = 0;
	int g_src_y = 0;
	int src_x, src_y, src_xx, src_yy;
	int dst_x, dst_y, dst_xx, dst_yy;
	int width, height;

	if (door_part_skip[nr])
	  continue;

	if (!(door_state & door_token))
	  continue;

	if (!g->bitmap)
	  continue;

	if (!is_panel)
	{
	  int k2_door = (door_opening ? k : num_move_steps_doors_only - k - 1);
	  int kk_door = MAX(0, k2_door);
	  int sync_frame = kk_door * door_delay_value;
	  int frame = getGraphicAnimationFrame(dpc->graphic, sync_frame);

	  getFixedGraphicSource(dpc->graphic, frame, &bitmap,
				&g_src_x, &g_src_y);
	}

	// draw door panel

	if (!door_panel_drawn[door_index])
	{
	  ClearRectangle(drawto, door_rect->x, door_rect->y,
			 door_rect->width, door_rect->height);

	  door_panel_drawn[door_index] = TRUE;
	}

	// draw opening or closing door parts

	if (pos->step_xoffset < 0)	// door part on right side
	{
	  src_xx = 0;
	  dst_xx = pos->x + ABS(kk * pos->step_xoffset);
	  width = g->width;

	  if (dst_xx + width > door_rect->width)
	    width = door_rect->width - dst_xx;
	}
	else				// door part on left side
	{
	  src_xx = 0;
	  dst_xx = pos->x - kk * pos->step_xoffset;

	  if (dst_xx < 0)
	  {
	    src_xx = ABS(dst_xx);
	    dst_xx = 0;
	  }

	  width = g->width - src_xx;

	  if (width > door_rect->width)
	    width = door_rect->width;

	  // printf("::: k == %d [%d] \n", k, start_step);
	}

	if (pos->step_yoffset < 0)	// door part on bottom side
	{
	  src_yy = 0;
	  dst_yy = pos->y + ABS(kk * pos->step_yoffset);
	  height = g->height;

	  if (dst_yy + height > door_rect->height)
	    height = door_rect->height - dst_yy;
	}
	else				// door part on top side
	{
	  src_yy = 0;
	  dst_yy = pos->y - kk * pos->step_yoffset;

	  if (dst_yy < 0)
	  {
	    src_yy = ABS(dst_yy);
	    dst_yy = 0;
	  }

	  height = g->height - src_yy;
	}

	src_x = g_src_x + src_xx;
	src_y = g_src_y + src_yy;

	dst_x = door_rect->x + dst_xx;
	dst_y = door_rect->y + dst_yy;

	is_panel_and_door_has_closed =
	  (is_panel &&
	   door_closing &&
	   panel_has_doors[door_index] &&
	   k >= num_move_steps_doors_only - 1);

	if (width  >= 0 && width  <= g->width &&
	    height >= 0 && height <= g->height &&
	    !is_panel_and_door_has_closed)
	{
	  if (is_panel || !pos->draw_masked)
	    BlitBitmap(bitmap, drawto, src_x, src_y, width, height,
		       dst_x, dst_y);
	  else
	    BlitBitmapMasked(bitmap, drawto, src_x, src_y, width, height,
			     dst_x, dst_y);
	}

	redraw_mask |= REDRAW_DOOR_FROM_TOKEN(door_token);

	if ((part_opening && (width < 0         || height < 0)) ||
	    (part_closing && (width >= g->width && height >= g->height)))
	  door_part_done[nr] = TRUE;

	// continue door part animations, but not panel after door has closed
	if (!door_part_done[nr] && !is_panel_and_door_has_closed)
	  door_part_done_all = FALSE;
      }

      if (!(door_state & DOOR_NO_DELAY))
      {
	BackToFront();

	SkipUntilDelayReached(&door_delay, door_delay_value, &k, last_frame);

	current_move_delay += max_step_delay;

	/* prevent OS (Windows) from complaining about program not responding */
	CheckQuitEvent();
      }

      if (door_part_done_all)
	break;
    }

    if (!(door_state & DOOR_NO_DELAY))
    {
      /* wait for specified door action post delay */
      if (door_state & DOOR_ACTION_1 && door_state & DOOR_ACTION_2)
	Delay(MAX(door_1.post_delay, door_2.post_delay));
      else if (door_state & DOOR_ACTION_1)
	Delay(door_1.post_delay);
      else if (door_state & DOOR_ACTION_2)
	Delay(door_2.post_delay);
    }
  }

  if (door_state & DOOR_ACTION_1)
    door1 = door_state & DOOR_ACTION_1;
  if (door_state & DOOR_ACTION_2)
    door2 = door_state & DOOR_ACTION_2;

  // draw masked border over door area
  DrawMaskedBorder(REDRAW_DOOR_1);
  DrawMaskedBorder(REDRAW_DOOR_2);

  return (door1 | door2);
}

static boolean useSpecialEditorDoor()
{
  int graphic = IMG_GLOBAL_BORDER_EDITOR;
  boolean redefined = getImageListEntryFromImageID(graphic)->redefined;

  // do not draw special editor door if editor border defined or redefined
  if (graphic_info[graphic].bitmap != NULL || redefined)
    return FALSE;

  // do not draw special editor door if global border defined to be empty
  if (graphic_info[IMG_GLOBAL_BORDER].bitmap == NULL)
    return FALSE;

  // do not draw special editor door if viewport definitions do not match
  if (EX != VX ||
      EY >= VY ||
      EXSIZE != VXSIZE ||
      EY + EYSIZE != VY + VYSIZE)
    return FALSE;

  return TRUE;
}

void DrawSpecialEditorDoor()
{
  struct GraphicInfo *gfx1 = &graphic_info[IMG_DOOR_2_TOP_BORDER_CORRECTION];
  int top_border_width = gfx1->width;
  int top_border_height = gfx1->height;
  int outer_border = viewport.door_2[GAME_MODE_EDITOR].border_size;
  int ex = EX - outer_border;
  int ey = EY - outer_border;
  int vy = VY - outer_border;
  int exsize = EXSIZE + 2 * outer_border;

  if (!useSpecialEditorDoor())
    return;

  /* draw bigger level editor toolbox window */
  BlitBitmap(gfx1->bitmap, drawto, gfx1->src_x, gfx1->src_y,
	     top_border_width, top_border_height, ex, ey - top_border_height);
  BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto, ex, vy,
	     exsize, EYSIZE - VYSIZE + outer_border, ex, ey);

  redraw_mask |= REDRAW_ALL;
}

void UndrawSpecialEditorDoor()
{
  struct GraphicInfo *gfx1 = &graphic_info[IMG_DOOR_2_TOP_BORDER_CORRECTION];
  int top_border_width = gfx1->width;
  int top_border_height = gfx1->height;
  int outer_border = viewport.door_2[GAME_MODE_EDITOR].border_size;
  int ex = EX - outer_border;
  int ey = EY - outer_border;
  int ey_top = ey - top_border_height;
  int exsize = EXSIZE + 2 * outer_border;
  int eysize = EYSIZE + 2 * outer_border;

  if (!useSpecialEditorDoor())
    return;

  /* draw normal tape recorder window */
  if (graphic_info[IMG_GLOBAL_BORDER].bitmap)
  {
    BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	       ex, ey_top, top_border_width, top_border_height,
	       ex, ey_top);
    BlitBitmap(graphic_info[IMG_GLOBAL_BORDER].bitmap, drawto,
	       ex, ey, exsize, eysize, ex, ey);
  }
  else
  {
    // if screen background is set to "[NONE]", clear editor toolbox window
    ClearRectangle(drawto, ex, ey_top, top_border_width, top_border_height);
    ClearRectangle(drawto, ex, ey, exsize, eysize);
  }

  redraw_mask |= REDRAW_ALL;
}


/* ---------- new tool button stuff ---------------------------------------- */

static struct
{
  int graphic;
  struct TextPosInfo *pos;
  int gadget_id;
  char *infotext;
} toolbutton_info[NUM_TOOL_BUTTONS] =
{
  {
    IMG_GFX_REQUEST_BUTTON_YES,		&request.button.yes,
    TOOL_CTRL_ID_YES,			"yes"
  },
  {
    IMG_GFX_REQUEST_BUTTON_NO,		&request.button.no,
    TOOL_CTRL_ID_NO,			"no"
  },
  {
    IMG_GFX_REQUEST_BUTTON_CONFIRM,	&request.button.confirm,
    TOOL_CTRL_ID_CONFIRM,		"confirm"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_1,	&request.button.player_1,
    TOOL_CTRL_ID_PLAYER_1,		"player 1"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_2,	&request.button.player_2,
    TOOL_CTRL_ID_PLAYER_2,		"player 2"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_3,	&request.button.player_3,
    TOOL_CTRL_ID_PLAYER_3,		"player 3"
  },
  {
    IMG_GFX_REQUEST_BUTTON_PLAYER_4,	&request.button.player_4,
    TOOL_CTRL_ID_PLAYER_4,		"player 4"
  }
};

void CreateToolButtons()
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
  {
    struct GraphicInfo *gfx = &graphic_info[toolbutton_info[i].graphic];
    struct TextPosInfo *pos = toolbutton_info[i].pos;
    struct GadgetInfo *gi;
    Bitmap *deco_bitmap = None;
    int deco_x = 0, deco_y = 0, deco_xpos = 0, deco_ypos = 0;
    unsigned int event_mask = GD_EVENT_RELEASED;
    int dx = DX;
    int dy = DY;
    int gd_x = gfx->src_x;
    int gd_y = gfx->src_y;
    int gd_xp = gfx->src_x + gfx->pressed_xoffset;
    int gd_yp = gfx->src_y + gfx->pressed_yoffset;
    int id = i;

    if (global.use_envelope_request)
      setRequestPosition(&dx, &dy, TRUE);

    if (id >= TOOL_CTRL_ID_PLAYER_1 && id <= TOOL_CTRL_ID_PLAYER_4)
    {
      int player_nr = id - TOOL_CTRL_ID_PLAYER_1;

      getSizedGraphicSource(PLAYER_NR_GFX(IMG_PLAYER_1, player_nr), 0,
			    pos->size, &deco_bitmap, &deco_x, &deco_y);
      deco_xpos = (gfx->width  - pos->size) / 2;
      deco_ypos = (gfx->height - pos->size) / 2;
    }

    gi = CreateGadget(GDI_CUSTOM_ID, id,
		      GDI_INFO_TEXT, toolbutton_info[i].infotext,
		      GDI_X, dx + GDI_ACTIVE_POS(pos->x),
		      GDI_Y, dy + GDI_ACTIVE_POS(pos->y),
		      GDI_WIDTH, gfx->width,
		      GDI_HEIGHT, gfx->height,
		      GDI_TYPE, GD_TYPE_NORMAL_BUTTON,
		      GDI_STATE, GD_BUTTON_UNPRESSED,
		      GDI_DESIGN_UNPRESSED, gfx->bitmap, gd_x, gd_y,
		      GDI_DESIGN_PRESSED, gfx->bitmap, gd_xp, gd_yp,
		      GDI_DECORATION_DESIGN, deco_bitmap, deco_x, deco_y,
		      GDI_DECORATION_POSITION, deco_xpos, deco_ypos,
		      GDI_DECORATION_SIZE, pos->size, pos->size,
		      GDI_DECORATION_SHIFTING, 1, 1,
		      GDI_DIRECT_DRAW, FALSE,
		      GDI_EVENT_MASK, event_mask,
		      GDI_CALLBACK_ACTION, HandleToolButtons,
		      GDI_END);

    if (gi == NULL)
      Error(ERR_EXIT, "cannot create gadget");

    tool_gadget[id] = gi;
  }
}

void FreeToolButtons()
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
    FreeGadget(tool_gadget[i]);
}

static void UnmapToolButtons()
{
  int i;

  for (i = 0; i < NUM_TOOL_BUTTONS; i++)
    UnmapGadget(tool_gadget[i]);
}

static void HandleToolButtons(struct GadgetInfo *gi)
{
  request_gadget_id = gi->custom_id;
}

static struct Mapping_EM_to_RND_object
{
  int element_em;
  boolean is_rnd_to_em_mapping;		/* unique mapping EM <-> RND */
  boolean is_backside;			/* backside of moving element */

  int element_rnd;
  int action;
  int direction;
}
em_object_mapping_list[] =
{
  {
    Xblank,				TRUE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Yacid_splash_eB,			FALSE,	FALSE,
    EL_ACID_SPLASH_RIGHT,		-1, -1
  },
  {
    Yacid_splash_wB,			FALSE,	FALSE,
    EL_ACID_SPLASH_LEFT,		-1, -1
  },

#ifdef EM_ENGINE_BAD_ROLL
  {
    Xstone_force_e,			FALSE,	FALSE,
    EL_ROCK,				-1, MV_BIT_RIGHT
  },
  {
    Xstone_force_w,			FALSE,	FALSE,
    EL_ROCK,				-1, MV_BIT_LEFT
  },
  {
    Xnut_force_e,			FALSE,	FALSE,
    EL_NUT,				-1, MV_BIT_RIGHT
  },
  {
    Xnut_force_w,			FALSE,	FALSE,
    EL_NUT,				-1, MV_BIT_LEFT
  },
  {
    Xspring_force_e,			FALSE,	FALSE,
    EL_SPRING,				-1, MV_BIT_RIGHT
  },
  {
    Xspring_force_w,			FALSE,	FALSE,
    EL_SPRING,				-1, MV_BIT_LEFT
  },
  {
    Xemerald_force_e,			FALSE,	FALSE,
    EL_EMERALD,				-1, MV_BIT_RIGHT
  },
  {
    Xemerald_force_w,			FALSE,	FALSE,
    EL_EMERALD,				-1, MV_BIT_LEFT
  },
  {
    Xdiamond_force_e,			FALSE,	FALSE,
    EL_DIAMOND,				-1, MV_BIT_RIGHT
  },
  {
    Xdiamond_force_w,			FALSE,	FALSE,
    EL_DIAMOND,				-1, MV_BIT_LEFT
  },
  {
    Xbomb_force_e,			FALSE,	FALSE,
    EL_BOMB,				-1, MV_BIT_RIGHT
  },
  {
    Xbomb_force_w,			FALSE,	FALSE,
    EL_BOMB,				-1, MV_BIT_LEFT
  },
#endif	/* EM_ENGINE_BAD_ROLL */

  {
    Xstone,				TRUE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Xstone_pause,			FALSE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Xstone_fall,			FALSE,	FALSE,
    EL_ROCK,				-1, -1
  },
  {
    Ystone_s,				FALSE,	FALSE,
    EL_ROCK,				ACTION_FALLING, -1
  },
  {
    Ystone_sB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_FALLING, -1
  },
  {
    Ystone_e,				FALSE,	FALSE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ystone_eB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ystone_w,				FALSE,	FALSE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ystone_wB,				FALSE,	TRUE,
    EL_ROCK,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Xnut,				TRUE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Xnut_pause,				FALSE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Xnut_fall,				FALSE,	FALSE,
    EL_NUT,				-1, -1
  },
  {
    Ynut_s,				FALSE,	FALSE,
    EL_NUT,				ACTION_FALLING, -1
  },
  {
    Ynut_sB,				FALSE,	TRUE,
    EL_NUT,				ACTION_FALLING, -1
  },
  {
    Ynut_e,				FALSE,	FALSE,
    EL_NUT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ynut_eB,				FALSE,	TRUE,
    EL_NUT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ynut_w,				FALSE,	FALSE,
    EL_NUT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ynut_wB,				FALSE,	TRUE,
    EL_NUT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Xbug_n,				TRUE,	FALSE,
    EL_BUG_UP,				-1, -1
  },
  {
    Xbug_e,				TRUE,	FALSE,
    EL_BUG_RIGHT,			-1, -1
  },
  {
    Xbug_s,				TRUE,	FALSE,
    EL_BUG_DOWN,			-1, -1
  },
  {
    Xbug_w,				TRUE,	FALSE,
    EL_BUG_LEFT,			-1, -1
  },
  {
    Xbug_gon,				FALSE,	FALSE,
    EL_BUG_UP,				-1, -1
  },
  {
    Xbug_goe,				FALSE,	FALSE,
    EL_BUG_RIGHT,			-1, -1
  },
  {
    Xbug_gos,				FALSE,	FALSE,
    EL_BUG_DOWN,			-1, -1
  },
  {
    Xbug_gow,				FALSE,	FALSE,
    EL_BUG_LEFT,			-1, -1
  },
  {
    Ybug_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Ybug_nB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Ybug_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybug_eB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybug_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ybug_sB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ybug_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybug_wB,				FALSE,	TRUE,
    EL_BUG,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybug_w_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_LEFT, MV_BIT_UP
  },
  {
    Ybug_n_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_UP, MV_BIT_RIGHT
  },
  {
    Ybug_e_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_RIGHT, MV_BIT_DOWN
  },
  {
    Ybug_s_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_DOWN, MV_BIT_LEFT
  },
  {
    Ybug_e_n,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_RIGHT, MV_BIT_UP
  },
  {
    Ybug_s_e,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_DOWN, MV_BIT_RIGHT
  },
  {
    Ybug_w_s,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_LEFT, MV_BIT_DOWN
  },
  {
    Ybug_n_w,				FALSE,	FALSE,
    EL_BUG,				ACTION_TURNING_FROM_UP, MV_BIT_LEFT
  },
  {
    Ybug_stone,				FALSE,	FALSE,
    EL_BUG,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Ybug_spring,			FALSE,	FALSE,
    EL_BUG,				ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xtank_n,				TRUE,	FALSE,
    EL_SPACESHIP_UP,			-1, -1
  },
  {
    Xtank_e,				TRUE,	FALSE,
    EL_SPACESHIP_RIGHT,			-1, -1
  },
  {
    Xtank_s,				TRUE,	FALSE,
    EL_SPACESHIP_DOWN,			-1, -1
  },
  {
    Xtank_w,				TRUE,	FALSE,
    EL_SPACESHIP_LEFT,			-1, -1
  },
  {
    Xtank_gon,				FALSE,	FALSE,
    EL_SPACESHIP_UP,			-1, -1
  },
  {
    Xtank_goe,				FALSE,	FALSE,
    EL_SPACESHIP_RIGHT,			-1, -1
  },
  {
    Xtank_gos,				FALSE,	FALSE,
    EL_SPACESHIP_DOWN,			-1, -1
  },
  {
    Xtank_gow,				FALSE,	FALSE,
    EL_SPACESHIP_LEFT,			-1, -1
  },
  {
    Ytank_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Ytank_nB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Ytank_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ytank_eB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ytank_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ytank_sB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Ytank_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ytank_wB,				FALSE,	TRUE,
    EL_SPACESHIP,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ytank_w_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_LEFT, MV_BIT_UP
  },
  {
    Ytank_n_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_UP, MV_BIT_RIGHT
  },
  {
    Ytank_e_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_RIGHT, MV_BIT_DOWN
  },
  {
    Ytank_s_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_DOWN, MV_BIT_LEFT
  },
  {
    Ytank_e_n,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_RIGHT, MV_BIT_UP
  },
  {
    Ytank_s_e,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_DOWN, MV_BIT_RIGHT
  },
  {
    Ytank_w_s,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_LEFT, MV_BIT_DOWN
  },
  {
    Ytank_n_w,				FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_TURNING_FROM_UP, MV_BIT_LEFT
  },
  {
    Ytank_stone,			FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Ytank_spring,			FALSE,	FALSE,
    EL_SPACESHIP,			ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xandroid,				TRUE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, -1
  },
  {
    Xandroid_1_n,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_UP
  },
  {
    Xandroid_2_n,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_UP
  },
  {
    Xandroid_1_e,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_RIGHT
  },
  {
    Xandroid_2_e,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_RIGHT
  },
  {
    Xandroid_1_w,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_LEFT
  },
  {
    Xandroid_2_w,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_LEFT
  },
  {
    Xandroid_1_s,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_DOWN
  },
  {
    Xandroid_2_s,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_ACTIVE, MV_BIT_DOWN
  },
  {
    Yandroid_n,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Yandroid_nB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_UP
  },
  {
    Yandroid_ne,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_UPRIGHT
  },
  {
    Yandroid_neB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_UPRIGHT
  },
  {
    Yandroid_e,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yandroid_eB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yandroid_se,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_DOWNRIGHT
  },
  {
    Yandroid_seB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_DOWNRIGHT
  },
  {
    Yandroid_s,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yandroid_sB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yandroid_sw,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_DOWNLEFT
  },
  {
    Yandroid_swB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_DOWNLEFT
  },
  {
    Yandroid_w,				FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yandroid_wB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yandroid_nw,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_GROWING, MV_BIT_UPLEFT
  },
  {
    Yandroid_nwB,			FALSE,	TRUE,
    EL_EMC_ANDROID,			ACTION_SHRINKING, MV_BIT_UPLEFT
  },
  {
    Xspring,				TRUE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_pause,			FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_e,				FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_w,				FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Xspring_fall,			FALSE,	FALSE,
    EL_SPRING,				-1, -1
  },
  {
    Yspring_s,				FALSE,	FALSE,
    EL_SPRING,				ACTION_FALLING, -1
  },
  {
    Yspring_sB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_FALLING, -1
  },
  {
    Yspring_e,				FALSE,	FALSE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yspring_eB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yspring_w,				FALSE,	FALSE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yspring_wB,				FALSE,	TRUE,
    EL_SPRING,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yspring_kill_e,			FALSE,	FALSE,
    EL_SPRING,				ACTION_EATING, MV_BIT_RIGHT
  },
  {
    Yspring_kill_eB,			FALSE,	TRUE,
    EL_SPRING,				ACTION_EATING, MV_BIT_RIGHT
  },
  {
    Yspring_kill_w,			FALSE,	FALSE,
    EL_SPRING,				ACTION_EATING, MV_BIT_LEFT
  },
  {
    Yspring_kill_wB,			FALSE,	TRUE,
    EL_SPRING,				ACTION_EATING, MV_BIT_LEFT
  },
  {
    Xeater_n,				TRUE,	FALSE,
    EL_YAMYAM_UP,			-1, -1
  },
  {
    Xeater_e,				TRUE,	FALSE,
    EL_YAMYAM_RIGHT,			-1, -1
  },
  {
    Xeater_w,				TRUE,	FALSE,
    EL_YAMYAM_LEFT,			-1, -1
  },
  {
    Xeater_s,				TRUE,	FALSE,
    EL_YAMYAM_DOWN,			-1, -1
  },
  {
    Yeater_n,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yeater_nB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yeater_e,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yeater_eB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yeater_s,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yeater_sB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yeater_w,				FALSE,	FALSE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yeater_wB,				FALSE,	TRUE,
    EL_YAMYAM,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yeater_stone,			FALSE,	FALSE,
    EL_YAMYAM,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Yeater_spring,			FALSE,	FALSE,
    EL_YAMYAM,				ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xalien,				TRUE,	FALSE,
    EL_ROBOT,				-1, -1
  },
  {
    Xalien_pause,			FALSE,	FALSE,
    EL_ROBOT,				-1, -1
  },
  {
    Yalien_n,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yalien_nB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yalien_e,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yalien_eB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yalien_s,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yalien_sB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yalien_w,				FALSE,	FALSE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yalien_wB,				FALSE,	TRUE,
    EL_ROBOT,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yalien_stone,			FALSE,	FALSE,
    EL_ROBOT,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Yalien_spring,			FALSE,	FALSE,
    EL_ROBOT,				ACTION_SMASHED_BY_SPRING, -1
  },
  {
    Xemerald,				TRUE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_pause,			FALSE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_fall,			FALSE,	FALSE,
    EL_EMERALD,				-1, -1
  },
  {
    Xemerald_shine,			FALSE,	FALSE,
    EL_EMERALD,				ACTION_TWINKLING, -1
  },
  {
    Yemerald_s,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_FALLING, -1
  },
  {
    Yemerald_sB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_FALLING, -1
  },
  {
    Yemerald_e,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yemerald_eB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yemerald_w,				FALSE,	FALSE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yemerald_wB,			FALSE,	TRUE,
    EL_EMERALD,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yemerald_eat,			FALSE,	FALSE,
    EL_EMERALD,				ACTION_COLLECTING, -1
  },
  {
    Yemerald_stone,			FALSE,	FALSE,
    EL_NUT,				ACTION_BREAKING, -1
  },
  {
    Xdiamond,				TRUE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_pause,			FALSE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_fall,			FALSE,	FALSE,
    EL_DIAMOND,				-1, -1
  },
  {
    Xdiamond_shine,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_TWINKLING, -1
  },
  {
    Ydiamond_s,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_FALLING, -1
  },
  {
    Ydiamond_sB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_FALLING, -1
  },
  {
    Ydiamond_e,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ydiamond_eB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ydiamond_w,				FALSE,	FALSE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ydiamond_wB,			FALSE,	TRUE,
    EL_DIAMOND,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ydiamond_eat,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_COLLECTING, -1
  },
  {
    Ydiamond_stone,			FALSE,	FALSE,
    EL_DIAMOND,				ACTION_SMASHED_BY_ROCK, -1
  },
  {
    Xdrip_fall,				TRUE,	FALSE,
    EL_AMOEBA_DROP,			-1, -1
  },
  {
    Xdrip_stretch,			FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Xdrip_stretchB,			FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Xdrip_eat,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_GROWING, -1
  },
  {
    Ydrip_s1,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_s1B,				FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_s2,				FALSE,	FALSE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Ydrip_s2B,				FALSE,	TRUE,
    EL_AMOEBA_DROP,			ACTION_FALLING, -1
  },
  {
    Xbomb,				TRUE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Xbomb_pause,			FALSE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Xbomb_fall,				FALSE,	FALSE,
    EL_BOMB,				-1, -1
  },
  {
    Ybomb_s,				FALSE,	FALSE,
    EL_BOMB,				ACTION_FALLING, -1
  },
  {
    Ybomb_sB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_FALLING, -1
  },
  {
    Ybomb_e,				FALSE,	FALSE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybomb_eB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Ybomb_w,				FALSE,	FALSE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybomb_wB,				FALSE,	TRUE,
    EL_BOMB,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Ybomb_eat,				FALSE,	FALSE,
    EL_BOMB,				ACTION_ACTIVATING, -1
  },
  {
    Xballoon,				TRUE,	FALSE,
    EL_BALLOON,				-1, -1
  },
  {
    Yballoon_n,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yballoon_nB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_UP
  },
  {
    Yballoon_e,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yballoon_eB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_RIGHT
  },
  {
    Yballoon_s,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yballoon_sB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_DOWN
  },
  {
    Yballoon_w,				FALSE,	FALSE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Yballoon_wB,			FALSE,	TRUE,
    EL_BALLOON,				ACTION_MOVING, MV_BIT_LEFT
  },
  {
    Xgrass,				TRUE,	FALSE,
    EL_EMC_GRASS,			-1, -1
  },
  {
    Ygrass_nB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_UP
  },
  {
    Ygrass_eB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    Ygrass_sB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    Ygrass_wB,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    Xdirt,				TRUE,	FALSE,
    EL_SAND,				-1, -1
  },
  {
    Ydirt_nB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_UP
  },
  {
    Ydirt_eB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_RIGHT
  },
  {
    Ydirt_sB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_DOWN
  },
  {
    Ydirt_wB,				FALSE,	FALSE,
    EL_SAND,				ACTION_DIGGING, MV_BIT_LEFT
  },
  {
    Xacid_ne,				TRUE,	FALSE,
    EL_ACID_POOL_TOPRIGHT,		-1, -1
  },
  {
    Xacid_se,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOMRIGHT,		-1, -1
  },
  {
    Xacid_s,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOM,		-1, -1
  },
  {
    Xacid_sw,				TRUE,	FALSE,
    EL_ACID_POOL_BOTTOMLEFT,		-1, -1
  },
  {
    Xacid_nw,				TRUE,	FALSE,
    EL_ACID_POOL_TOPLEFT,		-1, -1
  },
  {
    Xacid_1,				TRUE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_2,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_3,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_4,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_5,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_6,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_7,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xacid_8,				FALSE,	FALSE,
    EL_ACID,				-1, -1
  },
  {
    Xball_1,				TRUE,	FALSE,
    EL_EMC_MAGIC_BALL,			-1, -1
  },
  {
    Xball_1B,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Xball_2,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Xball_2B,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_ACTIVE, -1
  },
  {
    Yball_eat,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL,			ACTION_DROPPING, -1
  },
  {
    Ykey_1_eat,				FALSE,	FALSE,
    EL_EM_KEY_1,			ACTION_COLLECTING, -1
  },
  {
    Ykey_2_eat,				FALSE,	FALSE,
    EL_EM_KEY_2,			ACTION_COLLECTING, -1
  },
  {
    Ykey_3_eat,				FALSE,	FALSE,
    EL_EM_KEY_3,			ACTION_COLLECTING, -1
  },
  {
    Ykey_4_eat,				FALSE,	FALSE,
    EL_EM_KEY_4,			ACTION_COLLECTING, -1
  },
  {
    Ykey_5_eat,				FALSE,	FALSE,
    EL_EMC_KEY_5,			ACTION_COLLECTING, -1
  },
  {
    Ykey_6_eat,				FALSE,	FALSE,
    EL_EMC_KEY_6,			ACTION_COLLECTING, -1
  },
  {
    Ykey_7_eat,				FALSE,	FALSE,
    EL_EMC_KEY_7,			ACTION_COLLECTING, -1
  },
  {
    Ykey_8_eat,				FALSE,	FALSE,
    EL_EMC_KEY_8,			ACTION_COLLECTING, -1
  },
  {
    Ylenses_eat,			FALSE,	FALSE,
    EL_EMC_LENSES,			ACTION_COLLECTING, -1
  },
  {
    Ymagnify_eat,			FALSE,	FALSE,
    EL_EMC_MAGNIFIER,			ACTION_COLLECTING, -1
  },
  {
    Ygrass_eat,				FALSE,	FALSE,
    EL_EMC_GRASS,			ACTION_SNAPPING, -1
  },
  {
    Ydirt_eat,				FALSE,	FALSE,
    EL_SAND,				ACTION_SNAPPING, -1
  },
  {
    Xgrow_ns,				TRUE,	FALSE,
    EL_EXPANDABLE_WALL_VERTICAL,	-1, -1
  },
  {
    Ygrow_ns_eat,			FALSE,	FALSE,
    EL_EXPANDABLE_WALL_VERTICAL,	ACTION_GROWING, -1
  },
  {
    Xgrow_ew,				TRUE,	FALSE,
    EL_EXPANDABLE_WALL_HORIZONTAL,	-1, -1
  },
  {
    Ygrow_ew_eat,			FALSE,	FALSE,
    EL_EXPANDABLE_WALL_HORIZONTAL,	ACTION_GROWING, -1
  },
  {
    Xwonderwall,			TRUE,	FALSE,
    EL_MAGIC_WALL,			-1, -1
  },
  {
    XwonderwallB,			FALSE,	FALSE,
    EL_MAGIC_WALL,			ACTION_ACTIVE, -1
  },
  {
    Xamoeba_1,				TRUE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_2,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_3,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_4,				FALSE,	FALSE,
    EL_AMOEBA_DRY,			ACTION_OTHER, -1
  },
  {
    Xamoeba_5,				TRUE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_6,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_7,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xamoeba_8,				FALSE,	FALSE,
    EL_AMOEBA_WET,			ACTION_OTHER, -1
  },
  {
    Xdoor_1,				TRUE,	FALSE,
    EL_EM_GATE_1,			-1, -1
  },
  {
    Xdoor_2,				TRUE,	FALSE,
    EL_EM_GATE_2,			-1, -1
  },
  {
    Xdoor_3,				TRUE,	FALSE,
    EL_EM_GATE_3,			-1, -1
  },
  {
    Xdoor_4,				TRUE,	FALSE,
    EL_EM_GATE_4,			-1, -1
  },
  {
    Xdoor_5,				TRUE,	FALSE,
    EL_EMC_GATE_5,			-1, -1
  },
  {
    Xdoor_6,				TRUE,	FALSE,
    EL_EMC_GATE_6,			-1, -1
  },
  {
    Xdoor_7,				TRUE,	FALSE,
    EL_EMC_GATE_7,			-1, -1
  },
  {
    Xdoor_8,				TRUE,	FALSE,
    EL_EMC_GATE_8,			-1, -1
  },
  {
    Xkey_1,				TRUE,	FALSE,
    EL_EM_KEY_1,			-1, -1
  },
  {
    Xkey_2,				TRUE,	FALSE,
    EL_EM_KEY_2,			-1, -1
  },
  {
    Xkey_3,				TRUE,	FALSE,
    EL_EM_KEY_3,			-1, -1
  },
  {
    Xkey_4,				TRUE,	FALSE,
    EL_EM_KEY_4,			-1, -1
  },
  {
    Xkey_5,				TRUE,	FALSE,
    EL_EMC_KEY_5,			-1, -1
  },
  {
    Xkey_6,				TRUE,	FALSE,
    EL_EMC_KEY_6,			-1, -1
  },
  {
    Xkey_7,				TRUE,	FALSE,
    EL_EMC_KEY_7,			-1, -1
  },
  {
    Xkey_8,				TRUE,	FALSE,
    EL_EMC_KEY_8,			-1, -1
  },
  {
    Xwind_n,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_UP,		-1, -1
  },
  {
    Xwind_e,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_RIGHT,		-1, -1
  },
  {
    Xwind_s,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_DOWN,		-1, -1
  },
  {
    Xwind_w,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_LEFT,		-1, -1
  },
  {
    Xwind_nesw,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_ANY,		-1, -1
  },
  {
    Xwind_stop,				TRUE,	FALSE,
    EL_BALLOON_SWITCH_NONE,		-1, -1
  },
  {
    Xexit,				TRUE,	FALSE,
    EL_EM_EXIT_CLOSED,			-1, -1
  },
  {
    Xexit_1,				TRUE,	FALSE,
    EL_EM_EXIT_OPEN,			-1, -1
  },
  {
    Xexit_2,				FALSE,	FALSE,
    EL_EM_EXIT_OPEN,			-1, -1
  },
  {
    Xexit_3,				FALSE,	FALSE,
    EL_EM_EXIT_OPEN,			-1, -1
  },
  {
    Xdynamite,				TRUE,	FALSE,
    EL_EM_DYNAMITE,			-1, -1
  },
  {
    Ydynamite_eat,			FALSE,	FALSE,
    EL_EM_DYNAMITE,			ACTION_COLLECTING, -1
  },
  {
    Xdynamite_1,			TRUE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_2,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_3,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xdynamite_4,			FALSE,	FALSE,
    EL_EM_DYNAMITE_ACTIVE,		-1, -1
  },
  {
    Xbumper,				TRUE,	FALSE,
    EL_EMC_SPRING_BUMPER,		-1, -1
  },
  {
    XbumperB,				FALSE,	FALSE,
    EL_EMC_SPRING_BUMPER,		ACTION_ACTIVE, -1
  },
  {
    Xwheel,				TRUE,	FALSE,
    EL_ROBOT_WHEEL,			-1, -1
  },
  {
    XwheelB,				FALSE,	FALSE,
    EL_ROBOT_WHEEL,			ACTION_ACTIVE, -1
  },
  {
    Xswitch,				TRUE,	FALSE,
    EL_EMC_MAGIC_BALL_SWITCH,		-1, -1
  },
  {
    XswitchB,				FALSE,	FALSE,
    EL_EMC_MAGIC_BALL_SWITCH,		ACTION_ACTIVE, -1
  },
  {
    Xsand,				TRUE,	FALSE,
    EL_QUICKSAND_EMPTY,			-1, -1
  },
  {
    Xsand_stone,			TRUE,	FALSE,
    EL_QUICKSAND_FULL,			-1, -1
  },
  {
    Xsand_stonein_1,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_2,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_3,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonein_4,			FALSE,	TRUE,
    EL_ROCK,				ACTION_FILLING, -1
  },
  {
    Xsand_stonesand_1,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_2,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_3,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_4,			FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_quickout_1,		FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stonesand_quickout_2,		FALSE,	FALSE,
    EL_QUICKSAND_EMPTYING,		-1, -1
  },
  {
    Xsand_stoneout_1,			FALSE,	FALSE,
    EL_ROCK,				ACTION_EMPTYING, -1
  },
  {
    Xsand_stoneout_2,			FALSE,	FALSE,
    EL_ROCK,				ACTION_EMPTYING, -1
  },
  {
    Xsand_sandstone_1,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xsand_sandstone_2,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xsand_sandstone_3,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xsand_sandstone_4,			FALSE,	FALSE,
    EL_QUICKSAND_FILLING,		-1, -1
  },
  {
    Xplant,				TRUE,	FALSE,
    EL_EMC_PLANT,			-1, -1
  },
  {
    Yplant,				FALSE,	FALSE,
    EL_EMC_PLANT,			-1, -1
  },
  {
    Xlenses,				TRUE,	FALSE,
    EL_EMC_LENSES,			-1, -1
  },
  {
    Xmagnify,				TRUE,	FALSE,
    EL_EMC_MAGNIFIER,			-1, -1
  },
  {
    Xdripper,				TRUE,	FALSE,
    EL_EMC_DRIPPER,			-1, -1
  },
  {
    XdripperB,				FALSE,	FALSE,
    EL_EMC_DRIPPER,			ACTION_ACTIVE, -1
  },
  {
    Xfake_blank,			TRUE,	FALSE,
    EL_INVISIBLE_WALL,			-1, -1
  },
  {
    Xfake_blankB,			FALSE,	FALSE,
    EL_INVISIBLE_WALL,			ACTION_ACTIVE, -1
  },
  {
    Xfake_grass,			TRUE,	FALSE,
    EL_EMC_FAKE_GRASS,			-1, -1
  },
  {
    Xfake_grassB,			FALSE,	FALSE,
    EL_EMC_FAKE_GRASS,			ACTION_ACTIVE, -1
  },
  {
    Xfake_door_1,			TRUE,	FALSE,
    EL_EM_GATE_1_GRAY,			-1, -1
  },
  {
    Xfake_door_2,			TRUE,	FALSE,
    EL_EM_GATE_2_GRAY,			-1, -1
  },
  {
    Xfake_door_3,			TRUE,	FALSE,
    EL_EM_GATE_3_GRAY,			-1, -1
  },
  {
    Xfake_door_4,			TRUE,	FALSE,
    EL_EM_GATE_4_GRAY,			-1, -1
  },
  {
    Xfake_door_5,			TRUE,	FALSE,
    EL_EMC_GATE_5_GRAY,			-1, -1
  },
  {
    Xfake_door_6,			TRUE,	FALSE,
    EL_EMC_GATE_6_GRAY,			-1, -1
  },
  {
    Xfake_door_7,			TRUE,	FALSE,
    EL_EMC_GATE_7_GRAY,			-1, -1
  },
  {
    Xfake_door_8,			TRUE,	FALSE,
    EL_EMC_GATE_8_GRAY,			-1, -1
  },
  {
    Xfake_acid_1,			TRUE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_2,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_3,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_4,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_5,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_6,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_7,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xfake_acid_8,			FALSE,	FALSE,
    EL_EMC_FAKE_ACID,			-1, -1
  },
  {
    Xsteel_1,				TRUE,	FALSE,
    EL_STEELWALL,			-1, -1
  },
  {
    Xsteel_2,				TRUE,	FALSE,
    EL_EMC_STEELWALL_2,			-1, -1
  },
  {
    Xsteel_3,				TRUE,	FALSE,
    EL_EMC_STEELWALL_3,			-1, -1
  },
  {
    Xsteel_4,				TRUE,	FALSE,
    EL_EMC_STEELWALL_4,			-1, -1
  },
  {
    Xwall_1,				TRUE,	FALSE,
    EL_WALL,				-1, -1
  },
  {
    Xwall_2,				TRUE,	FALSE,
    EL_EMC_WALL_14,			-1, -1
  },
  {
    Xwall_3,				TRUE,	FALSE,
    EL_EMC_WALL_15,			-1, -1
  },
  {
    Xwall_4,				TRUE,	FALSE,
    EL_EMC_WALL_16,			-1, -1
  },
  {
    Xround_wall_1,			TRUE,	FALSE,
    EL_WALL_SLIPPERY,			-1, -1
  },
  {
    Xround_wall_2,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_2,		-1, -1
  },
  {
    Xround_wall_3,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_3,		-1, -1
  },
  {
    Xround_wall_4,			TRUE,	FALSE,
    EL_EMC_WALL_SLIPPERY_4,		-1, -1
  },
  {
    Xdecor_1,				TRUE,	FALSE,
    EL_EMC_WALL_8,			-1, -1
  },
  {
    Xdecor_2,				TRUE,	FALSE,
    EL_EMC_WALL_6,			-1, -1
  },
  {
    Xdecor_3,				TRUE,	FALSE,
    EL_EMC_WALL_4,			-1, -1
  },
  {
    Xdecor_4,				TRUE,	FALSE,
    EL_EMC_WALL_7,			-1, -1
  },
  {
    Xdecor_5,				TRUE,	FALSE,
    EL_EMC_WALL_5,			-1, -1
  },
  {
    Xdecor_6,				TRUE,	FALSE,
    EL_EMC_WALL_9,			-1, -1
  },
  {
    Xdecor_7,				TRUE,	FALSE,
    EL_EMC_WALL_10,			-1, -1
  },
  {
    Xdecor_8,				TRUE,	FALSE,
    EL_EMC_WALL_1,			-1, -1
  },
  {
    Xdecor_9,				TRUE,	FALSE,
    EL_EMC_WALL_2,			-1, -1
  },
  {
    Xdecor_10,				TRUE,	FALSE,
    EL_EMC_WALL_3,			-1, -1
  },
  {
    Xdecor_11,				TRUE,	FALSE,
    EL_EMC_WALL_11,			-1, -1
  },
  {
    Xdecor_12,				TRUE,	FALSE,
    EL_EMC_WALL_12,			-1, -1
  },
  {
    Xalpha_0,				TRUE,	FALSE,
    EL_CHAR('0'),			-1, -1
  },
  {
    Xalpha_1,				TRUE,	FALSE,
    EL_CHAR('1'),			-1, -1
  },
  {
    Xalpha_2,				TRUE,	FALSE,
    EL_CHAR('2'),			-1, -1
  },
  {
    Xalpha_3,				TRUE,	FALSE,
    EL_CHAR('3'),			-1, -1
  },
  {
    Xalpha_4,				TRUE,	FALSE,
    EL_CHAR('4'),			-1, -1
  },
  {
    Xalpha_5,				TRUE,	FALSE,
    EL_CHAR('5'),			-1, -1
  },
  {
    Xalpha_6,				TRUE,	FALSE,
    EL_CHAR('6'),			-1, -1
  },
  {
    Xalpha_7,				TRUE,	FALSE,
    EL_CHAR('7'),			-1, -1
  },
  {
    Xalpha_8,				TRUE,	FALSE,
    EL_CHAR('8'),			-1, -1
  },
  {
    Xalpha_9,				TRUE,	FALSE,
    EL_CHAR('9'),			-1, -1
  },
  {
    Xalpha_excla,			TRUE,	FALSE,
    EL_CHAR('!'),			-1, -1
  },
  {
    Xalpha_quote,			TRUE,	FALSE,
    EL_CHAR('"'),			-1, -1
  },
  {
    Xalpha_comma,			TRUE,	FALSE,
    EL_CHAR(','),			-1, -1
  },
  {
    Xalpha_minus,			TRUE,	FALSE,
    EL_CHAR('-'),			-1, -1
  },
  {
    Xalpha_perio,			TRUE,	FALSE,
    EL_CHAR('.'),			-1, -1
  },
  {
    Xalpha_colon,			TRUE,	FALSE,
    EL_CHAR(':'),			-1, -1
  },
  {
    Xalpha_quest,			TRUE,	FALSE,
    EL_CHAR('?'),			-1, -1
  },
  {
    Xalpha_a,				TRUE,	FALSE,
    EL_CHAR('A'),			-1, -1
  },
  {
    Xalpha_b,				TRUE,	FALSE,
    EL_CHAR('B'),			-1, -1
  },
  {
    Xalpha_c,				TRUE,	FALSE,
    EL_CHAR('C'),			-1, -1
  },
  {
    Xalpha_d,				TRUE,	FALSE,
    EL_CHAR('D'),			-1, -1
  },
  {
    Xalpha_e,				TRUE,	FALSE,
    EL_CHAR('E'),			-1, -1
  },
  {
    Xalpha_f,				TRUE,	FALSE,
    EL_CHAR('F'),			-1, -1
  },
  {
    Xalpha_g,				TRUE,	FALSE,
    EL_CHAR('G'),			-1, -1
  },
  {
    Xalpha_h,				TRUE,	FALSE,
    EL_CHAR('H'),			-1, -1
  },
  {
    Xalpha_i,				TRUE,	FALSE,
    EL_CHAR('I'),			-1, -1
  },
  {
    Xalpha_j,				TRUE,	FALSE,
    EL_CHAR('J'),			-1, -1
  },
  {
    Xalpha_k,				TRUE,	FALSE,
    EL_CHAR('K'),			-1, -1
  },
  {
    Xalpha_l,				TRUE,	FALSE,
    EL_CHAR('L'),			-1, -1
  },
  {
    Xalpha_m,				TRUE,	FALSE,
    EL_CHAR('M'),			-1, -1
  },
  {
    Xalpha_n,				TRUE,	FALSE,
    EL_CHAR('N'),			-1, -1
  },
  {
    Xalpha_o,				TRUE,	FALSE,
    EL_CHAR('O'),			-1, -1
  },
  {
    Xalpha_p,				TRUE,	FALSE,
    EL_CHAR('P'),			-1, -1
  },
  {
    Xalpha_q,				TRUE,	FALSE,
    EL_CHAR('Q'),			-1, -1
  },
  {
    Xalpha_r,				TRUE,	FALSE,
    EL_CHAR('R'),			-1, -1
  },
  {
    Xalpha_s,				TRUE,	FALSE,
    EL_CHAR('S'),			-1, -1
  },
  {
    Xalpha_t,				TRUE,	FALSE,
    EL_CHAR('T'),			-1, -1
  },
  {
    Xalpha_u,				TRUE,	FALSE,
    EL_CHAR('U'),			-1, -1
  },
  {
    Xalpha_v,				TRUE,	FALSE,
    EL_CHAR('V'),			-1, -1
  },
  {
    Xalpha_w,				TRUE,	FALSE,
    EL_CHAR('W'),			-1, -1
  },
  {
    Xalpha_x,				TRUE,	FALSE,
    EL_CHAR('X'),			-1, -1
  },
  {
    Xalpha_y,				TRUE,	FALSE,
    EL_CHAR('Y'),			-1, -1
  },
  {
    Xalpha_z,				TRUE,	FALSE,
    EL_CHAR('Z'),			-1, -1
  },
  {
    Xalpha_arrow_e,			TRUE,	FALSE,
    EL_CHAR('>'),			-1, -1
  },
  {
    Xalpha_arrow_w,			TRUE,	FALSE,
    EL_CHAR('<'),			-1, -1
  },
  {
    Xalpha_copyr,			TRUE,	FALSE,
    EL_CHAR(CHAR_BYTE_COPYRIGHT),	-1, -1
  },

  {
    Xboom_bug,				FALSE,	FALSE,
    EL_BUG,				ACTION_EXPLODING, -1
  },
  {
    Xboom_bomb,				FALSE,	FALSE,
    EL_BOMB,				ACTION_EXPLODING, -1
  },
  {
    Xboom_android,			FALSE,	FALSE,
    EL_EMC_ANDROID,			ACTION_OTHER, -1
  },
  {
    Xboom_1,				FALSE,	FALSE,
    EL_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    Xboom_2,				FALSE,	FALSE,
    EL_DEFAULT,				ACTION_EXPLODING, -1
  },
  {
    Znormal,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zdynamite,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    Zplayer,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },
  {
    ZBORDER,				FALSE,	FALSE,
    EL_EMPTY,				-1, -1
  },

  {
    -1,					FALSE,	FALSE,
    -1,					-1, -1
  }
};

static struct Mapping_EM_to_RND_player
{
  int action_em;
  int player_nr;

  int element_rnd;
  int action;
  int direction;
}
em_player_mapping_list[] =
{
  {
    SPR_walk + 0,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			0,
    EL_PLAYER_1,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			0,
    EL_PLAYER_1,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			0,
    EL_PLAYER_1,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_walk + 0,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			1,
    EL_PLAYER_2,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			1,
    EL_PLAYER_2,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			1,
    EL_PLAYER_2,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_still,				0,
    EL_PLAYER_1,			ACTION_DEFAULT, -1,
  },
  {
    SPR_still,				1,
    EL_PLAYER_2,			ACTION_DEFAULT, -1,
  },
  {
    SPR_walk + 0,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			2,
    EL_PLAYER_3,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			2,
    EL_PLAYER_3,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			2,
    EL_PLAYER_3,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_walk + 0,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_UP,
  },
  {
    SPR_walk + 1,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_RIGHT,
  },
  {
    SPR_walk + 2,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_DOWN,
  },
  {
    SPR_walk + 3,			3,
    EL_PLAYER_4,			ACTION_MOVING, MV_BIT_LEFT,
  },
  {
    SPR_push + 0,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_UP,
  },
  {
    SPR_push + 1,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_RIGHT,
  },
  {
    SPR_push + 2,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_DOWN,
  },
  {
    SPR_push + 3,			3,
    EL_PLAYER_4,			ACTION_PUSHING, MV_BIT_LEFT,
  },
  {
    SPR_spray + 0,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_UP,
  },
  {
    SPR_spray + 1,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_RIGHT,
  },
  {
    SPR_spray + 2,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_DOWN,
  },
  {
    SPR_spray + 3,			3,
    EL_PLAYER_4,			ACTION_SNAPPING, MV_BIT_LEFT,
  },
  {
    SPR_still,				2,
    EL_PLAYER_3,			ACTION_DEFAULT, -1,
  },
  {
    SPR_still,				3,
    EL_PLAYER_4,			ACTION_DEFAULT, -1,
  },

  {
    -1,					-1,
    -1,					-1, -1
  }
};

int map_element_RND_to_EM(int element_rnd)
{
  static unsigned short mapping_RND_to_EM[NUM_FILE_ELEMENTS];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    /* return "Xalpha_quest" for all undefined elements in mapping array */
    for (i = 0; i < NUM_FILE_ELEMENTS; i++)
      mapping_RND_to_EM[i] = Xalpha_quest;

    for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
      if (em_object_mapping_list[i].is_rnd_to_em_mapping)
	mapping_RND_to_EM[em_object_mapping_list[i].element_rnd] =
	  em_object_mapping_list[i].element_em;

    mapping_initialized = TRUE;
  }

  if (element_rnd >= 0 && element_rnd < NUM_FILE_ELEMENTS)
    return mapping_RND_to_EM[element_rnd];

  Error(ERR_WARN, "invalid RND level element %d", element_rnd);

  return EL_UNKNOWN;
}

int map_element_EM_to_RND(int element_em)
{
  static unsigned short mapping_EM_to_RND[TILE_MAX];
  static boolean mapping_initialized = FALSE;

  if (!mapping_initialized)
  {
    int i;

    /* return "EL_UNKNOWN" for all undefined elements in mapping array */
    for (i = 0; i < TILE_MAX; i++)
      mapping_EM_to_RND[i] = EL_UNKNOWN;

    for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
      mapping_EM_to_RND[em_object_mapping_list[i].element_em] =
	em_object_mapping_list[i].element_rnd;

    mapping_initialized = TRUE;
  }

  if (element_em >= 0 && element_em < TILE_MAX)
    return mapping_EM_to_RND[element_em];

  Error(ERR_WARN, "invalid EM level element %d", element_em);

  return EL_UNKNOWN;
}

void map_android_clone_elements_RND_to_EM(struct LevelInfo *level)
{
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct LEVEL *lev = level_em->lev;
  int i, j;

  for (i = 0; i < TILE_MAX; i++)
    lev->android_array[i] = Xblank;

  for (i = 0; i < level->num_android_clone_elements; i++)
  {
    int element_rnd = level->android_clone_element[i];
    int element_em = map_element_RND_to_EM(element_rnd);

    for (j = 0; em_object_mapping_list[j].element_em != -1; j++)
      if (em_object_mapping_list[j].element_rnd == element_rnd)
	lev->android_array[em_object_mapping_list[j].element_em] = element_em;
  }
}

void map_android_clone_elements_EM_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct LEVEL *lev = level_em->lev;
  int i, j;

  level->num_android_clone_elements = 0;

  for (i = 0; i < TILE_MAX; i++)
  {
    int element_em = lev->android_array[i];
    int element_rnd;
    boolean element_found = FALSE;

    if (element_em == Xblank)
      continue;

    element_rnd = map_element_EM_to_RND(element_em);

    for (j = 0; j < level->num_android_clone_elements; j++)
      if (level->android_clone_element[j] == element_rnd)
	element_found = TRUE;

    if (!element_found)
    {
      level->android_clone_element[level->num_android_clone_elements++] =
	element_rnd;

      if (level->num_android_clone_elements == MAX_ANDROID_ELEMENTS)
	break;
    }
  }

  if (level->num_android_clone_elements == 0)
  {
    level->num_android_clone_elements = 1;
    level->android_clone_element[0] = EL_EMPTY;
  }
}

int map_direction_RND_to_EM(int direction)
{
  return (direction == MV_UP    ? 0 :
	  direction == MV_RIGHT ? 1 :
	  direction == MV_DOWN  ? 2 :
	  direction == MV_LEFT  ? 3 :
	  -1);
}

int map_direction_EM_to_RND(int direction)
{
  return (direction == 0 ? MV_UP    :
	  direction == 1 ? MV_RIGHT :
	  direction == 2 ? MV_DOWN  :
	  direction == 3 ? MV_LEFT  :
	  MV_NONE);
}

int map_element_RND_to_SP(int element_rnd)
{
  int element_sp = 0x20;	/* map unknown elements to yellow "hardware" */

  if (element_rnd >= EL_SP_START &&
      element_rnd <= EL_SP_END)
    element_sp = element_rnd - EL_SP_START;
  else if (element_rnd == EL_EMPTY_SPACE)
    element_sp = 0x00;
  else if (element_rnd == EL_INVISIBLE_WALL)
    element_sp = 0x28;

  return element_sp;
}

int map_element_SP_to_RND(int element_sp)
{
  int element_rnd = EL_UNKNOWN;

  if (element_sp >= 0x00 &&
      element_sp <= 0x27)
    element_rnd = EL_SP_START + element_sp;
  else if (element_sp == 0x28)
    element_rnd = EL_INVISIBLE_WALL;

  return element_rnd;
}

int map_action_SP_to_RND(int action_sp)
{
  switch (action_sp)
  {
    case actActive:		return ACTION_ACTIVE;
    case actImpact:		return ACTION_IMPACT;
    case actExploding:		return ACTION_EXPLODING;
    case actDigging:		return ACTION_DIGGING;
    case actSnapping:		return ACTION_SNAPPING;
    case actCollecting:		return ACTION_COLLECTING;
    case actPassing:		return ACTION_PASSING;
    case actPushing:		return ACTION_PUSHING;
    case actDropping:		return ACTION_DROPPING;

    default:			return ACTION_DEFAULT;
  }
}

int map_element_RND_to_MM(int element_rnd)
{
  return (element_rnd >= EL_MM_START_1 &&
	  element_rnd <= EL_MM_END_1 ?
	  EL_MM_START_1_NATIVE + element_rnd - EL_MM_START_1 :

	  element_rnd >= EL_MM_START_2 &&
	  element_rnd <= EL_MM_END_2 ?
	  EL_MM_START_2_NATIVE + element_rnd - EL_MM_START_2 :

	  element_rnd >= EL_CHAR_START &&
	  element_rnd <= EL_CHAR_END ?
	  EL_MM_CHAR_START_NATIVE + element_rnd - EL_CHAR_START :

	  element_rnd >= EL_MM_RUNTIME_START &&
	  element_rnd <= EL_MM_RUNTIME_END ?
	  EL_MM_RUNTIME_START_NATIVE + element_rnd - EL_MM_RUNTIME_START :

	  element_rnd >= EL_MM_DUMMY_START &&
	  element_rnd <= EL_MM_DUMMY_END ?
	  EL_MM_DUMMY_START_NATIVE + element_rnd - EL_MM_DUMMY_START :

	  EL_MM_EMPTY_NATIVE);
}

int map_element_MM_to_RND(int element_mm)
{
  return (element_mm == EL_MM_EMPTY_NATIVE ||
	  element_mm == EL_DF_EMPTY_NATIVE ?
	  EL_EMPTY :

	  element_mm >= EL_MM_START_1_NATIVE &&
	  element_mm <= EL_MM_END_1_NATIVE ?
	  EL_MM_START_1 + element_mm - EL_MM_START_1_NATIVE :

	  element_mm >= EL_MM_START_2_NATIVE &&
	  element_mm <= EL_MM_END_2_NATIVE ?
	  EL_MM_START_2 + element_mm - EL_MM_START_2_NATIVE :

	  element_mm >= EL_MM_CHAR_START_NATIVE &&
	  element_mm <= EL_MM_CHAR_END_NATIVE ?
	  EL_CHAR_START + element_mm - EL_MM_CHAR_START_NATIVE :

	  element_mm >= EL_MM_RUNTIME_START_NATIVE &&
	  element_mm <= EL_MM_RUNTIME_END_NATIVE ?
	  EL_MM_RUNTIME_START + element_mm - EL_MM_RUNTIME_START_NATIVE :

	  element_mm >= EL_MM_DUMMY_START_NATIVE &&
	  element_mm <= EL_MM_DUMMY_END_NATIVE ?
	  EL_MM_DUMMY_START + element_mm - EL_MM_DUMMY_START_NATIVE :

	  EL_EMPTY);
}

int map_action_MM_to_RND(int action_mm)
{
  /* all MM actions are defined to exactly match their RND counterparts */
  return action_mm;
}

int map_sound_MM_to_RND(int sound_mm)
{
  switch (sound_mm)
  {
    case SND_MM_GAME_LEVELTIME_CHARGING:
      return SND_GAME_LEVELTIME_CHARGING;

    case SND_MM_GAME_HEALTH_CHARGING:
      return SND_GAME_HEALTH_CHARGING;

    default:
      return SND_UNDEFINED;
  }
}

int map_mm_wall_element(int element)
{
  return (element >= EL_MM_STEEL_WALL_START &&
	  element <= EL_MM_STEEL_WALL_END ?
	  EL_MM_STEEL_WALL :

	  element >= EL_MM_WOODEN_WALL_START &&
	  element <= EL_MM_WOODEN_WALL_END ?
	  EL_MM_WOODEN_WALL :

	  element >= EL_MM_ICE_WALL_START &&
	  element <= EL_MM_ICE_WALL_END ?
	  EL_MM_ICE_WALL :

	  element >= EL_MM_AMOEBA_WALL_START &&
	  element <= EL_MM_AMOEBA_WALL_END ?
	  EL_MM_AMOEBA_WALL :

	  element >= EL_DF_STEEL_WALL_START &&
	  element <= EL_DF_STEEL_WALL_END ?
	  EL_DF_STEEL_WALL :

	  element >= EL_DF_WOODEN_WALL_START &&
	  element <= EL_DF_WOODEN_WALL_END ?
	  EL_DF_WOODEN_WALL :

	  element);
}

int map_mm_wall_element_editor(int element)
{
  switch (element)
  {
    case EL_MM_STEEL_WALL:	return EL_MM_STEEL_WALL_START;
    case EL_MM_WOODEN_WALL:	return EL_MM_WOODEN_WALL_START;
    case EL_MM_ICE_WALL:	return EL_MM_ICE_WALL_START;
    case EL_MM_AMOEBA_WALL:	return EL_MM_AMOEBA_WALL_START;
    case EL_DF_STEEL_WALL:	return EL_DF_STEEL_WALL_START;
    case EL_DF_WOODEN_WALL:	return EL_DF_WOODEN_WALL_START;

    default:			return element;
  }
}

int get_next_element(int element)
{
  switch (element)
  {
    case EL_QUICKSAND_FILLING:		return EL_QUICKSAND_FULL;
    case EL_QUICKSAND_EMPTYING:		return EL_QUICKSAND_EMPTY;
    case EL_QUICKSAND_FAST_FILLING:	return EL_QUICKSAND_FAST_FULL;
    case EL_QUICKSAND_FAST_EMPTYING:	return EL_QUICKSAND_FAST_EMPTY;
    case EL_MAGIC_WALL_FILLING:		return EL_MAGIC_WALL_FULL;
    case EL_MAGIC_WALL_EMPTYING:	return EL_MAGIC_WALL_ACTIVE;
    case EL_BD_MAGIC_WALL_FILLING:	return EL_BD_MAGIC_WALL_FULL;
    case EL_BD_MAGIC_WALL_EMPTYING:	return EL_BD_MAGIC_WALL_ACTIVE;
    case EL_DC_MAGIC_WALL_FILLING:	return EL_DC_MAGIC_WALL_FULL;
    case EL_DC_MAGIC_WALL_EMPTYING:	return EL_DC_MAGIC_WALL_ACTIVE;
    case EL_AMOEBA_DROPPING:		return EL_AMOEBA_WET;

    default:				return element;
  }
}

int el2img_mm(int element_mm)
{
  return el2img(map_element_MM_to_RND(element_mm));
}

int el_act_dir2img(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_TO_BIT(direction);	/* default: MV_NONE => MV_DOWN */

  /* direction_graphic[][] == graphic[] for undefined direction graphics */
  return element_info[element].direction_graphic[action][direction];
}

static int el_act_dir2crm(int element, int action, int direction)
{
  element = GFX_ELEMENT(element);
  direction = MV_DIR_TO_BIT(direction);	/* default: MV_NONE => MV_DOWN */

  /* direction_graphic[][] == graphic[] for undefined direction graphics */
  return element_info[element].direction_crumbled[action][direction];
}

int el_act2img(int element, int action)
{
  element = GFX_ELEMENT(element);

  return element_info[element].graphic[action];
}

int el_act2crm(int element, int action)
{
  element = GFX_ELEMENT(element);

  return element_info[element].crumbled[action];
}

int el_dir2img(int element, int direction)
{
  element = GFX_ELEMENT(element);

  return el_act_dir2img(element, ACTION_DEFAULT, direction);
}

int el2baseimg(int element)
{
  return element_info[element].graphic[ACTION_DEFAULT];
}

int el2img(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].graphic[ACTION_DEFAULT];
}

int el2edimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_EDITOR];
}

int el2preimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_PREVIEW];
}

int el2panelimg(int element)
{
  element = GFX_ELEMENT(element);

  return element_info[element].special_graphic[GFX_SPECIAL_ARG_PANEL];
}

int font2baseimg(int font_nr)
{
  return font_info[font_nr].special_graphic[GFX_SPECIAL_ARG_DEFAULT];
}

int getBeltNrFromBeltElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT_3_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT_4_LEFT ? 2 : 3);
}

int getBeltNrFromBeltActiveElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_LEFT_ACTIVE ? 0 :
	  element < EL_CONVEYOR_BELT_3_LEFT_ACTIVE ? 1 :
	  element < EL_CONVEYOR_BELT_4_LEFT_ACTIVE ? 2 : 3);
}

int getBeltNrFromBeltSwitchElement(int element)
{
  return (element < EL_CONVEYOR_BELT_2_SWITCH_LEFT ? 0 :
	  element < EL_CONVEYOR_BELT_3_SWITCH_LEFT ? 1 :
	  element < EL_CONVEYOR_BELT_4_SWITCH_LEFT ? 2 : 3);
}

int getBeltDirNrFromBeltElement(int element)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };

  int belt_nr = getBeltNrFromBeltElement(element);
  int belt_dir_nr = element - belt_base_element[belt_nr];

  return (belt_dir_nr % 3);
}

int getBeltDirNrFromBeltSwitchElement(int element)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT
  };

  int belt_nr = getBeltNrFromBeltSwitchElement(element);
  int belt_dir_nr = element - belt_base_element[belt_nr];

  return (belt_dir_nr % 3);
}

int getBeltDirFromBeltElement(int element)
{
  static int belt_move_dir[3] =
  {
    MV_LEFT,
    MV_NONE,
    MV_RIGHT
  };

  int belt_dir_nr = getBeltDirNrFromBeltElement(element);

  return belt_move_dir[belt_dir_nr];
}

int getBeltDirFromBeltSwitchElement(int element)
{
  static int belt_move_dir[3] =
  {
    MV_LEFT,
    MV_NONE,
    MV_RIGHT
  };

  int belt_dir_nr = getBeltDirNrFromBeltSwitchElement(element);

  return belt_move_dir[belt_dir_nr];
}

int getBeltElementFromBeltNrAndBeltDirNr(int belt_nr, int belt_dir_nr)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_4_LEFT
  };

  return belt_base_element[belt_nr] + belt_dir_nr;
}

int getBeltElementFromBeltNrAndBeltDir(int belt_nr, int belt_dir)
{
  int belt_dir_nr = (belt_dir == MV_LEFT ? 0 : belt_dir == MV_RIGHT ? 2 : 1);

  return getBeltElementFromBeltNrAndBeltDirNr(belt_nr, belt_dir_nr);
}

int getBeltSwitchElementFromBeltNrAndBeltDirNr(int belt_nr, int belt_dir_nr)
{
  static int belt_base_element[4] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT
  };

  return belt_base_element[belt_nr] + belt_dir_nr;
}

int getBeltSwitchElementFromBeltNrAndBeltDir(int belt_nr, int belt_dir)
{
  int belt_dir_nr = (belt_dir == MV_LEFT ? 0 : belt_dir == MV_RIGHT ? 2 : 1);

  return getBeltSwitchElementFromBeltNrAndBeltDirNr(belt_nr, belt_dir_nr);
}

boolean getTeamMode_EM()
{
  return game.team_mode;
}

int getGameFrameDelay_EM(int native_em_game_frame_delay)
{
  int game_frame_delay_value;

  game_frame_delay_value =
    (tape.playing && tape.fast_forward ? FfwdFrameDelay :
     GameFrameDelay == GAME_FRAME_DELAY ? native_em_game_frame_delay :
     GameFrameDelay);

  if (tape.playing && tape.warp_forward && !tape.pausing)
    game_frame_delay_value = 0;

  return game_frame_delay_value;
}

unsigned int InitRND(int seed)
{
  if (level.game_engine_type == GAME_ENGINE_TYPE_EM)
    return InitEngineRandom_EM(seed);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_SP)
    return InitEngineRandom_SP(seed);
  else if (level.game_engine_type == GAME_ENGINE_TYPE_MM)
    return InitEngineRandom_MM(seed);
  else
    return InitEngineRandom_RND(seed);
}

static struct Mapping_EM_to_RND_object object_mapping[TILE_MAX];
static struct Mapping_EM_to_RND_player player_mapping[MAX_PLAYERS][SPR_MAX];

inline static int get_effective_element_EM(int tile, int frame_em)
{
  int element             = object_mapping[tile].element_rnd;
  int action              = object_mapping[tile].action;
  boolean is_backside     = object_mapping[tile].is_backside;
  boolean action_removing = (action == ACTION_DIGGING ||
			     action == ACTION_SNAPPING ||
			     action == ACTION_COLLECTING);

  if (frame_em < 7)
  {
    switch (tile)
    {
      case Yacid_splash_eB:
      case Yacid_splash_wB:
	return (frame_em > 5 ? EL_EMPTY : element);

      default:
	return element;
    }
  }
  else	/* frame_em == 7 */
  {
    switch (tile)
    {
      case Yacid_splash_eB:
      case Yacid_splash_wB:
	return EL_EMPTY;

      case Yemerald_stone:
	return EL_EMERALD;

      case Ydiamond_stone:
	return EL_ROCK;

      case Xdrip_stretch:
      case Xdrip_stretchB:
      case Ydrip_s1:
      case Ydrip_s1B:
      case Xball_1B:
      case Xball_2:
      case Xball_2B:
      case Yball_eat:
      case Ykey_1_eat:
      case Ykey_2_eat:
      case Ykey_3_eat:
      case Ykey_4_eat:
      case Ykey_5_eat:
      case Ykey_6_eat:
      case Ykey_7_eat:
      case Ykey_8_eat:
      case Ylenses_eat:
      case Ymagnify_eat:
      case Ygrass_eat:
      case Ydirt_eat:
      case Xsand_stonein_1:
      case Xsand_stonein_2:
      case Xsand_stonein_3:
      case Xsand_stonein_4:
	return element;

      default:
	return (is_backside || action_removing ? EL_EMPTY : element);
    }
  }
}

inline static boolean check_linear_animation_EM(int tile)
{
  switch (tile)
  {
    case Xsand_stonesand_1:
    case Xsand_stonesand_quickout_1:
    case Xsand_sandstone_1:
    case Xsand_stonein_1:
    case Xsand_stoneout_1:
    case Xboom_1:
    case Xdynamite_1:
    case Ybug_w_n:
    case Ybug_n_e:
    case Ybug_e_s:
    case Ybug_s_w:
    case Ybug_e_n:
    case Ybug_s_e:
    case Ybug_w_s:
    case Ybug_n_w:
    case Ytank_w_n:
    case Ytank_n_e:
    case Ytank_e_s:
    case Ytank_s_w:
    case Ytank_e_n:
    case Ytank_s_e:
    case Ytank_w_s:
    case Ytank_n_w:
    case Yacid_splash_eB:
    case Yacid_splash_wB:
    case Yemerald_stone:
      return TRUE;
  }

  return FALSE;
}

inline static void set_crumbled_graphics_EM(struct GraphicInfo_EM *g_em,
					    boolean has_crumbled_graphics,
					    int crumbled, int sync_frame)
{
  /* if element can be crumbled, but certain action graphics are just empty
     space (like instantly snapping sand to empty space in 1 frame), do not
     treat these empty space graphics as crumbled graphics in EMC engine */
  if (crumbled == IMG_EMPTY_SPACE)
    has_crumbled_graphics = FALSE;

  if (has_crumbled_graphics)
  {
    struct GraphicInfo *g_crumbled = &graphic_info[crumbled];
    int frame_crumbled = getAnimationFrame(g_crumbled->anim_frames,
					   g_crumbled->anim_delay,
					   g_crumbled->anim_mode,
					   g_crumbled->anim_start_frame,
					   sync_frame);

    getGraphicSource(crumbled, frame_crumbled, &g_em->crumbled_bitmap,
		     &g_em->crumbled_src_x, &g_em->crumbled_src_y);

    g_em->crumbled_border_size = graphic_info[crumbled].border_size;
    g_em->crumbled_tile_size = graphic_info[crumbled].tile_size;

    g_em->has_crumbled_graphics = TRUE;
  }
  else
  {
    g_em->crumbled_bitmap = NULL;
    g_em->crumbled_src_x = 0;
    g_em->crumbled_src_y = 0;
    g_em->crumbled_border_size = 0;
    g_em->crumbled_tile_size = 0;

    g_em->has_crumbled_graphics = FALSE;
  }
}

void ResetGfxAnimation_EM(int x, int y, int tile)
{
  GfxFrame[x][y] = 0;
}

void SetGfxAnimation_EM(struct GraphicInfo_EM *g_em,
			int tile, int frame_em, int x, int y)
{
  int action = object_mapping[tile].action;
  int direction = object_mapping[tile].direction;
  int effective_element = get_effective_element_EM(tile, frame_em);
  int graphic = (direction == MV_NONE ?
		 el_act2img(effective_element, action) :
		 el_act_dir2img(effective_element, action, direction));
  struct GraphicInfo *g = &graphic_info[graphic];
  int sync_frame;
  boolean action_removing = (action == ACTION_DIGGING ||
			     action == ACTION_SNAPPING ||
			     action == ACTION_COLLECTING);
  boolean action_moving   = (action == ACTION_FALLING ||
			     action == ACTION_MOVING ||
			     action == ACTION_PUSHING ||
			     action == ACTION_EATING ||
			     action == ACTION_FILLING ||
			     action == ACTION_EMPTYING);
  boolean action_falling  = (action == ACTION_FALLING ||
			     action == ACTION_FILLING ||
			     action == ACTION_EMPTYING);

  /* special case: graphic uses "2nd movement tile" and has defined
     7 frames for movement animation (or less) => use default graphic
     for last (8th) frame which ends the movement animation */
  if (g->double_movement && g->anim_frames < 8 && frame_em == 7)
  {
    action = ACTION_DEFAULT;	/* (keep action_* unchanged for now) */
    graphic = (direction == MV_NONE ?
	       el_act2img(effective_element, action) :
	       el_act_dir2img(effective_element, action, direction));

    g = &graphic_info[graphic];
  }

  if ((action_removing || check_linear_animation_EM(tile)) && frame_em == 0)
  {
    GfxFrame[x][y] = 0;
  }
  else if (action_moving)
  {
    boolean is_backside = object_mapping[tile].is_backside;

    if (is_backside)
    {
      int direction = object_mapping[tile].direction;
      int move_dir = (action_falling ? MV_DOWN : direction);

      GfxFrame[x][y]++;

#if 1
      /* !!! TEST !!! NEW !!! DOES NOT WORK RIGHT YET !!! */
      if (g->double_movement && frame_em == 0)
	GfxFrame[x][y] = 0;
#endif

      if (move_dir == MV_LEFT)
	GfxFrame[x - 1][y] = GfxFrame[x][y];
      else if (move_dir == MV_RIGHT)
	GfxFrame[x + 1][y] = GfxFrame[x][y];
      else if (move_dir == MV_UP)
	GfxFrame[x][y - 1] = GfxFrame[x][y];
      else if (move_dir == MV_DOWN)
	GfxFrame[x][y + 1] = GfxFrame[x][y];
    }
  }
  else
  {
    GfxFrame[x][y]++;

    /* special case: animation for Xsand_stonesand_quickout_1/2 twice as fast */
    if (tile == Xsand_stonesand_quickout_1 ||
	tile == Xsand_stonesand_quickout_2)
      GfxFrame[x][y]++;
  }

  if (graphic_info[graphic].anim_global_sync)
    sync_frame = FrameCounter;
  else if (IN_FIELD(x, y, MAX_LEV_FIELDX, MAX_LEV_FIELDY))
    sync_frame = GfxFrame[x][y];
  else
    sync_frame = 0;	/* playfield border (pseudo steel) */

  SetRandomAnimationValue(x, y);

  int frame = getAnimationFrame(g->anim_frames,
				g->anim_delay,
				g->anim_mode,
				g->anim_start_frame,
				sync_frame);

  g_em->unique_identifier =
    (graphic << 16) | ((frame % 8) << 12) | (g_em->width << 6) | g_em->height;
}

void getGraphicSourceObjectExt_EM(struct GraphicInfo_EM *g_em,
				  int tile, int frame_em, int x, int y)
{
  int action = object_mapping[tile].action;
  int direction = object_mapping[tile].direction;
  boolean is_backside = object_mapping[tile].is_backside;
  int effective_element = get_effective_element_EM(tile, frame_em);
  int effective_action = action;
  int graphic = (direction == MV_NONE ?
		 el_act2img(effective_element, effective_action) :
		 el_act_dir2img(effective_element, effective_action,
				direction));
  int crumbled = (direction == MV_NONE ?
		  el_act2crm(effective_element, effective_action) :
		  el_act_dir2crm(effective_element, effective_action,
				 direction));
  int base_graphic = el_act2img(effective_element, ACTION_DEFAULT);
  int base_crumbled = el_act2crm(effective_element, ACTION_DEFAULT);
  boolean has_crumbled_graphics = (base_crumbled != base_graphic);
  struct GraphicInfo *g = &graphic_info[graphic];
  int sync_frame;

  /* special case: graphic uses "2nd movement tile" and has defined
     7 frames for movement animation (or less) => use default graphic
     for last (8th) frame which ends the movement animation */
  if (g->double_movement && g->anim_frames < 8 && frame_em == 7)
  {
    effective_action = ACTION_DEFAULT;
    graphic = (direction == MV_NONE ?
	       el_act2img(effective_element, effective_action) :
	       el_act_dir2img(effective_element, effective_action,
			      direction));
    crumbled = (direction == MV_NONE ?
		el_act2crm(effective_element, effective_action) :
		el_act_dir2crm(effective_element, effective_action,
			       direction));

    g = &graphic_info[graphic];
  }

  if (graphic_info[graphic].anim_global_sync)
    sync_frame = FrameCounter;
  else if (IN_FIELD(x, y, MAX_LEV_FIELDX, MAX_LEV_FIELDY))
    sync_frame = GfxFrame[x][y];
  else
    sync_frame = 0;	/* playfield border (pseudo steel) */

  SetRandomAnimationValue(x, y);

  int frame = getAnimationFrame(g->anim_frames,
				g->anim_delay,
				g->anim_mode,
				g->anim_start_frame,
				sync_frame);

  getGraphicSourceExt(graphic, frame, &g_em->bitmap, &g_em->src_x, &g_em->src_y,
		      g->double_movement && is_backside);

  /* (updating the "crumbled" graphic definitions is probably not really needed,
     as animations for crumbled graphics can't be longer than one EMC cycle) */
  set_crumbled_graphics_EM(g_em, has_crumbled_graphics, crumbled,
			   sync_frame);
}

void getGraphicSourcePlayerExt_EM(struct GraphicInfo_EM *g_em,
				  int player_nr, int anim, int frame_em)
{
  int element   = player_mapping[player_nr][anim].element_rnd;
  int action    = player_mapping[player_nr][anim].action;
  int direction = player_mapping[player_nr][anim].direction;
  int graphic = (direction == MV_NONE ?
		 el_act2img(element, action) :
		 el_act_dir2img(element, action, direction));
  struct GraphicInfo *g = &graphic_info[graphic];
  int sync_frame;

  InitPlayerGfxAnimation(&stored_player[player_nr], action, direction);

  stored_player[player_nr].StepFrame = frame_em;

  sync_frame = stored_player[player_nr].Frame;

  int frame = getAnimationFrame(g->anim_frames,
				g->anim_delay,
				g->anim_mode,
				g->anim_start_frame,
				sync_frame);

  getGraphicSourceExt(graphic, frame, &g_em->bitmap,
		      &g_em->src_x, &g_em->src_y, FALSE);
}

void InitGraphicInfo_EM(void)
{
  int i, j, p;

#if DEBUG_EM_GFX
  int num_em_gfx_errors = 0;

  if (graphic_info_em_object[0][0].bitmap == NULL)
  {
    /* EM graphics not yet initialized in em_open_all() */

    return;
  }

  printf("::: [4 errors can be ignored (1 x 'bomb', 3 x 'em_dynamite']\n");
#endif

  /* always start with reliable default values */
  for (i = 0; i < TILE_MAX; i++)
  {
    object_mapping[i].element_rnd = EL_UNKNOWN;
    object_mapping[i].is_backside = FALSE;
    object_mapping[i].action = ACTION_DEFAULT;
    object_mapping[i].direction = MV_NONE;
  }

  /* always start with reliable default values */
  for (p = 0; p < MAX_PLAYERS; p++)
  {
    for (i = 0; i < SPR_MAX; i++)
    {
      player_mapping[p][i].element_rnd = EL_UNKNOWN;
      player_mapping[p][i].action = ACTION_DEFAULT;
      player_mapping[p][i].direction = MV_NONE;
    }
  }

  for (i = 0; em_object_mapping_list[i].element_em != -1; i++)
  {
    int e = em_object_mapping_list[i].element_em;

    object_mapping[e].element_rnd = em_object_mapping_list[i].element_rnd;
    object_mapping[e].is_backside = em_object_mapping_list[i].is_backside;

    if (em_object_mapping_list[i].action != -1)
      object_mapping[e].action = em_object_mapping_list[i].action;

    if (em_object_mapping_list[i].direction != -1)
      object_mapping[e].direction =
	MV_DIR_FROM_BIT(em_object_mapping_list[i].direction);
  }

  for (i = 0; em_player_mapping_list[i].action_em != -1; i++)
  {
    int a = em_player_mapping_list[i].action_em;
    int p = em_player_mapping_list[i].player_nr;

    player_mapping[p][a].element_rnd = em_player_mapping_list[i].element_rnd;

    if (em_player_mapping_list[i].action != -1)
      player_mapping[p][a].action = em_player_mapping_list[i].action;

    if (em_player_mapping_list[i].direction != -1)
      player_mapping[p][a].direction =
	MV_DIR_FROM_BIT(em_player_mapping_list[i].direction);
  }

  for (i = 0; i < TILE_MAX; i++)
  {
    int element = object_mapping[i].element_rnd;
    int action = object_mapping[i].action;
    int direction = object_mapping[i].direction;
    boolean is_backside = object_mapping[i].is_backside;
    boolean action_exploding = ((action == ACTION_EXPLODING ||
				 action == ACTION_SMASHED_BY_ROCK ||
				 action == ACTION_SMASHED_BY_SPRING) &&
				element != EL_DIAMOND);
    boolean action_active = (action == ACTION_ACTIVE);
    boolean action_other = (action == ACTION_OTHER);

    for (j = 0; j < 8; j++)
    {
      int effective_element = get_effective_element_EM(i, j);
      int effective_action = (j < 7 ? action :
			      i == Xdrip_stretch ? action :
			      i == Xdrip_stretchB ? action :
			      i == Ydrip_s1 ? action :
			      i == Ydrip_s1B ? action :
			      i == Xball_1B ? action :
			      i == Xball_2 ? action :
			      i == Xball_2B ? action :
			      i == Yball_eat ? action :
			      i == Ykey_1_eat ? action :
			      i == Ykey_2_eat ? action :
			      i == Ykey_3_eat ? action :
			      i == Ykey_4_eat ? action :
			      i == Ykey_5_eat ? action :
			      i == Ykey_6_eat ? action :
			      i == Ykey_7_eat ? action :
			      i == Ykey_8_eat ? action :
			      i == Ylenses_eat ? action :
			      i == Ymagnify_eat ? action :
			      i == Ygrass_eat ? action :
			      i == Ydirt_eat ? action :
			      i == Xsand_stonein_1 ? action :
			      i == Xsand_stonein_2 ? action :
			      i == Xsand_stonein_3 ? action :
			      i == Xsand_stonein_4 ? action :
			      i == Xsand_stoneout_1 ? action :
			      i == Xsand_stoneout_2 ? action :
			      i == Xboom_android ? ACTION_EXPLODING :
			      action_exploding ? ACTION_EXPLODING :
			      action_active ? action :
			      action_other ? action :
			      ACTION_DEFAULT);
      int graphic = (el_act_dir2img(effective_element, effective_action,
				    direction));
      int crumbled = (el_act_dir2crm(effective_element, effective_action,
				     direction));
      int base_graphic = el_act2img(effective_element, ACTION_DEFAULT);
      int base_crumbled = el_act2crm(effective_element, ACTION_DEFAULT);
      boolean has_action_graphics = (graphic != base_graphic);
      boolean has_crumbled_graphics = (base_crumbled != base_graphic);
      struct GraphicInfo *g = &graphic_info[graphic];
      struct GraphicInfo_EM *g_em = &graphic_info_em_object[i][7 - j];
      Bitmap *src_bitmap;
      int src_x, src_y;
      /* ensure to get symmetric 3-frame, 2-delay animations as used in EM */
      boolean special_animation = (action != ACTION_DEFAULT &&
				   g->anim_frames == 3 &&
				   g->anim_delay == 2 &&
				   g->anim_mode & ANIM_LINEAR);
      int sync_frame = (i == Xdrip_stretch ? 7 :
			i == Xdrip_stretchB ? 7 :
			i == Ydrip_s2 ? j + 8 :
			i == Ydrip_s2B ? j + 8 :
			i == Xacid_1 ? 0 :
			i == Xacid_2 ? 10 :
			i == Xacid_3 ? 20 :
			i == Xacid_4 ? 30 :
			i == Xacid_5 ? 40 :
			i == Xacid_6 ? 50 :
			i == Xacid_7 ? 60 :
			i == Xacid_8 ? 70 :
			i == Xfake_acid_1 ? 0 :
			i == Xfake_acid_2 ? 10 :
			i == Xfake_acid_3 ? 20 :
			i == Xfake_acid_4 ? 30 :
			i == Xfake_acid_5 ? 40 :
			i == Xfake_acid_6 ? 50 :
			i == Xfake_acid_7 ? 60 :
			i == Xfake_acid_8 ? 70 :
			i == Xball_2 ? 7 :
			i == Xball_2B ? j + 8 :
			i == Yball_eat ? j + 1 :
			i == Ykey_1_eat ? j + 1 :
			i == Ykey_2_eat ? j + 1 :
			i == Ykey_3_eat ? j + 1 :
			i == Ykey_4_eat ? j + 1 :
			i == Ykey_5_eat ? j + 1 :
			i == Ykey_6_eat ? j + 1 :
			i == Ykey_7_eat ? j + 1 :
			i == Ykey_8_eat ? j + 1 :
			i == Ylenses_eat ? j + 1 :
			i == Ymagnify_eat ? j + 1 :
			i == Ygrass_eat ? j + 1 :
			i == Ydirt_eat ? j + 1 :
			i == Xamoeba_1 ? 0 :
			i == Xamoeba_2 ? 1 :
			i == Xamoeba_3 ? 2 :
			i == Xamoeba_4 ? 3 :
			i == Xamoeba_5 ? 0 :
			i == Xamoeba_6 ? 1 :
			i == Xamoeba_7 ? 2 :
			i == Xamoeba_8 ? 3 :
			i == Xexit_2 ? j + 8 :
			i == Xexit_3 ? j + 16 :
			i == Xdynamite_1 ? 0 :
			i == Xdynamite_2 ? 8 :
			i == Xdynamite_3 ? 16 :
			i == Xdynamite_4 ? 24 :
			i == Xsand_stonein_1 ? j + 1 :
			i == Xsand_stonein_2 ? j + 9 :
			i == Xsand_stonein_3 ? j + 17 :
			i == Xsand_stonein_4 ? j + 25 :
			i == Xsand_stoneout_1 && j == 0 ? 0 :
			i == Xsand_stoneout_1 && j == 1 ? 0 :
			i == Xsand_stoneout_1 && j == 2 ? 1 :
			i == Xsand_stoneout_1 && j == 3 ? 2 :
			i == Xsand_stoneout_1 && j == 4 ? 2 :
			i == Xsand_stoneout_1 && j == 5 ? 3 :
			i == Xsand_stoneout_1 && j == 6 ? 4 :
			i == Xsand_stoneout_1 && j == 7 ? 4 :
			i == Xsand_stoneout_2 && j == 0 ? 5 :
			i == Xsand_stoneout_2 && j == 1 ? 6 :
			i == Xsand_stoneout_2 && j == 2 ? 7 :
			i == Xsand_stoneout_2 && j == 3 ? 8 :
			i == Xsand_stoneout_2 && j == 4 ? 9 :
			i == Xsand_stoneout_2 && j == 5 ? 11 :
			i == Xsand_stoneout_2 && j == 6 ? 13 :
			i == Xsand_stoneout_2 && j == 7 ? 15 :
			i == Xboom_bug && j == 1 ? 2 :
			i == Xboom_bug && j == 2 ? 2 :
			i == Xboom_bug && j == 3 ? 4 :
			i == Xboom_bug && j == 4 ? 4 :
			i == Xboom_bug && j == 5 ? 2 :
			i == Xboom_bug && j == 6 ? 2 :
			i == Xboom_bug && j == 7 ? 0 :
			i == Xboom_bomb && j == 1 ? 2 :
			i == Xboom_bomb && j == 2 ? 2 :
			i == Xboom_bomb && j == 3 ? 4 :
			i == Xboom_bomb && j == 4 ? 4 :
			i == Xboom_bomb && j == 5 ? 2 :
			i == Xboom_bomb && j == 6 ? 2 :
			i == Xboom_bomb && j == 7 ? 0 :
			i == Xboom_android && j == 7 ? 6 :
			i == Xboom_1 && j == 1 ? 2 :
			i == Xboom_1 && j == 2 ? 2 :
			i == Xboom_1 && j == 3 ? 4 :
			i == Xboom_1 && j == 4 ? 4 :
			i == Xboom_1 && j == 5 ? 6 :
			i == Xboom_1 && j == 6 ? 6 :
			i == Xboom_1 && j == 7 ? 8 :
			i == Xboom_2 && j == 0 ? 8 :
			i == Xboom_2 && j == 1 ? 8 :
			i == Xboom_2 && j == 2 ? 10 :
			i == Xboom_2 && j == 3 ? 10 :
			i == Xboom_2 && j == 4 ? 10 :
			i == Xboom_2 && j == 5 ? 12 :
			i == Xboom_2 && j == 6 ? 12 :
			i == Xboom_2 && j == 7 ? 12 :
			special_animation && j == 4 ? 3 :
			effective_action != action ? 0 :
			j);

#if DEBUG_EM_GFX
      Bitmap *debug_bitmap = g_em->bitmap;
      int debug_src_x = g_em->src_x;
      int debug_src_y = g_em->src_y;
#endif

      int frame = getAnimationFrame(g->anim_frames,
				    g->anim_delay,
				    g->anim_mode,
				    g->anim_start_frame,
				    sync_frame);

      getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y,
			  g->double_movement && is_backside);

      g_em->bitmap = src_bitmap;
      g_em->src_x = src_x;
      g_em->src_y = src_y;
      g_em->src_offset_x = 0;
      g_em->src_offset_y = 0;
      g_em->dst_offset_x = 0;
      g_em->dst_offset_y = 0;
      g_em->width  = TILEX;
      g_em->height = TILEY;

      g_em->preserve_background = FALSE;

      set_crumbled_graphics_EM(g_em, has_crumbled_graphics, crumbled,
			       sync_frame);

      if ((!g->double_movement && (effective_action == ACTION_FALLING ||
				   effective_action == ACTION_MOVING  ||
				   effective_action == ACTION_PUSHING ||
				   effective_action == ACTION_EATING)) ||
	  (!has_action_graphics && (effective_action == ACTION_FILLING ||
				    effective_action == ACTION_EMPTYING)))
      {
	int move_dir =
	  (effective_action == ACTION_FALLING ||
	   effective_action == ACTION_FILLING ||
	   effective_action == ACTION_EMPTYING ? MV_DOWN : direction);
	int dx = (move_dir == MV_LEFT ? -1 : move_dir == MV_RIGHT ? 1 : 0);
	int dy = (move_dir == MV_UP   ? -1 : move_dir == MV_DOWN  ? 1 : 0);
	int num_steps = (i == Ydrip_s1  ? 16 :
			 i == Ydrip_s1B ? 16 :
			 i == Ydrip_s2  ? 16 :
			 i == Ydrip_s2B ? 16 :
			 i == Xsand_stonein_1 ? 32 :
			 i == Xsand_stonein_2 ? 32 :
			 i == Xsand_stonein_3 ? 32 :
			 i == Xsand_stonein_4 ? 32 :
			 i == Xsand_stoneout_1 ? 16 :
			 i == Xsand_stoneout_2 ? 16 : 8);
	int cx = ABS(dx) * (TILEX / num_steps);
	int cy = ABS(dy) * (TILEY / num_steps);
	int step_frame = (i == Ydrip_s2         ? j + 8 :
			  i == Ydrip_s2B        ? j + 8 :
			  i == Xsand_stonein_2  ? j + 8 :
			  i == Xsand_stonein_3  ? j + 16 :
			  i == Xsand_stonein_4  ? j + 24 :
			  i == Xsand_stoneout_2 ? j + 8 : j) + 1;
	int step = (is_backside ? step_frame : num_steps - step_frame);

	if (is_backside)	/* tile where movement starts */
	{
	  if (dx < 0 || dy < 0)
	  {
	    g_em->src_offset_x = cx * step;
	    g_em->src_offset_y = cy * step;
	  }
	  else
	  {
	    g_em->dst_offset_x = cx * step;
	    g_em->dst_offset_y = cy * step;
	  }
	}
	else			/* tile where movement ends */
	{
	  if (dx < 0 || dy < 0)
	  {
	    g_em->dst_offset_x = cx * step;
	    g_em->dst_offset_y = cy * step;
	  }
	  else
	  {
	    g_em->src_offset_x = cx * step;
	    g_em->src_offset_y = cy * step;
	  }
	}

	g_em->width  = TILEX - cx * step;
	g_em->height = TILEY - cy * step;
      }

      /* create unique graphic identifier to decide if tile must be redrawn */
      /* bit 31 - 16 (16 bit): EM style graphic
	 bit 15 - 12 ( 4 bit): EM style frame
	 bit 11 -  6 ( 6 bit): graphic width
	 bit  5 -  0 ( 6 bit): graphic height */
      g_em->unique_identifier =
	(graphic << 16) | (frame << 12) | (g_em->width << 6) | g_em->height;

#if DEBUG_EM_GFX

      /* skip check for EMC elements not contained in original EMC artwork */
      if (element == EL_EMC_FAKE_ACID)
	continue;

      if (g_em->bitmap != debug_bitmap ||
	  g_em->src_x != debug_src_x ||
	  g_em->src_y != debug_src_y ||
	  g_em->src_offset_x != 0 ||
	  g_em->src_offset_y != 0 ||
	  g_em->dst_offset_x != 0 ||
	  g_em->dst_offset_y != 0 ||
	  g_em->width != TILEX ||
	  g_em->height != TILEY)
      {
	static int last_i = -1;

	if (i != last_i)
	{
	  printf("\n");
	  last_i = i;
	}

	printf("::: EMC GFX ERROR for element %d -> %d ('%s', '%s', %d)",
	       i, element, element_info[element].token_name,
	       element_action_info[effective_action].suffix, direction);

	if (element != effective_element)
	  printf(" [%d ('%s')]",
		 effective_element,
		 element_info[effective_element].token_name);

	printf("\n");

	if (g_em->bitmap != debug_bitmap)
	  printf("    %d (%d): different bitmap! (0x%08x != 0x%08x)\n",
		 j, is_backside, (int)(g_em->bitmap), (int)(debug_bitmap));

	if (g_em->src_x != debug_src_x ||
	    g_em->src_y != debug_src_y)
	  printf("    frame %d (%c): %d,%d (%d,%d) should be %d,%d (%d,%d)\n",
		 j, (is_backside ? 'B' : 'F'),
		 g_em->src_x, g_em->src_y,
		 g_em->src_x / 32, g_em->src_y / 32,
		 debug_src_x, debug_src_y,
		 debug_src_x / 32, debug_src_y / 32);

	if (g_em->src_offset_x != 0 ||
	    g_em->src_offset_y != 0 ||
	    g_em->dst_offset_x != 0 ||
	    g_em->dst_offset_y != 0)
	  printf("    %d (%d): offsets %d,%d and %d,%d should be all 0\n",
		 j, is_backside,
		 g_em->src_offset_x, g_em->src_offset_y,
		 g_em->dst_offset_x, g_em->dst_offset_y);

	if (g_em->width != TILEX ||
	    g_em->height != TILEY)
	  printf("    %d (%d): size %d,%d should be %d,%d\n",
		 j, is_backside,
		 g_em->width, g_em->height, TILEX, TILEY);

	num_em_gfx_errors++;
      }
#endif

    }
  }

  for (i = 0; i < TILE_MAX; i++)
  {
    for (j = 0; j < 8; j++)
    {
      int element = object_mapping[i].element_rnd;
      int action = object_mapping[i].action;
      int direction = object_mapping[i].direction;
      boolean is_backside = object_mapping[i].is_backside;
      int graphic_action  = el_act_dir2img(element, action, direction);
      int graphic_default = el_act_dir2img(element, ACTION_DEFAULT, direction);

      if ((action == ACTION_SMASHED_BY_ROCK ||
	   action == ACTION_SMASHED_BY_SPRING ||
	   action == ACTION_EATING) &&
	  graphic_action == graphic_default)
      {
	int e = (action == ACTION_SMASHED_BY_ROCK   ? Ystone_s  :
		 action == ACTION_SMASHED_BY_SPRING ? Yspring_s :
		 direction == MV_LEFT  ? (is_backside? Yspring_wB: Yspring_w) :
		 direction == MV_RIGHT ? (is_backside? Yspring_eB: Yspring_e) :
		 Xspring);

	/* no separate animation for "smashed by rock" -- use rock instead */
	struct GraphicInfo_EM *g_em = &graphic_info_em_object[i][7 - j];
	struct GraphicInfo_EM *g_xx = &graphic_info_em_object[e][7 - j];

	g_em->bitmap		= g_xx->bitmap;
	g_em->src_x		= g_xx->src_x;
	g_em->src_y		= g_xx->src_y;
	g_em->src_offset_x	= g_xx->src_offset_x;
	g_em->src_offset_y	= g_xx->src_offset_y;
	g_em->dst_offset_x	= g_xx->dst_offset_x;
	g_em->dst_offset_y	= g_xx->dst_offset_y;
	g_em->width 		= g_xx->width;
	g_em->height		= g_xx->height;
	g_em->unique_identifier	= g_xx->unique_identifier;

	if (!is_backside)
	  g_em->preserve_background = TRUE;
      }
    }
  }

  for (p = 0; p < MAX_PLAYERS; p++)
  {
    for (i = 0; i < SPR_MAX; i++)
    {
      int element = player_mapping[p][i].element_rnd;
      int action = player_mapping[p][i].action;
      int direction = player_mapping[p][i].direction;

      for (j = 0; j < 8; j++)
      {
	int effective_element = element;
	int effective_action = action;
	int graphic = (direction == MV_NONE ?
		       el_act2img(effective_element, effective_action) :
		       el_act_dir2img(effective_element, effective_action,
				      direction));
	struct GraphicInfo *g = &graphic_info[graphic];
	struct GraphicInfo_EM *g_em = &graphic_info_em_player[p][i][7 - j];
	Bitmap *src_bitmap;
	int src_x, src_y;
	int sync_frame = j;

#if DEBUG_EM_GFX
	Bitmap *debug_bitmap = g_em->bitmap;
	int debug_src_x = g_em->src_x;
	int debug_src_y = g_em->src_y;
#endif

	int frame = getAnimationFrame(g->anim_frames,
				      g->anim_delay,
				      g->anim_mode,
				      g->anim_start_frame,
				      sync_frame);

	getGraphicSourceExt(graphic, frame, &src_bitmap, &src_x, &src_y, FALSE);

	g_em->bitmap = src_bitmap;
	g_em->src_x = src_x;
	g_em->src_y = src_y;
	g_em->src_offset_x = 0;
	g_em->src_offset_y = 0;
	g_em->dst_offset_x = 0;
	g_em->dst_offset_y = 0;
	g_em->width  = TILEX;
	g_em->height = TILEY;

#if DEBUG_EM_GFX

	/* skip check for EMC elements not contained in original EMC artwork */
	if (element == EL_PLAYER_3 ||
	    element == EL_PLAYER_4)
	  continue;

	if (g_em->bitmap != debug_bitmap ||
	    g_em->src_x != debug_src_x ||
	    g_em->src_y != debug_src_y)
	{
	  static int last_i = -1;

	  if (i != last_i)
	  {
	    printf("\n");
	    last_i = i;
	  }

	  printf("::: EMC GFX ERROR for p/a %d/%d -> %d ('%s', '%s', %d)",
		 p, i, element, element_info[element].token_name,
		 element_action_info[effective_action].suffix, direction);

	  if (element != effective_element)
	    printf(" [%d ('%s')]",
		   effective_element,
		   element_info[effective_element].token_name);

	  printf("\n");

	  if (g_em->bitmap != debug_bitmap)
	    printf("    %d: different bitmap! (0x%08x != 0x%08x)\n",
		   j, (int)(g_em->bitmap), (int)(debug_bitmap));

	  if (g_em->src_x != debug_src_x ||
	      g_em->src_y != debug_src_y)
	    printf("    frame %d: %d,%d (%d,%d) should be %d,%d (%d,%d)\n",
		   j,
		   g_em->src_x, g_em->src_y,
		   g_em->src_x / 32, g_em->src_y / 32,
		   debug_src_x, debug_src_y,
		   debug_src_x / 32, debug_src_y / 32);

	  num_em_gfx_errors++;
	}
#endif

      }
    }
  }

#if DEBUG_EM_GFX
  printf("\n");
  printf("::: [%d errors found]\n", num_em_gfx_errors);

  exit(0);
#endif
}

void CheckSaveEngineSnapshot_EM(byte action[MAX_PLAYERS], int frame,
				boolean any_player_moving,
				boolean any_player_snapping,
				boolean any_player_dropping)
{
  if (frame == 0 && !any_player_dropping)
  {
    if (!local_player->was_waiting)
    {
      if (!CheckSaveEngineSnapshotToList())
	return;

      local_player->was_waiting = TRUE;
    }
  }
  else if (any_player_moving || any_player_snapping || any_player_dropping)
  {
    local_player->was_waiting = FALSE;
  }
}

void CheckSaveEngineSnapshot_SP(boolean murphy_is_waiting,
				boolean murphy_is_dropping)
{
  if (murphy_is_waiting)
  {
    if (!local_player->was_waiting)
    {
      if (!CheckSaveEngineSnapshotToList())
	return;

      local_player->was_waiting = TRUE;
    }
  }
  else
  {
    local_player->was_waiting = FALSE;
  }
}

void CheckSaveEngineSnapshot_MM(boolean element_clicked,
				boolean button_released)
{
  if (button_released)
  {
    if (game.snapshot.mode == SNAPSHOT_MODE_EVERY_MOVE)
      CheckSaveEngineSnapshotToList();
  }
  else if (element_clicked)
  {
    if (game.snapshot.mode != SNAPSHOT_MODE_EVERY_MOVE)
      CheckSaveEngineSnapshotToList();

    game.snapshot.changed_action = TRUE;
  }
}

void CheckSingleStepMode_EM(byte action[MAX_PLAYERS], int frame,
			    boolean any_player_moving,
			    boolean any_player_snapping,
			    boolean any_player_dropping)
{
  if (tape.single_step && tape.recording && !tape.pausing)
    if (frame == 0 && !any_player_dropping)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  CheckSaveEngineSnapshot_EM(action, frame, any_player_moving,
			     any_player_snapping, any_player_dropping);
}

void CheckSingleStepMode_SP(boolean murphy_is_waiting,
			    boolean murphy_is_dropping)
{
  boolean murphy_starts_dropping = FALSE;
  int i;

  for (i = 0; i < MAX_PLAYERS; i++)
    if (stored_player[i].force_dropping)
      murphy_starts_dropping = TRUE;

  if (tape.single_step && tape.recording && !tape.pausing)
    if (murphy_is_waiting && !murphy_starts_dropping)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  CheckSaveEngineSnapshot_SP(murphy_is_waiting, murphy_is_dropping);
}

void CheckSingleStepMode_MM(boolean element_clicked,
			    boolean button_released)
{
  if (tape.single_step && tape.recording && !tape.pausing)
    if (button_released)
      TapeTogglePause(TAPE_TOGGLE_AUTOMATIC);

  CheckSaveEngineSnapshot_MM(element_clicked, button_released);
}

void getGraphicSource_SP(struct GraphicInfo_SP *g_sp,
			 int graphic, int sync_frame, int x, int y)
{
  int frame = getGraphicAnimationFrame(graphic, sync_frame);

  getGraphicSource(graphic, frame, &g_sp->bitmap, &g_sp->src_x, &g_sp->src_y);
}

boolean isNextAnimationFrame_SP(int graphic, int sync_frame)
{
  return (IS_NEXT_FRAME(sync_frame, graphic));
}

int getGraphicInfo_Delay(int graphic)
{
  return graphic_info[graphic].anim_delay;
}

void PlayMenuSoundExt(int sound)
{
  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
  else
    PlaySound(sound);
}

void PlayMenuSound()
{
  PlayMenuSoundExt(menu.sound[game_status]);
}

void PlayMenuSoundStereo(int sound, int stereo_position)
{
  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundExt(sound, SOUND_MAX_VOLUME, stereo_position, SND_CTRL_PLAY_LOOP);
  else
    PlaySoundStereo(sound, stereo_position);
}

void PlayMenuSoundIfLoopExt(int sound)
{
  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
}

void PlayMenuSoundIfLoop()
{
  PlayMenuSoundIfLoopExt(menu.sound[game_status]);
}

void PlayMenuMusicExt(int music)
{
  if (music == MUS_UNDEFINED)
    return;

  if (!setup.sound_music)
    return;

  PlayMusic(music);
}

void PlayMenuMusic()
{
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(menu.music[game_status]);

  if (!strEqual(curr_music, next_music))
    PlayMenuMusicExt(menu.music[game_status]);
}

void PlayMenuSoundsAndMusic()
{
  PlayMenuSound();
  PlayMenuMusic();
}

static void FadeMenuSounds()
{
  FadeSounds();
}

static void FadeMenuMusic()
{
  char *curr_music = getCurrentlyPlayingMusicFilename();
  char *next_music = getMusicInfoEntryFilename(menu.music[game_status]);

  if (!strEqual(curr_music, next_music))
    FadeMusic();
}

void FadeMenuSoundsAndMusic()
{
  FadeMenuSounds();
  FadeMenuMusic();
}

void PlaySoundActivating()
{
#if 0
  PlaySound(SND_MENU_ITEM_ACTIVATING);
#endif
}

void PlaySoundSelecting()
{
#if 0
  PlaySound(SND_MENU_ITEM_SELECTING);
#endif
}

void ToggleFullscreenOrChangeWindowScalingIfNeeded()
{
  boolean change_fullscreen = (setup.fullscreen !=
			       video.fullscreen_enabled);
  boolean change_window_scaling_percent = (!video.fullscreen_enabled &&
					   setup.window_scaling_percent !=
					   video.window_scaling_percent);

  if (change_window_scaling_percent && video.fullscreen_enabled)
    return;

  if (!change_window_scaling_percent && !video.fullscreen_available)
    return;

#if defined(TARGET_SDL2)
  if (change_window_scaling_percent)
  {
    SDLSetWindowScaling(setup.window_scaling_percent);

    return;
  }
  else if (change_fullscreen)
  {
    SDLSetWindowFullscreen(setup.fullscreen);

    /* set setup value according to successfully changed fullscreen mode */
    setup.fullscreen = video.fullscreen_enabled;

    return;
  }
#endif

  if (change_fullscreen ||
      change_window_scaling_percent)
  {
    Bitmap *tmp_backbuffer = CreateBitmap(WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH);

    /* save backbuffer content which gets lost when toggling fullscreen mode */
    BlitBitmap(backbuffer, tmp_backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    if (change_window_scaling_percent)
    {
      /* keep window mode, but change window scaling */
      video.fullscreen_enabled = TRUE;		/* force new window scaling */
    }

    /* toggle fullscreen */
    ChangeVideoModeIfNeeded(setup.fullscreen);

    /* set setup value according to successfully changed fullscreen mode */
    setup.fullscreen = video.fullscreen_enabled;

    /* restore backbuffer content from temporary backbuffer backup bitmap */
    BlitBitmap(tmp_backbuffer, backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);

    FreeBitmap(tmp_backbuffer);

    /* update visible window/screen */
    BlitBitmap(backbuffer, window, 0, 0, WIN_XSIZE, WIN_YSIZE, 0, 0);
  }
}

void JoinRectangles(int *x, int *y, int *width, int *height,
		    int x2, int y2, int width2, int height2)
{
  // do not join with "off-screen" rectangle
  if (x2 == -1 || y2 == -1)
    return;

  *x = MIN(*x, x2);
  *y = MIN(*y, y2);
  *width = MAX(*width, width2);
  *height = MAX(*height, height2);
}

void SetAnimStatus(int anim_status_new)
{
  if (anim_status_new == GAME_MODE_MAIN)
    anim_status_new = GAME_MODE_PSEUDO_MAINONLY;
  else if (anim_status_new == GAME_MODE_SCORES)
    anim_status_new = GAME_MODE_PSEUDO_SCORESOLD;

  global.anim_status_next = anim_status_new;

  // directly set screen modes that are entered without fading
  if ((global.anim_status      == GAME_MODE_PSEUDO_MAINONLY &&
       global.anim_status_next == GAME_MODE_PSEUDO_TYPENAME) ||
      (global.anim_status      == GAME_MODE_PSEUDO_TYPENAME &&
       global.anim_status_next == GAME_MODE_PSEUDO_MAINONLY))
    global.anim_status = global.anim_status_next;
}

void SetGameStatus(int game_status_new)
{
  if (game_status_new != game_status)
    game_status_last_screen = game_status;

  game_status = game_status_new;

  SetAnimStatus(game_status_new);
}

void SetFontStatus(int game_status_new)
{
  static int last_game_status = -1;

  if (game_status_new != -1)
  {
    // set game status for font use after storing last game status
    last_game_status = game_status;
    game_status = game_status_new;
  }
  else
  {
    // reset game status after font use from last stored game status
    game_status = last_game_status;
  }
}

void ResetFontStatus()
{
  SetFontStatus(-1);
}

boolean CheckIfPlayfieldViewportHasChanged()
{
  // if game status has not changed, playfield viewport has not changed either
  if (game_status == game_status_last)
    return FALSE;

  // check if playfield viewport has changed with current game status
  struct RectWithBorder *vp_playfield = &viewport.playfield[game_status];
  int new_real_sx	= vp_playfield->x;
  int new_real_sy	= vp_playfield->y;
  int new_full_sxsize	= vp_playfield->width;
  int new_full_sysize	= vp_playfield->height;

  return (new_real_sx != REAL_SX ||
	  new_real_sy != REAL_SY ||
	  new_full_sxsize != FULL_SXSIZE ||
	  new_full_sysize != FULL_SYSIZE);
}

boolean CheckIfGlobalBorderOrPlayfieldViewportHasChanged()
{
  return (CheckIfGlobalBorderHasChanged() ||
	  CheckIfPlayfieldViewportHasChanged());
}

void ChangeViewportPropertiesIfNeeded()
{
  boolean use_mini_tilesize = (level.game_engine_type == GAME_ENGINE_TYPE_MM ?
			       FALSE : setup.small_game_graphics);
  int gfx_game_mode = game_status;
  int gfx_game_mode2 = (game_status == GAME_MODE_EDITOR ? GAME_MODE_DEFAULT :
			game_status);
  struct RectWithBorder *vp_window    = &viewport.window[gfx_game_mode];
  struct RectWithBorder *vp_playfield = &viewport.playfield[gfx_game_mode];
  struct RectWithBorder *vp_door_1    = &viewport.door_1[gfx_game_mode];
  struct RectWithBorder *vp_door_2    = &viewport.door_2[gfx_game_mode2];
  struct RectWithBorder *vp_door_3    = &viewport.door_2[GAME_MODE_EDITOR];
  int new_win_xsize	= vp_window->width;
  int new_win_ysize	= vp_window->height;
  int border_size	= vp_playfield->border_size;
  int new_sx		= vp_playfield->x + border_size;
  int new_sy		= vp_playfield->y + border_size;
  int new_sxsize	= vp_playfield->width  - 2 * border_size;
  int new_sysize	= vp_playfield->height - 2 * border_size;
  int new_real_sx	= vp_playfield->x;
  int new_real_sy	= vp_playfield->y;
  int new_full_sxsize	= vp_playfield->width;
  int new_full_sysize	= vp_playfield->height;
  int new_dx		= vp_door_1->x;
  int new_dy		= vp_door_1->y;
  int new_dxsize	= vp_door_1->width;
  int new_dysize	= vp_door_1->height;
  int new_vx		= vp_door_2->x;
  int new_vy		= vp_door_2->y;
  int new_vxsize	= vp_door_2->width;
  int new_vysize	= vp_door_2->height;
  int new_ex		= vp_door_3->x;
  int new_ey		= vp_door_3->y;
  int new_exsize	= vp_door_3->width;
  int new_eysize	= vp_door_3->height;
  int new_tilesize_var = (use_mini_tilesize ? MINI_TILESIZE : game.tile_size);
  int tilesize = (gfx_game_mode == GAME_MODE_PLAYING ? new_tilesize_var :
		  gfx_game_mode == GAME_MODE_EDITOR ? MINI_TILESIZE : TILESIZE);
  int new_scr_fieldx = new_sxsize / tilesize;
  int new_scr_fieldy = new_sysize / tilesize;
  int new_scr_fieldx_buffers = new_sxsize / new_tilesize_var;
  int new_scr_fieldy_buffers = new_sysize / new_tilesize_var;
  boolean init_gfx_buffers = FALSE;
  boolean init_video_buffer = FALSE;
  boolean init_gadgets_and_anims = FALSE;
  boolean init_em_graphics = FALSE;

  if (new_win_xsize != WIN_XSIZE ||
      new_win_ysize != WIN_YSIZE)
  {
    WIN_XSIZE = new_win_xsize;
    WIN_YSIZE = new_win_ysize;

    init_video_buffer = TRUE;
    init_gfx_buffers = TRUE;
    init_gadgets_and_anims = TRUE;

    // printf("::: video: init_video_buffer, init_gfx_buffers\n");
  }

  if (new_scr_fieldx != SCR_FIELDX ||
      new_scr_fieldy != SCR_FIELDY)
  {
    /* this always toggles between MAIN and GAME when using small tile size */

    SCR_FIELDX = new_scr_fieldx;
    SCR_FIELDY = new_scr_fieldy;

    // printf("::: new_scr_fieldx != SCR_FIELDX ...\n");
  }

  if (new_sx != SX ||
      new_sy != SY ||
      new_dx != DX ||
      new_dy != DY ||
      new_vx != VX ||
      new_vy != VY ||
      new_ex != EX ||
      new_ey != EY ||
      new_sxsize != SXSIZE ||
      new_sysize != SYSIZE ||
      new_dxsize != DXSIZE ||
      new_dysize != DYSIZE ||
      new_vxsize != VXSIZE ||
      new_vysize != VYSIZE ||
      new_exsize != EXSIZE ||
      new_eysize != EYSIZE ||
      new_real_sx != REAL_SX ||
      new_real_sy != REAL_SY ||
      new_full_sxsize != FULL_SXSIZE ||
      new_full_sysize != FULL_SYSIZE ||
      new_tilesize_var != TILESIZE_VAR
      )
  {
    // ------------------------------------------------------------------------
    // determine next fading area for changed viewport definitions
    // ------------------------------------------------------------------------

    // start with current playfield area (default fading area)
    FADE_SX = REAL_SX;
    FADE_SY = REAL_SY;
    FADE_SXSIZE = FULL_SXSIZE;
    FADE_SYSIZE = FULL_SYSIZE;

    // add new playfield area if position or size has changed
    if (new_real_sx != REAL_SX || new_real_sy != REAL_SY ||
	new_full_sxsize != FULL_SXSIZE || new_full_sysize != FULL_SYSIZE)
    {
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     new_real_sx, new_real_sy, new_full_sxsize,new_full_sysize);
    }

    // add current and new door 1 area if position or size has changed
    if (new_dx != DX || new_dy != DY ||
	new_dxsize != DXSIZE || new_dysize != DYSIZE)
    {
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     DX, DY, DXSIZE, DYSIZE);
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     new_dx, new_dy, new_dxsize, new_dysize);
    }

    // add current and new door 2 area if position or size has changed
    if (new_dx != VX || new_dy != VY ||
	new_dxsize != VXSIZE || new_dysize != VYSIZE)
    {
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     VX, VY, VXSIZE, VYSIZE);
      JoinRectangles(&FADE_SX, &FADE_SY, &FADE_SXSIZE, &FADE_SYSIZE,
		     new_vx, new_vy, new_vxsize, new_vysize);
    }

    // ------------------------------------------------------------------------
    // handle changed tile size
    // ------------------------------------------------------------------------

    if (new_tilesize_var != TILESIZE_VAR)
    {
      // printf("::: new_tilesize_var != TILESIZE_VAR\n");

      // changing tile size invalidates scroll values of engine snapshots
      FreeEngineSnapshotSingle();

      // changing tile size requires update of graphic mapping for EM engine
      init_em_graphics = TRUE;
    }

    SX = new_sx;
    SY = new_sy;
    DX = new_dx;
    DY = new_dy;
    VX = new_vx;
    VY = new_vy;
    EX = new_ex;
    EY = new_ey;
    SXSIZE = new_sxsize;
    SYSIZE = new_sysize;
    DXSIZE = new_dxsize;
    DYSIZE = new_dysize;
    VXSIZE = new_vxsize;
    VYSIZE = new_vysize;
    EXSIZE = new_exsize;
    EYSIZE = new_eysize;
    REAL_SX = new_real_sx;
    REAL_SY = new_real_sy;
    FULL_SXSIZE = new_full_sxsize;
    FULL_SYSIZE = new_full_sysize;
    TILESIZE_VAR = new_tilesize_var;

    init_gfx_buffers = TRUE;
    init_gadgets_and_anims = TRUE;

    // printf("::: viewports: init_gfx_buffers\n");
    // printf("::: viewports: init_gadgets_and_anims\n");
  }

  if (init_gfx_buffers)
  {
    // printf("::: init_gfx_buffers\n");

    SCR_FIELDX = new_scr_fieldx_buffers;
    SCR_FIELDY = new_scr_fieldy_buffers;

    InitGfxBuffers();

    SCR_FIELDX = new_scr_fieldx;
    SCR_FIELDY = new_scr_fieldy;

    SetDrawDeactivationMask(REDRAW_NONE);
    SetDrawBackgroundMask(REDRAW_FIELD);
  }

  if (init_video_buffer)
  {
    // printf("::: init_video_buffer\n");

    InitVideoBuffer(WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH, setup.fullscreen);
    InitImageTextures();
  }

  if (init_gadgets_and_anims)
  {
    // printf("::: init_gadgets_and_anims\n");

    InitGadgets();
    InitGlobalAnimations();
  }

  if (init_em_graphics)
  {
      InitGraphicInfo_EM();
  }
}
