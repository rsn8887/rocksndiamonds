// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// anim.c
// ============================================================================

#include "libgame/libgame.h"

#include "anim.h"
#include "main.h"
#include "tools.h"


/* values for global toon animation definition */
#define NUM_GLOBAL_TOON_ANIMS		1
#define NUM_GLOBAL_TOON_PARTS		MAX_NUM_TOONS

/* values for global animation definition (including toons) */
#define NUM_GLOBAL_ANIMS_AND_TOONS	(NUM_GLOBAL_ANIMS +		\
					 NUM_GLOBAL_TOON_ANIMS)
#define NUM_GLOBAL_ANIM_PARTS_AND_TOONS	MAX(NUM_GLOBAL_ANIM_PARTS_ALL,	\
					    NUM_GLOBAL_TOON_PARTS)

#define ANIM_CLASS_BIT_TITLE_INITIAL	0
#define ANIM_CLASS_BIT_TITLE		1
#define ANIM_CLASS_BIT_MAIN		2
#define ANIM_CLASS_BIT_SCORES		3
#define ANIM_CLASS_BIT_SUBMENU		4
#define ANIM_CLASS_BIT_MENU		5
#define ANIM_CLASS_BIT_TOONS		6

#define NUM_ANIM_CLASSES		7

#define ANIM_CLASS_NONE			0
#define ANIM_CLASS_TITLE_INITIAL	(1 << ANIM_CLASS_BIT_TITLE_INITIAL)
#define ANIM_CLASS_TITLE		(1 << ANIM_CLASS_BIT_TITLE)
#define ANIM_CLASS_MAIN			(1 << ANIM_CLASS_BIT_MAIN)
#define ANIM_CLASS_SCORES		(1 << ANIM_CLASS_BIT_SCORES)
#define ANIM_CLASS_SUBMENU		(1 << ANIM_CLASS_BIT_SUBMENU)
#define ANIM_CLASS_MENU			(1 << ANIM_CLASS_BIT_MENU)
#define ANIM_CLASS_TOONS		(1 << ANIM_CLASS_BIT_TOONS)

#define ANIM_CLASS_TOONS_SCORES		(ANIM_CLASS_TOONS |	\
					 ANIM_CLASS_SCORES)

#define ANIM_CLASS_TOONS_MENU_MAIN	(ANIM_CLASS_TOONS |	\
					 ANIM_CLASS_MENU  |	\
					 ANIM_CLASS_MAIN)

#define ANIM_CLASS_TOONS_MENU_SUBMENU	(ANIM_CLASS_TOONS |	\
					 ANIM_CLASS_MENU  |	\
					 ANIM_CLASS_SUBMENU)

/* values for global animation states */
#define ANIM_STATE_INACTIVE		0
#define ANIM_STATE_RESTART		(1 << 0)
#define ANIM_STATE_WAITING		(1 << 1)
#define ANIM_STATE_RUNNING		(1 << 2)

/* values for global animation control */
#define ANIM_NO_ACTION			0
#define ANIM_START			1
#define ANIM_CONTINUE			2
#define ANIM_STOP			3


struct GlobalAnimPartControlInfo
{
  int old_nr;		// position before mapping animation parts linearly
  int old_anim_nr;	// position before mapping animations linearly

  int nr;
  int anim_nr;
  int mode_nr;

  boolean is_base;	// animation part is base/main/default animation part

  int sound;
  int music;
  int graphic;

  struct GraphicInfo graphic_info;
  struct GraphicInfo control_info;

  int viewport_x;
  int viewport_y;
  int viewport_width;
  int viewport_height;

  int x, y;
  int step_xoffset, step_yoffset;

  unsigned int initial_anim_sync_frame;
  unsigned int step_delay, step_delay_value;

  int init_delay_counter;
  int anim_delay_counter;
  int post_delay_counter;

  boolean init_event_state;
  boolean anim_event_state;

  boolean clickable;
  boolean clicked;

  int drawing_stage;

  int state;
  int last_anim_status;
};

struct GlobalAnimMainControlInfo
{
  struct GlobalAnimPartControlInfo base;
  struct GlobalAnimPartControlInfo part[NUM_GLOBAL_ANIM_PARTS_AND_TOONS];

  int nr;
  int mode_nr;

  struct GraphicInfo control_info;

  int num_parts;	// number of animation parts, but without base part
  int num_parts_all;	// number of animation parts, including base part
  int part_counter;
  int active_part_nr;

  boolean has_base;	// animation has base/main/default animation part

  int last_x, last_y;

  int init_delay_counter;

  int state;

  int last_state, last_active_part_nr;
};

struct GlobalAnimControlInfo
{
  struct GlobalAnimMainControlInfo anim[NUM_GLOBAL_ANIMS_AND_TOONS];

  int nr;
  int num_anims;
};

struct GameModeAnimClass
{
  int game_mode;
  int class;
} game_mode_anim_classes_list[] =
{
  { GAME_MODE_TITLE_INITIAL_1,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_2,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_3,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_4,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_INITIAL_5,		ANIM_CLASS_TITLE_INITIAL	},
  { GAME_MODE_TITLE_1,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_2,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_3,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_4,			ANIM_CLASS_TITLE		},
  { GAME_MODE_TITLE_5,			ANIM_CLASS_TITLE		},
  { GAME_MODE_LEVELS, 			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_LEVELNR,			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_INFO,			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_SETUP,			ANIM_CLASS_TOONS_MENU_SUBMENU	},
  { GAME_MODE_PSEUDO_MAINONLY,		ANIM_CLASS_TOONS_MENU_MAIN	},
  { GAME_MODE_PSEUDO_TYPENAME,		ANIM_CLASS_TOONS_MENU_MAIN	},
  { GAME_MODE_PSEUDO_SCORESOLD,		ANIM_CLASS_TOONS_SCORES		},
  { GAME_MODE_PSEUDO_SCORESNEW,		ANIM_CLASS_TOONS_SCORES		},

  { -1,					-1				}
};

