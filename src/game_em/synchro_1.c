/* first part of synchro.
 *
 * game logic for players.
 *
 * large switch statement for tiles the player interacts with.
 */

#include "main_em.h"


#define USE_CHANGED_ACID_STUFF		1

extern boolean checkIfAllPlayersFitToScreen();

static void check_player(struct PLAYER *);
static void kill_player(struct PLAYER *);
static boolean player_digfield(struct PLAYER *, int, int);
static boolean player_killed(struct PLAYER *);

void synchro_1(void)
{
  int start_check_nr;
  int i;

  game_em.any_player_moving = FALSE;
  game_em.any_player_snapping = FALSE;

  /* must test for death and actually kill separately */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    boolean ply_kill = player_killed(&ply[i]);

    if (ply[i].alive && ply_kill)
      kill_player(&ply[i]);
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].oldx = ply[i].x;
    ply[i].oldy = ply[i].y;
    ply[i].anim = SPR_still;
  }

  start_check_nr = (RandomEM & 128 ? 0 : 1) * 2 + (RandomEM & 256 ? 0 : 1);

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    int check_nr = (start_check_nr + i) % MAX_PLAYERS;

    if (ply[check_nr].alive)
      check_player(&ply[check_nr]);
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (!ply[i].alive)
      continue;

    if (Cave[ply[i].oldy][ply[i].oldx] == Zplayer)
    {
      Cave[ply[i].oldy][ply[i].oldx] = Xblank;
      Next[ply[i].oldy][ply[i].oldx] = Xblank;
    }

    if (Cave[ply[i].y][ply[i].x] == Xblank)
    {
      Cave[ply[i].y][ply[i].x] = Zplayer;
      Next[ply[i].y][ply[i].x] = Zplayer;
    }
  }
}

static boolean player_killed(struct PLAYER *ply)
{
  int x = ply->x;
  int y = ply->y;

  if (!ply->alive)
    return FALSE;

  if (lev.killed_out_of_time && setup.time_limit)
    return TRUE;

  switch(Cave[y-1][x])
  {
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
      return TRUE;
  }

  switch(Cave[y][x+1])
  {
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
      return TRUE;
  }

  switch(Cave[y+1][x])
  {
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
      return TRUE;
  }

  switch(Cave[y][x-1])
  {
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
      return TRUE;
  }

  switch(Cave[y][x])
  {
    case Xblank:
    case Yacid_splash_eB:
    case Yacid_splash_wB:
    case Zplayer:
    case Xdynamite_1:
    case Xdynamite_2:
    case Xdynamite_3:
    case Xdynamite_4:
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
      return FALSE;
  }

  return TRUE;
}

