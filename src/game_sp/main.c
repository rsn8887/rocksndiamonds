
#include "main_sp.h"
#include "global.h"


struct GameInfo_SP game_sp;
struct LevelInfo_SP native_sp_level;

int GfxElementLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
int GfxGraphicLast[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
int GfxGraphic[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];
int GfxFrame[SP_MAX_PLAYFIELD_WIDTH][SP_MAX_PLAYFIELD_HEIGHT];


void InitGameEngine_SP()
{
  int x, y;

  gfx.anim_random_frame = -1;	// (use simple, ad-hoc random numbers)

  game_sp.LevelSolved = FALSE;
  game_sp.GameOver = FALSE;

  game_sp.time_played = 0;
  game_sp.infotrons_still_needed = native_sp_level.header.InfotronsNeeded;
  game_sp.red_disk_count = 0;
  game_sp.score = 0;

  menBorder = setup.sp_show_border_elements;

  game_sp.scroll_xoffset = (EVEN(SCR_FIELDX) ? TILEX / 2 : 0);
  game_sp.scroll_yoffset = (EVEN(SCR_FIELDY) ? TILEY / 2 : 0);

  if (native_sp_level.width <= SCR_FIELDX)
    game_sp.scroll_xoffset = TILEX / 2;
  if (native_sp_level.height <= SCR_FIELDY)
    game_sp.scroll_yoffset = TILEY / 2;

  for (x = 0; x < SP_MAX_PLAYFIELD_WIDTH; x++)
  {
    for (y = 0; y < SP_MAX_PLAYFIELD_HEIGHT; y++)
    {
      GfxElementLast[x][y] = -1;
      GfxGraphicLast[x][y] = -1;
      GfxGraphic[x][y] = -1;
      GfxFrame[x][y] = 0;
    }
  }

  InitScrollPlayfield();

  menPlay_Click();
}

void RedrawPlayfield_SP(boolean force_redraw)
{
  // skip redrawing playfield in warp mode or when testing tapes with "autotest"
  if (DrawingDeactivatedField())
    return;

  if (force_redraw)
    RestorePlayfield();

  UpdatePlayfield(force_redraw);
}

void UpdateGameDoorValues_SP()
{
  game_sp.time_played = TimerVar / FRAMES_PER_SECOND;
  game_sp.infotrons_still_needed = InfotronsNeeded;
  game_sp.red_disk_count = RedDiskCount;
  game_sp.score = 0;		// (currently no score in Supaplex engine)
}

void GameActions_SP(byte action[MAX_PLAYERS], boolean warp_mode)
{
  byte single_player_action = action[0];
  int x, y;

  UpdateEngineValues(mScrollX / TILEX, mScrollY / TILEY,
		     MurphyScreenXPos / TILEX, MurphyScreenYPos / TILEY);

  subMainGameLoop_Main(single_player_action, warp_mode);

  RedrawPlayfield_SP(FALSE);

  UpdateGameDoorValues_SP();

  CheckSingleStepMode_SP(PlayField16[MurphyPosIndex] == fiMurphy,
			 HighByte(PlayField16[MurphyPosIndex]) == 0x2A);

  for (x = DisplayMinX; x <= DisplayMaxX; x++)
    for (y = DisplayMinY; y <= DisplayMaxY; y++)
      GfxFrame[x][y]++;
}

int getRedDiskReleaseFlag_SP()
{
  /* 0: when Murphy is moving (including the destination tile!) */
  /* 1: when Murphy is not moving for at least one game frame */

  return RedDiskReleaseFlag;
}