struct AnimClassGameMode
{
  int class_bit;
  int game_mode;
} anim_class_game_modes_list[] =
{
  { ANIM_CLASS_BIT_TITLE_INITIAL,	GAME_MODE_TITLE_INITIAL		},
  { ANIM_CLASS_BIT_TITLE,		GAME_MODE_TITLE			},
  { ANIM_CLASS_BIT_MAIN,		GAME_MODE_MAIN			},
  { ANIM_CLASS_BIT_SCORES,		GAME_MODE_SCORES		},
  { ANIM_CLASS_BIT_SUBMENU,		GAME_MODE_PSEUDO_SUBMENU	},
  { ANIM_CLASS_BIT_MENU,		GAME_MODE_PSEUDO_MENU		},
  { ANIM_CLASS_BIT_TOONS,		GAME_MODE_PSEUDO_TOONS		},

  { -1,					-1				}
};

/* forward declaration for internal use */
static void HandleGlobalAnim(int, int);
static void DoAnimationExt(void);
static void ResetGlobalAnim_Clickable();
static void ResetGlobalAnim_Clicked();

static struct GlobalAnimControlInfo global_anim_ctrl[NUM_GAME_MODES];

static unsigned int anim_sync_frame = 0;

static int game_mode_anim_classes[NUM_GAME_MODES];
static int anim_class_game_modes[NUM_ANIM_CLASSES];

static int anim_status_last_before_fading = GAME_MODE_DEFAULT;
static int anim_status_last = GAME_MODE_DEFAULT;
static int anim_classes_last = ANIM_CLASS_NONE;

static boolean drawing_to_fading_buffer = FALSE;


/* ========================================================================= */
/* generic animation frame calculation                                       */
/* ========================================================================= */

int getAnimationFrame(int num_frames, int delay, int mode, int start_frame,
		      int sync_frame)
{
  int frame = 0;

  sync_frame += start_frame * delay;

  if (mode & ANIM_LOOP)			/* looping animation */
  {
    frame = (sync_frame % (delay * num_frames)) / delay;
  }
  else if (mode & ANIM_LINEAR)		/* linear (non-looping) animation */
  {
    frame = sync_frame / delay;

    if (frame > num_frames - 1)
      frame = num_frames - 1;
  }
  else if (mode & ANIM_PINGPONG)	/* oscillate (border frames once) */
  {
    int max_anim_frames = (num_frames > 1 ? 2 * num_frames - 2 : 1);

    frame = (sync_frame % (delay * max_anim_frames)) / delay;
    frame = (frame < num_frames ? frame : max_anim_frames - frame);
  }
  else if (mode & ANIM_PINGPONG2)	/* oscillate (border frames twice) */
  {
    int max_anim_frames = 2 * num_frames;

    frame = (sync_frame % (delay * max_anim_frames)) / delay;
    frame = (frame < num_frames ? frame : max_anim_frames - frame - 1);
  }
  else if (mode & ANIM_RANDOM)		/* play frames in random order */
  {
    /* note: expect different frames for the same delay cycle! */

    if (gfx.anim_random_frame < 0)
      frame = GetSimpleRandom(num_frames);
    else
      frame = gfx.anim_random_frame % num_frames;
  }
  else if (mode & (ANIM_CE_VALUE | ANIM_CE_SCORE | ANIM_CE_DELAY))
  {
    frame = sync_frame % num_frames;
  }

  if (mode & ANIM_REVERSE)		/* use reverse animation direction */
    frame = num_frames - frame - 1;

  return frame;
}


/* ========================================================================= */
/* global animation functions                                                */
/* ========================================================================= */

static int getGlobalAnimationPart(struct GlobalAnimMainControlInfo *anim)
{
  struct GraphicInfo *c = &anim->control_info;
  int last_anim_random_frame = gfx.anim_random_frame;
  int part_nr;

  gfx.anim_random_frame = -1;	// (use simple, ad-hoc random numbers)

  part_nr = getAnimationFrame(anim->num_parts, 1,
			      c->anim_mode, c->anim_start_frame,
			      anim->part_counter);

  gfx.anim_random_frame = last_anim_random_frame;

  return part_nr;
}

static int compareGlobalAnimPartControlInfo(const void *obj1, const void *obj2)
{
  const struct GlobalAnimPartControlInfo *o1 =
    (struct GlobalAnimPartControlInfo *)obj1;
  const struct GlobalAnimPartControlInfo *o2 =
    (struct GlobalAnimPartControlInfo *)obj2;
  int compare_result;

  if (o1->control_info.draw_order != o2->control_info.draw_order)
    compare_result = o1->control_info.draw_order - o2->control_info.draw_order;
  else
    compare_result = o1->nr - o2->nr;

  return compare_result;
}

static int compareGlobalAnimMainControlInfo(const void *obj1, const void *obj2)
{
  const struct GlobalAnimMainControlInfo *o1 =
    (struct GlobalAnimMainControlInfo *)obj1;
  const struct GlobalAnimMainControlInfo *o2 =
    (struct GlobalAnimMainControlInfo *)obj2;
  int compare_result;

  if (o1->control_info.draw_order != o2->control_info.draw_order)
    compare_result = o1->control_info.draw_order - o2->control_info.draw_order;
  else
    compare_result = o1->nr - o2->nr;

  return compare_result;
}

static void InitToonControls()
{
  int mode_nr_toons = GAME_MODE_PSEUDO_TOONS;
  struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr_toons];
  struct GlobalAnimMainControlInfo *anim = &ctrl->anim[ctrl->num_anims];
  int mode_nr, anim_nr, part_nr;
  int control = IMG_INTERNAL_GLOBAL_TOON_DEFAULT;
  int num_toons = MAX_NUM_TOONS;
  int i;

  if (global.num_toons >= 0 && global.num_toons < MAX_NUM_TOONS)
    num_toons = global.num_toons;

  mode_nr = mode_nr_toons;
  anim_nr = ctrl->num_anims;

  anim->nr = anim_nr;
  anim->mode_nr = mode_nr;
  anim->control_info = graphic_info[control];

  anim->num_parts = 0;
  anim->num_parts_all = 0;
  anim->part_counter = 0;
  anim->active_part_nr = 0;

  anim->has_base = FALSE;

  anim->last_x = POS_OFFSCREEN;
  anim->last_y = POS_OFFSCREEN;

  anim->init_delay_counter = 0;

  anim->state = ANIM_STATE_INACTIVE;

  part_nr = 0;

  for (i = 0; i < num_toons; i++)
  {
    struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];
    int sound = SND_UNDEFINED;
    int music = MUS_UNDEFINED;
    int graphic = IMG_TOON_1 + i;
    int control = graphic;

    part->nr = part_nr;
    part->anim_nr = anim_nr;
    part->mode_nr = mode_nr;

    part->is_base = FALSE;

    part->sound = sound;
    part->music = music;
    part->graphic = graphic;

    part->graphic_info = graphic_info[graphic];
    part->control_info = graphic_info[control];

    part->graphic_info.anim_delay *= part->graphic_info.step_delay;

    part->control_info.init_delay_fixed = 0;
    part->control_info.init_delay_random = 150;

    part->control_info.x = ARG_UNDEFINED_VALUE;
    part->control_info.y = ARG_UNDEFINED_VALUE;

    part->initial_anim_sync_frame = 0;

    part->step_delay = 0;
    part->step_delay_value = graphic_info[control].step_delay;

    part->state = ANIM_STATE_INACTIVE;
    part->last_anim_status = -1;

    anim->num_parts++;
    anim->num_parts_all++;

    part_nr++;
  }

  ctrl->num_anims++;
}

