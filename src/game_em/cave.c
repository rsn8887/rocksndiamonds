/* 2000-08-10T16:43:50Z
 *
 * cave data structures
 */

#include "main_em.h"


struct LevelInfo_EM native_em_level;

void setLevelInfoToDefaults_EM(void)
{
  int i;

  native_em_level.file_version = FILE_VERSION_EM_ACTUAL;

  native_em_level.lev = &lev;
  for (i = 0; i < MAX_PLAYERS; i++)
    native_em_level.ply[i] = &ply[i];

  lev.width = 64;
  lev.height = 32;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].x_initial = 0;
    ply[i].y_initial = 0;
  }

  lev.lenses_cnt_initial = 0;
  lev.magnify_cnt_initial = 0;

  lev.wheel_cnt_initial = 0;
  lev.wheel_x_initial = 1;
  lev.wheel_y_initial = 1;

  lev.wind_time = 9999;
  lev.wind_cnt_initial = 0;

  lev.wonderwall_state_initial = 0;
  lev.wonderwall_time_initial = 0;

  lev.num_ball_arrays = 8;

  for (i = 0; i < TILE_MAX; i++)
    lev.android_array[i] = Xblank;

  /* initial number of players in this level */
  lev.home_initial = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].exists = 0;
    ply[i].alive_initial = FALSE;
  }
}


/* load cave
 * 
 * completely initializes the level structure, ready for a game
 */

#define MAX_EM_LEVEL_SIZE		16384

boolean LoadNativeLevel_EM(char *filename, boolean level_info_only)
{
  unsigned char raw_leveldata[MAX_EM_LEVEL_SIZE];
  int raw_leveldata_length;
  int file_version;
  File *file;

  /* always start with reliable default values */
  setLevelInfoToDefaults_EM();

  if (!(file = openFile(filename, MODE_READ)))
  {
    if (!level_info_only)
      Error(ERR_WARN, "cannot open level '%s' -- using empty level", filename);

    return FALSE;
  }

  raw_leveldata_length = readFile(file, raw_leveldata, 1, MAX_EM_LEVEL_SIZE);

  closeFile(file);

  if (raw_leveldata_length <= 0)
  {
    Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return FALSE;
  }

  file_version = cleanup_em_level(raw_leveldata, raw_leveldata_length,filename);

  if (file_version == FILE_VERSION_EM_UNKNOWN)
  {
    Error(ERR_WARN, "unknown EM level '%s' -- using empty level", filename);

    return FALSE;
  }

  convert_em_level(raw_leveldata, file_version);
  prepare_em_level();

  return TRUE;
}
