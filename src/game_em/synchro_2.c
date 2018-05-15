/* second part of synchro.
 *
 * game logic for monsters.
 *
 * one giant switch statement to process everything.
 *
 * this whole thing is a major bottleneck. the compiler must use registers.
 * compilers suck.
 */

#include "main_em.h"


#define RANDOM (random = random << 31 | random >> 1)

static void set_nearest_player_xy(int x, int y, int *dx, int *dy)
{
  int distance, distance_shortest = EM_MAX_CAVE_WIDTH + EM_MAX_CAVE_HEIGHT;
  int i;

  /* default values if no players are alive anymore */
  *dx = 0;
  *dy = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (!ply[i].alive)
      continue;

    distance = ABS(ply[i].x - x) + ABS(ply[i].y - y);

    if (distance < distance_shortest)
    {
      *dx = ply[i].x;
      *dy = ply[i].y;

      distance_shortest = distance;
    }
  }
}

void synchro_2(void)
{
  int x = 0;
  int y = 1;
  unsigned int random = RandomEM;
  short *cave_cache = Cave[y];	/* might be a win */
  int score = 0;

  int temp = 0;			/* initialized to make compilers happy */
  int dx;			/* only needed to find closest player */
  int dy;
  int element;

 loop:

  element = cave_cache[++x];

  switch (element)
  {
    default:
      goto loop;

    /* --------------------------------------------------------------------- */

#ifdef BAD_ROLL
    case Xstone_force_e:
      switch (Cave[y][x+1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xstone;
  	  Next[y][x] = Xstone;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ystone_eB;
  	  Cave[y][x+1] = Ystone_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xstone_pause;
  	  goto loop;
      }

    case Xstone_force_w:
      switch (Cave[y][x-1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xstone;
  	  Next[y][x] = Xstone;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ystone_wB;
  	  Cave[y][x-1] = Ystone_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xstone_pause;
  	  goto loop;
      }

    case Xnut_force_e:
      switch (Cave[y][x+1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xnut;
  	  Next[y][x] = Xnut;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ynut_eB;
  	  Cave[y][x+1] = Ynut_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xnut_pause;
  	  goto loop;
      }

    case Xnut_force_w:
      switch (Cave[y][x-1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xnut;
  	  Next[y][x] = Xnut;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ynut_wB;
  	  Cave[y][x-1] = Ynut_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xnut_pause;
  	  goto loop;
  	}

    case Xspring_force_e:
      switch (Cave[y][x+1])
      {
        case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xspring;
  	  Next[y][x] = Xspring;
  	  goto loop;

	default:
  	  Cave[y][x] = Yspring_eB;
  	  Cave[y][x+1] = Yspring_e;
  	  Next[y][x] = Xblank;

#ifdef BAD_SPRING
	  Next[y][x+1] = Xspring_e;
#else
	  Next[y][x+1] = Xspring_pause;
#endif

	  goto loop;
      }

    case Xspring_force_w:
      switch (Cave[y][x-1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xspring;
  	  Next[y][x] = Xspring;
  	  goto loop;

  	default:
  	  Cave[y][x] = Yspring_wB;
  	  Cave[y][x-1] = Yspring_w;
  	  Next[y][x] = Xblank;

#ifdef BAD_SPRING
  	  Next[y][x-1] = Xspring_w;
#else	
  	  Next[y][x-1] = Xspring_pause;
#endif	
  	  goto loop;
  	}

    case Xemerald_force_e:
      switch (Cave[y][x+1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xemerald;
  	  Next[y][x] = Xemerald;
  	  goto loop;

  	default:
  	  Cave[y][x] = Yemerald_eB;
  	  Cave[y][x+1] = Yemerald_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xemerald_pause;
  	  goto loop;
  	}

    case Xemerald_force_w:
      switch (Cave[y][x-1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xemerald;
  	  Next[y][x] = Xemerald;
  	  goto loop;

  	default:
  	  Cave[y][x] = Yemerald_wB;
  	  Cave[y][x-1] = Yemerald_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xemerald_pause;
  	  goto loop;
  	}

    case Xdiamond_force_e:
      switch (Cave[y][x+1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xdiamond;
  	  Next[y][x] = Xdiamond;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ydiamond_eB;
  	  Cave[y][x+1] = Ydiamond_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xdiamond_pause;
  	  goto loop;
  	}

    case Xdiamond_force_w:
      switch (Cave[y][x-1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xdiamond;
  	  Next[y][x] = Xdiamond;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ydiamond_wB;
  	  Cave[y][x-1] = Ydiamond_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xdiamond_pause;
  	  goto loop;
  	}

    case Xbomb_force_e:
      switch (Cave[y][x+1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xbomb;
  	  Next[y][x] = Xbomb;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ybomb_eB;
  	  Cave[y][x+1] = Ybomb_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xbomb_pause;
  	  goto loop;
  	}

    case Xbomb_force_w:
      switch (Cave[y][x-1])
      {
  	case ZBORDER:
  	case Znormal:
  	case Zdynamite:
  	case Xboom_bug:
  	case Xboom_bomb:
  	case Xboom_android:
  	case Xboom_1:
  	case Zplayer:
  	  Cave[y][x] = Xbomb;
  	  Next[y][x] = Xbomb;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ybomb_wB;
  	  Cave[y][x-1] = Ybomb_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xbomb_pause;
  	  goto loop;
  	}
#endif	/* BAD_ROLL */

    /* --------------------------------------------------------------------- */

    case Xstone:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ystone_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
#if 1
        case Xfake_acid_1:
        case Xfake_acid_2:
        case Xfake_acid_3:
        case Xfake_acid_4:
        case Xfake_acid_5:
        case Xfake_acid_6:
        case Xfake_acid_7:
        case Xfake_acid_8:
#endif
  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Ystone_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xstone_fall;
  	  goto loop;

  	case Xsand:
  	  Cave[y][x] = Xsand_stonein_1;
  	  Cave[y+1][x] = Xsand_sandstone_1;
  	  Next[y][x] = Xsand_stonein_2;
  	  Next[y+1][x] = Xsand_sandstone_2;
  	  goto loop;

        case Xspring:
  	case Xspring_pause:
  	case Xspring_e:
  	case Xspring_w:
  	case Xandroid:
  	case Xandroid_1_n:
  	case Xandroid_2_n:
  	case Xandroid_1_e:
  	case Xandroid_2_e:
  	case Xandroid_1_s:
  	case Xandroid_2_s:
  	case Xandroid_1_w:
  	case Xandroid_2_w:
  	case Xstone:
  	case Xstone_pause:
  	case Xemerald:
  	case Xemerald_pause:
  	case Xdiamond:
  	case Xdiamond_pause:
  	case Xbomb:
  	case Xbomb_pause:
  	case Xballoon:
  	case Xacid_ne:
  	case Xacid_nw:
  	case Xball_1:
  	case Xball_2:
  	case Xnut:
  	case Xnut_pause:
  	case Xgrow_ns:
  	case Xgrow_ew:
  	case Xkey_1:
  	case Xkey_2:
  	case Xkey_3:
  	case Xkey_4:
  	case Xkey_5:
  	case Xkey_6:
  	case Xkey_7:
  	case Xkey_8:
  	case Xbumper:
  	case Xswitch:
  	case Xlenses:
  	case Xmagnify:
  	case Xround_wall_1:
  	case Xround_wall_2:
  	case Xround_wall_3:
  	case Xround_wall_4:
  	  if (RANDOM & 1)
	  {
  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ystone_eB;
  	      Cave[y][x+1] = Ystone_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xstone_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ystone_wB;
  	      Cave[y][x-1] = Ystone_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xstone_pause;
  	      goto loop;
  	    }
  	  }
	  else
	  {
  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ystone_wB;
  	      Cave[y][x-1] = Ystone_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xstone_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ystone_eB;
  	      Cave[y][x+1] = Ystone_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xstone_pause;
  	      goto loop;
  	    }
  	  }

        default:
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xstone_pause:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ystone_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
#if 1
        case Xfake_acid_1:
        case Xfake_acid_2:
        case Xfake_acid_3:
        case Xfake_acid_4:
        case Xfake_acid_5:
        case Xfake_acid_6:
        case Xfake_acid_7:
        case Xfake_acid_8:
#endif
  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Ystone_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xstone_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xstone;
  	  Next[y][x] = Xstone;
  	  goto loop;
  	}

    /* --------------------------------------------------------------------- */

    case Xstone_fall:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ystone_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Zplayer:
#if 1
        case Xfake_acid_1:
        case Xfake_acid_2:
        case Xfake_acid_3:
        case Xfake_acid_4:
        case Xfake_acid_5:
        case Xfake_acid_6:
        case Xfake_acid_7:
        case Xfake_acid_8:
#endif
  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Ystone_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xstone_fall;
  	  goto loop;

  	case Xnut:
  	case Xnut_pause:
  	  Cave[y+1][x] = Yemerald_stone;
  	  Next[y][x] = Xstone;
  	  Next[y+1][x] = Xemerald;
  	  play_element_sound(x, y, SAMPLE_crack, Xnut);
  	  score += lev.nut_score;
  	  goto loop;

  	case Xbug_n:
  	case Xbug_e:
  	case Xbug_s:
  	case Xbug_w:
  	case Xbug_gon:
  	case Xbug_goe:
  	case Xbug_gos:
  	case Xbug_gow:
  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Ybug_stone;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xemerald;
  	  Boom[y][x] = Xemerald;
  	  Boom[y][x+1] = Xemerald;
  	  Boom[y+1][x-1] = Xemerald;
  	  Boom[y+1][x] = Xdiamond;
  	  Boom[y+1][x+1] = Xemerald;
  	  Boom[y+2][x-1] = Xemerald;
  	  Boom[y+2][x] = Xemerald;
  	  Boom[y+2][x+1] = Xemerald;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  score += lev.bug_score;
  	  goto loop;

  	case Xtank_n:
  	case Xtank_e:
  	case Xtank_s:
  	case Xtank_w:
  	case Xtank_gon:
  	case Xtank_goe:
  	case Xtank_gos:
  	case Xtank_gow:
  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Ytank_stone;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xblank;
  	  Boom[y][x] = Xblank;
  	  Boom[y][x+1] = Xblank;
  	  Boom[y+1][x-1] = Xblank;
  	  Boom[y+1][x] = Xblank;
  	  Boom[y+1][x+1] = Xblank;
  	  Boom[y+2][x-1] = Xblank;
  	  Boom[y+2][x] = Xblank;
  	  Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  score += lev.tank_score;
  	  goto loop;

  	case Xspring:
  	  if (RANDOM & 1)
	  {
  	    switch (Cave[y+1][x+1])
	    {
  	      case Xblank:
  	      case Yacid_splash_eB:
  	      case Yacid_splash_wB:
  	      case Xalien:
  	      case Xalien_pause:
  	        Cave[y+1][x] = Xspring_e;
		break;

  	      default:
		Cave[y+1][x] = Xspring_w;
		break;
  	    }
  	  }
	  else
	  {
  	    switch (Cave[y+1][x-1])
	    {
  	      case Xblank:
  	      case Yacid_splash_eB:
  	      case Yacid_splash_wB:
  	      case Xalien:
  	      case Xalien_pause:
		Cave[y+1][x] = Xspring_w;
		break;
  	      default:
		Cave[y+1][x] = Xspring_e;
		break;
  	    }
  	  }

  	  Next[y][x] = Xstone;
  	  goto loop;

  	case Xeater_n:
  	case Xeater_e:
  	case Xeater_s:
  	case Xeater_w:
  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Yeater_stone;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = lev.eater_array[lev.eater_pos][0];
  	  Boom[y][x] = lev.eater_array[lev.eater_pos][1];
  	  Boom[y][x+1] = lev.eater_array[lev.eater_pos][2];
  	  Boom[y+1][x-1] = lev.eater_array[lev.eater_pos][3];
  	  Boom[y+1][x] = lev.eater_array[lev.eater_pos][4];
  	  Boom[y+1][x+1] = lev.eater_array[lev.eater_pos][5];
  	  Boom[y+2][x-1] = lev.eater_array[lev.eater_pos][6];
  	  Boom[y+2][x] = lev.eater_array[lev.eater_pos][7];
  	  Boom[y+2][x+1] = lev.eater_array[lev.eater_pos][8];
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  lev.eater_pos = (lev.eater_pos + 1) & 7;
  	  score += lev.eater_score;
  	  goto loop;

  	case Xalien:
  	case Xalien_pause:
  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Yalien_stone;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xblank;
  	  Boom[y][x] = Xblank;
  	  Boom[y][x+1] = Xblank;
  	  Boom[y+1][x-1] = Xblank;
  	  Boom[y+1][x] = Xblank;
  	  Boom[y+1][x+1] = Xblank;
  	  Boom[y+2][x-1] = Xblank;
  	  Boom[y+2][x] = Xblank;
  	  Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  score += lev.alien_score;
  	  goto loop;

  	case Xdiamond:
  	case Xdiamond_pause:
  	  switch (Cave[y+2][x])
	  {
  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	    case Zplayer:
  	    case Xbug_n:
  	    case Xbug_e:
  	    case Xbug_s:
  	    case Xbug_w:
  	    case Xbug_gon:
  	    case Xbug_goe:
  	    case Xbug_gos:
  	    case Xbug_gow:
  	    case Xtank_n:
  	    case Xtank_e:
  	    case Xtank_s:
  	    case Xtank_w:
  	    case Xtank_gon:
  	    case Xtank_goe:
  	    case Xtank_gos:
  	    case Xtank_gow:
  	    case Xspring_fall:
  	    case Xandroid:
  	    case Xandroid_1_n:
  	    case Xandroid_2_n:
  	    case Xandroid_1_e:
  	    case Xandroid_2_e:
  	    case Xandroid_1_s:
  	    case Xandroid_2_s:
  	    case Xandroid_1_w:
  	    case Xandroid_2_w:
  	    case Xstone_fall:
  	    case Xemerald_fall:
  	    case Xdiamond_fall:
  	    case Xbomb_fall:
  	    case Xacid_s:
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	    case Xnut_fall:
  	    case Xplant:
  	    case Yplant:
  	      Next[y][x] = Xstone;
  	      play_element_sound(x, y, SAMPLE_stone, Xstone);
  	      goto loop;
  	  }

  	  Cave[y][x] = Ystone_sB;
  	  Cave[y+1][x] = Ydiamond_stone;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xstone_pause;
  	  play_element_sound(x, y, SAMPLE_squash, Xdiamond);
  	  goto loop;

  	case Xbomb:
  	case Xbomb_pause:
  	  Cave[y+1][x] = Ybomb_eat;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xblank;
  	  Boom[y][x] = Xblank;
  	  Boom[y][x+1] = Xblank;
  	  Boom[y+1][x-1] = Xblank;
  	  Boom[y+1][x] = Xblank;
  	  Boom[y+1][x+1] = Xblank;
  	  Boom[y+2][x-1] = Xblank;
  	  Boom[y+2][x] = Xblank;
  	  Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  goto loop;

  	case Xwonderwall:
  	  if (lev.wonderwall_time)
	  {
  	    lev.wonderwall_state = 1;
  	    Cave[y][x] = Ystone_sB;

  	    if (tab_blank[Cave[y+2][x]])
	    {
  	      Cave[y+2][x] = Yemerald_s;
  	      Next[y+2][x] = Xemerald_fall;
  	    }

  	    Next[y][x] = Xblank;
  	    play_element_sound(x, y, SAMPLE_wonderfall, Xwonderwall);
  	    goto loop;
  	  }

  	default:
  	  Cave[y][x] = Xstone;
  	  Next[y][x] = Xstone;
  	  play_element_sound(x, y, SAMPLE_stone, Xstone);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xnut:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ynut_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Ynut_sB;
  	  Cave[y+1][x] = Ynut_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xnut_fall;
  	  goto loop;

  	case Xspring:
  	case Xspring_pause:
  	case Xspring_e:
  	case Xspring_w:
  	case Xandroid:
  	case Xandroid_1_n:
  	case Xandroid_2_n:
  	case Xandroid_1_e:
  	case Xandroid_2_e:
  	case Xandroid_1_s:
  	case Xandroid_2_s:
  	case Xandroid_1_w:
  	case Xandroid_2_w:
  	case Xstone:
  	case Xstone_pause:
  	case Xemerald:
  	case Xemerald_pause:
  	case Xdiamond:
  	case Xdiamond_pause:
  	case Xbomb:
  	case Xbomb_pause:
  	case Xballoon:
  	case Xacid_ne:
  	case Xacid_nw:
  	case Xball_1:
  	case Xball_2:
  	case Xnut:
  	case Xnut_pause:
  	case Xgrow_ns:
  	case Xgrow_ew:
  	case Xkey_1:
  	case Xkey_2:
  	case Xkey_3:
  	case Xkey_4:
  	case Xkey_5:
  	case Xkey_6:
  	case Xkey_7:
  	case Xkey_8:
  	case Xbumper:
  	case Xswitch:
  	case Xround_wall_1:
  	case Xround_wall_2:
  	case Xround_wall_3:
  	case Xround_wall_4:
  	  if (RANDOM & 1)
	  {
  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ynut_eB;
  	      Cave[y][x+1] = Ynut_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xnut_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ynut_wB;
  	      Cave[y][x-1] = Ynut_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xnut_pause;
  	      goto loop;
  	    }
  	  }
	  else
	  {
  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ynut_wB;
  	      Cave[y][x-1] = Ynut_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xnut_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ynut_eB;
  	      Cave[y][x+1] = Ynut_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xnut_pause;
  	      goto loop;
  	    }
  	  }

        default:
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */


    case Xnut_pause:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ynut_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Ynut_sB;
  	  Cave[y+1][x] = Ynut_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xnut_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xnut;
  	  Next[y][x] = Xnut;
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xnut_fall:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ynut_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Zplayer:
  	  Cave[y][x] = Ynut_sB;
  	  Cave[y+1][x] = Ynut_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xnut_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xnut;
  	  Next[y][x] = Xnut;
  	  play_element_sound(x, y, SAMPLE_nut, Xnut);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xbug_n:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

      switch (Cave[y][x+1])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ybug_n_e;
  	  Next[y][x] = Xbug_goe;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  goto bug_gon;
  	}

    case Xbug_gon:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

    bug_gon:

      switch (Cave[y-1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ybug_nB;
  	  if (Cave[y-2][x+1] == Xblank)
	    Cave[y-2][x+1] = Yacid_splash_eB;
  	  if (Cave[y-2][x-1] == Xblank)
	    Cave[y-2][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ybug_nB;
  	  Cave[y-1][x] = Ybug_n;
  	  Next[y][x] = Xblank;
  	  Next[y-1][x] = Xbug_n;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ybug_n_w;
  	  Next[y][x] = Xbug_gow;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xbug_e:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

      switch (Cave[y+1][x])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ybug_e_s;
  	  Next[y][x] = Xbug_gos;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  goto bug_goe;
      }

    case Xbug_goe:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

    bug_goe:

      switch (Cave[y][x+1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ybug_eB;
  	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Yacid_splash_eB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ybug_eB;
  	  Cave[y][x+1] = Ybug_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xbug_e;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ybug_e_n;
  	  Next[y][x] = Xbug_gon;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xbug_s:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

      switch (Cave[y][x-1])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ybug_s_w;
  	  Next[y][x] = Xbug_gow;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  goto bug_gos;
      }

    case Xbug_gos:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

    bug_gos:

      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ybug_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ybug_sB;
  	  Cave[y+1][x] = Ybug_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xbug_s;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ybug_s_e;
  	  Next[y][x] = Xbug_goe;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xbug_w:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

      switch (Cave[y-1][x])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ybug_w_n;
  	  Next[y][x] = Xbug_gon;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  goto bug_gow;
      }

    case Xbug_gow:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto bug_boom;

    bug_gow:

      switch (Cave[y][x-1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ybug_wB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_eB;
  	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ybug_wB;
  	  Cave[y][x-1] = Ybug_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xbug_w;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ybug_w_s;
  	  Next[y][x] = Xbug_gos;
  	  play_element_sound(x, y, SAMPLE_bug, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xtank_n:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

      switch (Cave[y][x-1])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ytank_n_w;
  	  Next[y][x] = Xtank_gow;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  goto tank_gon;
      }

    case Xtank_gon:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

    tank_gon:

      switch (Cave[y-1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ytank_nB;
  	  if (Cave[y-2][x+1] == Xblank)
	    Cave[y-2][x+1] = Yacid_splash_eB;
  	  if (Cave[y-2][x-1] == Xblank)
	    Cave[y-2][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ytank_nB;
  	  Cave[y-1][x] = Ytank_n;
  	  Next[y][x] = Xblank;
  	  Next[y-1][x] = Xtank_n;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ytank_n_e;
  	  Next[y][x] = Xtank_goe;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xtank_e:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

      switch (Cave[y-1][x])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ytank_e_n;
  	  Next[y][x] = Xtank_gon;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  goto tank_goe;
      }

    case Xtank_goe:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

    tank_goe:

      switch (Cave[y][x+1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ytank_eB;
  	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Yacid_splash_eB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ytank_eB;
  	  Cave[y][x+1] = Ytank_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xtank_e;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ytank_e_s;
  	  Next[y][x] = Xtank_gos;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xtank_s:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

      switch (Cave[y][x+1])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ytank_s_e;
  	  Next[y][x] = Xtank_goe;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  goto tank_gos;
      }

    case Xtank_gos:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

    tank_gos:

      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ytank_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ytank_sB;
  	  Cave[y+1][x] = Ytank_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xtank_s;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ytank_s_w;
  	  Next[y][x] = Xtank_gow;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xtank_w:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

      switch (Cave[y+1][x])
      {
  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	case Zplayer:
  	  Cave[y][x] = Ytank_w_s;
  	  Next[y][x] = Xtank_gos;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  goto tank_gow;
      }

    case Xtank_gow:
      if (tab_amoeba[Cave[y-1][x]] ||
	  tab_amoeba[Cave[y][x+1]] ||
	  tab_amoeba[Cave[y+1][x]] ||
	  tab_amoeba[Cave[y][x-1]])
	goto tank_boom;

    tank_gow:

      switch (Cave[y][x-1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ytank_wB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_eB;
  	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ytank_wB;
  	  Cave[y][x-1] = Ytank_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xtank_w;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;

  	default:
  	  Cave[y][x] = Ytank_w_n;
  	  Next[y][x] = Xtank_gon;
  	  play_element_sound(x, y, SAMPLE_tank, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xandroid:

    android:

      if (lev.android_clone_cnt == 0)
      {
  	if (Cave[y-1][x-1] != Xblank &&
	    Cave[y-1][x]   != Xblank &&
	    Cave[y-1][x+1] != Xblank &&
	    Cave[y][x-1]   != Xblank &&
	    Cave[y][x+1]   != Xblank &&
	    Cave[y+1][x-1] != Xblank &&
	    Cave[y+1][x]   != Xblank &&
	    Cave[y+1][x+1] != Xblank)
	  goto android_move;

  	switch (RANDOM & 7)
	{
	  /* randomly find an object to clone */

  	  case 0: /* S,NE,W,NW,SE,E,SW,N */
	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	    goto android_move;

  	  case 1: /* NW,SE,N,S,NE,SW,E,W */
	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	    goto android_move;

  	  case 2: /* SW,E,S,W,N,NW,SE,NE */
	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	    goto android_move;

  	  case 3: /* N,SE,NE,E,W,S,NW,SW */
	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
	    goto android_move;

  	  case 4: /* SE,NW,E,NE,SW,W,N,S */
  	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
  	    goto android_move;

  	  case 5: /* NE,W,SE,SW,S,N,E,NW */
  	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
  	    goto android_move;

  	  case 6: /* E,N,SW,S,NW,NE,SE,W */
  	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
  	    goto android_move;

  	  case 7: /* W,SW,NW,N,E,SE,NE,S */
  	    temp= lev.android_array[Cave[y][x-1]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x-1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x-1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y][x+1]];   if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y-1][x+1]]; if (temp != Xblank) break;
  	    temp= lev.android_array[Cave[y+1][x]];   if (temp != Xblank) break;
  	    goto android_move;
  	}

  	Next[y][x] = temp;	/* the item we chose to clone */
  	play_element_sound(x, y, SAMPLE_android_clone, temp);

  	switch (RANDOM & 7)
	{
	  /* randomly find a direction to move */

  	  case 0: /* S,NE,W,NW,SE,E,SW,N */
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    goto android_move;

  	  case 1: /* NW,SE,N,S,NE,SW,E,W */
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    goto android_move;

  	  case 2: /* SW,E,S,W,N,NW,SE,NE */
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    goto android_move;

  	  case 3: /* N,SE,NE,E,W,S,NW,SW */
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    goto android_move;

  	  case 4: /* SE,NW,E,NE,SW,W,N,S */
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    goto android_move;

  	  case 5: /* NE,W,SE,SW,S,N,E,NW */
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    goto android_move;

  	  case 6: /* E,N,SW,S,NW,NE,SE,W */
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    goto android_move;

  	  case 7: /* W,SW,NW,N,E,SE,NE,S */
  	    if (Cave[y][x-1] == Xblank)   goto android_w;
  	    if (Cave[y+1][x-1] == Xblank) goto android_sw;
  	    if (Cave[y-1][x-1] == Xblank) goto android_nw;
  	    if (Cave[y-1][x] == Xblank)   goto android_n;
  	    if (Cave[y][x+1] == Xblank)   goto android_e;
  	    if (Cave[y+1][x+1] == Xblank) goto android_se;
  	    if (Cave[y-1][x+1] == Xblank) goto android_ne;
  	    if (Cave[y+1][x] == Xblank)   goto android_s;
  	    goto android_move;
  	}
      }

    android_move:
      if (lev.android_move_cnt == 0)
      {
  	if (Cave[y-1][x-1] == Zplayer ||
	    Cave[y-1][x]   == Zplayer ||
	    Cave[y-1][x+1] == Zplayer ||
	    Cave[y][x-1]   == Zplayer ||
	    Cave[y][x+1]   == Zplayer ||
	    Cave[y+1][x-1] == Zplayer ||
	    Cave[y+1][x]   == Zplayer ||
	    Cave[y+1][x+1] == Zplayer)
	  goto android_still;

	set_nearest_player_xy(x, y, &dx, &dy);

  	Next[y][x] = Xblank;	/* assume we will move */
  	temp = ((x < dx) + 1 - (x > dx)) + ((y < dy) + 1 - (y > dy)) * 3;

  	if (RANDOM & 1)
	{
  	  switch (temp)
	  {
	    /* attempt clockwise move first if direct path is blocked */

  	    case 0: /* north west */
  	      if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
  	      if (tab_android_move[Cave[y-1][x]])   goto android_n;
  	      if (tab_android_move[Cave[y][x-1]])   goto android_w;
  	      break;

  	    case 1: /* north */
  	      if (tab_android_move[Cave[y-1][x]])   goto android_n;
  	      if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
  	      if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
  	      break;

  	    case 2: /* north east */
  	      if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
  	      if (tab_android_move[Cave[y][x+1]])   goto android_e;
  	      if (tab_android_move[Cave[y-1][x]])   goto android_n;
  	      break;

  	    case 3: /* west */
  	      if (tab_android_move[Cave[y][x-1]])   goto android_w;
  	      if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
  	      if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
  	      break;

  	    case 4: /* nowhere */
  	      break;

  	    case 5: /* east */
  	      if (tab_android_move[Cave[y][x+1]])   goto android_e;
  	      if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
  	      if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
  	      break;

  	    case 6: /* south west */
  	      if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
  	      if (tab_android_move[Cave[y][x-1]])   goto android_w;
  	      if (tab_android_move[Cave[y+1][x]])   goto android_s;
  	      break;

  	    case 7: /* south */
  	      if (tab_android_move[Cave[y+1][x]])   goto android_s;
  	      if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
  	      if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
  	      break;

  	    case 8: /* south east */
  	      if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
  	      if (tab_android_move[Cave[y+1][x]])   goto android_s;
  	      if (tab_android_move[Cave[y][x+1]])   goto android_e;
  	      break;
	  }
	}
	else
	{
	  switch (temp)
	  {
	    /* attempt counterclockwise move first if direct path is blocked */

  	    case 0: /* north west */
  	      if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
  	      if (tab_android_move[Cave[y][x-1]])   goto android_w;
  	      if (tab_android_move[Cave[y-1][x]])   goto android_n;
  	      break;

  	    case 1: /* north */
  	      if (tab_android_move[Cave[y-1][x]])   goto android_n;
  	      if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
  	      if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
  	      break;

  	    case 2: /* north east */
  	      if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
  	      if (tab_android_move[Cave[y-1][x]])   goto android_n;
  	      if (tab_android_move[Cave[y][x+1]])   goto android_e;
  	      break;

  	    case 3: /* west */
  	      if (tab_android_move[Cave[y][x-1]])   goto android_w;
  	      if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
  	      if (tab_android_move[Cave[y-1][x-1]]) goto android_nw;
  	      break;

  	    case 4: /* nowhere */
  	      break;

  	    case 5: /* east */
  	      if (tab_android_move[Cave[y][x+1]])   goto android_e;
  	      if (tab_android_move[Cave[y-1][x+1]]) goto android_ne;
  	      if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
  	      break;

  	    case 6: /* south west */
  	      if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
  	      if (tab_android_move[Cave[y+1][x]])   goto android_s;
  	      if (tab_android_move[Cave[y][x-1]])   goto android_w;
  	      break;

  	    case 7: /* south */
  	      if (tab_android_move[Cave[y+1][x]])   goto android_s;
  	      if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
  	      if (tab_android_move[Cave[y+1][x-1]]) goto android_sw;
  	      break;

  	    case 8: /* south east */
  	      if (tab_android_move[Cave[y+1][x+1]]) goto android_se;
  	      if (tab_android_move[Cave[y][x+1]])   goto android_e;
  	      if (tab_android_move[Cave[y+1][x]])   goto android_s;
  	      break;
	  }
	}
      }

    android_still:

      Next[y][x] = Xandroid;
      goto loop;

    android_n:

      Cave[y][x] = Yandroid_nB;
      Cave[y-1][x] = Yandroid_n;
      Next[y-1][x] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    android_ne:

      Cave[y][x] = Yandroid_neB;
      Cave[y-1][x+1] = Yandroid_ne;
      Next[y-1][x+1] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    android_e:

      Cave[y][x] = Yandroid_eB;
      Cave[y][x+1] = Yandroid_e;
      Next[y][x+1] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    android_se:

      Cave[y][x] = Yandroid_seB;
      Cave[y+1][x+1] = Yandroid_se;
      Next[y+1][x+1] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    android_s:

      Cave[y][x] = Yandroid_sB;
      Cave[y+1][x] = Yandroid_s;
      Next[y+1][x] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    android_sw:

      Cave[y][x] = Yandroid_swB;
      Cave[y+1][x-1] = Yandroid_sw;
      Next[y+1][x-1] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    android_w:

      Cave[y][x] = Yandroid_wB;
      Cave[y][x-1] = Yandroid_w;
      Next[y][x-1] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    android_nw:

      Cave[y][x] = Yandroid_nwB;
      Cave[y-1][x-1] = Yandroid_nw;
      Next[y-1][x-1] = Xandroid;
      play_element_sound(x, y, SAMPLE_android_move, element);
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xandroid_1_n:
      switch (Cave[y-1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_nB;
  	  if (Cave[y-2][x+1] == Xblank)
	    Cave[y-2][x+1] = Yacid_splash_eB;
  	  if (Cave[y-2][x-1] == Xblank)
	    Cave[y-2][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_nB;
  	  Cave[y-1][x] = Yandroid_n;
  	  Next[y][x] = Xblank;
  	  Next[y-1][x] = Xandroid;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    case Xandroid_2_n:
      switch (Cave[y-1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_nB;
  	  if (Cave[y-2][x+1] == Xblank)
	    Cave[y-2][x+1] = Yacid_splash_eB;
  	  if (Cave[y-2][x-1] == Xblank)
	    Cave[y-2][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_nB;
  	  Cave[y-1][x] = Yandroid_n;
  	  Next[y][x] = Xblank;
  	  Next[y-1][x] = Xandroid_1_n;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    /* --------------------------------------------------------------------- */

    case Xandroid_1_e:
      switch (Cave[y][x+1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_eB;
  	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Yacid_splash_eB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_eB;
  	  Cave[y][x+1] = Yandroid_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xandroid;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    case Xandroid_2_e:
      switch (Cave[y][x+1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_eB;
  	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Yacid_splash_eB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_eB;
  	  Cave[y][x+1] = Yandroid_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xandroid_1_e;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    /* --------------------------------------------------------------------- */

    case Xandroid_1_s:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_sB;
  	  Cave[y+1][x] = Yandroid_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xandroid;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    case Xandroid_2_s:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_sB;
  	  Cave[y+1][x] = Yandroid_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xandroid_1_s;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    /* --------------------------------------------------------------------- */

    case Xandroid_1_w:
      switch (Cave[y][x-1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_wB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_eB;
  	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_wB;
  	  Cave[y][x-1] = Yandroid_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xandroid;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    case Xandroid_2_w:
      switch (Cave[y][x-1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yandroid_wB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_eB;
  	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yandroid_wB;
  	  Cave[y][x-1] = Yandroid_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xandroid_1_w;
  	  play_element_sound(x, y, SAMPLE_android_move, element);
  	  goto loop;

  	default:
  	  goto android;
      }

    /* --------------------------------------------------------------------- */

    case Xspring:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yspring_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Yspring_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xspring_fall;
  	  goto loop;

  	case Xspring:
  	case Xspring_pause:
  	case Xspring_e:
  	case Xspring_w:
  	case Xandroid:
  	case Xandroid_1_n:
  	case Xandroid_2_n:
  	case Xandroid_1_e:
  	case Xandroid_2_e:
  	case Xandroid_1_s:
  	case Xandroid_2_s:
  	case Xandroid_1_w:
  	case Xandroid_2_w:
  	case Xstone:
  	case Xstone_pause:
  	case Xemerald:
  	case Xemerald_pause:
  	case Xdiamond:
  	case Xdiamond_pause:
  	case Xbomb:
  	case Xbomb_pause:
  	case Xballoon:
  	case Xacid_ne:
  	case Xacid_nw:
  	case Xball_1:
  	case Xball_2:
  	case Xnut:
  	case Xnut_pause:
  	case Xgrow_ns:
  	case Xgrow_ew:
  	case Xkey_1:
  	case Xkey_2:
  	case Xkey_3:
  	case Xkey_4:
  	case Xkey_5:
  	case Xkey_6:
  	case Xkey_7:
  	case Xkey_8:
  	case Xbumper:
  	case Xswitch:
  	case Xround_wall_1:
  	case Xround_wall_2:
  	case Xround_wall_3:
  	case Xround_wall_4:
  	  if (RANDOM & 1)
	  {
  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Yspring_eB;
  	      Cave[y][x+1] = Yspring_e;
  	      if (Cave[y+1][x] == Xbumper)
		Cave[y+1][x] = XbumperB;
  	      Next[y][x] = Xblank;

#ifdef BAD_SPRING
  	      Next[y][x+1] = Xspring_e;
#else	
  	      Next[y][x+1] = Xspring_pause;
#endif

  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Yspring_wB;
  	      Cave[y][x-1] = Yspring_w;
  	      if (Cave[y+1][x] == Xbumper)
		Cave[y+1][x] = XbumperB;
  	      Next[y][x] = Xblank;

#ifdef BAD_SPRING
  	      Next[y][x-1] = Xspring_w;
#else
  	      Next[y][x-1] = Xspring_pause;
#endif

  	      goto loop;
  	    }
  	  }
	  else
	  {
  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Yspring_wB;
  	      Cave[y][x-1] = Yspring_w;
  	      if (Cave[y+1][x] == Xbumper)
		Cave[y+1][x] = XbumperB;
  	      Next[y][x] = Xblank;

#ifdef BAD_SPRING
  	      Next[y][x-1] = Xspring_w;
#else
  	      Next[y][x-1] = Xspring_pause;
#endif

  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Yspring_eB;
  	      Cave[y][x+1] = Yspring_e;
  	      if (Cave[y+1][x] == Xbumper)
		Cave[y+1][x] = XbumperB;
  	      Next[y][x] = Xblank;

#ifdef BAD_SPRING
  	      Next[y][x+1] = Xspring_e;
#else
  	      Next[y][x+1] = Xspring_pause;
#endif

  	      goto loop;
  	    }
  	  }

  	default:
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xspring_pause:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yspring_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Yspring_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xspring_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xspring;
  	  Next[y][x] = Xspring;
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xspring_e:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yspring_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Yspring_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xspring_fall;
  	  goto loop;

  	case Xbumper:
  	  Cave[y+1][x] = XbumperB;
      }

      switch (Cave[y][x+1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yspring_eB;
  	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Yacid_splash_eB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Yalien_nB:
  	case Yalien_eB:
  	case Yalien_sB:
  	case Yalien_wB:
  	  Cave[y][x] = Yspring_eB;
  	  Cave[y][x+1] = Yspring_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xspring_e;
  	  goto loop;

  	case Xalien:
  	case Xalien_pause:
  	case Yalien_n:
  	case Yalien_e:
  	case Yalien_s:
  	case Yalien_w:
  	  Cave[y][x] = Yspring_kill_eB;
  	  Cave[y][x+1] = Yspring_kill_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xspring_e;
  	  play_element_sound(x, y, SAMPLE_slurp, Xalien);
  	  score += lev.slurp_score;
  	  goto loop;

  	case Xbumper:
  	case XbumperB:
  	  Cave[y][x+1] = XbumperB;
  	  Next[y][x] = Xspring_w;
  	  play_element_sound(x, y, SAMPLE_spring, Xspring);
  	  goto loop;

  	default:
  	  Cave[y][x] = Xspring;
  	  Next[y][x] = Xspring;
  	  play_element_sound(x, y, SAMPLE_spring, Xspring);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xspring_w:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yspring_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Yspring_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xspring_fall;
  	  goto loop;

  	case Xbumper:
  	  Cave[y+1][x] = XbumperB;
      }

      switch (Cave[y][x-1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yspring_wB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_eB;
  	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Yalien_nB:
  	case Yalien_eB:
  	case Yalien_sB:
  	case Yalien_wB:
  	  Cave[y][x] = Yspring_wB;
  	  Cave[y][x-1] = Yspring_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xspring_w;
  	  goto loop;

  	case Xalien:
  	case Xalien_pause:
  	case Yalien_n:
  	case Yalien_e:
  	case Yalien_s:
  	case Yalien_w:
  	  Cave[y][x] = Yspring_kill_wB;
  	  Cave[y][x-1] = Yspring_kill_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xspring_w;
  	  play_element_sound(x, y, SAMPLE_slurp, Xalien);
  	  score += lev.slurp_score;
  	  goto loop;

  	case Xbumper:
  	case XbumperB:
  	  Cave[y][x-1] = XbumperB;
  	  Next[y][x] = Xspring_e;
  	  play_element_sound(x, y, SAMPLE_spring, Xspring);
  	  goto loop;

  	default:
  	  Cave[y][x] = Xspring;
  	  Next[y][x] = Xspring;
  	  play_element_sound(x, y, SAMPLE_spring, Xspring);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xspring_fall:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yspring_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Zplayer:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Yspring_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xspring_fall;
  	  goto loop;

  	case Xbomb:
  	case Xbomb_pause:
  	  Cave[y+1][x] = Ybomb_eat;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xblank;
  	  Boom[y][x] = Xblank;
  	  Boom[y][x+1] = Xblank;
  	  Boom[y+1][x-1] = Xblank;
  	  Boom[y+1][x] = Xblank;
  	  Boom[y+1][x+1] = Xblank;
  	  Boom[y+2][x-1] = Xblank;
  	  Boom[y+2][x] = Xblank;
  	  Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  goto loop;

  	case Xbug_n:
  	case Xbug_e:
  	case Xbug_s:
  	case Xbug_w:
  	case Xbug_gon:
  	case Xbug_goe:
  	case Xbug_gos:
  	case Xbug_gow:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Ybug_spring;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xemerald;
  	  Boom[y][x] = Xemerald;
  	  Boom[y][x+1] = Xemerald;
  	  Boom[y+1][x-1] = Xemerald;
  	  Boom[y+1][x] = Xdiamond;
  	  Boom[y+1][x+1] = Xemerald;
  	  Boom[y+2][x-1] = Xemerald;
  	  Boom[y+2][x] = Xemerald;
  	  Boom[y+2][x+1] = Xemerald;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  score += lev.bug_score;
  	  goto loop;

  	case Xtank_n:
  	case Xtank_e:
  	case Xtank_s:
  	case Xtank_w:
  	case Xtank_gon:
  	case Xtank_goe:
  	case Xtank_gos:
  	case Xtank_gow:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Ytank_spring;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xblank;
  	  Boom[y][x] = Xblank;
  	  Boom[y][x+1] = Xblank;
  	  Boom[y+1][x-1] = Xblank;
  	  Boom[y+1][x] = Xblank;
  	  Boom[y+1][x+1] = Xblank;
  	  Boom[y+2][x-1] = Xblank;
  	  Boom[y+2][x] = Xblank;
  	  Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  score += lev.tank_score;
  	  goto loop;

  	case Xeater_n:
  	case Xeater_e:
  	case Xeater_s:
  	case Xeater_w:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Yeater_spring;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = lev.eater_array[lev.eater_pos][0];
  	  Boom[y][x] = lev.eater_array[lev.eater_pos][1];
  	  Boom[y][x+1] = lev.eater_array[lev.eater_pos][2];
  	  Boom[y+1][x-1] = lev.eater_array[lev.eater_pos][3];
  	  Boom[y+1][x] = lev.eater_array[lev.eater_pos][4];
  	  Boom[y+1][x+1] = lev.eater_array[lev.eater_pos][5];
  	  Boom[y+2][x-1] = lev.eater_array[lev.eater_pos][6];
  	  Boom[y+2][x] = lev.eater_array[lev.eater_pos][7];
  	  Boom[y+2][x+1] = lev.eater_array[lev.eater_pos][8];
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  lev.eater_pos = (lev.eater_pos + 1) & 7;
  	  score += lev.eater_score;
  	  goto loop;

  	case Xalien:
  	case Xalien_pause:
  	  Cave[y][x] = Yspring_sB;
  	  Cave[y+1][x] = Yalien_spring;
  	  Next[y+1][x] = Znormal;
  	  Boom[y][x-1] = Xblank;
  	  Boom[y][x] = Xblank;
  	  Boom[y][x+1] = Xblank;
  	  Boom[y+1][x-1] = Xblank;
  	  Boom[y+1][x] = Xblank;
  	  Boom[y+1][x+1] = Xblank;
  	  Boom[y+2][x-1] = Xblank;
  	  Boom[y+2][x] = Xblank;
  	  Boom[y+2][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  score += lev.alien_score;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xspring;
  	  Next[y][x] = Xspring;
  	  play_element_sound(x, y, SAMPLE_spring, Xspring);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xeater_n:
      if (Cave[y][x+1] == Xdiamond)
      {
	Cave[y][x+1] = Ydiamond_eat;
	Next[y][x+1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y+1][x] == Xdiamond)
      {
	Cave[y+1][x] = Ydiamond_eat;
	Next[y+1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y][x-1] == Xdiamond)
      {
	Cave[y][x-1] = Ydiamond_eat;
	Next[y][x-1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y-1][x] == Xdiamond)
      {
	Cave[y-1][x] = Ydiamond_eat;
	Next[y-1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      switch (Cave[y-1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yeater_nB;
  	  if (Cave[y-2][x+1] == Xblank)
	    Cave[y-2][x+1] = Yacid_splash_eB;
  	  if (Cave[y-2][x-1] == Xblank)
	    Cave[y-2][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Yeater_nB;
  	  Cave[y-1][x] = Yeater_n;
  	  Next[y][x] = Xblank;
  	  Next[y-1][x] = Xeater_n;
  	  goto loop;

  	default:
  	  Next[y][x] = RANDOM & 1 ? Xeater_e : Xeater_w;
  	  play_element_sound(x, y, SAMPLE_eater, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xeater_e:
      if (Cave[y+1][x] == Xdiamond)
      {
	Cave[y+1][x] = Ydiamond_eat;
	Next[y+1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y][x-1] == Xdiamond)
      {
	Cave[y][x-1] = Ydiamond_eat;
	Next[y][x-1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y-1][x] == Xdiamond)
      {
	Cave[y-1][x] = Ydiamond_eat;
	Next[y-1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y][x+1] == Xdiamond)
      {
	Cave[y][x+1] = Ydiamond_eat;
	Next[y][x+1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      switch (Cave[y][x+1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yeater_eB;
  	  if (Cave[y-1][x+2] == Xblank)
	    Cave[y-1][x+2] = Yacid_splash_eB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Yeater_eB;
  	  Cave[y][x+1] = Yeater_e;
  	  Next[y][x] = Xblank;
  	  Next[y][x+1] = Xeater_e;
  	  goto loop;

  	default:
  	  Next[y][x] = RANDOM & 1 ? Xeater_n : Xeater_s;
  	  play_element_sound(x, y, SAMPLE_eater, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xeater_s:
      if (Cave[y][x-1] == Xdiamond)
      {
	Cave[y][x-1] = Ydiamond_eat;
	Next[y][x-1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y-1][x] == Xdiamond)
      {
	Cave[y-1][x] = Ydiamond_eat;
	Next[y-1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y][x+1] == Xdiamond)
      {
	Cave[y][x+1] = Ydiamond_eat;
	Next[y][x+1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y+1][x] == Xdiamond)
      {
	Cave[y+1][x] = Ydiamond_eat;
	Next[y+1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yeater_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Yeater_sB;
  	  Cave[y+1][x] = Yeater_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xeater_s;
  	  goto loop;

  	default:
  	  Next[y][x] = RANDOM & 1 ? Xeater_e : Xeater_w;
  	  play_element_sound(x, y, SAMPLE_eater, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xeater_w:
      if (Cave[y-1][x] == Xdiamond)
      {
	Cave[y-1][x] = Ydiamond_eat;
	Next[y-1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y][x+1] == Xdiamond)
      {
	Cave[y][x+1] = Ydiamond_eat;
	Next[y][x+1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y+1][x] == Xdiamond)
      {
	Cave[y+1][x] = Ydiamond_eat;
	Next[y+1][x] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      if (Cave[y][x-1] == Xdiamond)
      {
	Cave[y][x-1] = Ydiamond_eat;
	Next[y][x-1] = Xblank;
	play_element_sound(x, y, SAMPLE_eater_eat, element);
	goto loop;
      }

      switch (Cave[y][x-1])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yeater_wB;
  	  if (Cave[y-1][x] == Xblank)
	    Cave[y-1][x] = Yacid_splash_eB;
  	  if (Cave[y-1][x-2] == Xblank)
	    Cave[y-1][x-2] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Yeater_wB;
  	  Cave[y][x-1] = Yeater_w;
  	  Next[y][x] = Xblank;
  	  Next[y][x-1] = Xeater_w;
  	  goto loop;

  	default:
  	  Next[y][x] = RANDOM & 1 ? Xeater_n : Xeater_s;
  	  play_element_sound(x, y, SAMPLE_eater, element);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xalien:

      if (lev.wheel_cnt)
      {
	dx = lev.wheel_x;
	dy = lev.wheel_y;
      }
      else
      {
	set_nearest_player_xy(x, y, &dx, &dy);
      }

      if (RANDOM & 1)
      {
	if (y > dy)
	{
	  switch (Cave[y-1][x])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yalien_nB;
  	      if (Cave[y-2][x+1] == Xblank)
		Cave[y-2][x+1] = Yacid_splash_eB;
  	      if (Cave[y-2][x-1] == Xblank)
		Cave[y-2][x-1] = Yacid_splash_wB;
  	      Next[y][x] = Xblank;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	    case Xplant:
  	    case Yplant:
  	    case Zplayer:
  	      Cave[y][x] = Yalien_nB;
  	      Cave[y-1][x] = Yalien_n;
  	      Next[y][x] = Xblank;
  	      Next[y-1][x] = Xalien_pause;
  	      play_element_sound(x, y, SAMPLE_alien, Xalien);
  	      goto loop;
	  }
	}
	else if (y < dy)
	{
	  switch (Cave[y+1][x])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yalien_sB;
  	      Next[y][x] = Xblank;
  	      if (Cave[y][x+1] == Xblank)
		Cave[y][x+1] = Yacid_splash_eB;
  	      if (Cave[y][x-1] == Xblank)
		Cave[y][x-1] = Yacid_splash_wB;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	    case Xplant:
  	    case Yplant:
  	    case Zplayer:
  	      Cave[y][x] = Yalien_sB;
  	      Cave[y+1][x] = Yalien_s;
  	      Next[y][x] = Xblank;
  	      Next[y+1][x] = Xalien_pause;
  	      play_element_sound(x, y, SAMPLE_alien, Xalien);
  	      goto loop;
	  }
	}
      }
      else
      {
	if (x < dx)
	{
	  switch (Cave[y][x+1])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yalien_eB;
  	      if (Cave[y-1][x+2] == Xblank)
		Cave[y-1][x+2] = Yacid_splash_eB;
  	      if (Cave[y-1][x] == Xblank)
		Cave[y-1][x] = Yacid_splash_wB;
  	      Next[y][x] = Xblank;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	    case Xplant:
  	    case Yplant:
  	    case Zplayer:
  	      Cave[y][x] = Yalien_eB;
  	      Cave[y][x+1] = Yalien_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xalien_pause;
  	      play_element_sound(x, y, SAMPLE_alien, Xalien);
  	      goto loop;
	  }
	}
	else if (x > dx)
	{
	  switch (Cave[y][x-1])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yalien_wB;
  	      if (Cave[y-1][x] == Xblank)
		Cave[y-1][x] = Yacid_splash_eB;
  	      if (Cave[y-1][x-2] == Xblank)
		Cave[y-1][x-2] = Yacid_splash_wB;
  	      Next[y][x] = Xblank;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	    case Xplant:
  	    case Yplant:
  	    case Zplayer:
  	      Cave[y][x] = Yalien_wB;
  	      Cave[y][x-1] = Yalien_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xalien_pause;
  	      play_element_sound(x, y, SAMPLE_alien, Xalien);
  	      goto loop;
	  }
	}
      }

      goto loop;

    case Xalien_pause:
      Next[y][x] = Xalien;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xemerald:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yemerald_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yemerald_sB;
  	  Cave[y+1][x] = Yemerald_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xemerald_fall;
  	  goto loop;

  	case Xspring:
  	case Xspring_pause:
  	case Xspring_e:
  	case Xspring_w:
  	case Xandroid:
  	case Xandroid_1_n:
  	case Xandroid_2_n:
  	case Xandroid_1_e:
  	case Xandroid_2_e:
  	case Xandroid_1_s:
  	case Xandroid_2_s:
  	case Xandroid_1_w:
  	case Xandroid_2_w:
  	case Xstone:
  	case Xstone_pause:
  	case Xemerald:
  	case Xemerald_pause:
  	case Xdiamond:
  	case Xdiamond_pause:
  	case Xbomb:
  	case Xbomb_pause:
  	case Xballoon:
  	case Xacid_ne:
  	case Xacid_nw:
  	case Xball_1:
  	case Xball_2:
  	case Xnut:
  	case Xnut_pause:
  	case Xgrow_ns:
  	case Xgrow_ew:
  	case Xwonderwall:
  	case Xkey_1:
  	case Xkey_2:
  	case Xkey_3:
  	case Xkey_4:
  	case Xkey_5:
  	case Xkey_6:
  	case Xkey_7:
  	case Xkey_8:
  	case Xbumper:
  	case Xswitch:
  	case Xsteel_1:
  	case Xsteel_2:
  	case Xsteel_3:
  	case Xsteel_4:
  	case Xwall_1:
  	case Xwall_2:
  	case Xwall_3:
  	case Xwall_4:
  	case Xround_wall_1:
  	case Xround_wall_2:
  	case Xround_wall_3:
  	case Xround_wall_4:
  	  if (RANDOM & 1)
	  {
  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Yemerald_eB;
  	      Cave[y][x+1] = Yemerald_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xemerald_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Yemerald_wB;
  	      Cave[y][x-1] = Yemerald_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xemerald_pause;
  	      goto loop;
  	    }
  	  }
	  else
	  {
  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Yemerald_wB;
  	      Cave[y][x-1] = Yemerald_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xemerald_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Yemerald_eB;
  	      Cave[y][x+1] = Yemerald_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xemerald_pause;
  	      goto loop;
  	    }
  	  }

        default:
  	  if (++lev.shine_cnt > 50)
	  {
  	    lev.shine_cnt = RANDOM & 7;
  	    Cave[y][x] = Xemerald_shine;
  	  }

  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xemerald_pause:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yemerald_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Yemerald_sB;
  	  Cave[y+1][x] = Yemerald_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xemerald_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xemerald;
  	  Next[y][x] = Xemerald;
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xemerald_fall:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Yemerald_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Zplayer:
  	  Cave[y][x] = Yemerald_sB;
  	  Cave[y+1][x] = Yemerald_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xemerald_fall;
  	  goto loop;

  	case Xwonderwall:
  	  if (lev.wonderwall_time)
	  {
  	    lev.wonderwall_state = 1;
  	    Cave[y][x] = Yemerald_sB;
  	    if (tab_blank[Cave[y+2][x]])
	    {
  	      Cave[y+2][x] = Ydiamond_s;
  	      Next[y+2][x] = Xdiamond_fall;
  	    }

  	    Next[y][x] = Xblank;
  	    play_element_sound(x, y, SAMPLE_wonderfall, Xwonderwall);
  	    goto loop;
  	  }

  	default:
  	  Cave[y][x] = Xemerald;
  	  Next[y][x] = Xemerald;
  	  play_element_sound(x, y, SAMPLE_diamond, Xemerald);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xdiamond:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ydiamond_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Ydiamond_sB;
  	  Cave[y+1][x] = Ydiamond_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xdiamond_fall;
  	  goto loop;

  	case Xspring:
  	case Xspring_pause:
  	case Xspring_e:
  	case Xspring_w:
  	case Xandroid:
  	case Xandroid_1_n:
  	case Xandroid_2_n:
  	case Xandroid_1_e:
  	case Xandroid_2_e:
  	case Xandroid_1_s:
  	case Xandroid_2_s:
  	case Xandroid_1_w:
  	case Xandroid_2_w:
  	case Xstone:
  	case Xstone_pause:
  	case Xemerald:
  	case Xemerald_pause:
  	case Xdiamond:
  	case Xdiamond_pause:
  	case Xbomb:
  	case Xbomb_pause:
  	case Xballoon:
  	case Xacid_ne:
  	case Xacid_nw:
  	case Xball_1:
  	case Xball_2:
  	case Xnut:
  	case Xnut_pause:
  	case Xgrow_ns:
  	case Xgrow_ew:
  	case Xwonderwall:
  	case Xkey_1:
  	case Xkey_2:
  	case Xkey_3:
  	case Xkey_4:
  	case Xkey_5:
  	case Xkey_6:
  	case Xkey_7:
  	case Xkey_8:
  	case Xbumper:
  	case Xswitch:
  	case Xsteel_1:
  	case Xsteel_2:
  	case Xsteel_3:
  	case Xsteel_4:
  	case Xwall_1:
  	case Xwall_2:
  	case Xwall_3:
  	case Xwall_4:
  	case Xround_wall_1:
  	case Xround_wall_2:
  	case Xround_wall_3:
  	case Xround_wall_4:
  	  if (RANDOM & 1)
	  {
  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ydiamond_eB;
  	      Cave[y][x+1] = Ydiamond_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xdiamond_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ydiamond_wB;
  	      Cave[y][x-1] = Ydiamond_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xdiamond_pause;
  	      goto loop;
  	    }
  	  }
	  else
	  {
  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ydiamond_wB;
  	      Cave[y][x-1] = Ydiamond_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xdiamond_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ydiamond_eB;
  	      Cave[y][x+1] = Ydiamond_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xdiamond_pause;
  	      goto loop;
  	    }
  	  }

  	default:
  	  if (++lev.shine_cnt > 50)
	  {
  	    lev.shine_cnt = RANDOM & 7;
  	    Cave[y][x] = Xdiamond_shine;
  	  }

  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xdiamond_pause:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ydiamond_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Ydiamond_sB;
  	  Cave[y+1][x] = Ydiamond_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xdiamond_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xdiamond;
  	  Next[y][x] = Xdiamond;
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xdiamond_fall:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ydiamond_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Zplayer:
  	  Cave[y][x] = Ydiamond_sB;
  	  Cave[y+1][x] = Ydiamond_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xdiamond_fall;
  	  goto loop;

  	case Xwonderwall:
  	  if (lev.wonderwall_time)
	  {
  	    lev.wonderwall_state = 1;
  	    Cave[y][x] = Ydiamond_sB;
  	    if (tab_blank[Cave[y+2][x]])
	    {
  	      Cave[y+2][x] = Ystone_s;
  	      Next[y+2][x] = Xstone_fall;
  	    }

  	    Next[y][x] = Xblank;
  	    play_element_sound(x, y, SAMPLE_wonderfall, Xwonderwall);
  	    goto loop;
  	  }

  	default:
  	  Cave[y][x] = Xdiamond;
  	  Next[y][x] = Xdiamond;
  	  play_element_sound(x, y, SAMPLE_diamond, Xdiamond);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xdrip_fall:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ydrip_s1B;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xdrip_stretchB;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	case Xplant:
  	case Yplant:
  	case Zplayer:
  	  Cave[y][x] = Ydrip_s1B;
  	  Cave[y+1][x] = Ydrip_s1;
  	  Next[y][x] = Xdrip_stretchB;
  	  Next[y+1][x] = Xdrip_stretch;
  	  goto loop;

  	default:
  	  switch (RANDOM & 7)
	  {
  	    case 0:
	      temp = Xamoeba_1;
	      break;

  	    case 1:
	      temp = Xamoeba_2;
	      break;

  	    case 2:
	      temp = Xamoeba_3;
	      break;

  	    case 3:
	      temp = Xamoeba_4;
	      break;

  	    case 4:
	      temp = Xamoeba_5;
	      break;

  	    case 5:
	      temp = Xamoeba_6;
	      break;

  	    case 6:
	      temp = Xamoeba_7;
	      break;

  	    case 7:
	      temp = Xamoeba_8;
	      break;
  	  }

  	  Cave[y][x] = temp;
  	  Next[y][x] = temp;
  	  play_element_sound(x, y, SAMPLE_drip, Xdrip_fall);
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xdrip_stretch:
      Cave[y][x] = Ydrip_s2;
      Next[y][x] = Xdrip_fall;
      goto loop;

    case Xdrip_stretchB:
      Cave[y][x] = Ydrip_s2B;
      Next[y][x] = Xblank;
      goto loop;

    case Xdrip_eat:
      Next[y][x] = Xdrip_fall;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xbomb:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ybomb_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Ybomb_sB;
  	  Cave[y+1][x] = Ybomb_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xbomb_fall;
  	  goto loop;

  	case Xspring:
  	case Xspring_pause:
  	case Xspring_e:
  	case Xspring_w:
  	case Xandroid:
  	case Xandroid_1_n:
  	case Xandroid_2_n:
  	case Xandroid_1_e:
  	case Xandroid_2_e:
  	case Xandroid_1_s:
  	case Xandroid_2_s:
  	case Xandroid_1_w:
  	case Xandroid_2_w:
  	case Xstone:
  	case Xstone_pause:
  	case Xemerald:
  	case Xemerald_pause:
  	case Xdiamond:
  	case Xdiamond_pause:
  	case Xbomb:
  	case Xbomb_pause:
  	case Xballoon:
  	case Xacid_ne:
  	case Xacid_nw:
  	case Xball_1:
  	case Xball_2:
  	case Xnut:
  	case Xnut_pause:
  	case Xgrow_ns:
  	case Xgrow_ew:
  	case Xkey_1:
  	case Xkey_2:
  	case Xkey_3:
  	case Xkey_4:
  	case Xkey_5:
  	case Xkey_6:
  	case Xkey_7:
  	case Xkey_8:
  	case Xbumper:
  	case Xswitch:
  	case Xround_wall_1:
  	case Xround_wall_2:
  	case Xround_wall_3:
  	case Xround_wall_4:
  	  if (RANDOM & 1)
	  {
  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ybomb_eB;
  	      Cave[y][x+1] = Ybomb_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xbomb_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ybomb_wB;
  	      Cave[y][x-1] = Ybomb_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xbomb_pause;
  	      goto loop;
  	    }
  	  }
	  else
	  {
  	    if (tab_blank[Cave[y][x-1]] && tab_acid[Cave[y+1][x-1]])
	    {
  	      Cave[y][x] = Ybomb_wB;
  	      Cave[y][x-1] = Ybomb_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xbomb_pause;
  	      goto loop;
  	    }

  	    if (tab_blank[Cave[y][x+1]] && tab_acid[Cave[y+1][x+1]])
	    {
  	      Cave[y][x] = Ybomb_eB;
  	      Cave[y][x+1] = Ybomb_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xbomb_pause;
  	      goto loop;
  	    }
  	  }

  	default:
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xbomb_pause:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ybomb_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Ybomb_sB;
  	  Cave[y+1][x] = Ybomb_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xbomb_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Xbomb;
  	  Next[y][x] = Xbomb;
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xbomb_fall:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Ybomb_sB;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xblank;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Ybomb_sB;
  	  Cave[y+1][x] = Ybomb_s;
  	  Next[y][x] = Xblank;
  	  Next[y+1][x] = Xbomb_fall;
  	  goto loop;

  	default:
  	  Cave[y][x] = Ybomb_eat;
  	  Next[y][x] = Znormal;
  	  Boom[y-1][x-1] = Xblank;
  	  Boom[y-1][x] = Xblank;
  	  Boom[y-1][x+1] = Xblank;
  	  Boom[y][x-1] = Xblank;
  	  Boom[y][x] = Xblank;
  	  Boom[y][x+1] = Xblank;
  	  Boom[y+1][x-1] = Xblank;
  	  Boom[y+1][x] = Xblank;
  	  Boom[y+1][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
	  play_element_sound(x, y, SAMPLE_boom, element);
#endif
  	  goto loop;
      }

    /* --------------------------------------------------------------------- */

    case Xballoon:
      if (lev.wind_cnt == 0)
	goto loop;

      switch (lev.wind_direction)
      {
  	case 0: /* north */
  	  switch (Cave[y-1][x])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yballoon_nB;
  	      if (Cave[y-2][x+1] == Xblank)
		Cave[y-2][x+1] = Yacid_splash_eB;
  	      if (Cave[y-2][x-1] == Xblank)
		Cave[y-2][x-1] = Yacid_splash_wB;
  	      Next[y][x] = Xblank;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	      Cave[y][x] = Yballoon_nB;
  	      Cave[y-1][x] = Yballoon_n;
  	      Next[y][x] = Xblank;
  	      Next[y-1][x] = Xballoon;
  	      goto loop;

  	    default:
  	      goto loop;
  	  }

        case 1: /* east */
	  switch (Cave[y][x+1])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yballoon_eB;
  	      if (Cave[y-1][x+2] == Xblank)
		Cave[y-1][x+2] = Yacid_splash_eB;
  	      if (Cave[y-1][x] == Xblank)
		Cave[y-1][x] = Yacid_splash_wB;
  	      Next[y][x] = Xblank;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	      Cave[y][x] = Yballoon_eB;
  	      Cave[y][x+1] = Yballoon_e;
  	      Next[y][x] = Xblank;
  	      Next[y][x+1] = Xballoon;
  	      goto loop;

  	    default:
  	      goto loop;
	  }

        case 2: /* south */
	  switch (Cave[y+1][x])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yballoon_sB;
  	      if (Cave[y][x+1] == Xblank)
		Cave[y][x+1] = Yacid_splash_eB;
  	      if (Cave[y][x-1] == Xblank)
		Cave[y][x-1] = Yacid_splash_wB;
  	      Next[y][x] = Xblank;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	      Cave[y][x] = Yballoon_sB;
  	      Cave[y+1][x] = Yballoon_s;
  	      Next[y][x] = Xblank;
  	      Next[y+1][x] = Xballoon;
  	      goto loop;

  	    default:
  	      goto loop;
	  }

        case 3: /* west */
	  switch (Cave[y][x-1])
	  {
  	    case Xacid_1:
  	    case Xacid_2:
  	    case Xacid_3:
  	    case Xacid_4:
  	    case Xacid_5:
  	    case Xacid_6:
  	    case Xacid_7:
  	    case Xacid_8:
  	      Cave[y][x] = Yballoon_wB;
  	      if (Cave[y-1][x] == Xblank)
		Cave[y-1][x] = Yacid_splash_eB;
  	      if (Cave[y-1][x-2] == Xblank)
		Cave[y-1][x-2] = Yacid_splash_wB;
  	      Next[y][x] = Xblank;
  	      play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	      goto loop;

  	    case Xblank:
  	    case Yacid_splash_eB:
  	    case Yacid_splash_wB:
  	      Cave[y][x] = Yballoon_wB;
  	      Cave[y][x-1] = Yballoon_w;
  	      Next[y][x] = Xblank;
  	      Next[y][x-1] = Xballoon;
  	      goto loop;

  	    default:
  	      goto loop;
	  }
      }

    /* --------------------------------------------------------------------- */

    case Xacid_1:
      Next[y][x] = Xacid_2;
      goto loop;

    case Xacid_2:
      Next[y][x] = Xacid_3;
      goto loop;

    case Xacid_3:
      Next[y][x] = Xacid_4;
      goto loop;

    case Xacid_4:
      Next[y][x] = Xacid_5;
      goto loop;

    case Xacid_5:
      Next[y][x] = Xacid_6;
      goto loop;

    case Xacid_6:
      Next[y][x] = Xacid_7;
      goto loop;

    case Xacid_7:
      Next[y][x] = Xacid_8;
      goto loop;

    case Xacid_8:
      Next[y][x] = Xacid_1;
      goto loop;

    case Xfake_acid_1:
      Next[y][x] = Xfake_acid_2;
      goto loop;

    case Xfake_acid_2:
      Next[y][x] = Xfake_acid_3;
      goto loop;

    case Xfake_acid_3:
      Next[y][x] = Xfake_acid_4;
      goto loop;

    case Xfake_acid_4:
      Next[y][x] = Xfake_acid_5;
      goto loop;

    case Xfake_acid_5:
      Next[y][x] = Xfake_acid_6;
      goto loop;

    case Xfake_acid_6:
      Next[y][x] = Xfake_acid_7;
      goto loop;

    case Xfake_acid_7:
      Next[y][x] = Xfake_acid_8;
      goto loop;

    case Xfake_acid_8:
      Next[y][x] = Xfake_acid_1;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xball_1:
      if (lev.ball_state == 0)
	goto loop;

      Cave[y][x] = Xball_1B;
      Next[y][x] = Xball_2;
      if (lev.ball_cnt)
	goto loop;

      goto ball_common;

    case Xball_2:
      if (lev.ball_state == 0)
	goto loop;

      Cave[y][x] = Xball_2B;
      Next[y][x] = Xball_1;
      if (lev.ball_cnt)
	goto loop;

      goto ball_common;

    ball_common:

      play_element_sound(x, y, SAMPLE_ball, element);
      if (lev.ball_random)
      {
	switch (RANDOM & 7)
	{
  	  case 0:
  	    if (lev.ball_array[lev.ball_pos][0] != Xblank &&
		tab_blank[Cave[y-1][x-1]])
	    {
  	      Cave[y-1][x-1] = Yball_eat;
  	      Next[y-1][x-1] = lev.ball_array[lev.ball_pos][0];
  	    }
  	    break;

  	  case 1:
  	    if (lev.ball_array[lev.ball_pos][1] != Xblank &&
		tab_blank[Cave[y-1][x]])
	    {
  	      Cave[y-1][x] = Yball_eat;
  	      Next[y-1][x] = lev.ball_array[lev.ball_pos][1];
  	    }
  	    break;

  	  case 2:
  	    if (lev.ball_array[lev.ball_pos][2] != Xblank &&
		tab_blank[Cave[y-1][x+1]])
	    {
  	      Cave[y-1][x+1] = Yball_eat;
  	      Next[y-1][x+1] = lev.ball_array[lev.ball_pos][2];
  	    }
  	    break;

  	  case 3:
  	    if (lev.ball_array[lev.ball_pos][3] != Xblank &&
		tab_blank[Cave[y][x-1]])
	    {
  	      Cave[y][x-1] = Yball_eat;
  	      Next[y][x-1] = lev.ball_array[lev.ball_pos][3];
  	    }
  	    break;

  	  case 4:
  	    if (lev.ball_array[lev.ball_pos][4] != Xblank &&
		tab_blank[Cave[y][x+1]])
	    {
  	      Cave[y][x+1] = Yball_eat;
  	      Next[y][x+1] = lev.ball_array[lev.ball_pos][4];
  	    }
  	    break;

  	  case 5:
  	    if (lev.ball_array[lev.ball_pos][5] != Xblank &&
		tab_blank[Cave[y+1][x-1]])
	    {
  	      Cave[y+1][x-1] = Yball_eat;
  	      Next[y+1][x-1] = lev.ball_array[lev.ball_pos][5];
  	    }
  	    break;

  	  case 6:
  	    if (lev.ball_array[lev.ball_pos][6] != Xblank &&
		tab_blank[Cave[y+1][x]])
	    {
  	      Cave[y+1][x] = Yball_eat;
  	      Next[y+1][x] = lev.ball_array[lev.ball_pos][6];
  	    }
  	    break;

  	  case 7:
  	    if (lev.ball_array[lev.ball_pos][7] != Xblank &&
		tab_blank[Cave[y+1][x+1]])
	    {
  	      Cave[y+1][x+1] = Yball_eat;
  	      Next[y+1][x+1] = lev.ball_array[lev.ball_pos][7];
  	    }
  	    break;
	}
      }
      else
      {
	if (lev.ball_array[lev.ball_pos][0] != Xblank &&
	    tab_blank[Cave[y-1][x-1]])
	{
	  Cave[y-1][x-1] = Yball_eat;
	  Next[y-1][x-1] = lev.ball_array[lev.ball_pos][0];
	}

	if (lev.ball_array[lev.ball_pos][1] != Xblank &&
	    tab_blank[Cave[y-1][x]])
	{
	  Cave[y-1][x] = Yball_eat;
	  Next[y-1][x] = lev.ball_array[lev.ball_pos][1];
	}

	if (lev.ball_array[lev.ball_pos][2] != Xblank &&
	    tab_blank[Cave[y-1][x+1]])
	{
	  Cave[y-1][x+1] = Yball_eat;
	  Next[y-1][x+1] = lev.ball_array[lev.ball_pos][2];
	}

	if (lev.ball_array[lev.ball_pos][3] != Xblank &&
	    tab_blank[Cave[y][x-1]])
	{
	  Cave[y][x-1] = Yball_eat;
	  Next[y][x-1] = lev.ball_array[lev.ball_pos][3];
	}

	if (lev.ball_array[lev.ball_pos][4] != Xblank &&
	    tab_blank[Cave[y][x+1]])
	{
	  Cave[y][x+1] = Yball_eat;
	  Next[y][x+1] = lev.ball_array[lev.ball_pos][4];
	}

	if (lev.ball_array[lev.ball_pos][5] != Xblank &&
	    tab_blank[Cave[y+1][x-1]])
	{
	  Cave[y+1][x-1] = Yball_eat;
	  Next[y+1][x-1] = lev.ball_array[lev.ball_pos][5];
	}

	if (lev.ball_array[lev.ball_pos][6] != Xblank &&
	    tab_blank[Cave[y+1][x]])
	{
	  Cave[y+1][x] = Yball_eat;
	  Next[y+1][x] = lev.ball_array[lev.ball_pos][6];
	}

	if (lev.ball_array[lev.ball_pos][7] != Xblank &&
	    tab_blank[Cave[y+1][x+1]])
	{
	  Cave[y+1][x+1] = Yball_eat;
	  Next[y+1][x+1] = lev.ball_array[lev.ball_pos][7];
	}
      }

      lev.ball_pos = (lev.ball_pos + 1) % lev.num_ball_arrays;

      goto loop;

    /* --------------------------------------------------------------------- */

    case Xgrow_ns:
      if (tab_blank[Cave[y-1][x]])
      {
	Cave[y-1][x] = Ygrow_ns_eat;
	Next[y-1][x] = Xgrow_ns;
	play_element_sound(x, y, SAMPLE_grow, Xgrow_ns);
      }

      if (tab_blank[Cave[y+1][x]])
      {
	Cave[y+1][x] = Ygrow_ns_eat;
	Next[y+1][x] = Xgrow_ns;
	play_element_sound(x, y, SAMPLE_grow, Xgrow_ns);
      }

      goto loop;

    case Xgrow_ew:
      if (tab_blank[Cave[y][x+1]])
      {
	Cave[y][x+1] = Ygrow_ew_eat;
	Next[y][x+1] = Xgrow_ew;
	play_element_sound(x, y, SAMPLE_grow, Xgrow_ew);
      }

      if (tab_blank[Cave[y][x-1]])
      {
	Cave[y][x-1] = Ygrow_ew_eat;
	Next[y][x-1] = Xgrow_ew;
	play_element_sound(x, y, SAMPLE_grow, Xgrow_ew);
      }

      goto loop;

    /* --------------------------------------------------------------------- */

    case Xwonderwall:
      if (lev.wonderwall_time && lev.wonderwall_state)
      {
	Cave[y][x] = XwonderwallB;
	play_element_sound(x, y, SAMPLE_wonder, Xwonderwall);
      }

      goto loop;

    /* --------------------------------------------------------------------- */

    case Xexit:
      if (lev.required > 0)
	goto loop;

      temp = RANDOM & 63;
      if (temp < 21)
      {
	Cave[y][x] = Xexit_1;
	Next[y][x] = Xexit_2;
      }
      else if (temp < 42)
      {
	Cave[y][x] = Xexit_2;
	Next[y][x] = Xexit_3;
      }
      else
      {
	Cave[y][x] = Xexit_3;
	Next[y][x] = Xexit_1;
      }

      play_element_sound(x, y, SAMPLE_exit_open, Xexit);

      goto loop;

    case Xexit_1:
      Next[y][x] = Xexit_2;
      goto loop;

    case Xexit_2:
      Next[y][x] = Xexit_3;
      goto loop;

    case Xexit_3:
      Next[y][x] = Xexit_1;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xdynamite_1:
      play_element_sound(x, y, SAMPLE_tick, Xdynamite_1);
      Next[y][x] = Xdynamite_2;
      goto loop;

    case Xdynamite_2:
      play_element_sound(x, y, SAMPLE_tick, Xdynamite_2);
      Next[y][x] = Xdynamite_3;
      goto loop;

    case Xdynamite_3:
      play_element_sound(x, y, SAMPLE_tick, Xdynamite_3);
      Next[y][x] = Xdynamite_4;
      goto loop;

    case Xdynamite_4:
      play_element_sound(x, y, SAMPLE_tick, Xdynamite_4);
      Next[y][x] = Zdynamite;
      Boom[y-1][x-1] = Xblank;
      Boom[y-1][x] = Xblank;
      Boom[y-1][x+1] = Xblank;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xwheel:
      if (lev.wheel_cnt && x == lev.wheel_x && y == lev.wheel_y)
	Cave[y][x] = XwheelB;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xswitch:
      if (lev.ball_state)
	Cave[y][x] = XswitchB;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xsand_stone:
      switch (Cave[y+1][x])
      {
  	case Xacid_1:
  	case Xacid_2:
  	case Xacid_3:
  	case Xacid_4:
  	case Xacid_5:
  	case Xacid_6:
  	case Xacid_7:
  	case Xacid_8:
  	  Cave[y][x] = Xsand_stonesand_quickout_1;
  	  if (Cave[y][x+1] == Xblank)
	    Cave[y][x+1] = Yacid_splash_eB;
  	  if (Cave[y][x-1] == Xblank)
	    Cave[y][x-1] = Yacid_splash_wB;
  	  Next[y][x] = Xsand_stonesand_quickout_2;
  	  play_element_sound(x, y, SAMPLE_acid, Xacid_1);
  	  goto loop;

  	case Xblank:
  	case Yacid_splash_eB:
  	case Yacid_splash_wB:
  	  Cave[y][x] = Xsand_stonesand_quickout_1;
  	  Cave[y+1][x] = Xsand_stoneout_1;
  	  Next[y][x] = Xsand_stonesand_quickout_2;
  	  Next[y+1][x] = Xsand_stoneout_2;
  	  goto loop;

  	case Xsand:
  	  Cave[y][x] = Xsand_stonesand_1;
  	  Cave[y+1][x] = Xsand_sandstone_1;
  	  Next[y][x] = Xsand_stonesand_2;
  	  Next[y+1][x] = Xsand_sandstone_2;
  	  goto loop;

  	default:
  	  goto loop;
      }

    case Xsand_stonein_1:
      Next[y][x] = Xsand_stonein_2;
      goto loop;

    case Xsand_stonein_2:
      Next[y][x] = Xsand_stonein_3;
      goto loop;

    case Xsand_stonein_3:
      Next[y][x] = Xsand_stonein_4;
      goto loop;

    case Xsand_stonein_4:
      Next[y][x] = Xblank;
      goto loop;

    case Xsand_stonesand_1:
      Next[y][x] = Xsand_stonesand_2;
      goto loop;

    case Xsand_stonesand_2:
      Next[y][x] = Xsand_stonesand_3;
      goto loop;

    case Xsand_stonesand_3:
      Next[y][x] = Xsand_stonesand_4;
      goto loop;

    case Xsand_stonesand_4:
      Next[y][x] = Xsand;
      goto loop;

#if 1
    case Xsand_stonesand_quickout_1:
      Next[y][x] = Xsand_stonesand_quickout_2;
      goto loop;

    case Xsand_stonesand_quickout_2:
      Next[y][x] = Xsand;
      goto loop;
#endif

    case Xsand_stoneout_1:
      Next[y][x] = Xsand_stoneout_2;
      goto loop;

    case Xsand_stoneout_2:
      Next[y][x] = Xstone_fall;
      goto loop;

    case Xsand_sandstone_1:
      Next[y][x] = Xsand_sandstone_2;
      goto loop;

    case Xsand_sandstone_2:
      Next[y][x] = Xsand_sandstone_3;
      goto loop;

    case Xsand_sandstone_3:
      Next[y][x] = Xsand_sandstone_4;
      goto loop;

    case Xsand_sandstone_4:
      Next[y][x] = Xsand_stone;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xdripper:
      if (lev.lenses_cnt)
	Cave[y][x] = XdripperB;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xfake_blank:
      if (lev.lenses_cnt)
	Cave[y][x] = Xfake_blankB;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xfake_grass:
      if (lev.magnify_cnt)
	Cave[y][x] = Xfake_grassB;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xfake_door_1:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_1;
      goto loop;

    case Xfake_door_2:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_2;
      goto loop;

    case Xfake_door_3:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_3;
      goto loop;

    case Xfake_door_4:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_4;
      goto loop;

    case Xfake_door_5:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_5;
      goto loop;

    case Xfake_door_6:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_6;
      goto loop;

    case Xfake_door_7:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_7;
      goto loop;

    case Xfake_door_8:
      if (lev.magnify_cnt)
	Cave[y][x] = Xdoor_8;
      goto loop;

    /* --------------------------------------------------------------------- */

    case Xboom_bug:
      bug_boom:
      Next[y][x] = Znormal;
      Boom[y-1][x-1] = Xemerald;
      Boom[y-1][x] = Xemerald;
      Boom[y-1][x+1] = Xemerald;
      Boom[y][x-1] = Xemerald;
      Boom[y][x] = Xdiamond;
      Boom[y][x+1] = Xemerald;
      Boom[y+1][x-1] = Xemerald;
      Boom[y+1][x] = Xemerald;
      Boom[y+1][x+1] = Xemerald;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SAMPLE_boom, element);
#endif
      goto loop;

    case Xboom_bomb:

    tank_boom:

      Next[y][x] = Znormal;
      Boom[y-1][x-1] = Xblank;
      Boom[y-1][x] = Xblank;
      Boom[y-1][x+1] = Xblank;
      Boom[y][x-1] = Xblank;
      Boom[y][x] = Xblank;
      Boom[y][x+1] = Xblank;
      Boom[y+1][x-1] = Xblank;
      Boom[y+1][x] = Xblank;
      Boom[y+1][x+1] = Xblank;
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SAMPLE_boom, element);
#endif
      goto loop;

    case Xboom_android:
#if PLAY_ELEMENT_SOUND
      play_element_sound(x, y, SAMPLE_boom, Xandroid);
#endif
    case Xboom_1:
      Next[y][x] = Xboom_2;
#if !PLAY_ELEMENT_SOUND
      if (x != lev.exit_x && y != lev.exit_y)
	play_sound(x, y, SAMPLE_boom);
      else
	lev.exit_x = lev.exit_y = -1;
#endif
      goto loop;

    case Xboom_2:
      Next[y][x] = Boom[y][x];
      goto loop;

    /* --------------------------------------------------------------------- */

    case ZBORDER:
      if (++y < HEIGHT - 1)
      {
	x = 0;
	cave_cache = Cave[y];
	goto loop;
      }

      goto done;
  }

#undef RANDOM
#undef PLAY
#undef PLAY_FORCE

 done:

  if (ply[0].alive || ply[1].alive || ply[2].alive || ply[3].alive)
    lev.score += score;		/* only add a score if someone is alive */

  RandomEM = random;

  {
    void *temp = Cave;

    /* triple buffering */
    Cave = Next;
    Next = Draw;
    Draw = temp;
  }
}