void InitGlobalAnimControls()
{
  int i, m, a, p;
  int mode_nr, anim_nr, part_nr;
  int sound, music, graphic, control;

  anim_sync_frame = 0;

  for (m = 0; m < NUM_GAME_MODES; m++)
  {
    mode_nr = m;

    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr];

    ctrl->nr = mode_nr;
    ctrl->num_anims = 0;

    anim_nr = 0;

    for (a = 0; a < NUM_GLOBAL_ANIMS; a++)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[anim_nr];
      int ctrl_id = GLOBAL_ANIM_ID_CONTROL_FIRST + a;

      control = global_anim_info[ctrl_id].graphic[GLOBAL_ANIM_ID_PART_BASE][m];

      // if no base animation parameters defined, use default values
      if (control == IMG_UNDEFINED)
	control = IMG_INTERNAL_GLOBAL_ANIM_DEFAULT;

      anim->nr = anim_nr;
      anim->mode_nr = mode_nr;
      anim->control_info = graphic_info[control];

      anim->num_parts = 0;
      anim->num_parts_all = 0;
      anim->part_counter = 0;
      anim->active_part_nr = 0;

      anim->has_base = FALSE;

      anim->last_x = POS_OFFSCREEN;
      anim->last_y = POS_OFFSCREEN;

      anim->init_delay_counter = 0;

      anim->state = ANIM_STATE_INACTIVE;

      part_nr = 0;

      for (p = 0; p < NUM_GLOBAL_ANIM_PARTS_ALL; p++)
      {
	struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];

	sound   = global_anim_info[a].sound[p][m];
	music   = global_anim_info[a].music[p][m];
	graphic = global_anim_info[a].graphic[p][m];
	control = global_anim_info[ctrl_id].graphic[p][m];

	if (graphic == IMG_UNDEFINED || graphic_info[graphic].bitmap == NULL ||
	    control == IMG_UNDEFINED)
	  continue;

#if 0
	printf("::: mode == %d, anim = %d, part = %d [%d, %d, %d] [%d]\n",
	       m, a, p, mode_nr, anim_nr, part_nr, control);
#endif

#if 0
	printf("::: mode == %d, anim = %d, part = %d [%d, %d, %d] [%d]\n",
	       m, a, p, mode_nr, anim_nr, part_nr, sound);
#endif

	part->old_nr = p;
	part->old_anim_nr = a;

	part->nr = part_nr;
	part->anim_nr = anim_nr;
	part->mode_nr = mode_nr;

	part->sound = sound;
	part->music = music;
	part->graphic = graphic;

	part->graphic_info = graphic_info[graphic];
	part->control_info = graphic_info[control];

	part->initial_anim_sync_frame = 0;

	part->step_delay = 0;
	part->step_delay_value = graphic_info[control].step_delay;

	part->state = ANIM_STATE_INACTIVE;
	part->last_anim_status = -1;

	anim->num_parts_all++;

	if (p < GLOBAL_ANIM_ID_PART_BASE)
	{
	  part->is_base = FALSE;

	  anim->num_parts++;
	  part_nr++;
	}
	else
	{
	  part->is_base = TRUE;

	  anim->base = *part;
	  anim->has_base = TRUE;
	}
      }

      if (anim->num_parts > 0 || anim->has_base)
      {
	ctrl->num_anims++;
	anim_nr++;
      }
    }
  }

  InitToonControls();

  /* sort all animations according to draw_order and animation number */
  for (m = 0; m < NUM_GAME_MODES; m++)
  {
    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[m];

    /* sort all main animations for this game mode */
    qsort(ctrl->anim, ctrl->num_anims,
	  sizeof(struct GlobalAnimMainControlInfo),
	  compareGlobalAnimMainControlInfo);

    for (a = 0; a < ctrl->num_anims; a++)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[a];

      /* sort all animation parts for this main animation */
      qsort(anim->part, anim->num_parts,
	    sizeof(struct GlobalAnimPartControlInfo),
	    compareGlobalAnimPartControlInfo);
    }
  }

  for (i = 0; i < NUM_GAME_MODES; i++)
    game_mode_anim_classes[i] = ANIM_CLASS_NONE;
  for (i = 0; game_mode_anim_classes_list[i].game_mode != -1; i++)
    game_mode_anim_classes[game_mode_anim_classes_list[i].game_mode] =
      game_mode_anim_classes_list[i].class;

  for (i = 0; i < NUM_ANIM_CLASSES; i++)
    anim_class_game_modes[i] = GAME_MODE_DEFAULT;
  for (i = 0; anim_class_game_modes_list[i].game_mode != -1; i++)
    anim_class_game_modes[anim_class_game_modes_list[i].class_bit] =
      anim_class_game_modes_list[i].game_mode;

  anim_status_last_before_fading = GAME_MODE_LOADING;
  anim_status_last = GAME_MODE_LOADING;
  anim_classes_last = ANIM_CLASS_NONE;
}

void InitGlobalAnimations()
{
  InitGlobalAnimControls();
}