static void kill_player(struct PLAYER *ply)
{
  int x = ply->x;
  int y = ply->y;

  ply->alive = 0;

  switch(Cave[y-1][x])
  {
    case Xbug_n:
    case Xbug_e:
    case Xbug_s:
    case Xbug_w:
    case Xbug_gon:
    case Xbug_goe:
    case Xbug_gos:
    case Xbug_gow:
      Cave[y-1][x] = Xboom_bug;
      break;

    case Xtank_n:
    case Xtank_e:
    case Xtank_s:
    case Xtank_w:
    case Xtank_gon:
    case Xtank_goe:
    case Xtank_gos:
    case Xtank_gow:
      Cave[y-1][x] = Xboom_bomb;
      break;
  }

  switch(Cave[y][x+1])
  {
    case Xbug_n:
    case Xbug_e:
    case Xbug_s:
    case Xbug_w:
    case Xbug_gon:
    case Xbug_goe:
    case Xbug_gos:
    case Xbug_gow:
      Cave[y][x+1] = Xboom_bug;
      break;

    case Xtank_n:
    case Xtank_e:
    case Xtank_s:
    case Xtank_w:
    case Xtank_gon:
    case Xtank_goe:
    case Xtank_gos:
    case Xtank_gow:
      Cave[y][x+1] = Xboom_bomb;
      break;
  }

  switch(Cave[y+1][x])
  {
    case Xbug_n:
    case Xbug_e:
    case Xbug_s:
    case Xbug_w:
    case Xbug_gon:
    case Xbug_goe:
    case Xbug_gos:
    case Xbug_gow:
      Cave[y+1][x] = Xboom_bug;
      break;

    case Xtank_n:
    case Xtank_e:
    case Xtank_s:
    case Xtank_w:
    case Xtank_gon:
    case Xtank_goe:
    case Xtank_gos:
    case Xtank_gow:
      Cave[y+1][x] = Xboom_bomb;
      break;
  }

  switch(Cave[y][x-1])
  {
    case Xbug_n:
    case Xbug_e:
    case Xbug_s:
    case Xbug_w:
    case Xbug_gon:
    case Xbug_goe:
    case Xbug_gos:
    case Xbug_gow:
      Cave[y][x-1] = Xboom_bug;
      break;

    case Xtank_n:
    case Xtank_e:
    case Xtank_s:
    case Xtank_w:
    case Xtank_gon:
    case Xtank_goe:
    case Xtank_gos:
    case Xtank_gow:
      Cave[y][x-1] = Xboom_bomb;
      break;
  }

  switch(Cave[y][x])
  {
    case Xexit_1:
    case Xexit_2:
    case Xexit_3:
      lev.exit_x = x;
      lev.exit_y = y;
      play_element_sound(x, y, SAMPLE_exit_leave, Xexit_1);
      break;

    default:
      play_element_sound(x, y, SAMPLE_die, Zplayer);
      break;
  }

  switch(Cave[y][x])
  {
#if USE_CHANGED_ACID_STUFF
    case Xacid_1:
    case Xacid_2:
    case Xacid_3:
    case Xacid_4:
    case Xacid_5:
    case Xacid_6:
    case Xacid_7:
    case Xacid_8:
      break;
#endif

    default:
      Cave[y][x] = Xboom_1;
      Boom[y][x] = Xblank;
      break;
  }
}

static void check_player(struct PLAYER *ply)
{
  int oldx = ply->x;
  int oldy = ply->y;
  int x = oldx;
  int y = oldy;
  int dx = 0, dy = 0;

  game_em.last_player_direction[ply->num] = MV_NONE;

  if (ply->joy_w)		/* west */
  {
    x--;
    dx = -1;
  }
  else if (ply->joy_e)		/* east */
  {
    x++;
    dx = 1;
  }

  if (ply->joy_n)		/* north */
  {
    y--;
    dy = -1;
  }
  else if (ply->joy_s)		/* south */
  {
    y++;
    dy = 1;
  }

  if (dx || dy)
  {
    int oldx = ply->x;
    int oldy = ply->y;
    int x = oldx + dx;
    int y = oldy + dy;
    boolean players_visible_before_move;
    boolean players_visible_after_move;
    boolean can_move;

    players_visible_before_move = checkIfAllPlayersFitToScreen();

    ply->x = x;
    ply->y = y;

    players_visible_after_move = checkIfAllPlayersFitToScreen();

    /*
      player is allowed to move only in the following cases:
      - it is not needed to display all players (not focussed to all players)
      - all players are (still or again) visible after the move
      - some players were already outside visible screen area before the move
    */
    can_move = (game.centered_player_nr != -1 ||
		players_visible_after_move ||
		!players_visible_before_move);

    ply->x = oldx;
    ply->y = oldy;

    if (!can_move)
    {
      ply->joy_n = ply->joy_e = ply->joy_s = ply->joy_w = 0;

      return;
    }
  }

  if (dx == 0 && dy == 0)
  {
    ply->joy_stick = 0;

    if (ply->joy_drop)
    {
      if (++ply->dynamite_cnt == 5 && ply->dynamite)
      {
	Cave[y][x] = Xdynamite_1;
	play_element_sound(x, y, SAMPLE_dynamite, Xdynamite_1);
	ply->dynamite--;
      }
    }
    else
    {
      ply->dynamite_cnt = 0;
    }

    RandomEM += 7;	/* be a bit more random if the player doesn't move */

    return;
  }

  ply->joy_stick = 1;
  ply->joy_n = ply->joy_e = ply->joy_s = ply->joy_w = 0;
  ply->dynamite_cnt = 0;	/* reset dynamite timer if we move */
  ply->joy_spin = !ply->joy_spin;

  if (ply->joy_snap == 0)		/* player wants to move */
  {
    boolean moved = FALSE;

    if (ply->last_move_dir & MV_HORIZONTAL)
    {
      if (!(moved = player_digfield(ply, 0, dy)))
	moved = player_digfield(ply, dx, 0);
    }
    else
    {
      if (!(moved = player_digfield(ply, dx, 0)))
	moved = player_digfield(ply, 0, dy);
    }

    if (moved)
    {
      if (oldx != ply->x)
	ply->last_move_dir = (dx < 0 ? MV_LEFT : MV_RIGHT);
      else if (oldy != ply->y)
	ply->last_move_dir = (dy < 0 ? MV_UP : MV_DOWN);

      game_em.any_player_moving = TRUE;
      game_em.last_moving_player = ply->num;
      game_em.last_player_direction[ply->num] = ply->last_move_dir;
    }
  }
  else					/* player wants to snap */
  {
    game_em.any_player_snapping = player_digfield(ply, dx, dy);
  }
}

