/* third part of synchro.
 *
 * handle global elements.
 *
 * this should be spread over the frames for reduced cpu load.
 */

#include "main_em.h"


void synchro_3(void)
{
  int x;
  int y;
  int count;
  unsigned int random;

  /* update variables */

  if (lev.score > 9999)
    lev.score = 9999;

  if (lev.android_move_cnt-- == 0)
    lev.android_move_cnt = lev.android_move_time;
  if (lev.android_clone_cnt-- == 0)
    lev.android_clone_cnt = lev.android_clone_time;
  if (lev.ball_state)
    if (lev.ball_cnt-- == 0)
      lev.ball_cnt = lev.ball_time;
  if (lev.lenses_cnt)
    lev.lenses_cnt--;
  if (lev.magnify_cnt)
    lev.magnify_cnt--;
  if (lev.wheel_cnt)
    lev.wheel_cnt--;
  if (lev.wind_cnt)
    lev.wind_cnt--;
  if (lev.wonderwall_time && lev.wonderwall_state)
    lev.wonderwall_time--;

  if (lev.wheel_cnt)
    play_element_sound(lev.wheel_x, lev.wheel_y, SAMPLE_wheel, Xwheel);

  /* grow amoeba */

  random = RandomEM;

  for (count = lev.amoeba_time; count--;)
  {
    x = (random >> 10) % (WIDTH - 2);
    y = (random >> 20) % (HEIGHT - 2);
    switch (Cave[y][x])
    {
      case Xblank:
      case Yacid_splash_eB:
      case Yacid_splash_wB:
      case Xgrass:
      case Xdirt:
      case Xsand:
      case Xplant:
      case Yplant:
	if (tab_amoeba[Cave[y-1][x]] ||
	    tab_amoeba[Cave[y][x+1]] ||
	    tab_amoeba[Cave[y+1][x]] ||
	    tab_amoeba[Cave[y][x-1]])
	  Cave[y][x] = Xdrip_eat;
    }

    random = random * 129 + 1;
  }

  RandomEM = random;

  /* handle explosions */

  for (y = 1; y < HEIGHT - 1; y++)
    for (x = 1; x < WIDTH - 1; x++)
    {
      switch (Cave[y][x])
      {
        case Znormal:
	  Cave[y][x] = Xboom_1;
	  Cave[y-1][x] = tab_explode_normal[Cave[y-1][x]];
	  Cave[y][x-1] = tab_explode_normal[Cave[y][x-1]];
	  Cave[y][x+1] = tab_explode_normal[Cave[y][x+1]];
	  Cave[y+1][x] = tab_explode_normal[Cave[y+1][x]];
	  Cave[y-1][x-1] = tab_explode_normal[Cave[y-1][x-1]];
	  Cave[y-1][x+1] = tab_explode_normal[Cave[y-1][x+1]];
	  Cave[y+1][x-1] = tab_explode_normal[Cave[y+1][x-1]];
	  Cave[y+1][x+1] = tab_explode_normal[Cave[y+1][x+1]];
	  break;

        case Zdynamite:
	  Cave[y][x] = Xboom_1;
	  Cave[y-1][x] = tab_explode_dynamite[Cave[y-1][x]];
	  Cave[y][x-1] = tab_explode_dynamite[Cave[y][x-1]];
	  Cave[y][x+1] = tab_explode_dynamite[Cave[y][x+1]];
	  Cave[y+1][x] = tab_explode_dynamite[Cave[y+1][x]];
	  Cave[y-1][x-1] = tab_explode_dynamite[Cave[y-1][x-1]];
	  Cave[y-1][x+1] = tab_explode_dynamite[Cave[y-1][x+1]];
	  Cave[y+1][x-1] = tab_explode_dynamite[Cave[y+1][x-1]];
	  Cave[y+1][x+1] = tab_explode_dynamite[Cave[y+1][x+1]];
	  break;
      }
    }

  /* triple buffering */

  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Next[y][x] = Cave[y][x];
}