void DrawGlobalAnimationsExt(int drawing_target, int drawing_stage)
{
  Bitmap *fade_bitmap =
    (drawing_target == DRAW_TO_FADE_SOURCE ? gfx.fade_bitmap_source :
     drawing_target == DRAW_TO_FADE_TARGET ? gfx.fade_bitmap_target : NULL);
  int game_mode_anim_action[NUM_GAME_MODES];
  int mode_nr;

  if (!setup.toons)
    return;

  if (drawing_stage == DRAW_GLOBAL_ANIM_STAGE_1 &&
      drawing_target == DRAW_TO_SCREEN)
    DoAnimationExt();

  // always start with reliable default values (no animation actions)
  for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
    game_mode_anim_action[mode_nr] = ANIM_NO_ACTION;

  if (global.anim_status != anim_status_last)
  {
    boolean before_fading = (global.anim_status == GAME_MODE_PSEUDO_FADING);
    boolean after_fading  = (anim_status_last   == GAME_MODE_PSEUDO_FADING);
    int anim_classes_next = game_mode_anim_classes[global.anim_status_next];
    int i;

    if (drawing_target == DRAW_TO_FADE_TARGET)
      after_fading = TRUE;

    // special case: changing from/to this screen is done without fading
    if (global.anim_status == GAME_MODE_PSEUDO_TYPENAME ||
	anim_status_last   == GAME_MODE_PSEUDO_TYPENAME)
      after_fading = TRUE;

    // ---------- part 1 ------------------------------------------------------
    // start or stop global animations by change of game mode
    // (special handling of animations for "current screen" and "all screens")

    if (global.anim_status_next != anim_status_last_before_fading)
    {
      // stop animations for last screen before fading to new screen
      game_mode_anim_action[anim_status_last] = ANIM_STOP;

      // start animations for current screen after fading to new screen
      game_mode_anim_action[global.anim_status] = ANIM_START;
    }

    // start animations for all screens after loading new artwork set
    if (anim_status_last == GAME_MODE_LOADING)
      game_mode_anim_action[GAME_MODE_DEFAULT] = ANIM_START;

    // ---------- part 2 ------------------------------------------------------
    // start or stop global animations by change of animation class
    // (generic handling of animations for "class of screens")

    for (i = 0; i < NUM_ANIM_CLASSES; i++)
    {
      int anim_class_check = (1 << i);
      int anim_class_game_mode = anim_class_game_modes[i];
      int anim_class_last = anim_classes_last & anim_class_check;
      int anim_class_next = anim_classes_next & anim_class_check;

      // stop animations for changed screen class before fading to new screen
      if (before_fading && anim_class_last && !anim_class_next)
	game_mode_anim_action[anim_class_game_mode] = ANIM_STOP;

      // start animations for changed screen class after fading to new screen
      if (after_fading && !anim_class_last && anim_class_next)
	game_mode_anim_action[anim_class_game_mode] = ANIM_START;
    }

    if (drawing_target == DRAW_TO_SCREEN)
    {
      if (after_fading)
      {
	anim_classes_last = anim_classes_next;
	anim_status_last_before_fading = global.anim_status;
      }

      anim_status_last = global.anim_status;

      // start or stop animations determined to be started or stopped above
      for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
	if (game_mode_anim_action[mode_nr] != ANIM_NO_ACTION)
	  HandleGlobalAnim(game_mode_anim_action[mode_nr], mode_nr);
    }
    else if (drawing_target == DRAW_TO_FADE_TARGET)
    {
      drawing_to_fading_buffer = TRUE;

      // start animations determined to be (temporary) started above
      for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
	if (game_mode_anim_action[mode_nr] == ANIM_START)
	  HandleGlobalAnim(ANIM_START, mode_nr);
    }
  }

  if (global.anim_status == GAME_MODE_LOADING)
    return;

  for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
  {
    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr];
    int anim_nr;

    // when preparing source fading buffer, only draw animations to be stopped
    if (drawing_target == DRAW_TO_FADE_SOURCE &&
	game_mode_anim_action[mode_nr] != ANIM_STOP)
      continue;

    // when preparing target fading buffer, only draw animations to be started
    if (drawing_target == DRAW_TO_FADE_TARGET &&
	game_mode_anim_action[mode_nr] != ANIM_START)
      continue;

#if 0
    if (mode_nr != GFX_SPECIAL_ARG_DEFAULT &&
	mode_nr != game_status)
      continue;
#endif

    for (anim_nr = 0; anim_nr < ctrl->num_anims; anim_nr++)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[anim_nr];
      struct GraphicInfo *c = &anim->control_info;
      int part_first, part_last;
      int part_nr;

      if (!(anim->state & ANIM_STATE_RUNNING))
	continue;

      part_first = part_last = anim->active_part_nr;

      if (c->anim_mode & ANIM_ALL || anim->num_parts == 0)
      {
	int num_parts = anim->num_parts + (anim->has_base ? 1 : 0);

	part_first = 0;
	part_last = num_parts - 1;
      }

      for (part_nr = part_first; part_nr <= part_last; part_nr++)
      {
	struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];
	struct GraphicInfo *g = &part->graphic_info;
	Bitmap *src_bitmap;
	int src_x, src_y;
	int width  = g->width;
	int height = g->height;
	int dst_x = part->x;
	int dst_y = part->y;
	int cut_x = 0;
	int cut_y = 0;
	int sync_frame;
	int frame;
	void (*blit_bitmap)(Bitmap *, Bitmap *, int, int, int, int, int, int) =
	  (g->draw_masked ? BlitBitmapMasked : BlitBitmap);
	void (*blit_screen)(Bitmap *, int, int, int, int, int, int) =
	  (g->draw_masked ? BlitToScreenMasked : BlitToScreen);

	if (!(part->state & ANIM_STATE_RUNNING))
	  continue;

	if (part->drawing_stage != drawing_stage)
	  continue;

	if (part->x < 0)
	{
	  dst_x = 0;
	  width += part->x;
	  cut_x = -part->x;
	}
	else if (part->x > part->viewport_width - g->width)
	  width -= (part->x - (part->viewport_width - g->width));

	if (part->y < 0)
	{
	  dst_y = 0;
	  height += part->y;
	  cut_y = -part->y;
	}
	else if (part->y > part->viewport_height - g->height)
	  height -= (part->y - (part->viewport_height - g->height));

	if (width <= 0 || height <= 0)
	  continue;

	dst_x += part->viewport_x;
	dst_y += part->viewport_y;

	sync_frame = anim_sync_frame - part->initial_anim_sync_frame;
	frame = getAnimationFrame(g->anim_frames, g->anim_delay,
				  g->anim_mode, g->anim_start_frame,
				  sync_frame);

	getFixedGraphicSource(part->graphic, frame, &src_bitmap,
			      &src_x, &src_y);

	src_x += cut_x;
	src_y += cut_y;

	if (drawing_target == DRAW_TO_SCREEN)
	  blit_screen(src_bitmap, src_x, src_y, width, height,
		      dst_x, dst_y);
	else
	  blit_bitmap(src_bitmap, fade_bitmap, src_x, src_y, width, height,
		      dst_x, dst_y);
      }
    }
  }

  if (drawing_target == DRAW_TO_FADE_TARGET)
  {
    // stop animations determined to be (temporary) started above
    for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
      if (game_mode_anim_action[mode_nr] == ANIM_START)
	HandleGlobalAnim(ANIM_STOP, mode_nr);

    drawing_to_fading_buffer = FALSE;
  }
}