static boolean player_digfield(struct PLAYER *ply, int dx, int dy)
{
  int anim = (dx < 0 ? 3 : dx > 0 ? 1 : dy < 0 ? 0 : dy > 0 ? 2 : 2);
  int oldx = ply->x;
  int oldy = ply->y;
  int x = oldx + dx;
  int y = oldy + dy;
  boolean result = TRUE;

  if (!dx && !dy)			/* no direction specified */
    return FALSE;

  if (dx && dy && ply->joy_snap)	/* more than one direction specified */
    return FALSE;

  if (ply->joy_snap == 0)		/* player wants to move */
  {
    int element = Cave[y][x];

    switch(Cave[y][x])
    {
      /* fire is released */
      case Xblank:
      case Yacid_splash_eB:
      case Yacid_splash_wB:
	Cave[y][x] = Zplayer;
	Next[y][x] = Zplayer;
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
	play_element_sound(x, y, SAMPLE_blank, Xblank);
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

#if USE_CHANGED_ACID_STUFF
      case Xacid_1:
      case Xacid_2:
      case Xacid_3:
      case Xacid_4:
      case Xacid_5:
      case Xacid_6:
      case Xacid_7:
      case Xacid_8:
	if (Cave[y-1][x+1] == Xblank)
	  Cave[y-1][x+1] = Yacid_splash_eB;
	if (Cave[y-1][x-1] == Xblank)
	  Cave[y-1][x-1] = Yacid_splash_wB;
	play_element_sound(x, y, SAMPLE_acid, Xacid_1);
#endif

      case Xboom_android:
      case Xboom_1:
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

#if !USE_CHANGED_ACID_STUFF
      case Xacid_1:
      case Xacid_2:
      case Xacid_3:
      case Xacid_4:
      case Xacid_5:
      case Xacid_6:
      case Xacid_7:
      case Xacid_8:
#endif
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xgrass:
	Cave[y][x] = (dy ? (dy < 0 ? Ygrass_nB : Ygrass_sB) :
		      (dx > 0 ? Ygrass_eB : Ygrass_wB));
	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_dirt, Xgrass);
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xdirt:
	Cave[y][x] = (dy ? (dy < 0 ? Ydirt_nB : Ydirt_sB) :
		      (dx > 0 ? Ydirt_eB : Ydirt_wB));
	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_dirt, Xdirt);
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xdiamond:
      case Xdiamond_pause:
	Cave[y][x] = Ydiamond_eat;
	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.diamond_score;
	lev.required = lev.required < 3 ? 0 : lev.required - 3;
	game.snapshot.collected_item = TRUE;
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xemerald:
      case Xemerald_pause:
	Cave[y][x] = Yemerald_eat;
	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.emerald_score;
	lev.required = lev.required < 1 ? 0 : lev.required - 1;
	game.snapshot.collected_item = TRUE;
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xdynamite:
	Cave[y][x] = Ydynamite_eat;
	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.dynamite_score;
	ply->dynamite = ply->dynamite > 9998 ? 9999 : ply->dynamite + 1;
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xkey_1:
	ply->keys |= 0x01;
	Cave[y][x] = Ykey_1_eat;
	goto key_walk;

      case Xkey_2:
	ply->keys |= 0x02;
	Cave[y][x] = Ykey_2_eat;
	goto key_walk;

      case Xkey_3:
	ply->keys |= 0x04;
	Cave[y][x] = Ykey_3_eat;
	goto key_walk;

      case Xkey_4:
	ply->keys |= 0x08;
	Cave[y][x] = Ykey_4_eat;
	goto key_walk;

      case Xkey_5:
	ply->keys |= 0x10;
	Cave[y][x] = Ykey_5_eat;
	goto key_walk;

      case Xkey_6:
	ply->keys |= 0x20;
	Cave[y][x] = Ykey_6_eat;
	goto key_walk;

      case Xkey_7:
	ply->keys |= 0x40;
	Cave[y][x] = Ykey_7_eat;
	goto key_walk;

      case Xkey_8:
	ply->keys |= 0x80;
	Cave[y][x] = Ykey_8_eat;
	goto key_walk;

      key_walk:

	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.key_score;
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xlenses:
	Cave[y][x] = Ylenses_eat;
	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.lenses_score;
	lev.lenses_cnt = lev.lenses_time;
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xmagnify:
	Cave[y][x] = Ymagnify_eat;
	Next[y][x] = Zplayer;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.magnify_score;
	lev.magnify_cnt = lev.magnify_time;
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xstone:
	if (dy)
	  break;

	switch(Cave[y][x+dx])
	{
          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (Cave[y-1][x+dx+1] == Xblank)
	      Cave[y-1][x+dx+1] = Yacid_splash_eB;
	    if (Cave[y-1][x+dx-1] == Xblank)
	      Cave[y-1][x+dx-1] = Yacid_splash_wB;
	    play_element_sound(x, y, SAMPLE_acid, Xacid_1);
	    goto stone_walk;

          case Xblank:
          case Yacid_splash_eB:
          case Yacid_splash_wB:
	    Cave[y][x+dx] = dx > 0 ? Ystone_e : Ystone_w;
	    Next[y][x+dx] = Xstone_pause;

          stone_walk:

	    Cave[y][x] = dx > 0 ? Ystone_eB : Ystone_wB;
	    Next[y][x] = Zplayer;
	    play_element_sound(x, y, SAMPLE_roll, Xstone);
	    ply->x = x;
	}

	ply->anim = SPR_push + anim;
	break;

      case Xbomb:
	if (dy)
	  break;

	switch(Cave[y][x+dx])
	{
          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (Cave[y-1][x+dx+1] == Xblank)
	      Cave[y-1][x+dx+1] = Yacid_splash_eB;
	    if (Cave[y-1][x+dx-1] == Xblank)
	      Cave[y-1][x+dx-1] = Yacid_splash_wB;
	    play_element_sound(x, y, SAMPLE_acid, Xacid_1);
	    goto bomb_walk;

	  case Xblank:
          case Yacid_splash_eB:
          case Yacid_splash_wB:
	    Cave[y][x+dx] = dx > 0 ? Ybomb_e : Ybomb_w;
	    Next[y][x+dx] = Xbomb_pause;

          bomb_walk:

	    Cave[y][x] = dx > 0 ? Ybomb_eB : Ybomb_wB;
	    Next[y][x] = Zplayer;
	    play_element_sound(x, y, SAMPLE_roll, Xbomb);
	    ply->x = x;
	}

	ply->anim = SPR_push + anim;
	break;

      case Xnut:
	if (dy)
	  break;

	switch(Cave[y][x+dx])
	{
          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (Cave[y-1][x+dx+1] == Xblank)
	      Cave[y-1][x+dx+1] = Yacid_splash_eB;
	    if (Cave[y-1][x+dx-1] == Xblank)
	      Cave[y-1][x+dx-1] = Yacid_splash_wB;
	    play_element_sound(x, y, SAMPLE_acid, Xacid_1);
	    goto nut_walk;

          case Xblank:
          case Yacid_splash_eB:
          case Yacid_splash_wB:
	    Cave[y][x+dx] = dx > 0 ? Ynut_e : Ynut_w;
	    Next[y][x+dx] = Xnut_pause;

          nut_walk:

	    Cave[y][x] = dx > 0 ? Ynut_eB : Ynut_wB;
	    Next[y][x] = Zplayer;
	    play_element_sound(x, y, SAMPLE_roll, Xnut);
	    ply->x = x;
	}

	ply->anim = SPR_push + anim;
	break;

      case Xspring:
	if (dy)
	  break;

	switch(Cave[y][x+dx])
	{
          case Xalien:
          case Xalien_pause:
	    Cave[y][x] = dx > 0 ? Yspring_kill_eB : Yspring_kill_wB;
	    Cave[y][x+dx] = dx > 0 ? Yspring_kill_e : Yspring_kill_w;
	    Next[y][x] = Zplayer;
	    Next[y][x+dx] = dx > 0 ? Xspring_e : Xspring_w;
	    play_element_sound(x, y, SAMPLE_slurp, Xalien);
	    lev.score += lev.slurp_score;
	    ply->x = x;
	    break;

          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (Cave[y-1][x+dx+1] == Xblank)
	      Cave[y-1][x+dx+1] = Yacid_splash_eB;
	    if (Cave[y-1][x+dx-1] == Xblank)
	      Cave[y-1][x+dx-1] = Yacid_splash_wB;
	    play_element_sound(x, y, SAMPLE_acid, Xacid_1);
	    goto spring_walk;

          case Xblank:
          case Yacid_splash_eB:
          case Yacid_splash_wB:
	    Cave[y][x+dx] = dx > 0 ? Yspring_e : Yspring_w;
	    Next[y][x+dx] = dx > 0 ? Xspring_e : Xspring_w;

	  spring_walk:
	    Cave[y][x] = dx > 0 ? Yspring_eB : Yspring_wB;
	    Next[y][x] = Zplayer;
	    play_element_sound(x, y, SAMPLE_roll, Xspring);
	    ply->x = x;
	}

	ply->anim = SPR_push + anim;
	break;

      case Xspring_pause:
      case Xstone_pause:
      case Xbomb_pause:
      case Xnut_pause:
      case Xsand_stonein_1:
      case Xsand_stonein_2:
      case Xsand_stonein_3:
      case Xsand_stonein_4:
	if (dy)
	  break;

	ply->anim = SPR_push + anim;
	break;

      case Xballoon:
	switch(Cave[y+dy][x+dx])
	{
          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (Cave[y+dy-1][x+dx+1] == Xblank)
	      Cave[y+dy-1][x+dx+1] = Yacid_splash_eB;
	    if (Cave[y+dy-1][x+dx-1] == Xblank)
	      Cave[y+dy-1][x+dx-1] = Yacid_splash_wB;
	    play_element_sound(x, y, SAMPLE_acid, Xacid_1);
	    goto balloon_walk;

          case Xblank:
          case Yacid_splash_eB:
          case Yacid_splash_wB:
	    Cave[y+dy][x+dx] = (dy ? (dy < 0 ? Yballoon_n : Yballoon_s) :
				(dx > 0 ? Yballoon_e : Yballoon_w));
	    Next[y+dy][x+dx] = Xballoon;

	  balloon_walk:
	    Cave[y][x] = (dy ? (dy < 0 ? Yballoon_nB : Yballoon_sB) :
			  (dx > 0 ? Yballoon_eB : Yballoon_wB));
	    Next[y][x] = Zplayer;
	    play_element_sound(x, y, SAMPLE_push, Xballoon);
	    ply->x = x;
	    ply->y = y;
	}

	ply->anim = SPR_push + anim;
	break;

      case Xandroid:
      case Xandroid_1_n:
      case Xandroid_2_n:
      case Xandroid_1_e:
      case Xandroid_2_e:
      case Xandroid_1_s:
      case Xandroid_2_s:
      case Xandroid_1_w:
      case Xandroid_2_w:
	switch(Cave[y+dy][x+dx])
	{
          case Xacid_1:
          case Xacid_2:
          case Xacid_3:
          case Xacid_4:
          case Xacid_5:
          case Xacid_6:
          case Xacid_7:
          case Xacid_8:
	    if (Cave[y+dy-1][x+dx+1] == Xblank)
	      Cave[y+dy-1][x+dx+1] = Yacid_splash_eB;
	    if (Cave[y+dy-1][x+dx-1] == Xblank)
	      Cave[y+dy-1][x+dx-1] = Yacid_splash_wB;
	    play_element_sound(x, y, SAMPLE_acid, Xacid_1);
	    goto android_walk;

          case Xblank:
          case Yacid_splash_eB:
          case Yacid_splash_wB:
	    Cave[y+dy][x+dx] = (dy ? (dy < 0 ? Yandroid_n : Yandroid_s) :
				(dx > 0 ? Yandroid_e : Yandroid_w));
	    Next[y+dy][x+dx] = (dy ? (dy < 0 ? Xandroid_2_n : Xandroid_2_s) :
				(dx > 0 ? Xandroid_2_e : Xandroid_2_w));

	  android_walk:
	    Cave[y][x] = (dy ? (dy < 0 ? Yandroid_nB : Yandroid_sB) :
			  (dx > 0 ? Yandroid_eB : Yandroid_wB));
	    Next[y][x] = Zplayer;
	    play_element_sound(x, y, SAMPLE_push, Xandroid);
	    ply->x = x;
	    ply->y = y;
	}

	ply->anim = SPR_push + anim;
	break;

      case Xdoor_1:
      case Xfake_door_1:
	if (ply->keys & 0x01)
	  goto door_walk;
	else
	  break;

      case Xdoor_2:
      case Xfake_door_2:
	if (ply->keys & 0x02)
	  goto door_walk;
	else
	  break;

      case Xdoor_3:
      case Xfake_door_3:
	if (ply->keys & 0x04)
	  goto door_walk;
	else
	  break;

      case Xdoor_4:
      case Xfake_door_4:
	if (ply->keys & 0x08)
	  goto door_walk;
	else
	  break;

      case Xdoor_5:
      case Xfake_door_5:
	if (ply->keys & 0x10)
	  goto door_walk;
	else
	  break;

      case Xdoor_6:
      case Xfake_door_6:
	if (ply->keys & 0x20)
	  goto door_walk;
	else
	  break;

      case Xdoor_7:
      case Xfake_door_7:
	if (ply->keys & 0x40)
	  goto door_walk;
	else
	  break;

      case Xdoor_8:
      case Xfake_door_8:
	if (ply->keys & 0x80)
	  goto door_walk;
	else
	  break;

      door_walk:
	if (!tab_blank[Cave[y+dy][x+dx]])
	  break;

	Cave[y+dy][x+dx] = Zplayer;
	Next[y+dy][x+dx] = Zplayer;
	play_element_sound(x, y, SAMPLE_door, element);
	ply->anim = SPR_walk + anim;
	ply->x = x + dx;
	ply->y = y + dy;
	break;

      case Xwheel:
	play_element_sound(x, y, SAMPLE_press, element);
	lev.wheel_cnt = lev.wheel_time;
	lev.wheel_x = x;
	lev.wheel_y = y;
	break;

      case Xwind_n:
	lev.wind_direction = 0;
	goto wind_walk;

      case Xwind_e:
	lev.wind_direction = 1;
	goto wind_walk;

      case Xwind_s:
	lev.wind_direction = 2;
	goto wind_walk;

      case Xwind_w:
	lev.wind_direction = 3;
	goto wind_walk;

      case Xwind_nesw:
	lev.wind_direction = dy ? (dy < 0 ? 0 : 2) : (dx > 0 ? 1 : 3);
	goto wind_walk;

      wind_walk:
	play_element_sound(x, y, SAMPLE_press, element);
	lev.wind_cnt = lev.wind_time;
	break;

      case Xwind_stop:
	play_element_sound(x, y, SAMPLE_press, element);
	lev.wind_cnt = 0;
	break;

      case Xswitch:
	play_element_sound(x, y, SAMPLE_press, element);
	lev.ball_cnt = lev.ball_time;
	lev.ball_state = !lev.ball_state;
	break;

      case Xplant:
	Cave[y][x] = Yplant;
	Next[y][x] = Xplant;
	play_element_sound(x, y, SAMPLE_blank, Xplant);
	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;
	break;

      case Xexit_1:
      case Xexit_2:
      case Xexit_3:
	lev.home--;

	ply->anim = SPR_walk + anim;
	ply->x = x;
	ply->y = y;

	break;
    }

    if (ply->x == oldx && ply->y == oldy)	/* no movement */
      result = FALSE;
  }
  else					/* player wants to snap */
  {
    int element = Cave[y][x];

    switch(Cave[y][x])
    {
      /* fire is pressed */

      case Xgrass:
	Cave[y][x] = Ygrass_eat;
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_dirt, element);
	ply->anim = SPR_spray + anim;
	break;

      case Xdirt:
	Cave[y][x] = Ydirt_eat;
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_dirt, element);
	ply->anim = SPR_spray + anim;
	break;

      case Xdiamond:
      case Xdiamond_pause:
	Cave[y][x] = Ydiamond_eat;
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.diamond_score;
	lev.required = lev.required < 3 ? 0 : lev.required - 3;
	game.snapshot.collected_item = TRUE;
	ply->anim = SPR_walk + anim;
	break;

      case Xemerald:
      case Xemerald_pause:
	Cave[y][x] = Yemerald_eat;
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.emerald_score;
	lev.required = lev.required < 1 ? 0 : lev.required - 1;
	game.snapshot.collected_item = TRUE;
	ply->anim = SPR_walk + anim;
	break;

      case Xdynamite:
	Cave[y][x] = Ydynamite_eat;
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.dynamite_score;
	ply->dynamite = ply->dynamite > 9998 ? 9999 : ply->dynamite + 1;
	ply->anim = SPR_walk + anim;
	break;

      case Xkey_1:
	ply->keys |= 0x01;
	Cave[y][x] = Ykey_1_eat;
	goto key_shoot;

      case Xkey_2:
	ply->keys |= 0x02;
	Cave[y][x] = Ykey_2_eat;
	goto key_shoot;

      case Xkey_3:
	ply->keys |= 0x04;
	Cave[y][x] = Ykey_3_eat;
	goto key_shoot;

      case Xkey_4:
	ply->keys |= 0x08;
	Cave[y][x] = Ykey_4_eat;
	goto key_shoot;

      case Xkey_5:
	ply->keys |= 0x10;
	Cave[y][x] = Ykey_5_eat;
	goto key_shoot;

      case Xkey_6:
	ply->keys |= 0x20;
	Cave[y][x] = Ykey_6_eat;
	goto key_shoot;

      case Xkey_7:
	ply->keys |= 0x40;
	Cave[y][x] = Ykey_7_eat;
	goto key_shoot;

      case Xkey_8:
	ply->keys |= 0x80;
	Cave[y][x] = Ykey_8_eat;
	goto key_shoot;

      key_shoot:
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.key_score;
	ply->anim = SPR_walk + anim;
	break;

      case Xlenses:
	Cave[y][x] = Ylenses_eat;
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.lenses_score;
	lev.lenses_cnt = lev.lenses_time;
	ply->anim = SPR_walk + anim;
	break;

      case Xmagnify:
	Cave[y][x] = Ymagnify_eat;
	Next[y][x] = Xblank;
	play_element_sound(x, y, SAMPLE_collect, element);
	lev.score += lev.magnify_score;
	lev.magnify_cnt = lev.magnify_time;
	ply->anim = SPR_walk + anim;
	break;

      default:
	result = FALSE;
    }
  }

  return result;
}