void DrawGlobalAnimations(int drawing_target, int drawing_stage)
{
  if (drawing_stage == DRAW_GLOBAL_ANIM_STAGE_1)
    ResetGlobalAnim_Clickable();

  DrawGlobalAnimationsExt(drawing_target, drawing_stage);

  if (drawing_stage == DRAW_GLOBAL_ANIM_STAGE_2)
    ResetGlobalAnim_Clicked();
}

boolean SetGlobalAnimPart_Viewport(struct GlobalAnimPartControlInfo *part)
{
  int viewport_x;
  int viewport_y;
  int viewport_width;
  int viewport_height;
  boolean changed = FALSE;

  if (part->last_anim_status == global.anim_status)
    return FALSE;

  part->last_anim_status = global.anim_status;

  part->drawing_stage = DRAW_GLOBAL_ANIM_STAGE_1;

  if (part->control_info.class == get_hash_from_key("window") ||
      part->control_info.class == get_hash_from_key("border"))
  {
    viewport_x = 0;
    viewport_y = 0;
    viewport_width  = WIN_XSIZE;
    viewport_height = WIN_YSIZE;

    part->drawing_stage = DRAW_GLOBAL_ANIM_STAGE_2;
  }
  else if (part->control_info.class == get_hash_from_key("door_1"))
  {
    viewport_x = DX;
    viewport_y = DY;
    viewport_width  = DXSIZE;
    viewport_height = DYSIZE;
  }
  else if (part->control_info.class == get_hash_from_key("door_2"))
  {
    viewport_x = VX;
    viewport_y = VY;
    viewport_width  = VXSIZE;
    viewport_height = VYSIZE;
  }
  else		// default: "playfield"
  {
    viewport_x = REAL_SX;
    viewport_y = REAL_SY;
    viewport_width  = FULL_SXSIZE;
    viewport_height = FULL_SYSIZE;
  }

  if (viewport_x != part->viewport_x ||
      viewport_y != part->viewport_y ||
      viewport_width  != part->viewport_width ||
      viewport_height != part->viewport_height)
  {
    part->viewport_x = viewport_x;
    part->viewport_y = viewport_y;
    part->viewport_width  = viewport_width;
    part->viewport_height = viewport_height;

    changed = TRUE;
  }

  return changed;
}

static void PlayGlobalAnimSound(struct GlobalAnimPartControlInfo *part)
{
  int sound = part->sound;

  if (sound == SND_UNDEFINED)
    return;

  if ((!setup.sound_simple && !IS_LOOP_SOUND(sound)) ||
      (!setup.sound_loops && IS_LOOP_SOUND(sound)))
    return;

  // !!! TODO: ADD STEREO POSITION FOR MOVING ANIMATIONS !!!
  if (IS_LOOP_SOUND(sound))
    PlaySoundLoop(sound);
  else
    PlaySound(sound);

#if 0
  printf("::: PLAY SOUND %d.%d.%d: %d\n",
	 part->anim_nr, part->nr, part->mode_nr, sound);
#endif
}

static void StopGlobalAnimSound(struct GlobalAnimPartControlInfo *part)
{
  int sound = part->sound;

  if (sound == SND_UNDEFINED)
    return;

  StopSound(sound);

#if 0
  printf("::: STOP SOUND %d.%d.%d: %d\n",
	 part->anim_nr, part->nr, part->mode_nr, sound);
#endif
}

static void PlayGlobalAnimMusic(struct GlobalAnimPartControlInfo *part)
{
  int music = part->music;

  if (music == MUS_UNDEFINED)
    return;

  if (!setup.sound_music)
    return;

  PlayMusic(music);

#if 0
  printf("::: PLAY MUSIC %d.%d.%d: %d\n",
	 part->anim_nr, part->nr, part->mode_nr, music);
#endif
}

static void StopGlobalAnimMusic(struct GlobalAnimPartControlInfo *part)
{
  int music = part->music;

  if (music == MUS_UNDEFINED)
    return;

  StopMusic();

#if 0
  printf("::: STOP MUSIC %d.%d.%d: %d\n",
	 part->anim_nr, part->nr, part->mode_nr, music);
#endif
}

static void PlayGlobalAnimSoundAndMusic(struct GlobalAnimPartControlInfo *part)
{
  // when drawing animations to fading buffer, do not play sounds or music
  if (drawing_to_fading_buffer)
    return;

  PlayGlobalAnimSound(part);
  PlayGlobalAnimMusic(part);
}

static void StopGlobalAnimSoundAndMusic(struct GlobalAnimPartControlInfo *part)
{
  StopGlobalAnimSound(part);
  StopGlobalAnimMusic(part);
}

static boolean isClickablePart(struct GlobalAnimPartControlInfo *part, int mask)
{
  struct GraphicInfo *c = &part->control_info;
  int trigger_mask = ANIM_EVENT_ANIM_MASK | ANIM_EVENT_PART_MASK;
  int mask_anim_only = mask & ANIM_EVENT_ANIM_MASK;

  if (mask & ANIM_EVENT_ANY)
    return (c->init_event & ANIM_EVENT_ANY ||
	    c->anim_event & ANIM_EVENT_ANY);
  else if (mask & ANIM_EVENT_SELF)
    return (c->init_event & ANIM_EVENT_SELF ||
	    c->anim_event & ANIM_EVENT_SELF);
  else
    return ((c->init_event & trigger_mask) == mask ||
	    (c->anim_event & trigger_mask) == mask ||
	    (c->init_event & trigger_mask) == mask_anim_only ||
	    (c->anim_event & trigger_mask) == mask_anim_only);
}

static boolean isClickedPart(struct GlobalAnimPartControlInfo *part,
			     int mx, int my, boolean clicked)
{
  struct GraphicInfo *g = &part->graphic_info;
  int part_x = part->viewport_x + part->x;
  int part_y = part->viewport_y + part->y;
  int part_width  = g->width;
  int part_height = g->height;

  // check if mouse click was detected at all
  if (!clicked)
    return FALSE;

  // check if mouse click is inside the animation part's viewport
  if (mx <  part->viewport_x ||
      mx >= part->viewport_x + part->viewport_width ||
      my <  part->viewport_y ||
      my >= part->viewport_y + part->viewport_height)
    return FALSE;

  // check if mouse click is inside the animation part's graphic
  if (mx <  part_x ||
      mx >= part_x + part_width ||
      my <  part_y ||
      my >= part_y + part_height)
    return FALSE;

  return TRUE;
}

int HandleGlobalAnim_Part(struct GlobalAnimPartControlInfo *part, int state)
{
  struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[part->mode_nr];
  struct GlobalAnimMainControlInfo *anim = &ctrl->anim[part->anim_nr];
  struct GraphicInfo *g = &part->graphic_info;
  struct GraphicInfo *c = &part->control_info;
  boolean viewport_changed = SetGlobalAnimPart_Viewport(part);

  if (viewport_changed)
    state |= ANIM_STATE_RESTART;

  if (state & ANIM_STATE_RESTART)
  {
    // when drawing animations to fading buffer, only start fixed animations
    if (drawing_to_fading_buffer && (c->x == ARG_UNDEFINED_VALUE ||
				     c->y == ARG_UNDEFINED_VALUE))
      return ANIM_STATE_INACTIVE;

    ResetDelayCounterExt(&part->step_delay, anim_sync_frame);

    part->init_delay_counter =
      (c->init_delay_fixed + GetSimpleRandom(c->init_delay_random));

    part->anim_delay_counter =
      (c->anim_delay_fixed + GetSimpleRandom(c->anim_delay_random));

    part->init_event_state = c->init_event;
    part->anim_event_state = c->anim_event;

    part->initial_anim_sync_frame =
      (g->anim_global_sync ? 0 : anim_sync_frame + part->init_delay_counter);

    if (c->direction & MV_HORIZONTAL)
    {
      int pos_bottom = part->viewport_height - g->height;

      if (c->position == POS_TOP)
	part->y = 0;
      else if (c->position == POS_UPPER)
	part->y = GetSimpleRandom(pos_bottom / 2);
      else if (c->position == POS_MIDDLE)
	part->y = pos_bottom / 2;
      else if (c->position == POS_LOWER)
	part->y = pos_bottom / 2 + GetSimpleRandom(pos_bottom / 2);
      else if (c->position == POS_BOTTOM)
	part->y = pos_bottom;
      else
	part->y = GetSimpleRandom(pos_bottom);

      if (c->direction == MV_RIGHT)
      {
	part->step_xoffset = c->step_offset;
	part->x = -g->width + part->step_xoffset;
      }
      else
      {
	part->step_xoffset = -c->step_offset;
	part->x = part->viewport_width + part->step_xoffset;
      }

      part->step_yoffset = 0;
    }
    else if (c->direction & MV_VERTICAL)
    {
      int pos_right = part->viewport_width - g->width;

      if (c->position == POS_LEFT)
	part->x = 0;
      else if (c->position == POS_RIGHT)
	part->x = pos_right;
      else
	part->x = GetSimpleRandom(pos_right);

      if (c->direction == MV_DOWN)
      {
	part->step_yoffset = c->step_offset;
	part->y = -g->height + part->step_yoffset;
      }
      else
      {
	part->step_yoffset = -c->step_offset;
	part->y = part->viewport_height + part->step_yoffset;
      }

      part->step_xoffset = 0;
    }
    else
    {
      part->x = 0;
      part->y = 0;

      part->step_xoffset = 0;
      part->step_yoffset = 0;
    }

    if (c->x != ARG_UNDEFINED_VALUE)
      part->x = c->x;
    if (c->y != ARG_UNDEFINED_VALUE)
      part->y = c->y;

    if (c->position == POS_LAST &&
	anim->last_x > -g->width  && anim->last_x < part->viewport_width &&
	anim->last_y > -g->height && anim->last_y < part->viewport_height)
    {
      part->x = anim->last_x;
      part->y = anim->last_y;
    }

    if (c->step_xoffset != ARG_UNDEFINED_VALUE)
      part->step_xoffset = c->step_xoffset;
    if (c->step_yoffset != ARG_UNDEFINED_VALUE)
      part->step_yoffset = c->step_yoffset;

    if (part->init_delay_counter == 0 &&
	part->init_event_state == ANIM_EVENT_NONE)
      PlayGlobalAnimSoundAndMusic(part);
  }

  if (part->clicked &&
      part->init_event_state != ANIM_EVENT_NONE)
  {
    if (part->initial_anim_sync_frame > 0)
      part->initial_anim_sync_frame -= part->init_delay_counter - 1;

    part->init_delay_counter = 1;
    part->init_event_state = ANIM_EVENT_NONE;

    part->clicked = FALSE;
  }

  if (part->clicked &&
      part->anim_event_state != ANIM_EVENT_NONE)
  {
    part->anim_delay_counter = 1;
    part->anim_event_state = ANIM_EVENT_NONE;

    part->clicked = FALSE;
  }

  if (part->init_delay_counter > 0)
  {
    part->init_delay_counter--;

    if (part->init_delay_counter == 0)
    {
      part->init_event_state = ANIM_EVENT_NONE;

      PlayGlobalAnimSoundAndMusic(part);
    }

    return ANIM_STATE_WAITING;
  }

  if (part->init_event_state != ANIM_EVENT_NONE)
    return ANIM_STATE_WAITING;

  // animation part is now running/visible and therefore clickable
  part->clickable = TRUE;

  // check if moving animation has left the visible screen area
  if ((part->x <= -g->width              && part->step_xoffset <= 0) ||
      (part->x >=  part->viewport_width  && part->step_xoffset >= 0) ||
      (part->y <= -g->height             && part->step_yoffset <= 0) ||
      (part->y >=  part->viewport_height && part->step_yoffset >= 0))
  {
    // do not wait for "anim" events for off-screen animations
    part->anim_event_state = ANIM_EVENT_NONE;

    // do not stop animation before "anim" or "post" counter are finished
    if (part->anim_delay_counter == 0 &&
	part->post_delay_counter == 0)
    {
      StopGlobalAnimSoundAndMusic(part);

      part->post_delay_counter =
	(c->post_delay_fixed + GetSimpleRandom(c->post_delay_random));

      if (part->post_delay_counter > 0)
	return ANIM_STATE_RUNNING;

      // drawing last frame not needed here -- animation not visible anymore
      return ANIM_STATE_RESTART;
    }
  }

  if (part->anim_delay_counter > 0)
  {
    part->anim_delay_counter--;

    if (part->anim_delay_counter == 0)
    {
      part->anim_event_state = ANIM_EVENT_NONE;

      StopGlobalAnimSoundAndMusic(part);

      part->post_delay_counter =
	(c->post_delay_fixed + GetSimpleRandom(c->post_delay_random));

      if (part->post_delay_counter > 0)
	return ANIM_STATE_RUNNING;

      // additional state "RUNNING" required to not skip drawing last frame
      return ANIM_STATE_RESTART | ANIM_STATE_RUNNING;
    }
  }

  if (part->post_delay_counter > 0)
  {
    part->post_delay_counter--;

    if (part->post_delay_counter == 0)
      return ANIM_STATE_RESTART;

    return ANIM_STATE_WAITING;
  }

  if (!DelayReachedExt(&part->step_delay, part->step_delay_value,
		       anim_sync_frame))
    return ANIM_STATE_RUNNING;

#if 0
  {
    static unsigned int last_counter = -1;
    unsigned int counter = Counter();

    printf("::: NEXT ANIM PART [%d, %d]\n",
	   anim_sync_frame, counter - last_counter);

    last_counter = counter;
  }
#endif

  part->x += part->step_xoffset;
  part->y += part->step_yoffset;

  anim->last_x = part->x;
  anim->last_y = part->y;

  return ANIM_STATE_RUNNING;
}

void HandleGlobalAnim_Main(struct GlobalAnimMainControlInfo *anim, int action)
{
  struct GlobalAnimPartControlInfo *part;
  struct GraphicInfo *c = &anim->control_info;
  int state, active_part_nr;

#if 0
  printf("::: HandleGlobalAnim_Main: %d, %d => %d\n",
	 anim->mode_nr, anim->nr, anim->num_parts);
  printf("::: %d, %d, %d\n", global.num_toons, setup.toons, num_toons);
#endif

#if 0
  printf("::: %s(%d): %d, %d, %d [%d]\n",
	 (action == ANIM_START ? "ANIM_START" :
	  action == ANIM_CONTINUE ? "ANIM_CONTINUE" :
	  action == ANIM_STOP ? "ANIM_STOP" : "(should not happen)"),
	 anim->nr,
	 anim->state & ANIM_STATE_RESTART,
	 anim->state & ANIM_STATE_WAITING,
	 anim->state & ANIM_STATE_RUNNING,
	 anim->num_parts);
#endif

  switch (action)
  {
    case ANIM_START:
      anim->state = anim->last_state = ANIM_STATE_RESTART;
      anim->active_part_nr = anim->last_active_part_nr = 0;
      anim->part_counter = 0;

      break;

    case ANIM_CONTINUE:
      if (anim->state == ANIM_STATE_INACTIVE)
	return;

      anim->state = anim->last_state;
      anim->active_part_nr = anim->last_active_part_nr;

      break;

    case ANIM_STOP:
      anim->state = ANIM_STATE_INACTIVE;

      {
	int num_parts = anim->num_parts + (anim->has_base ? 1 : 0);
	int i;

	for (i = 0; i < num_parts; i++)
	  StopGlobalAnimSoundAndMusic(&anim->part[i]);
      }

      return;

    default:
      break;
  }

  if (c->anim_mode & ANIM_ALL || anim->num_parts == 0)
  {
    int num_parts = anim->num_parts + (anim->has_base ? 1 : 0);
    int i;

#if 0
    printf("::: HandleGlobalAnim_Main: %d, %d => %d\n",
	   anim->mode_nr, anim->nr, num_parts);
#endif

    for (i = 0; i < num_parts; i++)
    {
      part = &anim->part[i];

      switch (action)
      {
        case ANIM_START:
	  anim->state = ANIM_STATE_RUNNING;
	  part->state = ANIM_STATE_RESTART;

	  break;

        case ANIM_CONTINUE:
	  if (part->state == ANIM_STATE_INACTIVE)
	    continue;

	  break;

        case ANIM_STOP:
	  part->state = ANIM_STATE_INACTIVE;

	  continue;

        default:
	  break;
      }

      part->state = HandleGlobalAnim_Part(part, part->state);

      // when animation mode is "once", stop after animation was played once
      if (c->anim_mode & ANIM_ONCE &&
	  part->state & ANIM_STATE_RESTART)
	part->state = ANIM_STATE_INACTIVE;
    }

    anim->last_state = anim->state;
    anim->last_active_part_nr = anim->active_part_nr;

    return;
  }

  if (anim->state & ANIM_STATE_RESTART)		// directly after restart
    anim->active_part_nr = getGlobalAnimationPart(anim);

  part = &anim->part[anim->active_part_nr];

  part->state = ANIM_STATE_RUNNING;

  anim->state = HandleGlobalAnim_Part(part, anim->state);

  if (anim->state & ANIM_STATE_RESTART)
    anim->part_counter++;

  // when animation mode is "once", stop after all animations were played once
  if (c->anim_mode & ANIM_ONCE &&
      anim->part_counter == anim->num_parts)
    anim->state = ANIM_STATE_INACTIVE;

  state = anim->state;
  active_part_nr = anim->active_part_nr;

  // while the animation parts are pausing (waiting or inactive), play the base
  // (main) animation; this corresponds to the "boring player animation" logic
  // (!!! KLUDGE WARNING: I THINK THIS IS A MESS THAT SHOULD BE CLEANED UP !!!)
  if (anim->has_base)
  {
    if (anim->state == ANIM_STATE_WAITING ||
	anim->state == ANIM_STATE_INACTIVE)
    {
      anim->active_part_nr = anim->num_parts;	// part nr of base animation
      part = &anim->part[anim->active_part_nr];

      if (anim->state != anim->last_state)
	part->state = ANIM_STATE_RESTART;

      anim->state = ANIM_STATE_RUNNING;
      part->state = HandleGlobalAnim_Part(part, part->state);
    }
  }

  anim->last_state = state;
  anim->last_active_part_nr = active_part_nr;
}

void HandleGlobalAnim_Mode(struct GlobalAnimControlInfo *ctrl, int action)
{
  int i;

#if 0
  printf("::: HandleGlobalAnim_Mode: %d => %d\n",
	 ctrl->nr, ctrl->num_anims);
#endif

  for (i = 0; i < ctrl->num_anims; i++)
    HandleGlobalAnim_Main(&ctrl->anim[i], action);
}

static void HandleGlobalAnim(int action, int game_mode)
{
#if 0
  printf("::: HandleGlobalAnim [mode == %d]\n", game_status);
#endif

  HandleGlobalAnim_Mode(&global_anim_ctrl[game_mode], action);
}

static void DoAnimationExt()
{
  int i;

#if 0
  printf("::: DoAnimation [%d, %d]\n", anim_sync_frame, Counter());
#endif

  // global animations now synchronized with frame delay of screen update
  anim_sync_frame++;

  for (i = 0; i < NUM_GAME_MODES; i++)
    HandleGlobalAnim(ANIM_CONTINUE, i);

#if 0
  // force screen redraw in next frame to continue drawing global animations
  redraw_mask = REDRAW_ALL;
#endif
}

static void InitGlobalAnim_Clickable()
{
  int mode_nr;

  for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
  {
    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr];
    int anim_nr;

    for (anim_nr = 0; anim_nr < ctrl->num_anims; anim_nr++)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[anim_nr];
      int part_nr;

      for (part_nr = 0; part_nr < anim->num_parts_all; part_nr++)
      {
	struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];

	part->clickable = FALSE;
      }
    }
  }
}

static boolean InitGlobalAnim_Clicked(int mx, int my, boolean clicked)
{
  boolean anything_clicked = FALSE;
  boolean any_part_clicked = FALSE;
  int mode_nr;

  for (mode_nr = 0; mode_nr < NUM_GAME_MODES; mode_nr++)
  {
    struct GlobalAnimControlInfo *ctrl = &global_anim_ctrl[mode_nr];
    int anim_nr;

    // check animations in reverse draw order (to stop when clicked)
    for (anim_nr = ctrl->num_anims - 1; anim_nr >= 0; anim_nr--)
    {
      struct GlobalAnimMainControlInfo *anim = &ctrl->anim[anim_nr];
      int part_nr;

      // check animation parts in reverse draw order (to stop when clicked)
      for (part_nr = anim->num_parts_all - 1; part_nr >= 0; part_nr--)
      {
	struct GlobalAnimPartControlInfo *part = &anim->part[part_nr];

	if (!clicked)
	{
	  part->clicked = FALSE;

	  continue;
	}

	if (!part->clickable)
	  continue;

	// always handle "any" click events (clicking anywhere on screen) ...
	if (isClickablePart(part, ANIM_EVENT_ANY))
	  anything_clicked = part->clicked = TRUE;

	// ... but only handle the first (topmost) clickable animation
	if (any_part_clicked)
	  continue;

	if (isClickedPart(part, mx, my, clicked))
	{
#if 0
	  printf("::: %d.%d CLICKED\n", anim_nr, part_nr);
#endif

	  any_part_clicked = TRUE;

	  if (isClickablePart(part, ANIM_EVENT_SELF))
	    anything_clicked = part->clicked = TRUE;

	  // check if this click is defined to trigger other animations
	  int gic_anim_nr = part->old_anim_nr + 1;	// X as in "anim_X"
	  int gic_part_nr = part->old_nr + 1;		// Y as in "part_Y"
	  int mask = gic_anim_nr << ANIM_EVENT_ANIM_BIT;

	  if (!part->is_base)
	    mask |= gic_part_nr << ANIM_EVENT_PART_BIT;

	  int anim2_nr;

	  for (anim2_nr = 0; anim2_nr < ctrl->num_anims; anim2_nr++)
	  {
	    struct GlobalAnimMainControlInfo *anim2 = &ctrl->anim[anim2_nr];
	    int part2_nr;

	    for (part2_nr = 0; part2_nr < anim2->num_parts_all; part2_nr++)
	    {
	      struct GlobalAnimPartControlInfo *part2 = &anim2->part[part2_nr];

	      if (isClickablePart(part2, mask))
		anything_clicked = part2->clicked = TRUE;

#if 0
	      struct GraphicInfo *c = &part2->control_info;

	      printf("::: - %d.%d: 0x%08x, 0x%08x [0x%08x]",
		     anim2_nr, part2_nr, c->init_event, c->anim_event, mask);

	      if (isClickablePart(part2, mask))
		printf(" <--- TRIGGERED BY %d.%d",
		       anim_nr, part_nr);

	      printf("\n");
#endif
	    }
	  }
	}
      }
    }
  }

  return anything_clicked;
}

static void ResetGlobalAnim_Clickable()
{
  InitGlobalAnim_Clickable();
}

static void ResetGlobalAnim_Clicked()
{
  InitGlobalAnim_Clicked(-1, -1, FALSE);
}

boolean HandleGlobalAnimClicks(int mx, int my, int button)
{
  static boolean click_consumed = FALSE;
  static int last_button = 0;
  boolean press_event;
  boolean release_event;
  boolean click_consumed_current = click_consumed;

  /* check if button state has changed since last invocation */
  press_event   = (button != 0 && last_button == 0);
  release_event = (button == 0 && last_button != 0);
  last_button = button;

  if (press_event)
  {
    click_consumed = InitGlobalAnim_Clicked(mx, my, TRUE);
    click_consumed_current = click_consumed;
  }

  if (release_event)
    click_consumed = FALSE;

  return click_consumed_current;
}
