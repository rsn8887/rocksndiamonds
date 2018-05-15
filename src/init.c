// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// init.c
// ============================================================================

#include "libgame/libgame.h"

#include "init.h"
#include "events.h"
#include "screens.h"
#include "editor.h"
#include "game.h"
#include "tape.h"
#include "tools.h"
#include "files.h"
#include "network.h"
#include "netserv.h"
#include "anim.h"
#include "config.h"

#include "conf_e2g.c"	/* include auto-generated data structure definitions */
#include "conf_esg.c"	/* include auto-generated data structure definitions */
#include "conf_e2s.c"	/* include auto-generated data structure definitions */
#include "conf_fnt.c"	/* include auto-generated data structure definitions */
#include "conf_g2s.c"	/* include auto-generated data structure definitions */
#include "conf_g2m.c"	/* include auto-generated data structure definitions */
#include "conf_act.c"	/* include auto-generated data structure definitions */


#define CONFIG_TOKEN_FONT_INITIAL		"font.initial"
#define CONFIG_TOKEN_GLOBAL_BUSY		"global.busy"


static struct FontBitmapInfo font_initial[NUM_INITIAL_FONTS];
static struct GraphicInfo    anim_initial;

static int copy_properties[][5] =
{
  {
    EL_BUG,
    EL_BUG_LEFT,		EL_BUG_RIGHT,
    EL_BUG_UP,			EL_BUG_DOWN
  },
  {
    EL_SPACESHIP,
    EL_SPACESHIP_LEFT,		EL_SPACESHIP_RIGHT,
    EL_SPACESHIP_UP,		EL_SPACESHIP_DOWN
  },
  {
    EL_BD_BUTTERFLY,
    EL_BD_BUTTERFLY_LEFT,	EL_BD_BUTTERFLY_RIGHT,
    EL_BD_BUTTERFLY_UP,		EL_BD_BUTTERFLY_DOWN
  },
  {
    EL_BD_FIREFLY,
    EL_BD_FIREFLY_LEFT,		EL_BD_FIREFLY_RIGHT,
    EL_BD_FIREFLY_UP,		EL_BD_FIREFLY_DOWN
  },
  {
    EL_PACMAN,
    EL_PACMAN_LEFT,		EL_PACMAN_RIGHT,
    EL_PACMAN_UP,		EL_PACMAN_DOWN
  },
  {
    EL_YAMYAM,
    EL_YAMYAM_LEFT,		EL_YAMYAM_RIGHT,
    EL_YAMYAM_UP,		EL_YAMYAM_DOWN
  },
  {
    EL_MOLE,
    EL_MOLE_LEFT,		EL_MOLE_RIGHT,
    EL_MOLE_UP,			EL_MOLE_DOWN
  },
  {
    -1,
    -1, -1, -1, -1
  }
};


/* forward declaration for internal use */
static int get_graphic_parameter_value(char *, char *, int);


void DrawInitAnim()
{
  struct GraphicInfo *graphic_info_last = graphic_info;
  int graphic = 0;
  static unsigned int action_delay = 0;
  unsigned int action_delay_value = GameFrameDelay;
  int sync_frame = FrameCounter;
  int x, y;

  /* prevent OS (Windows) from complaining about program not responding */
  CheckQuitEvent();

  if (game_status != GAME_MODE_LOADING)
    return;

  if (anim_initial.bitmap == NULL || window == NULL)
    return;

  if (!DelayReached(&action_delay, action_delay_value))
    return;

  if (init_last.busy.x == -1)
    init_last.busy.x = WIN_XSIZE / 2;
  if (init_last.busy.y == -1)
    init_last.busy.y = WIN_YSIZE / 2;

  x = ALIGNED_TEXT_XPOS(&init_last.busy);
  y = ALIGNED_TEXT_YPOS(&init_last.busy);

  graphic_info = &anim_initial;		/* graphic == 0 => anim_initial */

  if (sync_frame % anim_initial.anim_delay == 0)
  {
    Bitmap *src_bitmap;
    int src_x, src_y;
    int width = graphic_info[graphic].width;
    int height = graphic_info[graphic].height;
    int frame = getGraphicAnimationFrame(graphic, sync_frame);

    getFixedGraphicSource(graphic, frame, &src_bitmap, &src_x, &src_y);
    BlitBitmap(src_bitmap, window, src_x, src_y, width, height, x, y);
  }

  graphic_info = graphic_info_last;

  FrameCounter++;
}

void FreeGadgets()
{
  FreeLevelEditorGadgets();
  FreeGameButtons();
  FreeTapeButtons();
  FreeToolButtons();
  FreeScreenGadgets();
}

void InitGadgets()
{
  static boolean gadgets_initialized = FALSE;

  if (gadgets_initialized)
    FreeGadgets();

  CreateLevelEditorGadgets();
  CreateGameButtons();
  CreateTapeButtons();
  CreateToolButtons();
  CreateScreenGadgets();

  InitGadgetsSoundCallback(PlaySoundActivating, PlaySoundSelecting);

  gadgets_initialized = TRUE;
}

inline static void InitElementSmallImagesScaledUp(int graphic)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  // create small and game tile sized bitmaps (and scale up, if needed)
  CreateImageWithSmallImages(graphic, g->scale_up_factor, g->tile_size);
}

void InitElementSmallImages()
{
  print_timestamp_init("InitElementSmallImages");

  static int special_graphics[] =
  {
    IMG_FLAMES_1_LEFT,
    IMG_FLAMES_2_LEFT,
    IMG_FLAMES_3_LEFT,
    IMG_FLAMES_1_RIGHT,
    IMG_FLAMES_2_RIGHT,
    IMG_FLAMES_3_RIGHT,
    IMG_FLAMES_1_UP,
    IMG_FLAMES_2_UP,
    IMG_FLAMES_3_UP,
    IMG_FLAMES_1_DOWN,
    IMG_FLAMES_2_DOWN,
    IMG_FLAMES_3_DOWN,
    IMG_EDITOR_ELEMENT_BORDER,
    IMG_EDITOR_ELEMENT_BORDER_INPUT,
    IMG_EDITOR_CASCADE_LIST,
    IMG_EDITOR_CASCADE_LIST_ACTIVE,
    -1
  };
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i;

  print_timestamp_time("getImageListPropertyMapping/Size");

  print_timestamp_init("InitElementSmallImagesScaledUp (1)");
  /* initialize normal element images from static configuration */
  for (i = 0; element_to_graphic[i].element > -1; i++)
    InitElementSmallImagesScaledUp(element_to_graphic[i].graphic);
  print_timestamp_done("InitElementSmallImagesScaledUp (1)");

  /* initialize special element images from static configuration */
  for (i = 0; element_to_special_graphic[i].element > -1; i++)
    InitElementSmallImagesScaledUp(element_to_special_graphic[i].graphic);
  print_timestamp_time("InitElementSmallImagesScaledUp (2)");

  /* initialize element images from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
    if (property_mapping[i].base_index < MAX_NUM_ELEMENTS)
      InitElementSmallImagesScaledUp(property_mapping[i].artwork_index);
  print_timestamp_time("InitElementSmallImagesScaledUp (3)");

  /* initialize special non-element images from above list */
  for (i = 0; special_graphics[i] > -1; i++)
    InitElementSmallImagesScaledUp(special_graphics[i]);
  print_timestamp_time("InitElementSmallImagesScaledUp (4)");

  print_timestamp_done("InitElementSmallImages");
}

inline static void InitScaledImagesScaledUp(int graphic)
{
  struct GraphicInfo *g = &graphic_info[graphic];

  ScaleImage(graphic, g->scale_up_factor);
}

void InitScaledImages()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i;

  /* scale normal images from static configuration, if not already scaled */
  for (i = 0; i < NUM_IMAGE_FILES; i++)
    InitScaledImagesScaledUp(i);

  /* scale images from dynamic configuration, if not already scaled */
  for (i = 0; i < num_property_mappings; i++)
    InitScaledImagesScaledUp(property_mapping[i].artwork_index);
}

void InitBitmapPointers()
{
  int num_images = getImageListSize();
  int i;

  // standard size bitmap may have changed -- update default bitmap pointer
  for (i = 0; i < num_images; i++)
    if (graphic_info[i].bitmaps)
      graphic_info[i].bitmap = graphic_info[i].bitmaps[IMG_BITMAP_STANDARD];
}

void InitImageTextures()
{
  int i, j, k;

  FreeAllImageTextures();

  for (i = IMG_GLOBAL_BORDER_FIRST; i <= IMG_GLOBAL_BORDER_LAST; i++)
    CreateImageTextures(i);

  for (i = 0; i < MAX_NUM_TOONS; i++)
    CreateImageTextures(IMG_TOON_1 + i);

  for (i = 0; i < NUM_GLOBAL_ANIMS; i++)
  {
    for (j = 0; j < NUM_GLOBAL_ANIM_PARTS_ALL; j++)
    {
      for (k = 0; k < NUM_SPECIAL_GFX_ARGS; k++)
      {
	int graphic = global_anim_info[i].graphic[j][k];

	if (graphic == IMG_UNDEFINED)
	  continue;

	CreateImageTextures(graphic);
      }
    }
  }
}

#if 1
/* !!! FIX THIS (CHANGE TO USING NORMAL ELEMENT GRAPHIC DEFINITIONS) !!! */
void SetBitmaps_EM(Bitmap **em_bitmap)
{
  em_bitmap[0] = graphic_info[IMG_EMC_OBJECT].bitmap;
  em_bitmap[1] = graphic_info[IMG_EMC_SPRITE].bitmap;
}
#endif

#if 0
/* !!! FIX THIS (CHANGE TO USING NORMAL ELEMENT GRAPHIC DEFINITIONS) !!! */
void SetBitmaps_SP(Bitmap **sp_bitmap)
{
  *sp_bitmap = graphic_info[IMG_SP_OBJECTS].bitmap;
}
#endif

static int getFontBitmapID(int font_nr)
{
  int special = -1;

  /* (special case: do not use special font for GAME_MODE_LOADING) */
  if (game_status >= GAME_MODE_TITLE_INITIAL &&
      game_status <= GAME_MODE_PSEUDO_PREVIEW)
    special = game_status;
  else if (game_status == GAME_MODE_PSEUDO_TYPENAME)
    special = GFX_SPECIAL_ARG_MAIN;

  if (special != -1)
    return font_info[font_nr].special_bitmap_id[special];
  else
    return font_nr;
}

static int getFontFromToken(char *token)
{
  char *value = getHashEntry(font_token_hash, token);

  if (value != NULL)
    return atoi(value);

  /* if font not found, use reliable default value */
  return FONT_INITIAL_1;
}

void InitFontGraphicInfo()
{
  static struct FontBitmapInfo *font_bitmap_info = NULL;
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int num_font_bitmaps = NUM_FONTS;
  int i, j;

  if (graphic_info == NULL)		/* still at startup phase */
  {
    InitFontInfo(font_initial, NUM_INITIAL_FONTS,
		 getFontBitmapID, getFontFromToken);

    return;
  }

  /* ---------- initialize font graphic definitions ---------- */

  /* always start with reliable default values (normal font graphics) */
  for (i = 0; i < NUM_FONTS; i++)
    font_info[i].graphic = IMG_FONT_INITIAL_1;

  /* initialize normal font/graphic mapping from static configuration */
  for (i = 0; font_to_graphic[i].font_nr > -1; i++)
  {
    int font_nr = font_to_graphic[i].font_nr;
    int special = font_to_graphic[i].special;
    int graphic = font_to_graphic[i].graphic;

    if (special != -1)
      continue;

    font_info[font_nr].graphic = graphic;
  }

  /* always start with reliable default values (special font graphics) */
  for (i = 0; i < NUM_FONTS; i++)
  {
    for (j = 0; j < NUM_SPECIAL_GFX_ARGS; j++)
    {
      font_info[i].special_graphic[j] = font_info[i].graphic;
      font_info[i].special_bitmap_id[j] = i;
    }
  }

  /* initialize special font/graphic mapping from static configuration */
  for (i = 0; font_to_graphic[i].font_nr > -1; i++)
  {
    int font_nr      = font_to_graphic[i].font_nr;
    int special      = font_to_graphic[i].special;
    int graphic      = font_to_graphic[i].graphic;
    int base_graphic = font2baseimg(font_nr);

    if (IS_SPECIAL_GFX_ARG(special))
    {
      boolean base_redefined =
	getImageListEntryFromImageID(base_graphic)->redefined;
      boolean special_redefined =
	getImageListEntryFromImageID(graphic)->redefined;
      boolean special_cloned = (graphic_info[graphic].clone_from != -1);

      /* if the base font ("font.title_1", for example) has been redefined,
      	 but not the special font ("font.title_1.LEVELS", for example), do not
	 use an existing (in this case considered obsolete) special font
	 anymore, but use the automatically determined default font */
      /* special case: cloned special fonts must be explicitly redefined,
	 but are not automatically redefined by redefining base font */
      if (base_redefined && !special_redefined && !special_cloned)
	continue;

      font_info[font_nr].special_graphic[special] = graphic;
      font_info[font_nr].special_bitmap_id[special] = num_font_bitmaps;
      num_font_bitmaps++;
    }
  }

  /* initialize special font/graphic mapping from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int font_nr = property_mapping[i].base_index - MAX_NUM_ELEMENTS;
    int special = property_mapping[i].ext3_index;
    int graphic = property_mapping[i].artwork_index;

    if (font_nr < 0 || font_nr >= NUM_FONTS)
      continue;

    if (IS_SPECIAL_GFX_ARG(special))
    {
      font_info[font_nr].special_graphic[special] = graphic;
      font_info[font_nr].special_bitmap_id[special] = num_font_bitmaps;
      num_font_bitmaps++;
    }
  }

  /* correct special font/graphic mapping for cloned fonts for downwards
     compatibility of PREVIEW fonts -- this is only needed for implicit
     redefinition of special font by redefined base font, and only if other
     fonts are cloned from this special font (like in the "Zelda" level set) */
  for (i = 0; font_to_graphic[i].font_nr > -1; i++)
  {
    int font_nr = font_to_graphic[i].font_nr;
    int special = font_to_graphic[i].special;
    int graphic = font_to_graphic[i].graphic;

    if (IS_SPECIAL_GFX_ARG(special))
    {
      boolean special_redefined =
	getImageListEntryFromImageID(graphic)->redefined;
      boolean special_cloned = (graphic_info[graphic].clone_from != -1);

      if (special_cloned && !special_redefined)
      {
	int j;

	for (j = 0; font_to_graphic[j].font_nr > -1; j++)
	{
	  int font_nr2 = font_to_graphic[j].font_nr;
	  int special2 = font_to_graphic[j].special;
	  int graphic2 = font_to_graphic[j].graphic;

	  if (IS_SPECIAL_GFX_ARG(special2) &&
	      graphic2 == graphic_info[graphic].clone_from)
	  {
	    font_info[font_nr].special_graphic[special] =
	      font_info[font_nr2].special_graphic[special2];
	    font_info[font_nr].special_bitmap_id[special] =
	      font_info[font_nr2].special_bitmap_id[special2];
	  }
	}
      }
    }
  }

  /* reset non-redefined ".active" font graphics if normal font is redefined */
  /* (this different treatment is needed because normal and active fonts are
     independently defined ("active" is not a property of font definitions!) */
  for (i = 0; i < NUM_FONTS; i++)
  {
    int font_nr_base = i;
    int font_nr_active = FONT_ACTIVE(font_nr_base);

    /* check only those fonts with exist as normal and ".active" variant */
    if (font_nr_base != font_nr_active)
    {
      int base_graphic = font_info[font_nr_base].graphic;
      int active_graphic = font_info[font_nr_active].graphic;
      boolean base_redefined =
	getImageListEntryFromImageID(base_graphic)->redefined;
      boolean active_redefined =
	getImageListEntryFromImageID(active_graphic)->redefined;

      /* if the base font ("font.menu_1", for example) has been redefined,
	 but not the active font ("font.menu_1.active", for example), do not
	 use an existing (in this case considered obsolete) active font
	 anymore, but use the automatically determined default font */
      if (base_redefined && !active_redefined)
	font_info[font_nr_active].graphic = base_graphic;

      /* now also check each "special" font (which may be the same as above) */
      for (j = 0; j < NUM_SPECIAL_GFX_ARGS; j++)
      {
	int base_graphic = font_info[font_nr_base].special_graphic[j];
	int active_graphic = font_info[font_nr_active].special_graphic[j];
	boolean base_redefined =
	  getImageListEntryFromImageID(base_graphic)->redefined;
	boolean active_redefined =
	  getImageListEntryFromImageID(active_graphic)->redefined;

	/* same as above, but check special graphic definitions, for example:
	   redefined "font.menu_1.MAIN" invalidates "font.menu_1.active.MAIN" */
	if (base_redefined && !active_redefined)
	{
	  font_info[font_nr_active].special_graphic[j] =
	    font_info[font_nr_base].special_graphic[j];
	  font_info[font_nr_active].special_bitmap_id[j] =
	    font_info[font_nr_base].special_bitmap_id[j];
	}
      }
    }
  }

  /* ---------- initialize font bitmap array ---------- */

  if (font_bitmap_info != NULL)
    FreeFontInfo(font_bitmap_info);

  font_bitmap_info =
    checked_calloc(num_font_bitmaps * sizeof(struct FontBitmapInfo));

  /* ---------- initialize font bitmap definitions ---------- */

  for (i = 0; i < NUM_FONTS; i++)
  {
    if (i < NUM_INITIAL_FONTS)
    {
      font_bitmap_info[i] = font_initial[i];
      continue;
    }

    for (j = 0; j < NUM_SPECIAL_GFX_ARGS; j++)
    {
      int font_bitmap_id = font_info[i].special_bitmap_id[j];
      int graphic = font_info[i].special_graphic[j];

      /* set 'graphic_info' for font entries, if uninitialized (guessed) */
      if (graphic_info[graphic].anim_frames < MIN_NUM_CHARS_PER_FONT)
      {
	graphic_info[graphic].anim_frames = DEFAULT_NUM_CHARS_PER_FONT;
	graphic_info[graphic].anim_frames_per_line = DEFAULT_NUM_CHARS_PER_LINE;
      }

      /* copy font relevant information from graphics information */
      font_bitmap_info[font_bitmap_id].bitmap = graphic_info[graphic].bitmap;
      font_bitmap_info[font_bitmap_id].src_x  = graphic_info[graphic].src_x;
      font_bitmap_info[font_bitmap_id].src_y  = graphic_info[graphic].src_y;
      font_bitmap_info[font_bitmap_id].width  = graphic_info[graphic].width;
      font_bitmap_info[font_bitmap_id].height = graphic_info[graphic].height;

      font_bitmap_info[font_bitmap_id].offset_x =
	graphic_info[graphic].offset_x;
      font_bitmap_info[font_bitmap_id].offset_y =
	graphic_info[graphic].offset_y;

      font_bitmap_info[font_bitmap_id].draw_xoffset =
	graphic_info[graphic].draw_xoffset;
      font_bitmap_info[font_bitmap_id].draw_yoffset =
	graphic_info[graphic].draw_yoffset;

      font_bitmap_info[font_bitmap_id].num_chars =
	graphic_info[graphic].anim_frames;
      font_bitmap_info[font_bitmap_id].num_chars_per_line =
	graphic_info[graphic].anim_frames_per_line;
    }
  }

  InitFontInfo(font_bitmap_info, num_font_bitmaps,
	       getFontBitmapID, getFontFromToken);
}

void InitGlobalAnimGraphicInfo()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i, j, k;

  if (graphic_info == NULL)		/* still at startup phase */
    return;

  /* always start with reliable default values (no global animations) */
  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS; i++)
    for (j = 0; j < NUM_GLOBAL_ANIM_PARTS_ALL; j++)
      for (k = 0; k < NUM_SPECIAL_GFX_ARGS; k++)
	global_anim_info[i].graphic[j][k] = IMG_UNDEFINED;

  /* initialize global animation definitions from static configuration */
  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS; i++)
  {
    int j = GLOBAL_ANIM_ID_PART_BASE;
    int k = GFX_SPECIAL_ARG_DEFAULT;

    global_anim_info[i].graphic[j][k] = IMG_GFX_GLOBAL_ANIM_1 + i;
  }

  /* initialize global animation definitions from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int anim_nr = property_mapping[i].base_index - MAX_NUM_ELEMENTS - NUM_FONTS;
    int part_nr = property_mapping[i].ext1_index - ACTION_PART_1;
    int special = property_mapping[i].ext3_index;
    int graphic = property_mapping[i].artwork_index;

    if (anim_nr < 0 || anim_nr >= NUM_GLOBAL_ANIM_TOKENS)
      continue;

    /* set animation part to base part, if not specified */
    if (!IS_GLOBAL_ANIM_PART(part_nr))
      part_nr = GLOBAL_ANIM_ID_PART_BASE;

    /* set animation screen to default, if not specified */
    if (!IS_SPECIAL_GFX_ARG(special))
      special = GFX_SPECIAL_ARG_DEFAULT;

    global_anim_info[anim_nr].graphic[part_nr][special] = graphic;

    /* fix default value for ".draw_masked" (for backward compatibility) */
    struct GraphicInfo *g = &graphic_info[graphic];
    struct FileInfo *image = getImageListEntryFromImageID(graphic);
    char **parameter_raw = image->parameter;
    int p = GFX_ARG_DRAW_MASKED;
    int draw_masked = get_graphic_parameter_value(parameter_raw[p],
						  image_config_suffix[p].token,
						  image_config_suffix[p].type);

    /* if ".draw_masked" parameter is undefined, use default value "TRUE" */
    if (draw_masked == ARG_UNDEFINED_VALUE)
      g->draw_masked = TRUE;
  }

#if 0
  printf("::: InitGlobalAnimGraphicInfo\n");

  for (i = 0; i < NUM_GLOBAL_ANIMS; i++)
    for (j = 0; j < NUM_GLOBAL_ANIM_PARTS_ALL; j++)
      for (k = 0; k < NUM_SPECIAL_GFX_ARGS; k++)
	if (global_anim_info[i].graphic[j][k] != IMG_UNDEFINED &&
	    graphic_info[global_anim_info[i].graphic[j][k]].bitmap != NULL)
	  printf("::: - anim %d, part %d, mode %d => %d\n",
		 i, j, k, global_anim_info[i].graphic[j][k]);
#endif
}

void InitGlobalAnimSoundInfo()
{
  struct PropertyMapping *property_mapping = getSoundListPropertyMapping();
  int num_property_mappings = getSoundListPropertyMappingSize();
  int i, j, k;

  /* always start with reliable default values (no global animation sounds) */
  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS; i++)
    for (j = 0; j < NUM_GLOBAL_ANIM_PARTS_ALL; j++)
      for (k = 0; k < NUM_SPECIAL_GFX_ARGS; k++)
	global_anim_info[i].sound[j][k] = SND_UNDEFINED;

  /* initialize global animation sound definitions from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int anim_nr = property_mapping[i].base_index - 2 * MAX_NUM_ELEMENTS;
    int part_nr = property_mapping[i].ext1_index - ACTION_PART_1;
    int special = property_mapping[i].ext3_index;
    int sound   = property_mapping[i].artwork_index;

    // sound uses control definition; map it to position of graphic (artwork)
    anim_nr -= GLOBAL_ANIM_ID_CONTROL_FIRST;

    if (anim_nr < 0 || anim_nr >= NUM_GLOBAL_ANIM_TOKENS)
      continue;

    /* set animation part to base part, if not specified */
    if (!IS_GLOBAL_ANIM_PART(part_nr))
      part_nr = GLOBAL_ANIM_ID_PART_BASE;

    /* set animation screen to default, if not specified */
    if (!IS_SPECIAL_GFX_ARG(special))
      special = GFX_SPECIAL_ARG_DEFAULT;

    global_anim_info[anim_nr].sound[part_nr][special] = sound;
  }

#if 0
  printf("::: InitGlobalAnimSoundInfo\n");

  for (i = 0; i < NUM_GLOBAL_ANIMS; i++)
    for (j = 0; j < NUM_GLOBAL_ANIM_PARTS_ALL; j++)
      for (k = 0; k < NUM_SPECIAL_GFX_ARGS; k++)
	if (global_anim_info[i].sound[j][k] != SND_UNDEFINED)
	  printf("::: - anim %d, part %d, mode %d => %d\n",
		 i, j, k, global_anim_info[i].sound[j][k]);
#endif
}

void InitGlobalAnimMusicInfo()
{
  struct PropertyMapping *property_mapping = getMusicListPropertyMapping();
  int num_property_mappings = getMusicListPropertyMappingSize();
  int i, j, k;

  /* always start with reliable default values (no global animation music) */
  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS; i++)
    for (j = 0; j < NUM_GLOBAL_ANIM_PARTS_ALL; j++)
      for (k = 0; k < NUM_SPECIAL_GFX_ARGS; k++)
	global_anim_info[i].music[j][k] = MUS_UNDEFINED;

  /* initialize global animation music definitions from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int anim_nr = property_mapping[i].base_index - NUM_MUSIC_PREFIXES;
    int part_nr = property_mapping[i].ext1_index - ACTION_PART_1;
    int special = property_mapping[i].ext2_index;
    int music   = property_mapping[i].artwork_index;

    // music uses control definition; map it to position of graphic (artwork)
    anim_nr -= GLOBAL_ANIM_ID_CONTROL_FIRST;

    if (anim_nr < 0 || anim_nr >= NUM_GLOBAL_ANIM_TOKENS)
      continue;

    /* set animation part to base part, if not specified */
    if (!IS_GLOBAL_ANIM_PART(part_nr))
      part_nr = GLOBAL_ANIM_ID_PART_BASE;

    /* set animation screen to default, if not specified */
    if (!IS_SPECIAL_GFX_ARG(special))
      special = GFX_SPECIAL_ARG_DEFAULT;

    global_anim_info[anim_nr].music[part_nr][special] = music;
  }

#if 0
  printf("::: InitGlobalAnimMusicInfo\n");

  for (i = 0; i < NUM_GLOBAL_ANIMS; i++)
    for (j = 0; j < NUM_GLOBAL_ANIM_PARTS_ALL; j++)
      for (k = 0; k < NUM_SPECIAL_GFX_ARGS; k++)
	if (global_anim_info[i].music[j][k] != MUS_UNDEFINED)
	  printf("::: - anim %d, part %d, mode %d => %d\n",
		 i, j, k, global_anim_info[i].music[j][k]);
#endif
}

void InitElementGraphicInfo()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i, act, dir;

  if (graphic_info == NULL)		/* still at startup phase */
    return;

  /* set values to -1 to identify later as "uninitialized" values */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    for (act = 0; act < NUM_ACTIONS; act++)
    {
      element_info[i].graphic[act] = -1;
      element_info[i].crumbled[act] = -1;

      for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
      {
	element_info[i].direction_graphic[act][dir] = -1;
	element_info[i].direction_crumbled[act][dir] = -1;
      }
    }
  }

  UPDATE_BUSY_STATE();

  /* initialize normal element/graphic mapping from static configuration */
  for (i = 0; element_to_graphic[i].element > -1; i++)
  {
    int element      = element_to_graphic[i].element;
    int action       = element_to_graphic[i].action;
    int direction    = element_to_graphic[i].direction;
    boolean crumbled = element_to_graphic[i].crumbled;
    int graphic      = element_to_graphic[i].graphic;
    int base_graphic = el2baseimg(element);

    if (graphic_info[graphic].bitmap == NULL)
      continue;

    if ((action > -1 || direction > -1 || crumbled == TRUE) &&
	base_graphic != -1)
    {
      boolean base_redefined =
	getImageListEntryFromImageID(base_graphic)->redefined;
      boolean act_dir_redefined =
	getImageListEntryFromImageID(graphic)->redefined;

      /* if the base graphic ("emerald", for example) has been redefined,
      	 but not the action graphic ("emerald.falling", for example), do not
	 use an existing (in this case considered obsolete) action graphic
	 anymore, but use the automatically determined default graphic */
      if (base_redefined && !act_dir_redefined)
	continue;
    }

    if (action < 0)
      action = ACTION_DEFAULT;

    if (crumbled)
    {
      if (direction > -1)
	element_info[element].direction_crumbled[action][direction] = graphic;
      else
	element_info[element].crumbled[action] = graphic;
    }
    else
    {
      if (direction > -1)
	element_info[element].direction_graphic[action][direction] = graphic;
      else
	element_info[element].graphic[action] = graphic;
    }
  }

  /* initialize normal element/graphic mapping from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int element   = property_mapping[i].base_index;
    int action    = property_mapping[i].ext1_index;
    int direction = property_mapping[i].ext2_index;
    int special   = property_mapping[i].ext3_index;
    int graphic   = property_mapping[i].artwork_index;
    boolean crumbled = FALSE;

    if (special == GFX_SPECIAL_ARG_CRUMBLED)
    {
      special = -1;
      crumbled = TRUE;
    }

    if (graphic_info[graphic].bitmap == NULL)
      continue;

    if (element >= MAX_NUM_ELEMENTS || special != -1)
      continue;

    if (action < 0)
      action = ACTION_DEFAULT;

    if (crumbled)
    {
      if (direction < 0)
	for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
	  element_info[element].direction_crumbled[action][dir] = -1;

      if (direction > -1)
	element_info[element].direction_crumbled[action][direction] = graphic;
      else
	element_info[element].crumbled[action] = graphic;
    }
    else
    {
      if (direction < 0)
	for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
	  element_info[element].direction_graphic[action][dir] = -1;

      if (direction > -1)
	element_info[element].direction_graphic[action][direction] = graphic;
      else
	element_info[element].graphic[action] = graphic;
    }
  }

  /* now copy all graphics that are defined to be cloned from other graphics */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    int graphic = element_info[i].graphic[ACTION_DEFAULT];
    int crumbled_like, diggable_like;

    if (graphic == -1)
      continue;

    crumbled_like = graphic_info[graphic].crumbled_like;
    diggable_like = graphic_info[graphic].diggable_like;

    if (crumbled_like != -1 && element_info[i].crumbled[ACTION_DEFAULT] == -1)
    {
      for (act = 0; act < NUM_ACTIONS; act++)
	element_info[i].crumbled[act] =
	  element_info[crumbled_like].crumbled[act];
      for (act = 0; act < NUM_ACTIONS; act++)
	for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
	  element_info[i].direction_crumbled[act][dir] =
	    element_info[crumbled_like].direction_crumbled[act][dir];
    }

    if (diggable_like != -1 && element_info[i].graphic[ACTION_DIGGING] == -1)
    {
      element_info[i].graphic[ACTION_DIGGING] =
	element_info[diggable_like].graphic[ACTION_DIGGING];
      for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
	element_info[i].direction_graphic[ACTION_DIGGING][dir] =
	  element_info[diggable_like].direction_graphic[ACTION_DIGGING][dir];
    }
  }

  /* set hardcoded definitions for some runtime elements without graphic */
  element_info[EL_AMOEBA_TO_DIAMOND].graphic[ACTION_DEFAULT] = IMG_AMOEBA_DEAD;

  /* set hardcoded definitions for some internal elements without graphic */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    if (IS_EDITOR_CASCADE_INACTIVE(i))
      element_info[i].graphic[ACTION_DEFAULT] = IMG_EDITOR_CASCADE_LIST;
    else if (IS_EDITOR_CASCADE_ACTIVE(i))
      element_info[i].graphic[ACTION_DEFAULT] = IMG_EDITOR_CASCADE_LIST_ACTIVE;
  }

  /* now set all undefined/invalid graphics to -1 to set to default after it */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    for (act = 0; act < NUM_ACTIONS; act++)
    {
      int graphic;

      graphic = element_info[i].graphic[act];
      if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	element_info[i].graphic[act] = -1;

      graphic = element_info[i].crumbled[act];
      if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	element_info[i].crumbled[act] = -1;

      for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
      {
	graphic = element_info[i].direction_graphic[act][dir];
	if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	  element_info[i].direction_graphic[act][dir] = -1;

	graphic = element_info[i].direction_crumbled[act][dir];
	if (graphic > 0 && graphic_info[graphic].bitmap == NULL)
	  element_info[i].direction_crumbled[act][dir] = -1;
      }
    }
  }

  UPDATE_BUSY_STATE();

  /* adjust graphics with 2nd tile for movement according to direction
     (do this before correcting '-1' values to minimize calculations) */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    for (act = 0; act < NUM_ACTIONS; act++)
    {
      for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
      {
	int graphic = element_info[i].direction_graphic[act][dir];
	int move_dir = (act == ACTION_FALLING ? MV_BIT_DOWN : dir);

	if (act == ACTION_FALLING)	/* special case */
	  graphic = element_info[i].graphic[act];

	if (graphic != -1 &&
	    graphic_info[graphic].double_movement &&
	    graphic_info[graphic].swap_double_tiles != 0)
	{
	  struct GraphicInfo *g = &graphic_info[graphic];
	  int src_x_front = g->src_x;
	  int src_y_front = g->src_y;
	  int src_x_back = g->src_x + g->offset2_x;
	  int src_y_back = g->src_y + g->offset2_y;
	  boolean frames_are_ordered_diagonally = (g->offset_x != 0 &&
						   g->offset_y != 0);
	  boolean front_is_left_or_upper = (src_x_front < src_x_back ||
					    src_y_front < src_y_back);
	  boolean swap_movement_tiles_always = (g->swap_double_tiles == 1);
	  boolean swap_movement_tiles_autodetected =
	    (!frames_are_ordered_diagonally &&
	     ((move_dir == MV_BIT_LEFT  && !front_is_left_or_upper) ||
	      (move_dir == MV_BIT_UP    && !front_is_left_or_upper) ||
	      (move_dir == MV_BIT_RIGHT &&  front_is_left_or_upper) ||
	      (move_dir == MV_BIT_DOWN  &&  front_is_left_or_upper)));

	  /* swap frontside and backside graphic tile coordinates, if needed */
	  if (swap_movement_tiles_always || swap_movement_tiles_autodetected)
	  {
	    /* get current (wrong) backside tile coordinates */
	    getGraphicSourceXY(graphic, 0, &src_x_back, &src_y_back, TRUE);

	    /* set frontside tile coordinates to backside tile coordinates */
	    g->src_x = src_x_back;
	    g->src_y = src_y_back;

	    /* invert tile offset to point to new backside tile coordinates */
	    g->offset2_x *= -1;
	    g->offset2_y *= -1;

	    /* do not swap front and backside tiles again after correction */
	    g->swap_double_tiles = 0;
	  }
	}
      }
    }
  }

  UPDATE_BUSY_STATE();

  /* now set all '-1' values to element specific default values */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    int default_graphic = element_info[i].graphic[ACTION_DEFAULT];
    int default_crumbled = element_info[i].crumbled[ACTION_DEFAULT];
    int default_direction_graphic[NUM_DIRECTIONS_FULL];
    int default_direction_crumbled[NUM_DIRECTIONS_FULL];

    if (default_graphic == -1)
      default_graphic = IMG_UNKNOWN;

    if (default_crumbled == -1)
      default_crumbled = default_graphic;

    for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
    {
      default_direction_graphic[dir] =
	element_info[i].direction_graphic[ACTION_DEFAULT][dir];
      default_direction_crumbled[dir] =
	element_info[i].direction_crumbled[ACTION_DEFAULT][dir];

      if (default_direction_graphic[dir] == -1)
	default_direction_graphic[dir] = default_graphic;

      if (default_direction_crumbled[dir] == -1)
	default_direction_crumbled[dir] = default_direction_graphic[dir];
    }

    for (act = 0; act < NUM_ACTIONS; act++)
    {
      boolean act_remove = ((IS_DIGGABLE(i)    && act == ACTION_DIGGING)  ||
			    (IS_SNAPPABLE(i)   && act == ACTION_SNAPPING) ||
			    (IS_COLLECTIBLE(i) && act == ACTION_COLLECTING));
      boolean act_turning = (act == ACTION_TURNING_FROM_LEFT ||
			     act == ACTION_TURNING_FROM_RIGHT ||
			     act == ACTION_TURNING_FROM_UP ||
			     act == ACTION_TURNING_FROM_DOWN);

      /* generic default action graphic (defined by "[default]" directive) */
      int default_action_graphic = element_info[EL_DEFAULT].graphic[act];
      int default_action_crumbled = element_info[EL_DEFAULT].crumbled[act];
      int default_remove_graphic = IMG_EMPTY;

      if (act_remove && default_action_graphic != -1)
	default_remove_graphic = default_action_graphic;

      /* look for special default action graphic (classic game specific) */
      if (IS_BD_ELEMENT(i) && element_info[EL_BD_DEFAULT].graphic[act] != -1)
	default_action_graphic = element_info[EL_BD_DEFAULT].graphic[act];
      if (IS_SP_ELEMENT(i) && element_info[EL_SP_DEFAULT].graphic[act] != -1)
	default_action_graphic = element_info[EL_SP_DEFAULT].graphic[act];
      if (IS_SB_ELEMENT(i) && element_info[EL_SB_DEFAULT].graphic[act] != -1)
	default_action_graphic = element_info[EL_SB_DEFAULT].graphic[act];
      if (IS_MM_ELEMENT(i) && element_info[EL_MM_DEFAULT].graphic[act] != -1)
	default_action_graphic = element_info[EL_MM_DEFAULT].graphic[act];

      if (IS_BD_ELEMENT(i) && element_info[EL_BD_DEFAULT].crumbled[act] != -1)
	default_action_crumbled = element_info[EL_BD_DEFAULT].crumbled[act];
      if (IS_SP_ELEMENT(i) && element_info[EL_SP_DEFAULT].crumbled[act] != -1)
	default_action_crumbled = element_info[EL_SP_DEFAULT].crumbled[act];
      if (IS_SB_ELEMENT(i) && element_info[EL_SB_DEFAULT].crumbled[act] != -1)
	default_action_crumbled = element_info[EL_SB_DEFAULT].crumbled[act];
      if (IS_MM_ELEMENT(i) && element_info[EL_MM_DEFAULT].crumbled[act] != -1)
	default_action_crumbled = element_info[EL_MM_DEFAULT].crumbled[act];

      /* !!! needed because EL_EMPTY_SPACE treated as IS_SP_ELEMENT !!! */
      /* !!! make this better !!! */
      if (i == EL_EMPTY_SPACE)
      {
	default_action_graphic = element_info[EL_DEFAULT].graphic[act];
	default_action_crumbled = element_info[EL_DEFAULT].crumbled[act];
      }

      if (default_action_graphic == -1)
	default_action_graphic = default_graphic;

      if (default_action_crumbled == -1)
	default_action_crumbled = default_action_graphic;

      for (dir = 0; dir < NUM_DIRECTIONS_FULL; dir++)
      {
	/* use action graphic as the default direction graphic, if undefined */
	int default_action_direction_graphic = element_info[i].graphic[act];
	int default_action_direction_crumbled = element_info[i].crumbled[act];

	/* no graphic for current action -- use default direction graphic */
	if (default_action_direction_graphic == -1)
	  default_action_direction_graphic =
	    (act_remove ? default_remove_graphic :
	     act_turning ?
	     element_info[i].direction_graphic[ACTION_TURNING][dir] :
	     default_action_graphic != default_graphic ?
	     default_action_graphic :
	     default_direction_graphic[dir]);

	if (element_info[i].direction_graphic[act][dir] == -1)
	  element_info[i].direction_graphic[act][dir] =
	    default_action_direction_graphic;

	if (default_action_direction_crumbled == -1)
	  default_action_direction_crumbled =
	    element_info[i].direction_graphic[act][dir];

	if (element_info[i].direction_crumbled[act][dir] == -1)
	  element_info[i].direction_crumbled[act][dir] =
	    default_action_direction_crumbled;
      }

      /* no graphic for this specific action -- use default action graphic */
      if (element_info[i].graphic[act] == -1)
	element_info[i].graphic[act] =
	  (act_remove ? default_remove_graphic :
	   act_turning ? element_info[i].graphic[ACTION_TURNING] :
	   default_action_graphic);

      if (element_info[i].crumbled[act] == -1)
	element_info[i].crumbled[act] = element_info[i].graphic[act];
    }
  }

  UPDATE_BUSY_STATE();
}

void InitElementSpecialGraphicInfo()
{
  struct PropertyMapping *property_mapping = getImageListPropertyMapping();
  int num_property_mappings = getImageListPropertyMappingSize();
  int i, j;

  /* always start with reliable default values */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    for (j = 0; j < NUM_SPECIAL_GFX_ARGS; j++)
      element_info[i].special_graphic[j] =
	element_info[i].graphic[ACTION_DEFAULT];

  /* initialize special element/graphic mapping from static configuration */
  for (i = 0; element_to_special_graphic[i].element > -1; i++)
  {
    int element = element_to_special_graphic[i].element;
    int special = element_to_special_graphic[i].special;
    int graphic = element_to_special_graphic[i].graphic;
    int base_graphic = el2baseimg(element);
    boolean base_redefined =
      getImageListEntryFromImageID(base_graphic)->redefined;
    boolean special_redefined =
      getImageListEntryFromImageID(graphic)->redefined;

    /* if the base graphic ("emerald", for example) has been redefined,
       but not the special graphic ("emerald.EDITOR", for example), do not
       use an existing (in this case considered obsolete) special graphic
       anymore, but use the automatically created (down-scaled) graphic */
    if (base_redefined && !special_redefined)
      continue;

    element_info[element].special_graphic[special] = graphic;
  }

  /* initialize special element/graphic mapping from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int element   = property_mapping[i].base_index;
    int action    = property_mapping[i].ext1_index;
    int direction = property_mapping[i].ext2_index;
    int special   = property_mapping[i].ext3_index;
    int graphic   = property_mapping[i].artwork_index;

    /* for action ".active", replace element with active element, if exists */
    if (action == ACTION_ACTIVE && element != ELEMENT_ACTIVE(element))
    {
      element = ELEMENT_ACTIVE(element);
      action = -1;
    }

    if (element >= MAX_NUM_ELEMENTS)
      continue;

    /* do not change special graphic if action or direction was specified */
    if (action != -1 || direction != -1)
      continue;

    if (IS_SPECIAL_GFX_ARG(special))
      element_info[element].special_graphic[special] = graphic;
  }

  /* now set all undefined/invalid graphics to default */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    for (j = 0; j < NUM_SPECIAL_GFX_ARGS; j++)
      if (graphic_info[element_info[i].special_graphic[j]].bitmap == NULL)
	element_info[i].special_graphic[j] =
	  element_info[i].graphic[ACTION_DEFAULT];
}

static int get_graphic_parameter_value(char *value_raw, char *suffix, int type)
{
  if (type != TYPE_ELEMENT && type != TYPE_GRAPHIC)
    return get_parameter_value(value_raw, suffix, type);

  if (strEqual(value_raw, ARG_UNDEFINED))
    return ARG_UNDEFINED_VALUE;

  if (type == TYPE_ELEMENT)
  {
    char *value = getHashEntry(element_token_hash, value_raw);

    if (value == NULL)
    {
      Error(ERR_INFO_LINE, "-");
      Error(ERR_INFO, "warning: error found in config file:");
      Error(ERR_INFO, "- config file: '%s'", getImageConfigFilename());
      Error(ERR_INFO, "error: invalid element token '%s'", value_raw);
      Error(ERR_INFO, "custom graphic rejected for this element/action");
      Error(ERR_INFO, "fallback done to undefined element for this graphic");
      Error(ERR_INFO_LINE, "-");
    }

    return (value != NULL ? atoi(value) : EL_UNDEFINED);
  }
  else if (type == TYPE_GRAPHIC)
  {
    char *value = getHashEntry(graphic_token_hash, value_raw);
    int fallback_graphic = IMG_CHAR_EXCLAM;

    if (value == NULL)
    {
      Error(ERR_INFO_LINE, "-");
      Error(ERR_INFO, "warning: error found in config file:");
      Error(ERR_INFO, "- config file: '%s'", getImageConfigFilename());
      Error(ERR_INFO, "error: invalid graphic token '%s'", value_raw);
      Error(ERR_INFO, "custom graphic rejected for this element/action");
      Error(ERR_INFO, "fallback done to 'char_exclam' for this graphic");
      Error(ERR_INFO_LINE, "-");
    }

    return (value != NULL ? atoi(value) : fallback_graphic);
  }

  return -1;
}

static int get_scaled_graphic_width(int graphic)
{
  int original_width = getOriginalImageWidthFromImageID(graphic);
  int scale_up_factor = graphic_info[graphic].scale_up_factor;

  return original_width * scale_up_factor;
}

static int get_scaled_graphic_height(int graphic)
{
  int original_height = getOriginalImageHeightFromImageID(graphic);
  int scale_up_factor = graphic_info[graphic].scale_up_factor;

  return original_height * scale_up_factor;
}

static void set_graphic_parameters_ext(int graphic, int *parameter,
				       Bitmap **src_bitmaps)
{
  struct GraphicInfo *g = &graphic_info[graphic];
  Bitmap *src_bitmap = (src_bitmaps ? src_bitmaps[IMG_BITMAP_STANDARD] : NULL);
  int anim_frames_per_row = 1, anim_frames_per_col = 1;
  int anim_frames_per_line = 1;

  /* always start with reliable default values */
  g->src_image_width = 0;
  g->src_image_height = 0;
  g->src_x = 0;
  g->src_y = 0;
  g->width  = TILEX;			/* default for element graphics */
  g->height = TILEY;			/* default for element graphics */
  g->offset_x = 0;			/* one or both of these values ... */
  g->offset_y = 0;			/* ... will be corrected later */
  g->offset2_x = 0;			/* one or both of these values ... */
  g->offset2_y = 0;			/* ... will be corrected later */
  g->swap_double_tiles = -1;		/* auto-detect tile swapping */
  g->crumbled_like = -1;		/* do not use clone element */
  g->diggable_like = -1;		/* do not use clone element */
  g->border_size = TILEX / 8;		/* "CRUMBLED" border size */
  g->scale_up_factor = 1;		/* default: no scaling up */
  g->tile_size = TILESIZE;		/* default: standard tile size */
  g->clone_from = -1;			/* do not use clone graphic */
  g->init_delay_fixed = 0;
  g->init_delay_random = 0;
  g->anim_delay_fixed = 0;
  g->anim_delay_random = 0;
  g->post_delay_fixed = 0;
  g->post_delay_random = 0;
  g->init_event = ANIM_EVENT_DEFAULT;
  g->anim_event = ANIM_EVENT_DEFAULT;
  g->draw_masked = FALSE;
  g->draw_order = 0;
  g->fade_mode = FADE_MODE_DEFAULT;
  g->fade_delay = -1;
  g->post_delay = -1;
  g->auto_delay = -1;
  g->align = ALIGN_CENTER;		/* default for title screens */
  g->valign = VALIGN_MIDDLE;		/* default for title screens */
  g->sort_priority = 0;			/* default for title screens */
  g->class = 0;
  g->style = STYLE_DEFAULT;

  g->bitmaps = src_bitmaps;
  g->bitmap = src_bitmap;

  /* optional zoom factor for scaling up the image to a larger size */
  if (parameter[GFX_ARG_SCALE_UP_FACTOR] != ARG_UNDEFINED_VALUE)
    g->scale_up_factor = parameter[GFX_ARG_SCALE_UP_FACTOR];
  if (g->scale_up_factor < 1)
    g->scale_up_factor = 1;		/* no scaling */

  /* optional tile size for using non-standard image size */
  if (parameter[GFX_ARG_TILE_SIZE] != ARG_UNDEFINED_VALUE)
  {
    g->tile_size = parameter[GFX_ARG_TILE_SIZE];

#if 0
    // CHECK: should tile sizes less than standard tile size be allowed?
    if (g->tile_size < TILESIZE)
      g->tile_size = TILESIZE;		/* standard tile size */
#endif

    // when setting tile size, also set width and height accordingly
    g->width  = g->tile_size;
    g->height = g->tile_size;
  }

  if (g->use_image_size)
  {
    /* set new default bitmap size (with scaling, but without small images) */
    g->width  = get_scaled_graphic_width(graphic);
    g->height = get_scaled_graphic_height(graphic);
  }

  /* optional width and height of each animation frame */
  if (parameter[GFX_ARG_WIDTH] != ARG_UNDEFINED_VALUE)
    g->width = parameter[GFX_ARG_WIDTH];
  if (parameter[GFX_ARG_HEIGHT] != ARG_UNDEFINED_VALUE)
    g->height = parameter[GFX_ARG_HEIGHT];

  /* optional x and y tile position of animation frame sequence */
  if (parameter[GFX_ARG_XPOS] != ARG_UNDEFINED_VALUE)
    g->src_x = parameter[GFX_ARG_XPOS] * g->width;
  if (parameter[GFX_ARG_YPOS] != ARG_UNDEFINED_VALUE)
    g->src_y = parameter[GFX_ARG_YPOS] * g->height;

  /* optional x and y pixel position of animation frame sequence */
  if (parameter[GFX_ARG_X] != ARG_UNDEFINED_VALUE)
    g->src_x = parameter[GFX_ARG_X];
  if (parameter[GFX_ARG_Y] != ARG_UNDEFINED_VALUE)
    g->src_y = parameter[GFX_ARG_Y];

  if (src_bitmap)
  {
    if (g->width <= 0)
    {
      Error(ERR_INFO_LINE, "-");
      Error(ERR_WARN, "invalid value %d for '%s.width' (fallback done to %d)",
	    g->width, getTokenFromImageID(graphic), TILEX);
      Error(ERR_INFO_LINE, "-");

      g->width = TILEX;		/* will be checked to be inside bitmap later */
    }

    if (g->height <= 0)
    {
      Error(ERR_INFO_LINE, "-");
      Error(ERR_WARN, "invalid value %d for '%s.height' (fallback done to %d)",
	    g->height, getTokenFromImageID(graphic), TILEY);
      Error(ERR_INFO_LINE, "-");

      g->height = TILEY;	/* will be checked to be inside bitmap later */
    }
  }

  if (src_bitmap)
  {
    /* get final bitmap size (with scaling, but without small images) */
    int src_image_width  = get_scaled_graphic_width(graphic);
    int src_image_height = get_scaled_graphic_height(graphic);

    if (src_image_width == 0 || src_image_height == 0)
    {
      /* only happens when loaded outside artwork system (like "global.busy") */
      src_image_width  = src_bitmap->width;
      src_image_height = src_bitmap->height;
    }

    if (parameter[GFX_ARG_TILE_SIZE] != ARG_UNDEFINED_VALUE)
    {
      anim_frames_per_row = MAX(1, src_image_width  / g->tile_size);
      anim_frames_per_col = MAX(1, src_image_height / g->tile_size);
    }
    else
    {
      anim_frames_per_row = MAX(1, src_image_width  / g->width);
      anim_frames_per_col = MAX(1, src_image_height / g->height);
    }

    g->src_image_width  = src_image_width;
    g->src_image_height = src_image_height;
  }

  /* correct x or y offset dependent of vertical or horizontal frame order */
  if (parameter[GFX_ARG_VERTICAL])	/* frames are ordered vertically */
  {
    g->offset_y = (parameter[GFX_ARG_OFFSET] != ARG_UNDEFINED_VALUE ?
		   parameter[GFX_ARG_OFFSET] : g->height);
    anim_frames_per_line = anim_frames_per_col;
  }
  else					/* frames are ordered horizontally */
  {
    g->offset_x = (parameter[GFX_ARG_OFFSET] != ARG_UNDEFINED_VALUE ?
		   parameter[GFX_ARG_OFFSET] : g->width);
    anim_frames_per_line = anim_frames_per_row;
  }

  /* optionally, the x and y offset of frames can be specified directly */
  if (parameter[GFX_ARG_XOFFSET] != ARG_UNDEFINED_VALUE)
    g->offset_x = parameter[GFX_ARG_XOFFSET];
  if (parameter[GFX_ARG_YOFFSET] != ARG_UNDEFINED_VALUE)
    g->offset_y = parameter[GFX_ARG_YOFFSET];

  /* optionally, moving animations may have separate start and end graphics */
  g->double_movement = parameter[GFX_ARG_2ND_MOVEMENT_TILE];

  if (parameter[GFX_ARG_2ND_VERTICAL] == ARG_UNDEFINED_VALUE)
    parameter[GFX_ARG_2ND_VERTICAL] = !parameter[GFX_ARG_VERTICAL];

  /* correct x or y offset2 dependent of vertical or horizontal frame order */
  if (parameter[GFX_ARG_2ND_VERTICAL])	/* frames are ordered vertically */
    g->offset2_y = (parameter[GFX_ARG_2ND_OFFSET] != ARG_UNDEFINED_VALUE ?
		    parameter[GFX_ARG_2ND_OFFSET] : g->height);
  else					/* frames are ordered horizontally */
    g->offset2_x = (parameter[GFX_ARG_2ND_OFFSET] != ARG_UNDEFINED_VALUE ?
		    parameter[GFX_ARG_2ND_OFFSET] : g->width);

  /* optionally, the x and y offset of 2nd graphic can be specified directly */
  if (parameter[GFX_ARG_2ND_XOFFSET] != ARG_UNDEFINED_VALUE)
    g->offset2_x = parameter[GFX_ARG_2ND_XOFFSET];
  if (parameter[GFX_ARG_2ND_YOFFSET] != ARG_UNDEFINED_VALUE)
    g->offset2_y = parameter[GFX_ARG_2ND_YOFFSET];

  /* optionally, the second movement tile can be specified as start tile */
  if (parameter[GFX_ARG_2ND_SWAP_TILES] != ARG_UNDEFINED_VALUE)
    g->swap_double_tiles= parameter[GFX_ARG_2ND_SWAP_TILES];

  /* automatically determine correct number of frames, if not defined */
  if (parameter[GFX_ARG_FRAMES] != ARG_UNDEFINED_VALUE)
    g->anim_frames = parameter[GFX_ARG_FRAMES];
  else if (parameter[GFX_ARG_XPOS] == 0 && !parameter[GFX_ARG_VERTICAL])
    g->anim_frames = anim_frames_per_row;
  else if (parameter[GFX_ARG_YPOS] == 0 && parameter[GFX_ARG_VERTICAL])
    g->anim_frames = anim_frames_per_col;
  else
    g->anim_frames = 1;

  if (g->anim_frames == 0)		/* frames must be at least 1 */
    g->anim_frames = 1;

  g->anim_frames_per_line =
    (parameter[GFX_ARG_FRAMES_PER_LINE] != ARG_UNDEFINED_VALUE ?
     parameter[GFX_ARG_FRAMES_PER_LINE] : anim_frames_per_line);

  g->anim_delay = parameter[GFX_ARG_DELAY];
  if (g->anim_delay == 0)		/* delay must be at least 1 */
    g->anim_delay = 1;

  g->anim_mode = parameter[GFX_ARG_ANIM_MODE];

  /* automatically determine correct start frame, if not defined */
  if (parameter[GFX_ARG_START_FRAME] == ARG_UNDEFINED_VALUE)
    g->anim_start_frame = 0;
  else if (g->anim_mode & ANIM_REVERSE)
    g->anim_start_frame = g->anim_frames - parameter[GFX_ARG_START_FRAME] - 1;
  else
    g->anim_start_frame = parameter[GFX_ARG_START_FRAME];

  /* animation synchronized with global frame counter, not move position */
  g->anim_global_sync = parameter[GFX_ARG_GLOBAL_SYNC];

  /* optional element for cloning crumble graphics */
  if (parameter[GFX_ARG_CRUMBLED_LIKE] != ARG_UNDEFINED_VALUE)
    g->crumbled_like = parameter[GFX_ARG_CRUMBLED_LIKE];

  /* optional element for cloning digging graphics */
  if (parameter[GFX_ARG_DIGGABLE_LIKE] != ARG_UNDEFINED_VALUE)
    g->diggable_like = parameter[GFX_ARG_DIGGABLE_LIKE];

  /* optional border size for "crumbling" diggable graphics */
  if (parameter[GFX_ARG_BORDER_SIZE] != ARG_UNDEFINED_VALUE)
    g->border_size = parameter[GFX_ARG_BORDER_SIZE];

  /* used for global animations and player "boring" and "sleeping" actions */
  if (parameter[GFX_ARG_INIT_DELAY_FIXED] != ARG_UNDEFINED_VALUE)
    g->init_delay_fixed = parameter[GFX_ARG_INIT_DELAY_FIXED];
  if (parameter[GFX_ARG_INIT_DELAY_RANDOM] != ARG_UNDEFINED_VALUE)
    g->init_delay_random = parameter[GFX_ARG_INIT_DELAY_RANDOM];
  if (parameter[GFX_ARG_ANIM_DELAY_FIXED] != ARG_UNDEFINED_VALUE)
    g->anim_delay_fixed = parameter[GFX_ARG_ANIM_DELAY_FIXED];
  if (parameter[GFX_ARG_ANIM_DELAY_RANDOM] != ARG_UNDEFINED_VALUE)
    g->anim_delay_random = parameter[GFX_ARG_ANIM_DELAY_RANDOM];
  if (parameter[GFX_ARG_POST_DELAY_FIXED] != ARG_UNDEFINED_VALUE)
    g->post_delay_fixed = parameter[GFX_ARG_POST_DELAY_FIXED];
  if (parameter[GFX_ARG_POST_DELAY_RANDOM] != ARG_UNDEFINED_VALUE)
    g->post_delay_random = parameter[GFX_ARG_POST_DELAY_RANDOM];

  /* used for global animations */
  if (parameter[GFX_ARG_INIT_EVENT] != ARG_UNDEFINED_VALUE)
    g->init_event = parameter[GFX_ARG_INIT_EVENT];
  if (parameter[GFX_ARG_ANIM_EVENT] != ARG_UNDEFINED_VALUE)
    g->anim_event = parameter[GFX_ARG_ANIM_EVENT];

  /* used for toon animations and global animations */
  g->step_offset  = parameter[GFX_ARG_STEP_OFFSET];
  g->step_xoffset = parameter[GFX_ARG_STEP_XOFFSET];
  g->step_yoffset = parameter[GFX_ARG_STEP_YOFFSET];
  g->step_delay   = parameter[GFX_ARG_STEP_DELAY];
  g->direction    = parameter[GFX_ARG_DIRECTION];
  g->position     = parameter[GFX_ARG_POSITION];
  g->x            = parameter[GFX_ARG_X];	// (may be uninitialized,
  g->y            = parameter[GFX_ARG_Y];	// unlike src_x and src_y)

  /* this is only used for drawing font characters */
  g->draw_xoffset = parameter[GFX_ARG_DRAW_XOFFSET];
  g->draw_yoffset = parameter[GFX_ARG_DRAW_YOFFSET];

  /* use a different default value for global animations and toons */
  if ((graphic >= IMG_GFX_GLOBAL_ANIM_1 && graphic <= IMG_GFX_GLOBAL_ANIM_8) ||
      (graphic >= IMG_TOON_1            && graphic <= IMG_TOON_20))
    g->draw_masked = TRUE;

  /* this is used for drawing envelopes, global animations and toons */
  if (parameter[GFX_ARG_DRAW_MASKED] != ARG_UNDEFINED_VALUE)
    g->draw_masked = parameter[GFX_ARG_DRAW_MASKED];

  /* used for toon animations and global animations */
  if (parameter[GFX_ARG_DRAW_ORDER] != ARG_UNDEFINED_VALUE)
    g->draw_order = parameter[GFX_ARG_DRAW_ORDER];

  /* optional graphic for cloning all graphics settings */
  if (parameter[GFX_ARG_CLONE_FROM] != ARG_UNDEFINED_VALUE)
    g->clone_from = parameter[GFX_ARG_CLONE_FROM];

  /* optional settings for drawing title screens and title messages */
  if (parameter[GFX_ARG_FADE_MODE] != ARG_UNDEFINED_VALUE)
    g->fade_mode = parameter[GFX_ARG_FADE_MODE];
  if (parameter[GFX_ARG_FADE_DELAY] != ARG_UNDEFINED_VALUE)
    g->fade_delay = parameter[GFX_ARG_FADE_DELAY];
  if (parameter[GFX_ARG_POST_DELAY] != ARG_UNDEFINED_VALUE)
    g->post_delay = parameter[GFX_ARG_POST_DELAY];
  if (parameter[GFX_ARG_AUTO_DELAY] != ARG_UNDEFINED_VALUE)
    g->auto_delay = parameter[GFX_ARG_AUTO_DELAY];
  if (parameter[GFX_ARG_ALIGN] != ARG_UNDEFINED_VALUE)
    g->align = parameter[GFX_ARG_ALIGN];
  if (parameter[GFX_ARG_VALIGN] != ARG_UNDEFINED_VALUE)
    g->valign = parameter[GFX_ARG_VALIGN];
  if (parameter[GFX_ARG_SORT_PRIORITY] != ARG_UNDEFINED_VALUE)
    g->sort_priority = parameter[GFX_ARG_SORT_PRIORITY];

  if (parameter[GFX_ARG_CLASS] != ARG_UNDEFINED_VALUE)
    g->class = parameter[GFX_ARG_CLASS];
  if (parameter[GFX_ARG_STYLE] != ARG_UNDEFINED_VALUE)
    g->style = parameter[GFX_ARG_STYLE];

  /* this is only used for drawing menu buttons and text */
  g->active_xoffset = parameter[GFX_ARG_ACTIVE_XOFFSET];
  g->active_yoffset = parameter[GFX_ARG_ACTIVE_YOFFSET];
  g->pressed_xoffset = parameter[GFX_ARG_PRESSED_XOFFSET];
  g->pressed_yoffset = parameter[GFX_ARG_PRESSED_YOFFSET];
}

static void set_graphic_parameters(int graphic)
{
  struct FileInfo *image = getImageListEntryFromImageID(graphic);
  char **parameter_raw = image->parameter;
  Bitmap **src_bitmaps = getBitmapsFromImageID(graphic);
  int parameter[NUM_GFX_ARGS];
  int i;

  /* if fallback to default artwork is done, also use the default parameters */
  if (image->fallback_to_default)
    parameter_raw = image->default_parameter;

  /* get integer values from string parameters */
  for (i = 0; i < NUM_GFX_ARGS; i++)
    parameter[i] = get_graphic_parameter_value(parameter_raw[i],
					       image_config_suffix[i].token,
					       image_config_suffix[i].type);

  set_graphic_parameters_ext(graphic, parameter, src_bitmaps);

  UPDATE_BUSY_STATE();
}

static void set_cloned_graphic_parameters(int graphic)
{
  int fallback_graphic = IMG_CHAR_EXCLAM;
  int max_num_images = getImageListSize();
  int clone_graphic = graphic_info[graphic].clone_from;
  int num_references_followed = 1;

  while (graphic_info[clone_graphic].clone_from != -1 &&
	 num_references_followed < max_num_images)
  {
    clone_graphic = graphic_info[clone_graphic].clone_from;

    num_references_followed++;
  }

  if (num_references_followed >= max_num_images)
  {
    Error(ERR_INFO_LINE, "-");
    Error(ERR_INFO, "warning: error found in config file:");
    Error(ERR_INFO, "- config file: '%s'", getImageConfigFilename());
    Error(ERR_INFO, "- config token: '%s'", getTokenFromImageID(graphic));
    Error(ERR_INFO, "error: loop discovered when resolving cloned graphics");
    Error(ERR_INFO, "custom graphic rejected for this element/action");

    if (graphic == fallback_graphic)
      Error(ERR_EXIT, "no fallback graphic available");

    Error(ERR_INFO, "fallback done to 'char_exclam' for this graphic");
    Error(ERR_INFO_LINE, "-");

    graphic_info[graphic] = graphic_info[fallback_graphic];
  }
  else
  {
    graphic_info[graphic] = graphic_info[clone_graphic];
    graphic_info[graphic].clone_from = clone_graphic;
  }
}

static void InitGraphicInfo()
{
  int fallback_graphic = IMG_CHAR_EXCLAM;
  int num_images = getImageListSize();
  int i;

  /* use image size as default values for width and height for these images */
  static int full_size_graphics[] =
  {
    IMG_GLOBAL_BORDER,
    IMG_GLOBAL_BORDER_MAIN,
    IMG_GLOBAL_BORDER_SCORES,
    IMG_GLOBAL_BORDER_EDITOR,
    IMG_GLOBAL_BORDER_PLAYING,
    IMG_GLOBAL_DOOR,

    IMG_BACKGROUND_ENVELOPE_1,
    IMG_BACKGROUND_ENVELOPE_2,
    IMG_BACKGROUND_ENVELOPE_3,
    IMG_BACKGROUND_ENVELOPE_4,
    IMG_BACKGROUND_REQUEST,

    IMG_BACKGROUND,
    IMG_BACKGROUND_TITLE_INITIAL,
    IMG_BACKGROUND_TITLE,
    IMG_BACKGROUND_MAIN,
    IMG_BACKGROUND_LEVELS,
    IMG_BACKGROUND_LEVELNR,
    IMG_BACKGROUND_SCORES,
    IMG_BACKGROUND_EDITOR,
    IMG_BACKGROUND_INFO,
    IMG_BACKGROUND_INFO_ELEMENTS,
    IMG_BACKGROUND_INFO_MUSIC,
    IMG_BACKGROUND_INFO_CREDITS,
    IMG_BACKGROUND_INFO_PROGRAM,
    IMG_BACKGROUND_INFO_VERSION,
    IMG_BACKGROUND_INFO_LEVELSET,
    IMG_BACKGROUND_SETUP,
    IMG_BACKGROUND_PLAYING,
    IMG_BACKGROUND_DOOR,
    IMG_BACKGROUND_TAPE,
    IMG_BACKGROUND_PANEL,
    IMG_BACKGROUND_PALETTE,
    IMG_BACKGROUND_TOOLBOX,

    IMG_TITLESCREEN_INITIAL_1,
    IMG_TITLESCREEN_INITIAL_2,
    IMG_TITLESCREEN_INITIAL_3,
    IMG_TITLESCREEN_INITIAL_4,
    IMG_TITLESCREEN_INITIAL_5,
    IMG_TITLESCREEN_1,
    IMG_TITLESCREEN_2,
    IMG_TITLESCREEN_3,
    IMG_TITLESCREEN_4,
    IMG_TITLESCREEN_5,

    IMG_BACKGROUND_TITLEMESSAGE_INITIAL_1,
    IMG_BACKGROUND_TITLEMESSAGE_INITIAL_2,
    IMG_BACKGROUND_TITLEMESSAGE_INITIAL_3,
    IMG_BACKGROUND_TITLEMESSAGE_INITIAL_4,
    IMG_BACKGROUND_TITLEMESSAGE_INITIAL_5,
    IMG_BACKGROUND_TITLEMESSAGE_1,
    IMG_BACKGROUND_TITLEMESSAGE_2,
    IMG_BACKGROUND_TITLEMESSAGE_3,
    IMG_BACKGROUND_TITLEMESSAGE_4,
    IMG_BACKGROUND_TITLEMESSAGE_5,

    -1
  };

  checked_free(graphic_info);

  graphic_info = checked_calloc(num_images * sizeof(struct GraphicInfo));

  /* initialize "use_image_size" flag with default value */
  for (i = 0; i < num_images; i++)
    graphic_info[i].use_image_size = FALSE;

  /* initialize "use_image_size" flag from static configuration above */
  for (i = 0; full_size_graphics[i] != -1; i++)
    graphic_info[full_size_graphics[i]].use_image_size = TRUE;

  /* first set all graphic paramaters ... */
  for (i = 0; i < num_images; i++)
    set_graphic_parameters(i);

  /* ... then copy these parameters for cloned graphics */
  for (i = 0; i < num_images; i++)
    if (graphic_info[i].clone_from != -1)
      set_cloned_graphic_parameters(i);

  for (i = 0; i < num_images; i++)
  {
    Bitmap *src_bitmap = graphic_info[i].bitmap;
    int src_x, src_y;
    int width, height;
    int last_frame;
    int src_bitmap_width, src_bitmap_height;

    /* now check if no animation frames are outside of the loaded image */

    if (graphic_info[i].bitmap == NULL)
      continue;		/* skip check for optional images that are undefined */

    /* get image size (this can differ from the standard element tile size!) */
    width  = graphic_info[i].width;
    height = graphic_info[i].height;

    /* get final bitmap size (with scaling, but without small images) */
    src_bitmap_width  = graphic_info[i].src_image_width;
    src_bitmap_height = graphic_info[i].src_image_height;

    /* check if first animation frame is inside specified bitmap */

    /* do not use getGraphicSourceXY() here to get position of first frame; */
    /* this avoids calculating wrong start position for out-of-bounds frame */
    src_x = graphic_info[i].src_x;
    src_y = graphic_info[i].src_y;

    if (program.headless)
      continue;

    if (src_x < 0 || src_y < 0 ||
	src_x + width  > src_bitmap_width ||
	src_y + height > src_bitmap_height)
    {
      Error(ERR_INFO_LINE, "-");
      Error(ERR_INFO, "warning: error found in config file:");
      Error(ERR_INFO, "- config file: '%s'", getImageConfigFilename());
      Error(ERR_INFO, "- config token: '%s'", getTokenFromImageID(i));
      Error(ERR_INFO, "- image file: '%s'", src_bitmap->source_filename);
      Error(ERR_INFO, "- frame size: %d, %d", width, height);
      Error(ERR_INFO,
	    "error: first animation frame out of bounds (%d, %d) [%d, %d]",
	    src_x, src_y, src_bitmap_width, src_bitmap_height);
      Error(ERR_INFO, "custom graphic rejected for this element/action");

      if (i == fallback_graphic)
	Error(ERR_EXIT, "no fallback graphic available");

      Error(ERR_INFO, "fallback done to 'char_exclam' for this graphic");
      Error(ERR_INFO_LINE, "-");

      graphic_info[i] = graphic_info[fallback_graphic];

      /* if first frame out of bounds, do not check last frame anymore */
      continue;
    }

    /* check if last animation frame is inside specified bitmap */

    last_frame = graphic_info[i].anim_frames - 1;
    getGraphicSourceXY(i, last_frame, &src_x, &src_y, FALSE);

    if (src_x < 0 || src_y < 0 ||
	src_x + width  > src_bitmap_width ||
	src_y + height > src_bitmap_height)
    {
      Error(ERR_INFO_LINE, "-");
      Error(ERR_INFO, "warning: error found in config file:");
      Error(ERR_INFO, "- config file: '%s'", getImageConfigFilename());
      Error(ERR_INFO, "- config token: '%s'", getTokenFromImageID(i));
      Error(ERR_INFO, "- image file: '%s'", src_bitmap->source_filename);
      Error(ERR_INFO, "- frame size: %d, %d", width, height);
      Error(ERR_INFO,
	    "error: last animation frame (%d) out of bounds (%d, %d) [%d, %d]",
	    last_frame, src_x, src_y, src_bitmap_width, src_bitmap_height);
      Error(ERR_INFO, "custom graphic rejected for this element/action");

      if (i == fallback_graphic)
	Error(ERR_EXIT, "no fallback graphic available");

      Error(ERR_INFO, "fallback done to 'char_exclam' for this graphic");
      Error(ERR_INFO_LINE, "-");

      graphic_info[i] = graphic_info[fallback_graphic];
    }
  }
}

static void InitGraphicCompatibilityInfo()
{
  struct FileInfo *fi_global_door =
    getImageListEntryFromImageID(IMG_GLOBAL_DOOR);
  int num_images = getImageListSize();
  int i;

  /* the following compatibility handling is needed for the following case:
     versions up to 3.3.0.0 used one large bitmap "global.door" for various
     graphics mainly used for door and panel graphics, like editor, tape and
     in-game buttons with hard-coded bitmap positions and button sizes; as
     these graphics now have individual definitions, redefining "global.door"
     to change all these graphics at once like before does not work anymore
     (because all those individual definitions still have their default values);
     to solve this, remap all those individual definitions that are not
     redefined to the new bitmap of "global.door" if it was redefined */

  /* special compatibility handling if image "global.door" was redefined */
  if (fi_global_door->redefined)
  {
    for (i = 0; i < num_images; i++)
    {
      struct FileInfo *fi = getImageListEntryFromImageID(i);

      /* process only those images that still use the default settings */
      if (!fi->redefined)
      {
	/* process all images which default to same image as "global.door" */
	if (strEqual(fi->default_filename, fi_global_door->default_filename))
	{
	  // printf("::: special treatment needed for token '%s'\n", fi->token);

	  graphic_info[i].bitmaps = graphic_info[IMG_GLOBAL_DOOR].bitmaps;
	  graphic_info[i].bitmap  = graphic_info[IMG_GLOBAL_DOOR].bitmap;
	}
      }
    }
  }

  InitGraphicCompatibilityInfo_Doors();
}

static void InitElementSoundInfo()
{
  struct PropertyMapping *property_mapping = getSoundListPropertyMapping();
  int num_property_mappings = getSoundListPropertyMappingSize();
  int i, j, act;

  /* set values to -1 to identify later as "uninitialized" values */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    for (act = 0; act < NUM_ACTIONS; act++)
      element_info[i].sound[act] = -1;

  /* initialize element/sound mapping from static configuration */
  for (i = 0; element_to_sound[i].element > -1; i++)
  {
    int element      = element_to_sound[i].element;
    int action       = element_to_sound[i].action;
    int sound        = element_to_sound[i].sound;
    boolean is_class = element_to_sound[i].is_class;

    if (action < 0)
      action = ACTION_DEFAULT;

    if (!is_class)
      element_info[element].sound[action] = sound;
    else
      for (j = 0; j < MAX_NUM_ELEMENTS; j++)
	if (strEqual(element_info[j].class_name,
		     element_info[element].class_name))
	  element_info[j].sound[action] = sound;
  }

  /* initialize element class/sound mapping from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int element_class = property_mapping[i].base_index - MAX_NUM_ELEMENTS;
    int action        = property_mapping[i].ext1_index;
    int sound         = property_mapping[i].artwork_index;

    if (element_class < 0 || element_class >= MAX_NUM_ELEMENTS)
      continue;

    if (action < 0)
      action = ACTION_DEFAULT;

    for (j = 0; j < MAX_NUM_ELEMENTS; j++)
      if (strEqual(element_info[j].class_name,
		   element_info[element_class].class_name))
	element_info[j].sound[action] = sound;
  }

  /* initialize element/sound mapping from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int element = property_mapping[i].base_index;
    int action  = property_mapping[i].ext1_index;
    int sound   = property_mapping[i].artwork_index;

    if (element >= MAX_NUM_ELEMENTS)
      continue;

    if (action < 0)
      action = ACTION_DEFAULT;

    element_info[element].sound[action] = sound;
  }

  /* now set all '-1' values to element specific default values */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    for (act = 0; act < NUM_ACTIONS; act++)
    {
      /* generic default action sound (defined by "[default]" directive) */
      int default_action_sound = element_info[EL_DEFAULT].sound[act];

      /* look for special default action sound (classic game specific) */
      if (IS_BD_ELEMENT(i) && element_info[EL_BD_DEFAULT].sound[act] != -1)
	default_action_sound = element_info[EL_BD_DEFAULT].sound[act];
      if (IS_SP_ELEMENT(i) && element_info[EL_SP_DEFAULT].sound[act] != -1)
	default_action_sound = element_info[EL_SP_DEFAULT].sound[act];
      if (IS_SB_ELEMENT(i) && element_info[EL_SB_DEFAULT].sound[act] != -1)
	default_action_sound = element_info[EL_SB_DEFAULT].sound[act];
      if (IS_MM_ELEMENT(i) && element_info[EL_MM_DEFAULT].sound[act] != -1)
	default_action_sound = element_info[EL_MM_DEFAULT].sound[act];

      /* !!! needed because EL_EMPTY_SPACE treated as IS_SP_ELEMENT !!! */
      /* !!! make this better !!! */
      if (i == EL_EMPTY_SPACE)
	default_action_sound = element_info[EL_DEFAULT].sound[act];

      /* no sound for this specific action -- use default action sound */
      if (element_info[i].sound[act] == -1)
	element_info[i].sound[act] = default_action_sound;
    }
  }

  /* copy sound settings to some elements that are only stored in level file
     in native R'n'D levels, but are used by game engine in native EM levels */
  for (i = 0; copy_properties[i][0] != -1; i++)
    for (j = 1; j <= 4; j++)
      for (act = 0; act < NUM_ACTIONS; act++)
	element_info[copy_properties[i][j]].sound[act] =
	  element_info[copy_properties[i][0]].sound[act];
}

static void InitGameModeSoundInfo()
{
  int i;

  /* set values to -1 to identify later as "uninitialized" values */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
    menu.sound[i] = -1;

  /* initialize gamemode/sound mapping from static configuration */
  for (i = 0; gamemode_to_sound[i].sound > -1; i++)
  {
    int gamemode = gamemode_to_sound[i].gamemode;
    int sound    = gamemode_to_sound[i].sound;

    if (gamemode < 0)
      gamemode = GAME_MODE_DEFAULT;

    menu.sound[gamemode] = sound;
  }

  /* now set all '-1' values to levelset specific default values */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
    if (menu.sound[i] == -1)
      menu.sound[i] = menu.sound[GAME_MODE_DEFAULT];
}

static void set_sound_parameters(int sound, char **parameter_raw)
{
  int parameter[NUM_SND_ARGS];
  int i;

  /* get integer values from string parameters */
  for (i = 0; i < NUM_SND_ARGS; i++)
    parameter[i] =
      get_parameter_value(parameter_raw[i],
			  sound_config_suffix[i].token,
			  sound_config_suffix[i].type);

  /* explicit loop mode setting in configuration overrides default value */
  if (parameter[SND_ARG_MODE_LOOP] != ARG_UNDEFINED_VALUE)
    sound_info[sound].loop = parameter[SND_ARG_MODE_LOOP];

  /* sound volume to change the original volume when loading the sound file */
  sound_info[sound].volume = parameter[SND_ARG_VOLUME];

  /* sound priority to give certain sounds a higher or lower priority */
  sound_info[sound].priority = parameter[SND_ARG_PRIORITY];
}

static void InitSoundInfo()
{
  int *sound_effect_properties;
  int num_sounds = getSoundListSize();
  int i, j;

  checked_free(sound_info);

  sound_effect_properties = checked_calloc(num_sounds * sizeof(int));
  sound_info = checked_calloc(num_sounds * sizeof(struct SoundInfo));

  /* initialize sound effect for all elements to "no sound" */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    for (j = 0; j < NUM_ACTIONS; j++)
      element_info[i].sound[j] = SND_UNDEFINED;

  for (i = 0; i < num_sounds; i++)
  {
    struct FileInfo *sound = getSoundListEntry(i);
    int len_effect_text = strlen(sound->token);

    sound_effect_properties[i] = ACTION_OTHER;
    sound_info[i].loop = FALSE;		/* default: play sound only once */

    /* determine all loop sounds and identify certain sound classes */

    for (j = 0; element_action_info[j].suffix; j++)
    {
      int len_action_text = strlen(element_action_info[j].suffix);

      if (len_action_text < len_effect_text &&
	  strEqual(&sound->token[len_effect_text - len_action_text],
		   element_action_info[j].suffix))
      {
	sound_effect_properties[i] = element_action_info[j].value;
	sound_info[i].loop = element_action_info[j].is_loop_sound;

	break;
      }
    }

    /* associate elements and some selected sound actions */

    for (j = 0; j < MAX_NUM_ELEMENTS; j++)
    {
      if (element_info[j].class_name)
      {
	int len_class_text = strlen(element_info[j].class_name);

	if (len_class_text + 1 < len_effect_text &&
	    strncmp(sound->token,
		    element_info[j].class_name, len_class_text) == 0 &&
	    sound->token[len_class_text] == '.')
	{
	  int sound_action_value = sound_effect_properties[i];

	  element_info[j].sound[sound_action_value] = i;
	}
      }
    }

    set_sound_parameters(i, sound->parameter);
  }

  free(sound_effect_properties);
}

static void InitGameModeMusicInfo()
{
  struct PropertyMapping *property_mapping = getMusicListPropertyMapping();
  int num_property_mappings = getMusicListPropertyMappingSize();
  int default_levelset_music = -1;
  int i;

  /* set values to -1 to identify later as "uninitialized" values */
  for (i = 0; i < MAX_LEVELS; i++)
    levelset.music[i] = -1;
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
    menu.music[i] = -1;

  /* initialize gamemode/music mapping from static configuration */
  for (i = 0; gamemode_to_music[i].music > -1; i++)
  {
    int gamemode = gamemode_to_music[i].gamemode;
    int music    = gamemode_to_music[i].music;

    if (gamemode < 0)
      gamemode = GAME_MODE_DEFAULT;

    menu.music[gamemode] = music;
  }

  /* initialize gamemode/music mapping from dynamic configuration */
  for (i = 0; i < num_property_mappings; i++)
  {
    int prefix   = property_mapping[i].base_index;
    int gamemode = property_mapping[i].ext2_index;
    int level    = property_mapping[i].ext3_index;
    int music    = property_mapping[i].artwork_index;

    if (prefix < 0 || prefix >= NUM_MUSIC_PREFIXES)
      continue;

    if (gamemode < 0)
      gamemode = GAME_MODE_DEFAULT;

    /* level specific music only allowed for in-game music */
    if (level != -1 && gamemode == GAME_MODE_DEFAULT)
      gamemode = GAME_MODE_PLAYING;

    if (level == -1)
    {
      level = 0;
      default_levelset_music = music;
    }

    if (gamemode == GAME_MODE_PLAYING || gamemode == GAME_MODE_DEFAULT)
      levelset.music[level] = music;
    if (gamemode != GAME_MODE_PLAYING)
      menu.music[gamemode] = music;
  }

  /* now set all '-1' values to menu specific default values */
  /* (undefined values of "levelset.music[]" might stay at "-1" to
     allow dynamic selection of music files from music directory!) */
  for (i = 0; i < MAX_LEVELS; i++)
    if (levelset.music[i] == -1)
      levelset.music[i] = default_levelset_music;
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
    if (menu.music[i] == -1)
      menu.music[i] = menu.music[GAME_MODE_DEFAULT];
}

static void set_music_parameters(int music, char **parameter_raw)
{
  int parameter[NUM_MUS_ARGS];
  int i;

  /* get integer values from string parameters */
  for (i = 0; i < NUM_MUS_ARGS; i++)
    parameter[i] =
      get_parameter_value(parameter_raw[i],
			  music_config_suffix[i].token,
			  music_config_suffix[i].type);

  /* explicit loop mode setting in configuration overrides default value */
  if (parameter[MUS_ARG_MODE_LOOP] != ARG_UNDEFINED_VALUE)
    music_info[music].loop = parameter[MUS_ARG_MODE_LOOP];
}

static void InitMusicInfo()
{
  int num_music = getMusicListSize();
  int i, j;

  checked_free(music_info);

  music_info = checked_calloc(num_music * sizeof(struct MusicInfo));

  for (i = 0; i < num_music; i++)
  {
    struct FileInfo *music = getMusicListEntry(i);
    int len_music_text = strlen(music->token);

    music_info[i].loop = TRUE;		/* default: play music in loop mode */

    /* determine all loop music */

    for (j = 0; music_prefix_info[j].prefix; j++)
    {
      int len_prefix_text = strlen(music_prefix_info[j].prefix);

      if (len_prefix_text < len_music_text &&
	  strncmp(music->token,
		  music_prefix_info[j].prefix, len_prefix_text) == 0)
      {
	music_info[i].loop = music_prefix_info[j].is_loop_music;

	break;
      }
    }

    set_music_parameters(i, music->parameter);
  }
}

static void ReinitializeGraphics()
{
  print_timestamp_init("ReinitializeGraphics");

  InitGfxTileSizeInfo(game.tile_size, TILESIZE);

  InitGraphicInfo();			/* graphic properties mapping */
  print_timestamp_time("InitGraphicInfo");
  InitElementGraphicInfo();		/* element game graphic mapping */
  print_timestamp_time("InitElementGraphicInfo");
  InitElementSpecialGraphicInfo();	/* element special graphic mapping */
  print_timestamp_time("InitElementSpecialGraphicInfo");

  InitElementSmallImages();		/* scale elements to all needed sizes */
  print_timestamp_time("InitElementSmallImages");
  InitScaledImages();			/* scale all other images, if needed */
  print_timestamp_time("InitScaledImages");
  InitBitmapPointers();			/* set standard size bitmap pointers */
  print_timestamp_time("InitBitmapPointers");
  InitFontGraphicInfo();		/* initialize text drawing functions */
  print_timestamp_time("InitFontGraphicInfo");
  InitGlobalAnimGraphicInfo();		/* initialize global animation config */
  print_timestamp_time("InitGlobalAnimGraphicInfo");

  InitImageTextures();			/* create textures for certain images */
  print_timestamp_time("InitImageTextures");

  InitGraphicInfo_EM();			/* graphic mapping for EM engine */
  print_timestamp_time("InitGraphicInfo_EM");

  InitGraphicCompatibilityInfo();
  print_timestamp_time("InitGraphicCompatibilityInfo");

  SetMainBackgroundImage(IMG_BACKGROUND);
  print_timestamp_time("SetMainBackgroundImage");
  SetDoorBackgroundImage(IMG_BACKGROUND_DOOR);
  print_timestamp_time("SetDoorBackgroundImage");

  InitGadgets();
  print_timestamp_time("InitGadgets");
  InitDoors();
  print_timestamp_time("InitDoors");

  print_timestamp_done("ReinitializeGraphics");
}

static void ReinitializeSounds()
{
  InitSoundInfo();		/* sound properties mapping */
  InitElementSoundInfo();	/* element game sound mapping */
  InitGameModeSoundInfo();	/* game mode sound mapping */
  InitGlobalAnimSoundInfo();	/* global animation sound settings */

  InitPlayLevelSound();		/* internal game sound settings */
}

static void ReinitializeMusic()
{
  InitMusicInfo();		/* music properties mapping */
  InitGameModeMusicInfo();	/* game mode music mapping */
  InitGlobalAnimMusicInfo();	/* global animation music settings */
}

static int get_special_property_bit(int element, int property_bit_nr)
{
  struct PropertyBitInfo
  {
    int element;
    int bit_nr;
  };

  static struct PropertyBitInfo pb_can_move_into_acid[] =
  {
    /* the player may be able fall into acid when gravity is activated */
    { EL_PLAYER_1,		0	},
    { EL_PLAYER_2,		0	},
    { EL_PLAYER_3,		0	},
    { EL_PLAYER_4,		0	},
    { EL_SP_MURPHY,		0	},
    { EL_SOKOBAN_FIELD_PLAYER,	0	},

    /* all elements that can move may be able to also move into acid */
    { EL_BUG,			1	},
    { EL_BUG_LEFT,		1	},
    { EL_BUG_RIGHT,		1	},
    { EL_BUG_UP,		1	},
    { EL_BUG_DOWN,		1	},
    { EL_SPACESHIP,		2	},
    { EL_SPACESHIP_LEFT,	2	},
    { EL_SPACESHIP_RIGHT,	2	},
    { EL_SPACESHIP_UP,		2	},
    { EL_SPACESHIP_DOWN,	2	},
    { EL_BD_BUTTERFLY,		3	},
    { EL_BD_BUTTERFLY_LEFT,	3	},
    { EL_BD_BUTTERFLY_RIGHT,	3	},
    { EL_BD_BUTTERFLY_UP,	3	},
    { EL_BD_BUTTERFLY_DOWN,	3	},
    { EL_BD_FIREFLY,		4	},
    { EL_BD_FIREFLY_LEFT,	4	},
    { EL_BD_FIREFLY_RIGHT,	4	},
    { EL_BD_FIREFLY_UP,		4	},
    { EL_BD_FIREFLY_DOWN,	4	},
    { EL_YAMYAM,		5	},
    { EL_YAMYAM_LEFT,		5	},
    { EL_YAMYAM_RIGHT,		5	},
    { EL_YAMYAM_UP,		5	},
    { EL_YAMYAM_DOWN,		5	},
    { EL_DARK_YAMYAM,		6	},
    { EL_ROBOT,			7	},
    { EL_PACMAN,		8	},
    { EL_PACMAN_LEFT,		8	},
    { EL_PACMAN_RIGHT,		8	},
    { EL_PACMAN_UP,		8	},
    { EL_PACMAN_DOWN,		8	},
    { EL_MOLE,			9	},
    { EL_MOLE_LEFT,		9	},
    { EL_MOLE_RIGHT,		9	},
    { EL_MOLE_UP,		9	},
    { EL_MOLE_DOWN,		9	},
    { EL_PENGUIN,		10	},
    { EL_PIG,			11	},
    { EL_DRAGON,		12	},
    { EL_SATELLITE,		13	},
    { EL_SP_SNIKSNAK,		14	},
    { EL_SP_ELECTRON,		15	},
    { EL_BALLOON,		16	},
    { EL_SPRING,	        17	},
    { EL_EMC_ANDROID,	        18	},

    { -1,			-1	},
  };

  static struct PropertyBitInfo pb_dont_collide_with[] =
  {
    { EL_SP_SNIKSNAK,		0	},
    { EL_SP_ELECTRON,		1	},

    { -1,			-1	},
  };

  static struct
  {
    int bit_nr;
    struct PropertyBitInfo *pb_info;
  } pb_definition[] =
  {
    { EP_CAN_MOVE_INTO_ACID,	pb_can_move_into_acid	},
    { EP_DONT_COLLIDE_WITH,	pb_dont_collide_with	},

    { -1,			NULL			},
  };

  struct PropertyBitInfo *pb_info = NULL;
  int i;

  for (i = 0; pb_definition[i].bit_nr != -1; i++)
    if (pb_definition[i].bit_nr == property_bit_nr)
      pb_info = pb_definition[i].pb_info;

  if (pb_info == NULL)
    return -1;

  for (i = 0; pb_info[i].element != -1; i++)
    if (pb_info[i].element == element)
      return pb_info[i].bit_nr;

  return -1;
}

void setBitfieldProperty(int *bitfield, int property_bit_nr, int element,
			 boolean property_value)
{
  int bit_nr = get_special_property_bit(element, property_bit_nr);

  if (bit_nr > -1)
  {
    if (property_value)
      *bitfield |=  (1 << bit_nr);
    else
      *bitfield &= ~(1 << bit_nr);
  }
}

boolean getBitfieldProperty(int *bitfield, int property_bit_nr, int element)
{
  int bit_nr = get_special_property_bit(element, property_bit_nr);

  if (bit_nr > -1)
    return ((*bitfield & (1 << bit_nr)) != 0);

  return FALSE;
}

static void ResolveGroupElementExt(int group_element, int recursion_depth)
{
  static int group_nr;
  static struct ElementGroupInfo *group;
  struct ElementGroupInfo *actual_group = element_info[group_element].group;
  int i;

  if (actual_group == NULL)			/* not yet initialized */
    return;

  if (recursion_depth > NUM_GROUP_ELEMENTS)	/* recursion too deep */
  {
    Error(ERR_WARN, "recursion too deep when resolving group element %d",
	  group_element - EL_GROUP_START + 1);

    /* replace element which caused too deep recursion by question mark */
    group->element_resolved[group->num_elements_resolved++] = EL_UNKNOWN;

    return;
  }

  if (recursion_depth == 0)			/* initialization */
  {
    group = actual_group;
    group_nr = GROUP_NR(group_element);

    group->num_elements_resolved = 0;
    group->choice_pos = 0;

    for (i = 0; i < MAX_NUM_ELEMENTS; i++)
      element_info[i].in_group[group_nr] = FALSE;
  }

  for (i = 0; i < actual_group->num_elements; i++)
  {
    int element = actual_group->element[i];

    if (group->num_elements_resolved == NUM_FILE_ELEMENTS)
      break;

    if (IS_GROUP_ELEMENT(element))
      ResolveGroupElementExt(element, recursion_depth + 1);
    else
    {
      group->element_resolved[group->num_elements_resolved++] = element;
      element_info[element].in_group[group_nr] = TRUE;
    }
  }
}

void ResolveGroupElement(int group_element)
{
  ResolveGroupElementExt(group_element, 0);
}

void InitElementPropertiesStatic()
{
  static boolean clipboard_elements_initialized = FALSE;

  static int ep_diggable[] =
  {
    EL_SAND,
    EL_SP_BASE,
    EL_SP_BUGGY_BASE,
    EL_SP_BUGGY_BASE_ACTIVATING,
    EL_TRAP,
    EL_INVISIBLE_SAND,
    EL_INVISIBLE_SAND_ACTIVE,
    EL_EMC_GRASS,

    /* !!! currently not diggable, but handled by 'ep_dont_run_into' !!! */
    /* (if amoeba can grow into anything diggable, maybe keep these out) */
#if 0
    EL_LANDMINE,
    EL_DC_LANDMINE,
    EL_TRAP_ACTIVE,
    EL_SP_BUGGY_BASE_ACTIVE,
    EL_EMC_PLANT,
#endif

    -1
  };

  static int ep_collectible_only[] =
  {
    EL_BD_DIAMOND,
    EL_EMERALD,
    EL_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_KEY_1,
    EL_KEY_2,
    EL_KEY_3,
    EL_KEY_4,
    EL_EM_KEY_1,
    EL_EM_KEY_2,
    EL_EM_KEY_3,
    EL_EM_KEY_4,
    EL_EMC_KEY_5,
    EL_EMC_KEY_6,
    EL_EMC_KEY_7,
    EL_EMC_KEY_8,
    EL_DYNAMITE,
    EL_EM_DYNAMITE,
    EL_DYNABOMB_INCREASE_NUMBER,
    EL_DYNABOMB_INCREASE_SIZE,
    EL_DYNABOMB_INCREASE_POWER,
    EL_SP_INFOTRON,
    EL_SP_DISK_RED,
    EL_PEARL,
    EL_CRYSTAL,
    EL_DC_KEY_WHITE,
    EL_SHIELD_NORMAL,
    EL_SHIELD_DEADLY,
    EL_EXTRA_TIME,
    EL_ENVELOPE_1,
    EL_ENVELOPE_2,
    EL_ENVELOPE_3,
    EL_ENVELOPE_4,
    EL_SPEED_PILL,
    EL_EMC_LENSES,
    EL_EMC_MAGNIFIER,

#if 0
    /* !!! handle separately !!! */
    EL_DC_LANDMINE,	/* deadly when running into, but can be snapped */
#endif

    -1
  };

  static int ep_dont_run_into[] =
  {
    /* same elements as in 'ep_dont_touch' */
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,

    /* same elements as in 'ep_dont_collide_with' */
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,

    /* new elements */
    EL_AMOEBA_DROP,
    EL_ACID,

    /* !!! maybe this should better be handled by 'ep_diggable' !!! */
#if 1
    EL_LANDMINE,
    EL_DC_LANDMINE,
    EL_TRAP_ACTIVE,
    EL_SP_BUGGY_BASE_ACTIVE,
    EL_EMC_PLANT,
#endif

    -1
  };

  static int ep_dont_collide_with[] =
  {
    /* same elements as in 'ep_dont_touch' */
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,

    /* new elements */
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,

    -1
  };

  static int ep_dont_touch[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,

    -1
  };

  static int ep_indestructible[] =
  {
    EL_STEELWALL,
    EL_ACID,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_ACID_POOL_BOTTOMLEFT,
    EL_ACID_POOL_BOTTOM,
    EL_ACID_POOL_BOTTOMRIGHT,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_NO_ENTRY,
    EL_SIGN_UNUSED_1,
    EL_SIGN_GIVE_WAY,
    EL_SIGN_ENTRY_FORBIDDEN,
    EL_SIGN_EMERGENCY_EXIT,
    EL_SIGN_YIN_YANG,
    EL_SIGN_UNUSED_2,
    EL_SIGN_SPERMS,
    EL_SIGN_BULLET,
    EL_SIGN_HEART,
    EL_SIGN_CROSS,
    EL_SIGN_FRANKIE,
    EL_STEEL_EXIT_CLOSED,
    EL_STEEL_EXIT_OPEN,
    EL_STEEL_EXIT_OPENING,
    EL_STEEL_EXIT_CLOSING,
    EL_EM_STEEL_EXIT_CLOSED,
    EL_EM_STEEL_EXIT_OPEN,
    EL_EM_STEEL_EXIT_OPENING,
    EL_EM_STEEL_EXIT_CLOSING,
    EL_DC_STEELWALL_1_LEFT,
    EL_DC_STEELWALL_1_RIGHT,
    EL_DC_STEELWALL_1_TOP,
    EL_DC_STEELWALL_1_BOTTOM,
    EL_DC_STEELWALL_1_HORIZONTAL,
    EL_DC_STEELWALL_1_VERTICAL,
    EL_DC_STEELWALL_1_TOPLEFT,
    EL_DC_STEELWALL_1_TOPRIGHT,
    EL_DC_STEELWALL_1_BOTTOMLEFT,
    EL_DC_STEELWALL_1_BOTTOMRIGHT,
    EL_DC_STEELWALL_1_TOPLEFT_2,
    EL_DC_STEELWALL_1_TOPRIGHT_2,
    EL_DC_STEELWALL_1_BOTTOMLEFT_2,
    EL_DC_STEELWALL_1_BOTTOMRIGHT_2,
    EL_DC_STEELWALL_2_LEFT,
    EL_DC_STEELWALL_2_RIGHT,
    EL_DC_STEELWALL_2_TOP,
    EL_DC_STEELWALL_2_BOTTOM,
    EL_DC_STEELWALL_2_HORIZONTAL,
    EL_DC_STEELWALL_2_VERTICAL,
    EL_DC_STEELWALL_2_MIDDLE,
    EL_DC_STEELWALL_2_SINGLE,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_CRYSTAL,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_GATE_1_GRAY_ACTIVE,
    EL_GATE_2_GRAY_ACTIVE,
    EL_GATE_3_GRAY_ACTIVE,
    EL_GATE_4_GRAY_ACTIVE,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EM_GATE_1_GRAY_ACTIVE,
    EL_EM_GATE_2_GRAY_ACTIVE,
    EL_EM_GATE_3_GRAY_ACTIVE,
    EL_EM_GATE_4_GRAY_ACTIVE,
    EL_EMC_GATE_5,
    EL_EMC_GATE_6,
    EL_EMC_GATE_7,
    EL_EMC_GATE_8,
    EL_EMC_GATE_5_GRAY,
    EL_EMC_GATE_6_GRAY,
    EL_EMC_GATE_7_GRAY,
    EL_EMC_GATE_8_GRAY,
    EL_EMC_GATE_5_GRAY_ACTIVE,
    EL_EMC_GATE_6_GRAY_ACTIVE,
    EL_EMC_GATE_7_GRAY_ACTIVE,
    EL_EMC_GATE_8_GRAY_ACTIVE,
    EL_DC_GATE_WHITE,
    EL_DC_GATE_WHITE_GRAY,
    EL_DC_GATE_WHITE_GRAY_ACTIVE,
    EL_DC_GATE_FAKE_GRAY,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_DC_SWITCHGATE_SWITCH_UP,
    EL_DC_SWITCHGATE_SWITCH_DOWN,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
    EL_DC_TIMEGATE_SWITCH,
    EL_DC_TIMEGATE_SWITCH_ACTIVE,
    EL_TUBE_ANY,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,
    EL_EXPANDABLE_STEELWALL_HORIZONTAL,
    EL_EXPANDABLE_STEELWALL_VERTICAL,
    EL_EXPANDABLE_STEELWALL_ANY,

    -1
  };

  static int ep_slippery[] =
  {
    EL_WALL_SLIPPERY,
    EL_BD_WALL,
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_ROBOT_WHEEL_ACTIVE,
    EL_ROBOT_WHEEL,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
    EL_LAMP_ACTIVE,
    EL_LAMP,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_SATELLITE,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SPEED_PILL,
    EL_STEELWALL_SLIPPERY,
    EL_PEARL,
    EL_CRYSTAL,
    EL_EMC_WALL_SLIPPERY_1,
    EL_EMC_WALL_SLIPPERY_2,
    EL_EMC_WALL_SLIPPERY_3,
    EL_EMC_WALL_SLIPPERY_4,
    EL_EMC_MAGIC_BALL,
    EL_EMC_MAGIC_BALL_ACTIVE,

    -1
  };

  static int ep_can_change[] =
  {
    -1
  };

  static int ep_can_move[] =
  {
    /* same elements as in 'pb_can_move_into_acid' */
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_MOLE,
    EL_PENGUIN,
    EL_PIG,
    EL_DRAGON,
    EL_SATELLITE,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
    EL_BALLOON,
    EL_SPRING,
    EL_EMC_ANDROID,

    -1
  };

  static int ep_can_fall[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_AMOEBA_DROP,
    EL_QUICKSAND_FULL,
    EL_QUICKSAND_FAST_FULL,
    EL_MAGIC_WALL_FULL,
    EL_BD_MAGIC_WALL_FULL,
    EL_DC_MAGIC_WALL_FULL,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_DISK_ORANGE,
    EL_PEARL,
    EL_CRYSTAL,
    EL_SPRING,
    EL_DX_SUPABOMB,

    -1
  };

  static int ep_can_smash_player[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_BOMB,
    EL_NUT,
    EL_AMOEBA_DROP,
    EL_TIME_ORB_FULL,
    EL_TIME_ORB_EMPTY,
    EL_SP_ZONK,
    EL_SP_INFOTRON,
    EL_SP_DISK_ORANGE,
    EL_PEARL,
    EL_CRYSTAL,
    EL_SPRING,
    EL_DX_SUPABOMB,

    -1
  };

  static int ep_can_smash_enemies[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_SP_ZONK,

    -1
  };

  static int ep_can_smash_everything[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_SP_ZONK,

    -1
  };

  static int ep_explodes_by_fire[] =
  {
    /* same elements as in 'ep_explodes_impact' */
    EL_BOMB,
    EL_SP_DISK_ORANGE,
    EL_DX_SUPABOMB,

    /* same elements as in 'ep_explodes_smashed' */
    EL_SATELLITE,
    EL_PIG,
    EL_DRAGON,
    EL_MOLE,

    /* new elements */
    EL_DYNAMITE,
    EL_DYNAMITE_ACTIVE,
    EL_EM_DYNAMITE,
    EL_EM_DYNAMITE_ACTIVE,
    EL_DYNABOMB_PLAYER_1_ACTIVE,
    EL_DYNABOMB_PLAYER_2_ACTIVE,
    EL_DYNABOMB_PLAYER_3_ACTIVE,
    EL_DYNABOMB_PLAYER_4_ACTIVE,
    EL_DYNABOMB_INCREASE_NUMBER,
    EL_DYNABOMB_INCREASE_SIZE,
    EL_DYNABOMB_INCREASE_POWER,
    EL_SP_DISK_RED_ACTIVE,
    EL_BUG,
    EL_PENGUIN,
    EL_SP_DISK_RED,
    EL_SP_DISK_YELLOW,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,
#if 0
    EL_BLACK_ORB,
#endif

    -1
  };

  static int ep_explodes_smashed[] =
  {
    /* same elements as in 'ep_explodes_impact' */
    EL_BOMB,
    EL_SP_DISK_ORANGE,
    EL_DX_SUPABOMB,

    /* new elements */
    EL_SATELLITE,
    EL_PIG,
    EL_DRAGON,
    EL_MOLE,

    -1
  };

  static int ep_explodes_impact[] =
  {
    EL_BOMB,
    EL_SP_DISK_ORANGE,
    EL_DX_SUPABOMB,

    -1
  };

  static int ep_walkable_over[] =
  {
    EL_EMPTY_SPACE,
    EL_SP_EMPTY_SPACE,
    EL_SOKOBAN_FIELD_EMPTY,
    EL_EXIT_OPEN,
    EL_EM_EXIT_OPEN,
    EL_EM_EXIT_OPENING,
    EL_SP_EXIT_OPEN,
    EL_SP_EXIT_OPENING,
    EL_STEEL_EXIT_OPEN,
    EL_EM_STEEL_EXIT_OPEN,
    EL_EM_STEEL_EXIT_OPENING,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_GATE_1_GRAY_ACTIVE,
    EL_GATE_2_GRAY_ACTIVE,
    EL_GATE_3_GRAY_ACTIVE,
    EL_GATE_4_GRAY_ACTIVE,
    EL_PENGUIN,
    EL_PIG,
    EL_DRAGON,

    -1
  };

  static int ep_walkable_inside[] =
  {
    EL_TUBE_ANY,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,

    -1
  };

  static int ep_walkable_under[] =
  {
    -1
  };

  static int ep_passable_over[] =
  {
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EM_GATE_1_GRAY_ACTIVE,
    EL_EM_GATE_2_GRAY_ACTIVE,
    EL_EM_GATE_3_GRAY_ACTIVE,
    EL_EM_GATE_4_GRAY_ACTIVE,
    EL_EMC_GATE_5,
    EL_EMC_GATE_6,
    EL_EMC_GATE_7,
    EL_EMC_GATE_8,
    EL_EMC_GATE_5_GRAY,
    EL_EMC_GATE_6_GRAY,
    EL_EMC_GATE_7_GRAY,
    EL_EMC_GATE_8_GRAY,
    EL_EMC_GATE_5_GRAY_ACTIVE,
    EL_EMC_GATE_6_GRAY_ACTIVE,
    EL_EMC_GATE_7_GRAY_ACTIVE,
    EL_EMC_GATE_8_GRAY_ACTIVE,
    EL_DC_GATE_WHITE,
    EL_DC_GATE_WHITE_GRAY,
    EL_DC_GATE_WHITE_GRAY_ACTIVE,
    EL_SWITCHGATE_OPEN,
    EL_TIMEGATE_OPEN,

    -1
  };

  static int ep_passable_inside[] =
  {
    EL_SP_PORT_LEFT,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_UP,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_ANY,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_GRAVITY_PORT_DOWN,
    EL_SP_GRAVITY_ON_PORT_LEFT,
    EL_SP_GRAVITY_ON_PORT_RIGHT,
    EL_SP_GRAVITY_ON_PORT_UP,
    EL_SP_GRAVITY_ON_PORT_DOWN,
    EL_SP_GRAVITY_OFF_PORT_LEFT,
    EL_SP_GRAVITY_OFF_PORT_RIGHT,
    EL_SP_GRAVITY_OFF_PORT_UP,
    EL_SP_GRAVITY_OFF_PORT_DOWN,

    -1
  };

  static int ep_passable_under[] =
  {
    -1
  };

  static int ep_droppable[] =
  {
    -1
  };

  static int ep_explodes_1x1_old[] =
  {
    -1
  };

  static int ep_pushable[] =
  {
    EL_ROCK,
    EL_BOMB,
    EL_DX_SUPABOMB,
    EL_NUT,
    EL_TIME_ORB_EMPTY,
    EL_SP_ZONK,
    EL_SP_DISK_ORANGE,
    EL_SPRING,
    EL_BD_ROCK,
    EL_SOKOBAN_OBJECT,
    EL_SOKOBAN_FIELD_FULL,
    EL_SATELLITE,
    EL_SP_DISK_YELLOW,
    EL_BALLOON,
    EL_EMC_ANDROID,

    -1
  };

  static int ep_explodes_cross_old[] =
  {
    -1
  };

  static int ep_protected[] =
  {
    /* same elements as in 'ep_walkable_inside' */
    EL_TUBE_ANY,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,

    /* same elements as in 'ep_passable_over' */
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EM_GATE_1_GRAY_ACTIVE,
    EL_EM_GATE_2_GRAY_ACTIVE,
    EL_EM_GATE_3_GRAY_ACTIVE,
    EL_EM_GATE_4_GRAY_ACTIVE,
    EL_EMC_GATE_5,
    EL_EMC_GATE_6,
    EL_EMC_GATE_7,
    EL_EMC_GATE_8,
    EL_EMC_GATE_5_GRAY,
    EL_EMC_GATE_6_GRAY,
    EL_EMC_GATE_7_GRAY,
    EL_EMC_GATE_8_GRAY,
    EL_EMC_GATE_5_GRAY_ACTIVE,
    EL_EMC_GATE_6_GRAY_ACTIVE,
    EL_EMC_GATE_7_GRAY_ACTIVE,
    EL_EMC_GATE_8_GRAY_ACTIVE,
    EL_DC_GATE_WHITE,
    EL_DC_GATE_WHITE_GRAY,
    EL_DC_GATE_WHITE_GRAY_ACTIVE,
    EL_SWITCHGATE_OPEN,
    EL_TIMEGATE_OPEN,

    /* same elements as in 'ep_passable_inside' */
    EL_SP_PORT_LEFT,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_UP,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_ANY,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_GRAVITY_PORT_DOWN,
    EL_SP_GRAVITY_ON_PORT_LEFT,
    EL_SP_GRAVITY_ON_PORT_RIGHT,
    EL_SP_GRAVITY_ON_PORT_UP,
    EL_SP_GRAVITY_ON_PORT_DOWN,
    EL_SP_GRAVITY_OFF_PORT_LEFT,
    EL_SP_GRAVITY_OFF_PORT_RIGHT,
    EL_SP_GRAVITY_OFF_PORT_UP,
    EL_SP_GRAVITY_OFF_PORT_DOWN,

    -1
  };

  static int ep_throwable[] =
  {
    -1
  };

  static int ep_can_explode[] =
  {
    /* same elements as in 'ep_explodes_impact' */
    EL_BOMB,
    EL_SP_DISK_ORANGE,
    EL_DX_SUPABOMB,

    /* same elements as in 'ep_explodes_smashed' */
    EL_SATELLITE,
    EL_PIG,
    EL_DRAGON,
    EL_MOLE,

    /* elements that can explode by explosion or by dragonfire */
    EL_DYNAMITE,
    EL_DYNAMITE_ACTIVE,
    EL_EM_DYNAMITE,
    EL_EM_DYNAMITE_ACTIVE,
    EL_DYNABOMB_PLAYER_1_ACTIVE,
    EL_DYNABOMB_PLAYER_2_ACTIVE,
    EL_DYNABOMB_PLAYER_3_ACTIVE,
    EL_DYNABOMB_PLAYER_4_ACTIVE,
    EL_DYNABOMB_INCREASE_NUMBER,
    EL_DYNABOMB_INCREASE_SIZE,
    EL_DYNABOMB_INCREASE_POWER,
    EL_SP_DISK_RED_ACTIVE,
    EL_BUG,
    EL_PENGUIN,
    EL_SP_DISK_RED,
    EL_SP_DISK_YELLOW,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,

    /* elements that can explode only by explosion */
    EL_BLACK_ORB,

    -1
  };

  static int ep_gravity_reachable[] =
  {
    EL_SAND,
    EL_SP_BASE,
    EL_TRAP,
    EL_INVISIBLE_SAND,
    EL_INVISIBLE_SAND_ACTIVE,
    EL_SP_PORT_LEFT,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_UP,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_ANY,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_GRAVITY_PORT_DOWN,
    EL_SP_GRAVITY_ON_PORT_LEFT,
    EL_SP_GRAVITY_ON_PORT_RIGHT,
    EL_SP_GRAVITY_ON_PORT_UP,
    EL_SP_GRAVITY_ON_PORT_DOWN,
    EL_SP_GRAVITY_OFF_PORT_LEFT,
    EL_SP_GRAVITY_OFF_PORT_RIGHT,
    EL_SP_GRAVITY_OFF_PORT_UP,
    EL_SP_GRAVITY_OFF_PORT_DOWN,
    EL_EMC_GRASS,

    -1
  };

  static int ep_player[] =
  {
    EL_PLAYER_1,
    EL_PLAYER_2,
    EL_PLAYER_3,
    EL_PLAYER_4,
    EL_SP_MURPHY,
    EL_SOKOBAN_FIELD_PLAYER,
    EL_TRIGGER_PLAYER,

    -1
  };

  static int ep_can_pass_magic_wall[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,

    -1
  };

  static int ep_can_pass_dc_magic_wall[] =
  {
    EL_ROCK,
    EL_BD_ROCK,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_PEARL,
    EL_CRYSTAL,

    -1
  };

  static int ep_switchable[] =
  {
    EL_ROBOT_WHEEL,
    EL_SP_TERMINAL,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_SWITCHGATE_SWITCH_UP,
    EL_SWITCHGATE_SWITCH_DOWN,
    EL_DC_SWITCHGATE_SWITCH_UP,
    EL_DC_SWITCHGATE_SWITCH_DOWN,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
    EL_TIMEGATE_SWITCH,
    EL_DC_TIMEGATE_SWITCH,
    EL_BALLOON_SWITCH_LEFT,
    EL_BALLOON_SWITCH_RIGHT,
    EL_BALLOON_SWITCH_UP,
    EL_BALLOON_SWITCH_DOWN,
    EL_BALLOON_SWITCH_ANY,
    EL_BALLOON_SWITCH_NONE,
    EL_LAMP,
    EL_TIME_ORB_FULL,
    EL_EMC_MAGIC_BALL_SWITCH,
    EL_EMC_MAGIC_BALL_SWITCH_ACTIVE,

    -1
  };

  static int ep_bd_element[] =
  {
    EL_EMPTY,
    EL_SAND,
    EL_WALL_SLIPPERY,
    EL_BD_WALL,
    EL_ROCK,
    EL_BD_ROCK,
    EL_BD_DIAMOND,
    EL_BD_MAGIC_WALL,
    EL_EXIT_CLOSED,
    EL_EXIT_OPEN,
    EL_STEELWALL,
    EL_PLAYER_1,
    EL_PLAYER_2,
    EL_PLAYER_3,
    EL_PLAYER_4,
    EL_BD_FIREFLY,
    EL_BD_FIREFLY_1,
    EL_BD_FIREFLY_2,
    EL_BD_FIREFLY_3,
    EL_BD_FIREFLY_4,
    EL_BD_BUTTERFLY,
    EL_BD_BUTTERFLY_1,
    EL_BD_BUTTERFLY_2,
    EL_BD_BUTTERFLY_3,
    EL_BD_BUTTERFLY_4,
    EL_BD_AMOEBA,
    EL_CHAR_QUESTION,
    EL_UNKNOWN,

    -1
  };

  static int ep_sp_element[] =
  {
    /* should always be valid */
    EL_EMPTY,

    /* standard classic Supaplex elements */
    EL_SP_EMPTY,
    EL_SP_ZONK,
    EL_SP_BASE,
    EL_SP_MURPHY,
    EL_SP_INFOTRON,
    EL_SP_CHIP_SINGLE,
    EL_SP_HARDWARE_GRAY,
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_SP_DISK_ORANGE,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_LEFT,
    EL_SP_PORT_UP,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_DOWN,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_SNIKSNAK,
    EL_SP_DISK_YELLOW,
    EL_SP_TERMINAL,
    EL_SP_DISK_RED,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_ANY,
    EL_SP_ELECTRON,
    EL_SP_BUGGY_BASE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,

    /* additional elements that appeared in newer Supaplex levels */
    EL_INVISIBLE_WALL,

    /* additional gravity port elements (not switching, but setting gravity) */
    EL_SP_GRAVITY_ON_PORT_LEFT,
    EL_SP_GRAVITY_ON_PORT_RIGHT,
    EL_SP_GRAVITY_ON_PORT_UP,
    EL_SP_GRAVITY_ON_PORT_DOWN,
    EL_SP_GRAVITY_OFF_PORT_LEFT,
    EL_SP_GRAVITY_OFF_PORT_RIGHT,
    EL_SP_GRAVITY_OFF_PORT_UP,
    EL_SP_GRAVITY_OFF_PORT_DOWN,

    /* more than one Murphy in a level results in an inactive clone */
    EL_SP_MURPHY_CLONE,

    /* runtime Supaplex elements */
    EL_SP_DISK_RED_ACTIVE,
    EL_SP_TERMINAL_ACTIVE,
    EL_SP_BUGGY_BASE_ACTIVATING,
    EL_SP_BUGGY_BASE_ACTIVE,
    EL_SP_EXIT_OPENING,
    EL_SP_EXIT_CLOSING,

    -1
  };

  static int ep_sb_element[] =
  {
    EL_EMPTY,
    EL_STEELWALL,
    EL_SOKOBAN_OBJECT,
    EL_SOKOBAN_FIELD_EMPTY,
    EL_SOKOBAN_FIELD_FULL,
    EL_SOKOBAN_FIELD_PLAYER,
    EL_PLAYER_1,
    EL_PLAYER_2,
    EL_PLAYER_3,
    EL_PLAYER_4,
    EL_INVISIBLE_STEELWALL,

    -1
  };

  static int ep_gem[] =
  {
    EL_BD_DIAMOND,
    EL_EMERALD,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,

    -1
  };

  static int ep_food_dark_yamyam[] =
  {
    EL_SAND,
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,
    EL_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_AMOEBA_DROP,
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_PEARL,
    EL_CRYSTAL,

    -1
  };

  static int ep_food_penguin[] =
  {
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,
    EL_PEARL,
    EL_CRYSTAL,

    -1
  };

  static int ep_food_pig[] =
  {
    EL_EMERALD,
    EL_BD_DIAMOND,
    EL_EMERALD_YELLOW,
    EL_EMERALD_RED,
    EL_EMERALD_PURPLE,
    EL_DIAMOND,

    -1
  };

  static int ep_historic_wall[] =
  {
    EL_STEELWALL,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_GATE_1_GRAY_ACTIVE,
    EL_GATE_2_GRAY_ACTIVE,
    EL_GATE_3_GRAY_ACTIVE,
    EL_GATE_4_GRAY_ACTIVE,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EM_GATE_1_GRAY_ACTIVE,
    EL_EM_GATE_2_GRAY_ACTIVE,
    EL_EM_GATE_3_GRAY_ACTIVE,
    EL_EM_GATE_4_GRAY_ACTIVE,
    EL_EXIT_CLOSED,
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    EL_WALL,
    EL_WALL_SLIPPERY,
    EL_EXPANDABLE_WALL,
    EL_EXPANDABLE_WALL_HORIZONTAL,
    EL_EXPANDABLE_WALL_VERTICAL,
    EL_EXPANDABLE_WALL_ANY,
    EL_EXPANDABLE_WALL_GROWING,
    EL_BD_EXPANDABLE_WALL,
    EL_BD_WALL,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_SP_TERMINAL,
    EL_SP_TERMINAL_ACTIVE,
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_WALL_ACTIVE,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8,

    -1
  };

  static int ep_historic_solid[] =
  {
    EL_WALL,
    EL_EXPANDABLE_WALL,
    EL_EXPANDABLE_WALL_HORIZONTAL,
    EL_EXPANDABLE_WALL_VERTICAL,
    EL_EXPANDABLE_WALL_ANY,
    EL_BD_EXPANDABLE_WALL,
    EL_BD_WALL,
    EL_WALL_SLIPPERY,
    EL_EXIT_CLOSED,
    EL_EXIT_OPENING,
    EL_EXIT_OPEN,
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_QUICKSAND_EMPTY,
    EL_QUICKSAND_FULL,
    EL_QUICKSAND_FILLING,
    EL_QUICKSAND_EMPTYING,
    EL_MAGIC_WALL,
    EL_MAGIC_WALL_ACTIVE,
    EL_MAGIC_WALL_EMPTYING,
    EL_MAGIC_WALL_FILLING,
    EL_MAGIC_WALL_FULL,
    EL_MAGIC_WALL_DEAD,
    EL_BD_MAGIC_WALL,
    EL_BD_MAGIC_WALL_ACTIVE,
    EL_BD_MAGIC_WALL_EMPTYING,
    EL_BD_MAGIC_WALL_FULL,
    EL_BD_MAGIC_WALL_FILLING,
    EL_BD_MAGIC_WALL_DEAD,
    EL_GAME_OF_LIFE,
    EL_BIOMAZE,
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SP_TERMINAL,
    EL_SP_TERMINAL_ACTIVE,
    EL_SP_EXIT_CLOSED,
    EL_SP_EXIT_OPEN,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_WALL_ACTIVE,
    EL_SWITCHGATE_SWITCH_UP,
    EL_SWITCHGATE_SWITCH_DOWN,
    EL_DC_SWITCHGATE_SWITCH_UP,
    EL_DC_SWITCHGATE_SWITCH_DOWN,
    EL_TIMEGATE_SWITCH,
    EL_TIMEGATE_SWITCH_ACTIVE,
    EL_DC_TIMEGATE_SWITCH,
    EL_DC_TIMEGATE_SWITCH_ACTIVE,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8,
    EL_WALL_PEARL,
    EL_WALL_CRYSTAL,

    /* the following elements are a direct copy of "indestructible" elements,
       except "EL_ACID", which is "indestructible", but not "solid"! */
#if 0
    EL_ACID,
#endif
    EL_STEELWALL,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_ACID_POOL_BOTTOMLEFT,
    EL_ACID_POOL_BOTTOM,
    EL_ACID_POOL_BOTTOMRIGHT,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_STEELWALL_ACTIVE,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_LIGHT_SWITCH,
    EL_LIGHT_SWITCH_ACTIVE,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_NO_ENTRY,
    EL_SIGN_UNUSED_1,
    EL_SIGN_GIVE_WAY,
    EL_SIGN_ENTRY_FORBIDDEN,
    EL_SIGN_EMERGENCY_EXIT,
    EL_SIGN_YIN_YANG,
    EL_SIGN_UNUSED_2,
    EL_SIGN_SPERMS,
    EL_SIGN_BULLET,
    EL_SIGN_HEART,
    EL_SIGN_CROSS,
    EL_SIGN_FRANKIE,
    EL_STEEL_EXIT_CLOSED,
    EL_STEEL_EXIT_OPEN,
    EL_DC_STEELWALL_1_LEFT,
    EL_DC_STEELWALL_1_RIGHT,
    EL_DC_STEELWALL_1_TOP,
    EL_DC_STEELWALL_1_BOTTOM,
    EL_DC_STEELWALL_1_HORIZONTAL,
    EL_DC_STEELWALL_1_VERTICAL,
    EL_DC_STEELWALL_1_TOPLEFT,
    EL_DC_STEELWALL_1_TOPRIGHT,
    EL_DC_STEELWALL_1_BOTTOMLEFT,
    EL_DC_STEELWALL_1_BOTTOMRIGHT,
    EL_DC_STEELWALL_1_TOPLEFT_2,
    EL_DC_STEELWALL_1_TOPRIGHT_2,
    EL_DC_STEELWALL_1_BOTTOMLEFT_2,
    EL_DC_STEELWALL_1_BOTTOMRIGHT_2,
    EL_DC_STEELWALL_2_LEFT,
    EL_DC_STEELWALL_2_RIGHT,
    EL_DC_STEELWALL_2_TOP,
    EL_DC_STEELWALL_2_BOTTOM,
    EL_DC_STEELWALL_2_HORIZONTAL,
    EL_DC_STEELWALL_2_VERTICAL,
    EL_DC_STEELWALL_2_MIDDLE,
    EL_DC_STEELWALL_2_SINGLE,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_CRYSTAL,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_GATE_1_GRAY_ACTIVE,
    EL_GATE_2_GRAY_ACTIVE,
    EL_GATE_3_GRAY_ACTIVE,
    EL_GATE_4_GRAY_ACTIVE,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EM_GATE_1_GRAY_ACTIVE,
    EL_EM_GATE_2_GRAY_ACTIVE,
    EL_EM_GATE_3_GRAY_ACTIVE,
    EL_EM_GATE_4_GRAY_ACTIVE,
    EL_SWITCHGATE_OPEN,
    EL_SWITCHGATE_OPENING,
    EL_SWITCHGATE_CLOSED,
    EL_SWITCHGATE_CLOSING,
    EL_TIMEGATE_OPEN,
    EL_TIMEGATE_OPENING,
    EL_TIMEGATE_CLOSED,
    EL_TIMEGATE_CLOSING,
    EL_TUBE_ANY,
    EL_TUBE_VERTICAL,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,

    -1
  };

  static int ep_classic_enemy[] =
  {
    EL_BUG,
    EL_SPACESHIP,
    EL_BD_BUTTERFLY,
    EL_BD_FIREFLY,

    EL_YAMYAM,
    EL_DARK_YAMYAM,
    EL_ROBOT,
    EL_PACMAN,
    EL_SP_SNIKSNAK,
    EL_SP_ELECTRON,

    -1
  };

  static int ep_belt[] =
  {
    EL_CONVEYOR_BELT_1_LEFT,
    EL_CONVEYOR_BELT_1_MIDDLE,
    EL_CONVEYOR_BELT_1_RIGHT,
    EL_CONVEYOR_BELT_2_LEFT,
    EL_CONVEYOR_BELT_2_MIDDLE,
    EL_CONVEYOR_BELT_2_RIGHT,
    EL_CONVEYOR_BELT_3_LEFT,
    EL_CONVEYOR_BELT_3_MIDDLE,
    EL_CONVEYOR_BELT_3_RIGHT,
    EL_CONVEYOR_BELT_4_LEFT,
    EL_CONVEYOR_BELT_4_MIDDLE,
    EL_CONVEYOR_BELT_4_RIGHT,

    -1
  };

  static int ep_belt_active[] =
  {
    EL_CONVEYOR_BELT_1_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_1_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_1_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT_2_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_2_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_2_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT_3_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_3_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_3_RIGHT_ACTIVE,
    EL_CONVEYOR_BELT_4_LEFT_ACTIVE,
    EL_CONVEYOR_BELT_4_MIDDLE_ACTIVE,
    EL_CONVEYOR_BELT_4_RIGHT_ACTIVE,

    -1
  };

  static int ep_belt_switch[] =
  {
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,

    -1
  };

  static int ep_tube[] =
  {
    EL_TUBE_LEFT_UP,
    EL_TUBE_LEFT_DOWN,
    EL_TUBE_RIGHT_UP,
    EL_TUBE_RIGHT_DOWN,
    EL_TUBE_HORIZONTAL,
    EL_TUBE_HORIZONTAL_UP,
    EL_TUBE_HORIZONTAL_DOWN,
    EL_TUBE_VERTICAL,
    EL_TUBE_VERTICAL_LEFT,
    EL_TUBE_VERTICAL_RIGHT,
    EL_TUBE_ANY,

    -1
  };

  static int ep_acid_pool[] =
  {
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_ACID_POOL_BOTTOMLEFT,
    EL_ACID_POOL_BOTTOM,
    EL_ACID_POOL_BOTTOMRIGHT,

    -1
  };

  static int ep_keygate[] =
  {
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_GATE_1_GRAY_ACTIVE,
    EL_GATE_2_GRAY_ACTIVE,
    EL_GATE_3_GRAY_ACTIVE,
    EL_GATE_4_GRAY_ACTIVE,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EM_GATE_1_GRAY_ACTIVE,
    EL_EM_GATE_2_GRAY_ACTIVE,
    EL_EM_GATE_3_GRAY_ACTIVE,
    EL_EM_GATE_4_GRAY_ACTIVE,
    EL_EMC_GATE_5,
    EL_EMC_GATE_6,
    EL_EMC_GATE_7,
    EL_EMC_GATE_8,
    EL_EMC_GATE_5_GRAY,
    EL_EMC_GATE_6_GRAY,
    EL_EMC_GATE_7_GRAY,
    EL_EMC_GATE_8_GRAY,
    EL_EMC_GATE_5_GRAY_ACTIVE,
    EL_EMC_GATE_6_GRAY_ACTIVE,
    EL_EMC_GATE_7_GRAY_ACTIVE,
    EL_EMC_GATE_8_GRAY_ACTIVE,
    EL_DC_GATE_WHITE,
    EL_DC_GATE_WHITE_GRAY,
    EL_DC_GATE_WHITE_GRAY_ACTIVE,

    -1
  };

  static int ep_amoeboid[] =
  {
    EL_AMOEBA_DEAD,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_EMC_DRIPPER,

    -1
  };

  static int ep_amoebalive[] =
  {
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_EMC_DRIPPER,

    -1
  };

  static int ep_has_editor_content[] =
  {
    EL_PLAYER_1,
    EL_PLAYER_2,
    EL_PLAYER_3,
    EL_PLAYER_4,
    EL_SOKOBAN_FIELD_PLAYER,
    EL_SP_MURPHY,
    EL_YAMYAM,
    EL_YAMYAM_LEFT,
    EL_YAMYAM_RIGHT,
    EL_YAMYAM_UP,
    EL_YAMYAM_DOWN,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_BD_AMOEBA,
    EL_EMC_MAGIC_BALL,
    EL_EMC_ANDROID,

    -1
  };

  static int ep_can_turn_each_move[] =
  {
    /* !!! do something with this one !!! */
    -1
  };

  static int ep_can_grow[] =
  {
    EL_BD_AMOEBA,
    EL_AMOEBA_DROP,
    EL_AMOEBA_WET,
    EL_AMOEBA_DRY,
    EL_AMOEBA_FULL,
    EL_GAME_OF_LIFE,
    EL_BIOMAZE,
    EL_EMC_DRIPPER,

    -1
  };

  static int ep_active_bomb[] =
  {
    EL_DYNAMITE_ACTIVE,
    EL_EM_DYNAMITE_ACTIVE,
    EL_DYNABOMB_PLAYER_1_ACTIVE,
    EL_DYNABOMB_PLAYER_2_ACTIVE,
    EL_DYNABOMB_PLAYER_3_ACTIVE,
    EL_DYNABOMB_PLAYER_4_ACTIVE,
    EL_SP_DISK_RED_ACTIVE,

    -1
  };

  static int ep_inactive[] =
  {
    EL_EMPTY,
    EL_SAND,
    EL_WALL,
    EL_BD_WALL,
    EL_WALL_SLIPPERY,
    EL_STEELWALL,
    EL_AMOEBA_DEAD,
    EL_QUICKSAND_EMPTY,
    EL_QUICKSAND_FAST_EMPTY,
    EL_STONEBLOCK,
    EL_ROBOT_WHEEL,
    EL_KEY_1,
    EL_KEY_2,
    EL_KEY_3,
    EL_KEY_4,
    EL_EM_KEY_1,
    EL_EM_KEY_2,
    EL_EM_KEY_3,
    EL_EM_KEY_4,
    EL_EMC_KEY_5,
    EL_EMC_KEY_6,
    EL_EMC_KEY_7,
    EL_EMC_KEY_8,
    EL_GATE_1,
    EL_GATE_2,
    EL_GATE_3,
    EL_GATE_4,
    EL_GATE_1_GRAY,
    EL_GATE_2_GRAY,
    EL_GATE_3_GRAY,
    EL_GATE_4_GRAY,
    EL_GATE_1_GRAY_ACTIVE,
    EL_GATE_2_GRAY_ACTIVE,
    EL_GATE_3_GRAY_ACTIVE,
    EL_GATE_4_GRAY_ACTIVE,
    EL_EM_GATE_1,
    EL_EM_GATE_2,
    EL_EM_GATE_3,
    EL_EM_GATE_4,
    EL_EM_GATE_1_GRAY,
    EL_EM_GATE_2_GRAY,
    EL_EM_GATE_3_GRAY,
    EL_EM_GATE_4_GRAY,
    EL_EM_GATE_1_GRAY_ACTIVE,
    EL_EM_GATE_2_GRAY_ACTIVE,
    EL_EM_GATE_3_GRAY_ACTIVE,
    EL_EM_GATE_4_GRAY_ACTIVE,
    EL_EMC_GATE_5,
    EL_EMC_GATE_6,
    EL_EMC_GATE_7,
    EL_EMC_GATE_8,
    EL_EMC_GATE_5_GRAY,
    EL_EMC_GATE_6_GRAY,
    EL_EMC_GATE_7_GRAY,
    EL_EMC_GATE_8_GRAY,
    EL_EMC_GATE_5_GRAY_ACTIVE,
    EL_EMC_GATE_6_GRAY_ACTIVE,
    EL_EMC_GATE_7_GRAY_ACTIVE,
    EL_EMC_GATE_8_GRAY_ACTIVE,
    EL_DC_GATE_WHITE,
    EL_DC_GATE_WHITE_GRAY,
    EL_DC_GATE_WHITE_GRAY_ACTIVE,
    EL_DC_GATE_FAKE_GRAY,
    EL_DYNAMITE,
    EL_EM_DYNAMITE,
    EL_INVISIBLE_STEELWALL,
    EL_INVISIBLE_WALL,
    EL_INVISIBLE_SAND,
    EL_LAMP,
    EL_LAMP_ACTIVE,
    EL_WALL_EMERALD,
    EL_WALL_DIAMOND,
    EL_WALL_BD_DIAMOND,
    EL_WALL_EMERALD_YELLOW,
    EL_DYNABOMB_INCREASE_NUMBER,
    EL_DYNABOMB_INCREASE_SIZE,
    EL_DYNABOMB_INCREASE_POWER,
#if 0
    EL_SOKOBAN_OBJECT,
#endif
    EL_SOKOBAN_FIELD_EMPTY,
    EL_SOKOBAN_FIELD_FULL,
    EL_WALL_EMERALD_RED,
    EL_WALL_EMERALD_PURPLE,
    EL_ACID_POOL_TOPLEFT,
    EL_ACID_POOL_TOPRIGHT,
    EL_ACID_POOL_BOTTOMLEFT,
    EL_ACID_POOL_BOTTOM,
    EL_ACID_POOL_BOTTOMRIGHT,
    EL_MAGIC_WALL,
    EL_MAGIC_WALL_DEAD,
    EL_BD_MAGIC_WALL,
    EL_BD_MAGIC_WALL_DEAD,
    EL_DC_MAGIC_WALL,
    EL_DC_MAGIC_WALL_DEAD,
    EL_AMOEBA_TO_DIAMOND,
    EL_BLOCKED,
    EL_SP_EMPTY,
    EL_SP_BASE,
    EL_SP_PORT_RIGHT,
    EL_SP_PORT_DOWN,
    EL_SP_PORT_LEFT,
    EL_SP_PORT_UP,
    EL_SP_GRAVITY_PORT_RIGHT,
    EL_SP_GRAVITY_PORT_DOWN,
    EL_SP_GRAVITY_PORT_LEFT,
    EL_SP_GRAVITY_PORT_UP,
    EL_SP_PORT_HORIZONTAL,
    EL_SP_PORT_VERTICAL,
    EL_SP_PORT_ANY,
    EL_SP_DISK_RED,
#if 0
    EL_SP_DISK_YELLOW,
#endif
    EL_SP_CHIP_SINGLE,
    EL_SP_CHIP_LEFT,
    EL_SP_CHIP_RIGHT,
    EL_SP_CHIP_TOP,
    EL_SP_CHIP_BOTTOM,
    EL_SP_HARDWARE_GRAY,
    EL_SP_HARDWARE_GREEN,
    EL_SP_HARDWARE_BLUE,
    EL_SP_HARDWARE_RED,
    EL_SP_HARDWARE_YELLOW,
    EL_SP_HARDWARE_BASE_1,
    EL_SP_HARDWARE_BASE_2,
    EL_SP_HARDWARE_BASE_3,
    EL_SP_HARDWARE_BASE_4,
    EL_SP_HARDWARE_BASE_5,
    EL_SP_HARDWARE_BASE_6,
    EL_SP_GRAVITY_ON_PORT_LEFT,
    EL_SP_GRAVITY_ON_PORT_RIGHT,
    EL_SP_GRAVITY_ON_PORT_UP,
    EL_SP_GRAVITY_ON_PORT_DOWN,
    EL_SP_GRAVITY_OFF_PORT_LEFT,
    EL_SP_GRAVITY_OFF_PORT_RIGHT,
    EL_SP_GRAVITY_OFF_PORT_UP,
    EL_SP_GRAVITY_OFF_PORT_DOWN,
    EL_CONVEYOR_BELT_1_SWITCH_LEFT,
    EL_CONVEYOR_BELT_1_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_1_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_2_SWITCH_LEFT,
    EL_CONVEYOR_BELT_2_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_2_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_3_SWITCH_LEFT,
    EL_CONVEYOR_BELT_3_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_3_SWITCH_RIGHT,
    EL_CONVEYOR_BELT_4_SWITCH_LEFT,
    EL_CONVEYOR_BELT_4_SWITCH_MIDDLE,
    EL_CONVEYOR_BELT_4_SWITCH_RIGHT,
    EL_SIGN_EXCLAMATION,
    EL_SIGN_RADIOACTIVITY,
    EL_SIGN_STOP,
    EL_SIGN_WHEELCHAIR,
    EL_SIGN_PARKING,
    EL_SIGN_NO_ENTRY,
    EL_SIGN_UNUSED_1,
    EL_SIGN_GIVE_WAY,
    EL_SIGN_ENTRY_FORBIDDEN,
    EL_SIGN_EMERGENCY_EXIT,
    EL_SIGN_YIN_YANG,
    EL_SIGN_UNUSED_2,
    EL_SIGN_SPERMS,
    EL_SIGN_BULLET,
    EL_SIGN_HEART,
    EL_SIGN_CROSS,
    EL_SIGN_FRANKIE,
    EL_DC_STEELWALL_1_LEFT,
    EL_DC_STEELWALL_1_RIGHT,
    EL_DC_STEELWALL_1_TOP,
    EL_DC_STEELWALL_1_BOTTOM,
    EL_DC_STEELWALL_1_HORIZONTAL,
    EL_DC_STEELWALL_1_VERTICAL,
    EL_DC_STEELWALL_1_TOPLEFT,
    EL_DC_STEELWALL_1_TOPRIGHT,
    EL_DC_STEELWALL_1_BOTTOMLEFT,
    EL_DC_STEELWALL_1_BOTTOMRIGHT,
    EL_DC_STEELWALL_1_TOPLEFT_2,
    EL_DC_STEELWALL_1_TOPRIGHT_2,
    EL_DC_STEELWALL_1_BOTTOMLEFT_2,
    EL_DC_STEELWALL_1_BOTTOMRIGHT_2,
    EL_DC_STEELWALL_2_LEFT,
    EL_DC_STEELWALL_2_RIGHT,
    EL_DC_STEELWALL_2_TOP,
    EL_DC_STEELWALL_2_BOTTOM,
    EL_DC_STEELWALL_2_HORIZONTAL,
    EL_DC_STEELWALL_2_VERTICAL,
    EL_DC_STEELWALL_2_MIDDLE,
    EL_DC_STEELWALL_2_SINGLE,
    EL_STEELWALL_SLIPPERY,
    EL_EMC_STEELWALL_1,
    EL_EMC_STEELWALL_2,
    EL_EMC_STEELWALL_3,
    EL_EMC_STEELWALL_4,
    EL_EMC_WALL_SLIPPERY_1,
    EL_EMC_WALL_SLIPPERY_2,
    EL_EMC_WALL_SLIPPERY_3,
    EL_EMC_WALL_SLIPPERY_4,
    EL_EMC_WALL_1,
    EL_EMC_WALL_2,
    EL_EMC_WALL_3,
    EL_EMC_WALL_4,
    EL_EMC_WALL_5,
    EL_EMC_WALL_6,
    EL_EMC_WALL_7,
    EL_EMC_WALL_8,
    EL_EMC_WALL_9,
    EL_EMC_WALL_10,
    EL_EMC_WALL_11,
    EL_EMC_WALL_12,
    EL_EMC_WALL_13,
    EL_EMC_WALL_14,
    EL_EMC_WALL_15,
    EL_EMC_WALL_16,

    -1
  };

  static int ep_em_slippery_wall[] =
  {
    -1
  };

  static int ep_gfx_crumbled[] =
  {
    EL_SAND,
    EL_LANDMINE,
    EL_DC_LANDMINE,
    EL_TRAP,
    EL_TRAP_ACTIVE,

    -1
  };

  static int ep_editor_cascade_active[] =
  {
    EL_INTERNAL_CASCADE_BD_ACTIVE,
    EL_INTERNAL_CASCADE_EM_ACTIVE,
    EL_INTERNAL_CASCADE_EMC_ACTIVE,
    EL_INTERNAL_CASCADE_RND_ACTIVE,
    EL_INTERNAL_CASCADE_SB_ACTIVE,
    EL_INTERNAL_CASCADE_SP_ACTIVE,
    EL_INTERNAL_CASCADE_DC_ACTIVE,
    EL_INTERNAL_CASCADE_DX_ACTIVE,
    EL_INTERNAL_CASCADE_MM_ACTIVE,
    EL_INTERNAL_CASCADE_DF_ACTIVE,
    EL_INTERNAL_CASCADE_CHARS_ACTIVE,
    EL_INTERNAL_CASCADE_STEEL_CHARS_ACTIVE,
    EL_INTERNAL_CASCADE_CE_ACTIVE,
    EL_INTERNAL_CASCADE_GE_ACTIVE,
    EL_INTERNAL_CASCADE_REF_ACTIVE,
    EL_INTERNAL_CASCADE_USER_ACTIVE,
    EL_INTERNAL_CASCADE_DYNAMIC_ACTIVE,

    -1
  };

  static int ep_editor_cascade_inactive[] =
  {
    EL_INTERNAL_CASCADE_BD,
    EL_INTERNAL_CASCADE_EM,
    EL_INTERNAL_CASCADE_EMC,
    EL_INTERNAL_CASCADE_RND,
    EL_INTERNAL_CASCADE_SB,
    EL_INTERNAL_CASCADE_SP,
    EL_INTERNAL_CASCADE_DC,
    EL_INTERNAL_CASCADE_DX,
    EL_INTERNAL_CASCADE_MM,
    EL_INTERNAL_CASCADE_DF,
    EL_INTERNAL_CASCADE_CHARS,
    EL_INTERNAL_CASCADE_STEEL_CHARS,
    EL_INTERNAL_CASCADE_CE,
    EL_INTERNAL_CASCADE_GE,
    EL_INTERNAL_CASCADE_REF,
    EL_INTERNAL_CASCADE_USER,
    EL_INTERNAL_CASCADE_DYNAMIC,

    -1
  };

  static int ep_obsolete[] =
  {
    EL_PLAYER_OBSOLETE,
    EL_KEY_OBSOLETE,
    EL_EM_KEY_1_FILE_OBSOLETE,
    EL_EM_KEY_2_FILE_OBSOLETE,
    EL_EM_KEY_3_FILE_OBSOLETE,
    EL_EM_KEY_4_FILE_OBSOLETE,
    EL_ENVELOPE_OBSOLETE,

    -1
  };

  static struct
  {
    int *elements;
    int property;
  } element_properties[] =
  {
    { ep_diggable,			EP_DIGGABLE			},
    { ep_collectible_only,		EP_COLLECTIBLE_ONLY		},
    { ep_dont_run_into,			EP_DONT_RUN_INTO		},
    { ep_dont_collide_with,		EP_DONT_COLLIDE_WITH		},
    { ep_dont_touch,			EP_DONT_TOUCH			},
    { ep_indestructible,		EP_INDESTRUCTIBLE		},
    { ep_slippery,			EP_SLIPPERY			},
    { ep_can_change,			EP_CAN_CHANGE			},
    { ep_can_move,			EP_CAN_MOVE			},
    { ep_can_fall,			EP_CAN_FALL			},
    { ep_can_smash_player,		EP_CAN_SMASH_PLAYER		},
    { ep_can_smash_enemies,		EP_CAN_SMASH_ENEMIES		},
    { ep_can_smash_everything,		EP_CAN_SMASH_EVERYTHING		},
    { ep_explodes_by_fire,		EP_EXPLODES_BY_FIRE		},
    { ep_explodes_smashed,		EP_EXPLODES_SMASHED		},
    { ep_explodes_impact,		EP_EXPLODES_IMPACT		},
    { ep_walkable_over,			EP_WALKABLE_OVER		},
    { ep_walkable_inside,		EP_WALKABLE_INSIDE		},
    { ep_walkable_under,		EP_WALKABLE_UNDER		},
    { ep_passable_over,			EP_PASSABLE_OVER		},
    { ep_passable_inside,		EP_PASSABLE_INSIDE		},
    { ep_passable_under,		EP_PASSABLE_UNDER		},
    { ep_droppable,			EP_DROPPABLE			},
    { ep_explodes_1x1_old,		EP_EXPLODES_1X1_OLD		},
    { ep_pushable,			EP_PUSHABLE			},
    { ep_explodes_cross_old,		EP_EXPLODES_CROSS_OLD		},
    { ep_protected,			EP_PROTECTED			},
    { ep_throwable,			EP_THROWABLE			},
    { ep_can_explode,			EP_CAN_EXPLODE			},
    { ep_gravity_reachable,		EP_GRAVITY_REACHABLE		},

    { ep_player,			EP_PLAYER			},
    { ep_can_pass_magic_wall,		EP_CAN_PASS_MAGIC_WALL		},
    { ep_can_pass_dc_magic_wall,	EP_CAN_PASS_DC_MAGIC_WALL	},
    { ep_switchable,			EP_SWITCHABLE			},
    { ep_bd_element,			EP_BD_ELEMENT			},
    { ep_sp_element,			EP_SP_ELEMENT			},
    { ep_sb_element,			EP_SB_ELEMENT			},
    { ep_gem,				EP_GEM				},
    { ep_food_dark_yamyam,		EP_FOOD_DARK_YAMYAM		},
    { ep_food_penguin,			EP_FOOD_PENGUIN			},
    { ep_food_pig,			EP_FOOD_PIG			},
    { ep_historic_wall,			EP_HISTORIC_WALL		},
    { ep_historic_solid,		EP_HISTORIC_SOLID		},
    { ep_classic_enemy,			EP_CLASSIC_ENEMY		},
    { ep_belt,				EP_BELT				},
    { ep_belt_active,			EP_BELT_ACTIVE			},
    { ep_belt_switch,			EP_BELT_SWITCH			},
    { ep_tube,				EP_TUBE				},
    { ep_acid_pool,			EP_ACID_POOL			},
    { ep_keygate,			EP_KEYGATE			},
    { ep_amoeboid,			EP_AMOEBOID			},
    { ep_amoebalive,			EP_AMOEBALIVE			},
    { ep_has_editor_content,		EP_HAS_EDITOR_CONTENT		},
    { ep_can_turn_each_move,		EP_CAN_TURN_EACH_MOVE		},
    { ep_can_grow,			EP_CAN_GROW			},
    { ep_active_bomb,			EP_ACTIVE_BOMB			},
    { ep_inactive,			EP_INACTIVE			},

    { ep_em_slippery_wall,		EP_EM_SLIPPERY_WALL		},

    { ep_gfx_crumbled,			EP_GFX_CRUMBLED			},

    { ep_editor_cascade_active,		EP_EDITOR_CASCADE_ACTIVE	},
    { ep_editor_cascade_inactive, 	EP_EDITOR_CASCADE_INACTIVE	},

    { ep_obsolete,		 	EP_OBSOLETE			},

    { NULL,				-1				}
  };

  int i, j, k;

  /* always start with reliable default values (element has no properties) */
  /* (but never initialize clipboard elements after the very first time) */
  /* (to be able to use clipboard elements between several levels) */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    if (!IS_CLIPBOARD_ELEMENT(i) || !clipboard_elements_initialized)
      for (j = 0; j < NUM_ELEMENT_PROPERTIES; j++)
	SET_PROPERTY(i, j, FALSE);

  /* set all base element properties from above array definitions */
  for (i = 0; element_properties[i].elements != NULL; i++)
    for (j = 0; (element_properties[i].elements)[j] != -1; j++)
      SET_PROPERTY((element_properties[i].elements)[j],
		   element_properties[i].property, TRUE);

  /* copy properties to some elements that are only stored in level file */
  for (i = 0; i < NUM_ELEMENT_PROPERTIES; i++)
    for (j = 0; copy_properties[j][0] != -1; j++)
      if (HAS_PROPERTY(copy_properties[j][0], i))
	for (k = 1; k <= 4; k++)
	  SET_PROPERTY(copy_properties[j][k], i, TRUE);

  /* set static element properties that are not listed in array definitions */
  for (i = EL_STEEL_CHAR_START; i <= EL_STEEL_CHAR_END; i++)
    SET_PROPERTY(i, EP_INDESTRUCTIBLE, TRUE);

  clipboard_elements_initialized = TRUE;
}

void InitElementPropertiesEngine(int engine_version)
{
  static int no_wall_properties[] =
  {
    EP_DIGGABLE,
    EP_COLLECTIBLE_ONLY,
    EP_DONT_RUN_INTO,
    EP_DONT_COLLIDE_WITH,
    EP_CAN_MOVE,
    EP_CAN_FALL,
    EP_CAN_SMASH_PLAYER,
    EP_CAN_SMASH_ENEMIES,
    EP_CAN_SMASH_EVERYTHING,
    EP_PUSHABLE,

    EP_PLAYER,
    EP_GEM,
    EP_FOOD_DARK_YAMYAM,
    EP_FOOD_PENGUIN,
    EP_BELT,
    EP_BELT_ACTIVE,
    EP_TUBE,
    EP_AMOEBOID,
    EP_AMOEBALIVE,
    EP_ACTIVE_BOMB,

    EP_ACCESSIBLE,

    -1
  };

  int i, j;

  /* important: after initialization in InitElementPropertiesStatic(), the
     elements are not again initialized to a default value; therefore all
     changes have to make sure that they leave the element with a defined
     property (which means that conditional property changes must be set to
     a reliable default value before) */

  /* resolve group elements */
  for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
    ResolveGroupElement(EL_GROUP_START + i);

  /* set all special, combined or engine dependent element properties */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    /* do not change (already initialized) clipboard elements here */
    if (IS_CLIPBOARD_ELEMENT(i))
      continue;

    /* ---------- INACTIVE ------------------------------------------------- */
    SET_PROPERTY(i, EP_INACTIVE, ((i >= EL_CHAR_START &&
				   i <= EL_CHAR_END) ||
				  (i >= EL_STEEL_CHAR_START &&
				   i <= EL_STEEL_CHAR_END)));

    /* ---------- WALKABLE, PASSABLE, ACCESSIBLE --------------------------- */
    SET_PROPERTY(i, EP_WALKABLE, (IS_WALKABLE_OVER(i) ||
				  IS_WALKABLE_INSIDE(i) ||
				  IS_WALKABLE_UNDER(i)));

    SET_PROPERTY(i, EP_PASSABLE, (IS_PASSABLE_OVER(i) ||
				  IS_PASSABLE_INSIDE(i) ||
				  IS_PASSABLE_UNDER(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE_OVER, (IS_WALKABLE_OVER(i) ||
					 IS_PASSABLE_OVER(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE_INSIDE, (IS_WALKABLE_INSIDE(i) ||
					   IS_PASSABLE_INSIDE(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE_UNDER, (IS_WALKABLE_UNDER(i) ||
					  IS_PASSABLE_UNDER(i)));

    SET_PROPERTY(i, EP_ACCESSIBLE, (IS_WALKABLE(i) ||
				    IS_PASSABLE(i)));

    /* ---------- COLLECTIBLE ---------------------------------------------- */
    SET_PROPERTY(i, EP_COLLECTIBLE, (IS_COLLECTIBLE_ONLY(i) ||
				     IS_DROPPABLE(i) ||
				     IS_THROWABLE(i)));

    /* ---------- SNAPPABLE ------------------------------------------------ */
    SET_PROPERTY(i, EP_SNAPPABLE, (IS_DIGGABLE(i) ||
				   IS_COLLECTIBLE(i) ||
				   IS_SWITCHABLE(i) ||
				   i == EL_BD_ROCK));

    /* ---------- WALL ----------------------------------------------------- */
    SET_PROPERTY(i, EP_WALL, TRUE);	/* default: element is wall */

    for (j = 0; no_wall_properties[j] != -1; j++)
      if (HAS_PROPERTY(i, no_wall_properties[j]) ||
	  i >= EL_FIRST_RUNTIME_UNREAL)
	SET_PROPERTY(i, EP_WALL, FALSE);

    if (IS_HISTORIC_WALL(i))
      SET_PROPERTY(i, EP_WALL, TRUE);

    /* ---------- SOLID_FOR_PUSHING ---------------------------------------- */
    if (engine_version < VERSION_IDENT(2,2,0,0))
      SET_PROPERTY(i, EP_SOLID_FOR_PUSHING, IS_HISTORIC_SOLID(i));
    else
      SET_PROPERTY(i, EP_SOLID_FOR_PUSHING, (!IS_WALKABLE(i) &&
					     !IS_DIGGABLE(i) &&
					     !IS_COLLECTIBLE(i)));

    /* ---------- DRAGONFIRE_PROOF ----------------------------------------- */
    if (IS_HISTORIC_SOLID(i) || i == EL_EXPLOSION)
      SET_PROPERTY(i, EP_DRAGONFIRE_PROOF, TRUE);
    else
      SET_PROPERTY(i, EP_DRAGONFIRE_PROOF, (IS_CUSTOM_ELEMENT(i) &&
					    IS_INDESTRUCTIBLE(i)));

    /* ---------- EXPLOSION_PROOF ------------------------------------------ */
    if (i == EL_FLAMES)
      SET_PROPERTY(i, EP_EXPLOSION_PROOF, TRUE);
    else if (engine_version < VERSION_IDENT(2,2,0,0))
      SET_PROPERTY(i, EP_EXPLOSION_PROOF, IS_INDESTRUCTIBLE(i));
    else
      SET_PROPERTY(i, EP_EXPLOSION_PROOF, (IS_INDESTRUCTIBLE(i) &&
					   (!IS_WALKABLE(i) ||
					    IS_PROTECTED(i))));

    if (IS_CUSTOM_ELEMENT(i))
    {
      /* these are additional properties which are initially false when set */

      /* ---------- DONT_COLLIDE_WITH / DONT_RUN_INTO ---------------------- */
      if (DONT_TOUCH(i))
	SET_PROPERTY(i, EP_DONT_COLLIDE_WITH, TRUE);
      if (DONT_COLLIDE_WITH(i))
	SET_PROPERTY(i, EP_DONT_RUN_INTO, TRUE);

      /* ---------- CAN_SMASH_ENEMIES / CAN_SMASH_PLAYER ------------------- */
      if (CAN_SMASH_EVERYTHING(i))
	SET_PROPERTY(i, EP_CAN_SMASH_ENEMIES, TRUE);
      if (CAN_SMASH_ENEMIES(i))
	SET_PROPERTY(i, EP_CAN_SMASH_PLAYER, TRUE);
    }

    /* ---------- CAN_SMASH ------------------------------------------------ */
    SET_PROPERTY(i, EP_CAN_SMASH, (CAN_SMASH_PLAYER(i) ||
				   CAN_SMASH_ENEMIES(i) ||
				   CAN_SMASH_EVERYTHING(i)));

    /* ---------- CAN_EXPLODE_BY_FIRE -------------------------------------- */
    SET_PROPERTY(i, EP_CAN_EXPLODE_BY_FIRE, (CAN_EXPLODE(i) &&
					     EXPLODES_BY_FIRE(i)));

    /* ---------- CAN_EXPLODE_SMASHED -------------------------------------- */
    SET_PROPERTY(i, EP_CAN_EXPLODE_SMASHED, (CAN_EXPLODE(i) &&
					     EXPLODES_SMASHED(i)));

    /* ---------- CAN_EXPLODE_IMPACT --------------------------------------- */
    SET_PROPERTY(i, EP_CAN_EXPLODE_IMPACT, (CAN_EXPLODE(i) &&
					    EXPLODES_IMPACT(i)));

    /* ---------- CAN_EXPLODE_BY_DRAGONFIRE -------------------------------- */
    SET_PROPERTY(i, EP_CAN_EXPLODE_BY_DRAGONFIRE, CAN_EXPLODE_BY_FIRE(i));

    /* ---------- CAN_EXPLODE_BY_EXPLOSION --------------------------------- */
    SET_PROPERTY(i, EP_CAN_EXPLODE_BY_EXPLOSION, (CAN_EXPLODE_BY_FIRE(i) ||
						  i == EL_BLACK_ORB));

    /* ---------- COULD_MOVE_INTO_ACID ------------------------------------- */
    SET_PROPERTY(i, EP_COULD_MOVE_INTO_ACID, (ELEM_IS_PLAYER(i) ||
					      CAN_MOVE(i) ||
					      IS_CUSTOM_ELEMENT(i)));

    /* ---------- MAYBE_DONT_COLLIDE_WITH ---------------------------------- */
    SET_PROPERTY(i, EP_MAYBE_DONT_COLLIDE_WITH, (i == EL_SP_SNIKSNAK ||
						 i == EL_SP_ELECTRON));

    /* ---------- CAN_MOVE_INTO_ACID --------------------------------------- */
    if (COULD_MOVE_INTO_ACID(i) && !IS_CUSTOM_ELEMENT(i))
      SET_PROPERTY(i, EP_CAN_MOVE_INTO_ACID,
		   getMoveIntoAcidProperty(&level, i));

    /* ---------- DONT_COLLIDE_WITH ---------------------------------------- */
    if (MAYBE_DONT_COLLIDE_WITH(i))
      SET_PROPERTY(i, EP_DONT_COLLIDE_WITH,
		   getDontCollideWithProperty(&level, i));

    /* ---------- SP_PORT -------------------------------------------------- */
    SET_PROPERTY(i, EP_SP_PORT, (IS_SP_ELEMENT(i) &&
				 IS_PASSABLE_INSIDE(i)));

    /* ---------- CAN_BE_CLONED_BY_ANDROID --------------------------------- */
    for (j = 0; j < level.num_android_clone_elements; j++)
      SET_PROPERTY(i, EP_CAN_BE_CLONED_BY_ANDROID,
		   (i != EL_EMPTY &&
		    IS_EQUAL_OR_IN_GROUP(i, level.android_clone_element[j])));

    /* ---------- CAN_CHANGE ----------------------------------------------- */
    SET_PROPERTY(i, EP_CAN_CHANGE, FALSE);	/* default: cannot change */
    for (j = 0; j < element_info[i].num_change_pages; j++)
      if (element_info[i].change_page[j].can_change)
	SET_PROPERTY(i, EP_CAN_CHANGE, TRUE);

    /* ---------- HAS_ACTION ----------------------------------------------- */
    SET_PROPERTY(i, EP_HAS_ACTION, FALSE);	/* default: has no action */
    for (j = 0; j < element_info[i].num_change_pages; j++)
      if (element_info[i].change_page[j].has_action)
	SET_PROPERTY(i, EP_HAS_ACTION, TRUE);

    /* ---------- CAN_CHANGE_OR_HAS_ACTION --------------------------------- */
    SET_PROPERTY(i, EP_CAN_CHANGE_OR_HAS_ACTION, (CAN_CHANGE(i) ||
						  HAS_ACTION(i)));

    /* ---------- GFX_CRUMBLED --------------------------------------------- */
    SET_PROPERTY(i, EP_GFX_CRUMBLED,
		 element_info[i].crumbled[ACTION_DEFAULT] !=
		 element_info[i].graphic[ACTION_DEFAULT]);

    /* ---------- EDITOR_CASCADE ------------------------------------------- */
    SET_PROPERTY(i, EP_EDITOR_CASCADE, (IS_EDITOR_CASCADE_ACTIVE(i) ||
					IS_EDITOR_CASCADE_INACTIVE(i)));
  }

  /* dynamically adjust element properties according to game engine version */
  {
    static int ep_em_slippery_wall[] =
    {
      EL_WALL,
      EL_STEELWALL,
      EL_EXPANDABLE_WALL,
      EL_EXPANDABLE_WALL_HORIZONTAL,
      EL_EXPANDABLE_WALL_VERTICAL,
      EL_EXPANDABLE_WALL_ANY,
      EL_EXPANDABLE_STEELWALL_HORIZONTAL,
      EL_EXPANDABLE_STEELWALL_VERTICAL,
      EL_EXPANDABLE_STEELWALL_ANY,
      EL_EXPANDABLE_STEELWALL_GROWING,
      -1
    };

    static int ep_em_explodes_by_fire[] =
    {
      EL_EM_DYNAMITE,
      EL_EM_DYNAMITE_ACTIVE,
      EL_MOLE,
      -1
    };

    /* special EM style gems behaviour */
    for (i = 0; ep_em_slippery_wall[i] != -1; i++)
      SET_PROPERTY(ep_em_slippery_wall[i], EP_EM_SLIPPERY_WALL,
		   level.em_slippery_gems);

    /* "EL_EXPANDABLE_WALL_GROWING" wasn't slippery for EM gems in 2.0.1 */
    SET_PROPERTY(EL_EXPANDABLE_WALL_GROWING, EP_EM_SLIPPERY_WALL,
		 (level.em_slippery_gems &&
		  engine_version > VERSION_IDENT(2,0,1,0)));

    /* special EM style explosion behaviour regarding chain reactions */
    for (i = 0; ep_em_explodes_by_fire[i] != -1; i++)
      SET_PROPERTY(ep_em_explodes_by_fire[i], EP_EXPLODES_BY_FIRE,
		   level.em_explodes_by_fire);
  }

  /* this is needed because some graphics depend on element properties */
  if (game_status == GAME_MODE_PLAYING)
    InitElementGraphicInfo();
}

void InitElementPropertiesGfxElement()
{
  int i;

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    struct ElementInfo *ei = &element_info[i];

    ei->gfx_element = (ei->use_gfx_element ? ei->gfx_element_initial : i);
  }
}

static void InitGlobal()
{
  int graphic;
  int i;

  for (i = 0; i < MAX_NUM_ELEMENTS + 1; i++)
  {
    /* check if element_name_info entry defined for each element in "main.h" */
    if (i < MAX_NUM_ELEMENTS && element_name_info[i].token_name == NULL)
      Error(ERR_EXIT, "undefined 'element_name_info' entry for element %d", i);

    element_info[i].token_name = element_name_info[i].token_name;
    element_info[i].class_name = element_name_info[i].class_name;
    element_info[i].editor_description= element_name_info[i].editor_description;
  }

  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS + 1; i++)
  {
    /* check if global_anim_name_info defined for each entry in "main.h" */
    if (i < NUM_GLOBAL_ANIM_TOKENS &&
	global_anim_name_info[i].token_name == NULL)
      Error(ERR_EXIT, "undefined 'global_anim_name_info' entry for anim %d", i);

    global_anim_info[i].token_name = global_anim_name_info[i].token_name;
  }

  /* create hash from image config list */
  image_config_hash = newSetupFileHash();
  for (i = 0; image_config[i].token != NULL; i++)
    setHashEntry(image_config_hash,
		 image_config[i].token,
		 image_config[i].value);

  /* create hash from element token list */
  element_token_hash = newSetupFileHash();
  for (i = 0; element_name_info[i].token_name != NULL; i++)
    setHashEntry(element_token_hash,
		 element_name_info[i].token_name,
		 int2str(i, 0));

  /* create hash from graphic token list */
  graphic_token_hash = newSetupFileHash();
  for (graphic = 0, i = 0; image_config[i].token != NULL; i++)
    if (strSuffix(image_config[i].value, ".png") ||
	strSuffix(image_config[i].value, ".pcx") ||
	strSuffix(image_config[i].value, ".wav") ||
	strEqual(image_config[i].value, UNDEFINED_FILENAME))
      setHashEntry(graphic_token_hash,
		   image_config[i].token,
		   int2str(graphic++, 0));

  /* create hash from font token list */
  font_token_hash = newSetupFileHash();
  for (i = 0; font_info[i].token_name != NULL; i++)
    setHashEntry(font_token_hash,
		 font_info[i].token_name,
		 int2str(i, 0));

  /* set default filenames for all cloned graphics in static configuration */
  for (i = 0; image_config[i].token != NULL; i++)
  {
    if (strEqual(image_config[i].value, UNDEFINED_FILENAME))
    {
      char *token = image_config[i].token;
      char *token_clone_from = getStringCat2(token, ".clone_from");
      char *token_cloned = getHashEntry(image_config_hash, token_clone_from);

      if (token_cloned != NULL)
      {
	char *value_cloned = getHashEntry(image_config_hash, token_cloned);

	if (value_cloned != NULL)
	{
	  /* set default filename in static configuration */
	  image_config[i].value = value_cloned;

	  /* set default filename in image config hash */
	  setHashEntry(image_config_hash, token, value_cloned);
	}
      }

      free(token_clone_from);
    }
  }

  /* always start with reliable default values (all elements) */
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    ActiveElement[i] = i;

  /* now add all entries that have an active state (active elements) */
  for (i = 0; element_with_active_state[i].element != -1; i++)
  {
    int element = element_with_active_state[i].element;
    int element_active = element_with_active_state[i].element_active;

    ActiveElement[element] = element_active;
  }

  /* always start with reliable default values (all buttons) */
  for (i = 0; i < NUM_IMAGE_FILES; i++)
    ActiveButton[i] = i;

  /* now add all entries that have an active state (active buttons) */
  for (i = 0; button_with_active_state[i].button != -1; i++)
  {
    int button = button_with_active_state[i].button;
    int button_active = button_with_active_state[i].button_active;

    ActiveButton[button] = button_active;
  }

  /* always start with reliable default values (all fonts) */
  for (i = 0; i < NUM_FONTS; i++)
    ActiveFont[i] = i;

  /* now add all entries that have an active state (active fonts) */
  for (i = 0; font_with_active_state[i].font_nr != -1; i++)
  {
    int font = font_with_active_state[i].font_nr;
    int font_active = font_with_active_state[i].font_nr_active;

    ActiveFont[font] = font_active;
  }

  global.autoplay_leveldir = NULL;
  global.convert_leveldir = NULL;
  global.create_images_dir = NULL;

  global.frames_per_second = 0;
  global.show_frames_per_second = FALSE;

  global.border_status = GAME_MODE_LOADING;
  global.anim_status = global.anim_status_next = GAME_MODE_LOADING;

  global.use_envelope_request = FALSE;
}

void Execute_Command(char *command)
{
  int i;

  if (strEqual(command, "print graphicsinfo.conf"))
  {
    Print("# You can configure additional/alternative image files here.\n");
    Print("# (The entries below are default and therefore commented out.)\n");
    Print("\n");
    Print("%s\n", getFormattedSetupEntry("name", "Classic Graphics"));
    Print("\n");
    Print("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    Print("\n");

    for (i = 0; image_config[i].token != NULL; i++)
      Print("# %s\n", getFormattedSetupEntry(image_config[i].token,
					     image_config[i].value));

    exit(0);
  }
  else if (strEqual(command, "print soundsinfo.conf"))
  {
    Print("# You can configure additional/alternative sound files here.\n");
    Print("# (The entries below are default and therefore commented out.)\n");
    Print("\n");
    Print("%s\n", getFormattedSetupEntry("name", "Classic Sounds"));
    Print("\n");
    Print("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    Print("\n");

    for (i = 0; sound_config[i].token != NULL; i++)
      Print("# %s\n", getFormattedSetupEntry(sound_config[i].token,
					     sound_config[i].value));

    exit(0);
  }
  else if (strEqual(command, "print musicinfo.conf"))
  {
    Print("# You can configure additional/alternative music files here.\n");
    Print("# (The entries below are default and therefore commented out.)\n");
    Print("\n");
    Print("%s\n", getFormattedSetupEntry("name", "Classic Music"));
    Print("\n");
    Print("%s\n", getFormattedSetupEntry("sort_priority", "100"));
    Print("\n");

    for (i = 0; music_config[i].token != NULL; i++)
      Print("# %s\n", getFormattedSetupEntry(music_config[i].token,
					     music_config[i].value));

    exit(0);
  }
  else if (strEqual(command, "print editorsetup.conf"))
  {
    Print("# You can configure your personal editor element list here.\n");
    Print("# (The entries below are default and therefore commented out.)\n");
    Print("\n");

    /* this is needed to be able to check element list for cascade elements */
    InitElementPropertiesStatic();
    InitElementPropertiesEngine(GAME_VERSION_ACTUAL);

    PrintEditorElementList();

    exit(0);
  }
  else if (strEqual(command, "print helpanim.conf"))
  {
    Print("# You can configure different element help animations here.\n");
    Print("# (The entries below are default and therefore commented out.)\n");
    Print("\n");

    for (i = 0; helpanim_config[i].token != NULL; i++)
    {
      Print("# %s\n", getFormattedSetupEntry(helpanim_config[i].token,
					     helpanim_config[i].value));

      if (strEqual(helpanim_config[i].token, "end"))
	Print("#\n");
    }

    exit(0);
  }
  else if (strEqual(command, "print helptext.conf"))
  {
    Print("# You can configure different element help text here.\n");
    Print("# (The entries below are default and therefore commented out.)\n");
    Print("\n");

    for (i = 0; helptext_config[i].token != NULL; i++)
      Print("# %s\n", getFormattedSetupEntry(helptext_config[i].token,
					     helptext_config[i].value));

    exit(0);
  }
  else if (strPrefix(command, "dump level "))
  {
    char *filename = &command[11];

    if (!fileExists(filename))
      Error(ERR_EXIT, "cannot open file '%s'", filename);

    LoadLevelFromFilename(&level, filename);
    DumpLevel(&level);

    exit(0);
  }
  else if (strPrefix(command, "dump tape "))
  {
    char *filename = &command[10];

    if (!fileExists(filename))
      Error(ERR_EXIT, "cannot open file '%s'", filename);

    LoadTapeFromFilename(filename);
    DumpTape(&tape);

    exit(0);
  }
  else if (strPrefix(command, "autotest ") ||
	   strPrefix(command, "autoplay ") ||
	   strPrefix(command, "autoffwd ") ||
	   strPrefix(command, "autowarp "))
  {
    char *str_ptr = getStringCopy(&command[9]);	/* read command parameters */

    global.autoplay_mode =
      (strPrefix(command, "autotest") ? AUTOPLAY_MODE_TEST :
       strPrefix(command, "autoplay") ? AUTOPLAY_MODE_PLAY :
       strPrefix(command, "autoffwd") ? AUTOPLAY_MODE_FFWD :
       strPrefix(command, "autowarp") ? AUTOPLAY_MODE_WARP :
       AUTOPLAY_MODE_NONE);

    while (*str_ptr != '\0')			/* continue parsing string */
    {
      /* cut leading whitespace from string, replace it by string terminator */
      while (*str_ptr == ' ' || *str_ptr == '\t')
	*str_ptr++ = '\0';

      if (*str_ptr == '\0')			/* end of string reached */
	break;

      if (global.autoplay_leveldir == NULL)	/* read level set string */
      {
	global.autoplay_leveldir = str_ptr;
	global.autoplay_all = TRUE;		/* default: play all tapes */

	for (i = 0; i < MAX_TAPES_PER_SET; i++)
	  global.autoplay_level[i] = FALSE;
      }
      else					/* read level number string */
      {
	int level_nr = atoi(str_ptr);		/* get level_nr value */

	if (level_nr >= 0 && level_nr < MAX_TAPES_PER_SET)
	  global.autoplay_level[level_nr] = TRUE;

	global.autoplay_all = FALSE;
      }

      /* advance string pointer to the next whitespace (or end of string) */
      while (*str_ptr != ' ' && *str_ptr != '\t' && *str_ptr != '\0')
	str_ptr++;
    }

    if (global.autoplay_mode == AUTOPLAY_MODE_TEST)
      program.headless = TRUE;
  }
  else if (strPrefix(command, "convert "))
  {
    char *str_copy = getStringCopy(strchr(command, ' ') + 1);
    char *str_ptr = strchr(str_copy, ' ');

    global.convert_leveldir = str_copy;
    global.convert_level_nr = -1;

    if (str_ptr != NULL)			/* level number follows */
    {
      *str_ptr++ = '\0';			/* terminate leveldir string */
      global.convert_level_nr = atoi(str_ptr);	/* get level_nr value */
    }

    program.headless = TRUE;
  }
  else if (strPrefix(command, "create images "))
  {
    global.create_images_dir = getStringCopy(&command[14]);

    if (access(global.create_images_dir, W_OK) != 0)
      Error(ERR_EXIT, "image target directory '%s' not found or not writable",
	    global.create_images_dir);
  }
  else if (strPrefix(command, "create CE image "))
  {
    CreateCustomElementImages(&command[16]);

    exit(0);
  }
  else
  {
    Error(ERR_EXIT_HELP, "unrecognized command '%s'", command);
  }
}

static void InitSetup()
{
  LoadSetup();					/* global setup info */
  LoadSetup_AutoSetup();			/* global auto setup info */

  /* set some options from setup file */

  if (setup.options.verbose)
    options.verbose = TRUE;

  if (setup.debug.show_frames_per_second)
    global.show_frames_per_second = TRUE;
}

static void InitGameInfo()
{
  game.restart_level = FALSE;
  game.restart_game_message = NULL;
}

static void InitPlayerInfo()
{
  int i;

  /* choose default local player */
  local_player = &stored_player[0];

  for (i = 0; i < MAX_PLAYERS; i++)
    stored_player[i].connected = FALSE;

  local_player->connected = TRUE;
}

static void InitArtworkInfo()
{
  LoadArtworkInfo();
}

static char *get_string_in_brackets(char *string)
{
  char *string_in_brackets = checked_malloc(strlen(string) + 3);

  sprintf(string_in_brackets, "[%s]", string);

  return string_in_brackets;
}

static char *get_level_id_suffix(int id_nr)
{
  char *id_suffix = checked_malloc(1 + 3 + 1);

  if (id_nr < 0 || id_nr > 999)
    id_nr = 0;

  sprintf(id_suffix, ".%03d", id_nr);

  return id_suffix;
}

static void InitArtworkConfig()
{
  static char *image_id_prefix[MAX_NUM_ELEMENTS +
			       NUM_FONTS +
			       NUM_GLOBAL_ANIM_TOKENS + 1];
  static char *sound_id_prefix[2 * MAX_NUM_ELEMENTS +
			       NUM_GLOBAL_ANIM_TOKENS + 1];
  static char *music_id_prefix[NUM_MUSIC_PREFIXES +
			       NUM_GLOBAL_ANIM_TOKENS + 1];
  static char *action_id_suffix[NUM_ACTIONS + 1];
  static char *direction_id_suffix[NUM_DIRECTIONS_FULL + 1];
  static char *special_id_suffix[NUM_SPECIAL_GFX_ARGS + 1];
  static char *level_id_suffix[MAX_LEVELS + 1];
  static char *dummy[1] = { NULL };
  static char *ignore_generic_tokens[] =
  {
    "name",
    "sort_priority",
    "program_title",
    "program_copyright",
    "program_company",

    NULL
  };
  static char **ignore_image_tokens;
  static char **ignore_sound_tokens;
  static char **ignore_music_tokens;
  int num_ignore_generic_tokens;
  int num_ignore_image_tokens;
  int num_ignore_sound_tokens;
  int num_ignore_music_tokens;
  int i;

  /* dynamically determine list of generic tokens to be ignored */
  num_ignore_generic_tokens = 0;
  for (i = 0; ignore_generic_tokens[i] != NULL; i++)
    num_ignore_generic_tokens++;

  /* dynamically determine list of image tokens to be ignored */
  num_ignore_image_tokens = num_ignore_generic_tokens;
  for (i = 0; image_config_vars[i].token != NULL; i++)
    num_ignore_image_tokens++;
  ignore_image_tokens =
    checked_malloc((num_ignore_image_tokens + 1) * sizeof(char *));
  for (i = 0; i < num_ignore_generic_tokens; i++)
    ignore_image_tokens[i] = ignore_generic_tokens[i];
  for (i = 0; i < num_ignore_image_tokens - num_ignore_generic_tokens; i++)
    ignore_image_tokens[num_ignore_generic_tokens + i] =
      image_config_vars[i].token;
  ignore_image_tokens[num_ignore_image_tokens] = NULL;

  /* dynamically determine list of sound tokens to be ignored */
  num_ignore_sound_tokens = num_ignore_generic_tokens;
  ignore_sound_tokens =
    checked_malloc((num_ignore_sound_tokens + 1) * sizeof(char *));
  for (i = 0; i < num_ignore_generic_tokens; i++)
    ignore_sound_tokens[i] = ignore_generic_tokens[i];
  ignore_sound_tokens[num_ignore_sound_tokens] = NULL;

  /* dynamically determine list of music tokens to be ignored */
  num_ignore_music_tokens = num_ignore_generic_tokens;
  ignore_music_tokens =
    checked_malloc((num_ignore_music_tokens + 1) * sizeof(char *));
  for (i = 0; i < num_ignore_generic_tokens; i++)
    ignore_music_tokens[i] = ignore_generic_tokens[i];
  ignore_music_tokens[num_ignore_music_tokens] = NULL;

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    image_id_prefix[i] = element_info[i].token_name;
  for (i = 0; i < NUM_FONTS; i++)
    image_id_prefix[MAX_NUM_ELEMENTS + i] = font_info[i].token_name;
  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS; i++)
    image_id_prefix[MAX_NUM_ELEMENTS + NUM_FONTS + i] =
      global_anim_info[i].token_name;
  image_id_prefix[MAX_NUM_ELEMENTS + NUM_FONTS + NUM_GLOBAL_ANIM_TOKENS] = NULL;

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    sound_id_prefix[i] = element_info[i].token_name;
  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    sound_id_prefix[MAX_NUM_ELEMENTS + i] =
      get_string_in_brackets(element_info[i].class_name);
  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS; i++)
    sound_id_prefix[2 * MAX_NUM_ELEMENTS + i] =
      global_anim_info[i].token_name;
  sound_id_prefix[2 * MAX_NUM_ELEMENTS + NUM_GLOBAL_ANIM_TOKENS] = NULL;

  for (i = 0; i < NUM_MUSIC_PREFIXES; i++)
    music_id_prefix[i] = music_prefix_info[i].prefix;
  for (i = 0; i < NUM_GLOBAL_ANIM_TOKENS; i++)
    music_id_prefix[NUM_MUSIC_PREFIXES + i] =
      global_anim_info[i].token_name;
  music_id_prefix[NUM_MUSIC_PREFIXES + NUM_GLOBAL_ANIM_TOKENS] = NULL;

  for (i = 0; i < NUM_ACTIONS; i++)
    action_id_suffix[i] = element_action_info[i].suffix;
  action_id_suffix[NUM_ACTIONS] = NULL;

  for (i = 0; i < NUM_DIRECTIONS_FULL; i++)
    direction_id_suffix[i] = element_direction_info[i].suffix;
  direction_id_suffix[NUM_DIRECTIONS_FULL] = NULL;

  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
    special_id_suffix[i] = special_suffix_info[i].suffix;
  special_id_suffix[NUM_SPECIAL_GFX_ARGS] = NULL;

  for (i = 0; i < MAX_LEVELS; i++)
    level_id_suffix[i] = get_level_id_suffix(i);
  level_id_suffix[MAX_LEVELS] = NULL;

  InitImageList(image_config, NUM_IMAGE_FILES, image_config_suffix,
		image_id_prefix, action_id_suffix, direction_id_suffix,
		special_id_suffix, ignore_image_tokens);
  InitSoundList(sound_config, NUM_SOUND_FILES, sound_config_suffix,
		sound_id_prefix, action_id_suffix, dummy,
		special_id_suffix, ignore_sound_tokens);
  InitMusicList(music_config, NUM_MUSIC_FILES, music_config_suffix,
		music_id_prefix, action_id_suffix, special_id_suffix,
		level_id_suffix, ignore_music_tokens);
}

static void InitMixer()
{
  OpenAudio();

  StartMixer();
}

void InitGfxBuffers()
{
  static int win_xsize_last = -1;
  static int win_ysize_last = -1;

  /* create additional image buffers for double-buffering and cross-fading */

  if (WIN_XSIZE != win_xsize_last || WIN_YSIZE != win_ysize_last)
  {
    /* used to temporarily store the backbuffer -- only re-create if changed */
    ReCreateBitmap(&bitmap_db_store_1, WIN_XSIZE, WIN_YSIZE);
    ReCreateBitmap(&bitmap_db_store_2, WIN_XSIZE, WIN_YSIZE);

    win_xsize_last = WIN_XSIZE;
    win_ysize_last = WIN_YSIZE;
  }

  ReCreateBitmap(&bitmap_db_field, FXSIZE, FYSIZE);
  ReCreateBitmap(&bitmap_db_panel, DXSIZE, DYSIZE);
  ReCreateBitmap(&bitmap_db_door_1, 3 * DXSIZE, DYSIZE);
  ReCreateBitmap(&bitmap_db_door_2, 3 * VXSIZE, VYSIZE);

  /* initialize screen properties */
  InitGfxFieldInfo(SX, SY, SXSIZE, SYSIZE,
		   REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE,
		   bitmap_db_field);
  InitGfxDoor1Info(DX, DY, DXSIZE, DYSIZE);
  InitGfxDoor2Info(VX, VY, VXSIZE, VYSIZE);
  InitGfxDoor3Info(EX, EY, EXSIZE, EYSIZE);
  InitGfxWindowInfo(WIN_XSIZE, WIN_YSIZE);
  InitGfxScrollbufferInfo(FXSIZE, FYSIZE);
  InitGfxClipRegion(FALSE, -1, -1, -1, -1);

  /* required if door size definitions have changed */
  InitGraphicCompatibilityInfo_Doors();

  InitGfxBuffers_EM();
  InitGfxBuffers_SP();
}

void InitGfx()
{
  struct GraphicInfo *graphic_info_last = graphic_info;
  char *filename_font_initial = NULL;
  char *filename_anim_initial = NULL;
  Bitmap *bitmap_font_initial = NULL;
  int font_height;
  int i, j;

  /* determine settings for initial font (for displaying startup messages) */
  for (i = 0; image_config[i].token != NULL; i++)
  {
    for (j = 0; j < NUM_INITIAL_FONTS; j++)
    {
      char font_token[128];
      int len_font_token;

      sprintf(font_token, "%s_%d", CONFIG_TOKEN_FONT_INITIAL, j + 1);
      len_font_token = strlen(font_token);

      if (strEqual(image_config[i].token, font_token))
	filename_font_initial = image_config[i].value;
      else if (strlen(image_config[i].token) > len_font_token &&
	       strncmp(image_config[i].token, font_token, len_font_token) == 0)
      {
	if (strEqual(&image_config[i].token[len_font_token], ".x"))
	  font_initial[j].src_x = atoi(image_config[i].value);
	else if (strEqual(&image_config[i].token[len_font_token], ".y"))
	  font_initial[j].src_y = atoi(image_config[i].value);
	else if (strEqual(&image_config[i].token[len_font_token], ".width"))
	  font_initial[j].width = atoi(image_config[i].value);
	else if (strEqual(&image_config[i].token[len_font_token], ".height"))
	  font_initial[j].height = atoi(image_config[i].value);
      }
    }
  }

  for (j = 0; j < NUM_INITIAL_FONTS; j++)
  {
    font_initial[j].num_chars = DEFAULT_NUM_CHARS_PER_FONT;
    font_initial[j].num_chars_per_line = DEFAULT_NUM_CHARS_PER_LINE;
  }

  if (filename_font_initial == NULL)	/* should not happen */
    Error(ERR_EXIT, "cannot get filename for '%s'", CONFIG_TOKEN_FONT_INITIAL);

  InitGfxBuffers();
  InitGfxCustomArtworkInfo();
  InitGfxOtherSettings();

  bitmap_font_initial = LoadCustomImage(filename_font_initial);

  for (j = 0; j < NUM_INITIAL_FONTS; j++)
    font_initial[j].bitmap = bitmap_font_initial;

  InitFontGraphicInfo();

  font_height = getFontHeight(FC_RED);

  DrawInitText(getProgramInitString(), 20, FC_YELLOW);
  DrawInitText(setup.internal.program_copyright, 50, FC_RED);
  DrawInitText(setup.internal.program_website, WIN_YSIZE - 20 - font_height,
	       FC_RED);

  DrawInitText("Loading graphics", 120, FC_GREEN);

  /* initialize settings for busy animation with default values */
  int parameter[NUM_GFX_ARGS];
  for (i = 0; i < NUM_GFX_ARGS; i++)
    parameter[i] = get_graphic_parameter_value(image_config_suffix[i].value,
                                               image_config_suffix[i].token,
                                               image_config_suffix[i].type);

  char *anim_token = CONFIG_TOKEN_GLOBAL_BUSY;
  int len_anim_token = strlen(anim_token);

  /* read settings for busy animation from default custom artwork config */
  char *gfx_config_filename = getPath3(options.graphics_directory,
				       GFX_DEFAULT_SUBDIR,
				       GRAPHICSINFO_FILENAME);

  if (fileExists(gfx_config_filename))
  {
    SetupFileHash *setup_file_hash = loadSetupFileHash(gfx_config_filename);

    if (setup_file_hash)
    {
      char *filename = getHashEntry(setup_file_hash, anim_token);

      if (filename)
      {
	filename_anim_initial = getStringCopy(filename);

	for (j = 0; image_config_suffix[j].token != NULL; j++)
	{
	  int type = image_config_suffix[j].type;
	  char *suffix = image_config_suffix[j].token;
	  char *token = getStringCat2(anim_token, suffix);
	  char *value = getHashEntry(setup_file_hash, token);

	  checked_free(token);

	  if (value)
	    parameter[j] = get_graphic_parameter_value(value, suffix, type);
	}
      }

      freeSetupFileHash(setup_file_hash);
    }
  }

  if (filename_anim_initial == NULL)
  {
    /* read settings for busy animation from static default artwork config */
    for (i = 0; image_config[i].token != NULL; i++)
    {
      if (strEqual(image_config[i].token, anim_token))
	filename_anim_initial = getStringCopy(image_config[i].value);
      else if (strlen(image_config[i].token) > len_anim_token &&
	       strncmp(image_config[i].token, anim_token, len_anim_token) == 0)
      {
	for (j = 0; image_config_suffix[j].token != NULL; j++)
	{
	  if (strEqual(&image_config[i].token[len_anim_token],
		       image_config_suffix[j].token))
	    parameter[j] =
	      get_graphic_parameter_value(image_config[i].value,
					  image_config_suffix[j].token,
					  image_config_suffix[j].type);
	}
      }
    }
  }

  if (filename_anim_initial == NULL)	/* should not happen */
    Error(ERR_EXIT, "cannot get filename for '%s'", CONFIG_TOKEN_GLOBAL_BUSY);

  anim_initial.bitmaps =
    checked_calloc(sizeof(Bitmap *) * NUM_IMG_BITMAP_POINTERS);

  anim_initial.bitmaps[IMG_BITMAP_STANDARD] =
    LoadCustomImage(filename_anim_initial);

  checked_free(filename_anim_initial);

  graphic_info = &anim_initial;		/* graphic == 0 => anim_initial */

  set_graphic_parameters_ext(0, parameter, anim_initial.bitmaps);

  graphic_info = graphic_info_last;

  init.busy.width  = anim_initial.width;
  init.busy.height = anim_initial.height;

  InitMenuDesignSettings_Static();

  InitGfxDrawBusyAnimFunction(DrawInitAnim);
  InitGfxDrawGlobalAnimFunction(DrawGlobalAnimations);
  InitGfxDrawGlobalBorderFunction(DrawMaskedBorderToTarget);
  InitGfxDrawTileCursorFunction(DrawTileCursor);

  gfx.fade_border_source_status = global.border_status;
  gfx.fade_border_target_status = global.border_status;
  gfx.masked_border_bitmap_ptr = backbuffer;

  /* use copy of busy animation to prevent change while reloading artwork */
  init_last = init;
}

void InitGfxBackground()
{
  fieldbuffer = bitmap_db_field;
  SetDrawtoField(DRAW_TO_BACKBUFFER);

  ClearRectangle(backbuffer, 0, 0, WIN_XSIZE, WIN_YSIZE);

  redraw_mask = REDRAW_ALL;
}

static void InitLevelInfo()
{
  LoadLevelInfo();				/* global level info */
  LoadLevelSetup_LastSeries();			/* last played series info */
  LoadLevelSetup_SeriesInfo();			/* last played level info */

  if (global.autoplay_leveldir &&
      global.autoplay_mode != AUTOPLAY_MODE_TEST)
  {
    leveldir_current = getTreeInfoFromIdentifier(leveldir_first,
                                                 global.autoplay_leveldir);
    if (leveldir_current == NULL)
      leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);
  }
}

static void InitLevelArtworkInfo()
{
  LoadLevelArtworkInfo();
}

static void InitImages()
{
  print_timestamp_init("InitImages");

#if 0
  printf("::: leveldir_current->identifier == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->identifier);
  printf("::: leveldir_current->graphics_path == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->graphics_path);
  printf("::: leveldir_current->graphics_set == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->graphics_set);
  printf("::: getLevelArtworkSet(ARTWORK_TYPE_GRAPHICS) == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : LEVELDIR_ARTWORK_SET(leveldir_current, ARTWORK_TYPE_GRAPHICS));
#endif

  setLevelArtworkDir(artwork.gfx_first);

#if 0
  printf("::: leveldir_current->identifier == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->identifier);
  printf("::: leveldir_current->graphics_path == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->graphics_path);
  printf("::: leveldir_current->graphics_set == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->graphics_set);
  printf("::: getLevelArtworkSet(ARTWORK_TYPE_GRAPHICS) == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : LEVELDIR_ARTWORK_SET(leveldir_current, ARTWORK_TYPE_GRAPHICS));
#endif

#if 0
  printf("::: InitImages for '%s' ['%s', '%s'] ['%s', '%s']\n",
	 leveldir_current->identifier,
	 artwork.gfx_current_identifier,
	 artwork.gfx_current->identifier,
	 leveldir_current->graphics_set,
	 leveldir_current->graphics_path);
#endif

  UPDATE_BUSY_STATE();

  ReloadCustomImages();
  print_timestamp_time("ReloadCustomImages");

  UPDATE_BUSY_STATE();

  LoadCustomElementDescriptions();
  print_timestamp_time("LoadCustomElementDescriptions");

  UPDATE_BUSY_STATE();

  LoadMenuDesignSettings();
  print_timestamp_time("LoadMenuDesignSettings");

  UPDATE_BUSY_STATE();

  ReinitializeGraphics();
  print_timestamp_time("ReinitializeGraphics");

  LoadMenuDesignSettings_AfterGraphics();
  print_timestamp_time("LoadMenuDesignSettings_AfterGraphics");

  UPDATE_BUSY_STATE();

  print_timestamp_done("InitImages");
}

static void InitSound(char *identifier)
{
  print_timestamp_init("InitSound");

  if (identifier == NULL)
    identifier = artwork.snd_current->identifier;

  /* set artwork path to send it to the sound server process */
  setLevelArtworkDir(artwork.snd_first);

  InitReloadCustomSounds(identifier);
  print_timestamp_time("InitReloadCustomSounds");

  ReinitializeSounds();
  print_timestamp_time("ReinitializeSounds");

  print_timestamp_done("InitSound");
}

static void InitMusic(char *identifier)
{
  print_timestamp_init("InitMusic");

  if (identifier == NULL)
    identifier = artwork.mus_current->identifier;

  /* set artwork path to send it to the sound server process */
  setLevelArtworkDir(artwork.mus_first);

  InitReloadCustomMusic(identifier);
  print_timestamp_time("InitReloadCustomMusic");

  ReinitializeMusic();
  print_timestamp_time("ReinitializeMusic");

  print_timestamp_done("InitMusic");
}

static void InitArtworkDone()
{
  if (program.headless)
    return;

  InitGlobalAnimations();
}

void InitNetworkServer()
{
#if defined(NETWORK_AVALIABLE)
  int nr_wanted;
#endif

  if (!options.network)
    return;

#if defined(NETWORK_AVALIABLE)
  nr_wanted = Request("Choose player", REQ_PLAYER | REQ_STAY_CLOSED);

  if (!ConnectToServer(options.server_host, options.server_port))
    Error(ERR_EXIT, "cannot connect to network game server");

  SendToServer_PlayerName(setup.player_name);
  SendToServer_ProtocolVersion();

  if (nr_wanted)
    SendToServer_NrWanted(nr_wanted);
#endif
}

static boolean CheckArtworkConfigForCustomElements(char *filename)
{
  SetupFileHash *setup_file_hash;
  boolean redefined_ce_found = FALSE;

  /* !!! CACHE THIS BY USING HASH 'filename' => 'true/false' !!! */

  if ((setup_file_hash = loadSetupFileHash(filename)) != NULL)
  {
    BEGIN_HASH_ITERATION(setup_file_hash, itr)
    {
      char *token = HASH_ITERATION_TOKEN(itr);

      if (strPrefix(token, "custom_"))
      {
	redefined_ce_found = TRUE;

	break;
      }
    }
    END_HASH_ITERATION(setup_file_hash, itr)

    freeSetupFileHash(setup_file_hash);
  }

  return redefined_ce_found;
}

static boolean CheckArtworkTypeForRedefinedCustomElements(int type)
{
  char *filename_base, *filename_local;
  boolean redefined_ce_found = FALSE;

  setLevelArtworkDir(ARTWORK_FIRST_NODE(artwork, type));

#if 0
  printf("::: leveldir_current->identifier == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->identifier);
  printf("::: leveldir_current->graphics_path == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->graphics_path);
  printf("::: leveldir_current->graphics_set == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" : leveldir_current->graphics_set);
  printf("::: getLevelArtworkSet(ARTWORK_TYPE_GRAPHICS) == '%s'\n",
	 leveldir_current == NULL ? "[NULL]" :
	 LEVELDIR_ARTWORK_SET(leveldir_current, type));
#endif

  /* first look for special artwork configured in level series config */
  filename_base = getCustomArtworkLevelConfigFilename(type);

#if 0
  printf("::: filename_base == '%s'\n", filename_base);
#endif

  if (fileExists(filename_base))
    redefined_ce_found |= CheckArtworkConfigForCustomElements(filename_base);

  filename_local = getCustomArtworkConfigFilename(type);

#if 0
  printf("::: filename_local == '%s'\n", filename_local);
#endif

  if (filename_local != NULL && !strEqual(filename_base, filename_local))
    redefined_ce_found |= CheckArtworkConfigForCustomElements(filename_local);

#if 0
  printf("::: redefined_ce_found == %d\n", redefined_ce_found);
#endif

  return redefined_ce_found;
}

static void InitOverrideArtwork()
{
  boolean redefined_ce_found = FALSE;

  /* to check if this level set redefines any CEs, do not use overriding */
  gfx.override_level_graphics = FALSE;
  gfx.override_level_sounds   = FALSE;
  gfx.override_level_music    = FALSE;

  /* now check if this level set has definitions for custom elements */
  if (setup.override_level_graphics == AUTO ||
      setup.override_level_sounds   == AUTO ||
      setup.override_level_music    == AUTO)
    redefined_ce_found =
      (CheckArtworkTypeForRedefinedCustomElements(ARTWORK_TYPE_GRAPHICS) |
       CheckArtworkTypeForRedefinedCustomElements(ARTWORK_TYPE_SOUNDS) |
       CheckArtworkTypeForRedefinedCustomElements(ARTWORK_TYPE_MUSIC));

#if 0
  printf("::: redefined_ce_found == %d\n", redefined_ce_found);
#endif

  if (redefined_ce_found)
  {
    /* this level set has CE definitions: change "AUTO" to "FALSE" */
    gfx.override_level_graphics = (setup.override_level_graphics == TRUE);
    gfx.override_level_sounds   = (setup.override_level_sounds   == TRUE);
    gfx.override_level_music    = (setup.override_level_music    == TRUE);
  }
  else
  {
    /* this level set has no CE definitions: change "AUTO" to "TRUE" */
    gfx.override_level_graphics = (setup.override_level_graphics != FALSE);
    gfx.override_level_sounds   = (setup.override_level_sounds   != FALSE);
    gfx.override_level_music    = (setup.override_level_music    != FALSE);
  }

#if 0
  printf("::: => %d, %d, %d\n",
	 gfx.override_level_graphics,
	 gfx.override_level_sounds,
	 gfx.override_level_music);
#endif
}

static char *getNewArtworkIdentifier(int type)
{
  static char *leveldir_current_identifier[3] = { NULL, NULL, NULL };
  static boolean last_override_level_artwork[3] = { FALSE, FALSE, FALSE };
  static boolean last_has_level_artwork_set[3] = { FALSE, FALSE, FALSE };
  static boolean initialized[3] = { FALSE, FALSE, FALSE };
  TreeInfo *artwork_first_node = ARTWORK_FIRST_NODE(artwork, type);
  boolean setup_override_artwork = GFX_OVERRIDE_ARTWORK(type);
  char *setup_artwork_set = SETUP_ARTWORK_SET(setup, type);
  char *leveldir_identifier = leveldir_current->identifier;
  /* !!! setLevelArtworkDir() should be moved to an earlier stage !!! */
  char *leveldir_artwork_set = setLevelArtworkDir(artwork_first_node);
  boolean has_level_artwork_set = (leveldir_artwork_set != NULL);
  char *artwork_current_identifier;
  char *artwork_new_identifier = NULL;	/* default: nothing has changed */

  /* leveldir_current may be invalid (level group, parent link) */
  if (!validLevelSeries(leveldir_current))
    return NULL;

  /* 1st step: determine artwork set to be activated in descending order:
     --------------------------------------------------------------------
     1. setup artwork (when configured to override everything else)
     2. artwork set configured in "levelinfo.conf" of current level set
        (artwork in level directory will have priority when loading later)
     3. artwork in level directory (stored in artwork sub-directory)
     4. setup artwork (currently configured in setup menu) */

  if (setup_override_artwork)
    artwork_current_identifier = setup_artwork_set;
  else if (leveldir_artwork_set != NULL)
    artwork_current_identifier = leveldir_artwork_set;
  else if (getTreeInfoFromIdentifier(artwork_first_node, leveldir_identifier))
    artwork_current_identifier = leveldir_identifier;
  else
    artwork_current_identifier = setup_artwork_set;


  /* 2nd step: check if it is really needed to reload artwork set
     ------------------------------------------------------------ */

  /* ---------- reload if level set and also artwork set has changed ------- */
  if (leveldir_current_identifier[type] != leveldir_identifier &&
      (last_has_level_artwork_set[type] || has_level_artwork_set))
    artwork_new_identifier = artwork_current_identifier;

  leveldir_current_identifier[type] = leveldir_identifier;
  last_has_level_artwork_set[type] = has_level_artwork_set;

  /* ---------- reload if "override artwork" setting has changed ----------- */
  if (last_override_level_artwork[type] != setup_override_artwork)
    artwork_new_identifier = artwork_current_identifier;

  last_override_level_artwork[type] = setup_override_artwork;

  /* ---------- reload if current artwork identifier has changed ----------- */
  if (!strEqual(ARTWORK_CURRENT_IDENTIFIER(artwork, type),
		artwork_current_identifier))
    artwork_new_identifier = artwork_current_identifier;

  *(ARTWORK_CURRENT_IDENTIFIER_PTR(artwork, type))= artwork_current_identifier;

  /* ---------- do not reload directly after starting ---------------------- */
  if (!initialized[type])
    artwork_new_identifier = NULL;

  initialized[type] = TRUE;

  return artwork_new_identifier;
}

void ReloadCustomArtwork(int force_reload)
{
  int last_game_status = game_status;	/* save current game status */
  char *gfx_new_identifier;
  char *snd_new_identifier;
  char *mus_new_identifier;
  boolean force_reload_gfx = (force_reload & (1 << ARTWORK_TYPE_GRAPHICS));
  boolean force_reload_snd = (force_reload & (1 << ARTWORK_TYPE_SOUNDS));
  boolean force_reload_mus = (force_reload & (1 << ARTWORK_TYPE_MUSIC));
  boolean reload_needed;

  InitOverrideArtwork();

  force_reload_gfx |= AdjustGraphicsForEMC();

  gfx_new_identifier = getNewArtworkIdentifier(ARTWORK_TYPE_GRAPHICS);
  snd_new_identifier = getNewArtworkIdentifier(ARTWORK_TYPE_SOUNDS);
  mus_new_identifier = getNewArtworkIdentifier(ARTWORK_TYPE_MUSIC);

  reload_needed = (gfx_new_identifier != NULL || force_reload_gfx ||
		   snd_new_identifier != NULL || force_reload_snd ||
		   mus_new_identifier != NULL || force_reload_mus);

  if (!reload_needed)
    return;

  print_timestamp_init("ReloadCustomArtwork");

  SetGameStatus(GAME_MODE_LOADING);

  FadeOut(REDRAW_ALL);

  ClearRectangle(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);
  print_timestamp_time("ClearRectangle");

  FadeIn(REDRAW_ALL);

  if (gfx_new_identifier != NULL || force_reload_gfx)
  {
#if 0
    printf("RELOADING GRAPHICS '%s' -> '%s' ['%s', '%s']\n",
	   artwork.gfx_current_identifier,
	   gfx_new_identifier,
	   artwork.gfx_current->identifier,
	   leveldir_current->graphics_set);
#endif

    InitImages();
    print_timestamp_time("InitImages");
  }

  if (snd_new_identifier != NULL || force_reload_snd)
  {
    InitSound(snd_new_identifier);
    print_timestamp_time("InitSound");
  }

  if (mus_new_identifier != NULL || force_reload_mus)
  {
    InitMusic(mus_new_identifier);
    print_timestamp_time("InitMusic");
  }

  InitArtworkDone();

  SetGameStatus(last_game_status);	/* restore current game status */

  init_last = init;			/* switch to new busy animation */

  FadeOut(REDRAW_ALL);

  RedrawGlobalBorder();

  /* force redraw of (open or closed) door graphics */
  SetDoorState(DOOR_OPEN_ALL);
  CloseDoor(DOOR_CLOSE_ALL | DOOR_NO_DELAY);

  FadeSetEnterScreen();
  FadeSkipNextFadeOut();

  print_timestamp_done("ReloadCustomArtwork");

  LimitScreenUpdates(FALSE);
}

void KeyboardAutoRepeatOffUnlessAutoplay()
{
  if (global.autoplay_leveldir == NULL)
    KeyboardAutoRepeatOff();
}

void DisplayExitMessage(char *format, va_list ap)
{
  // also check for initialized video (headless flag may be temporarily unset)
  if (program.headless || !video.initialized)
    return;

  // check if draw buffer and fonts for exit message are already available
  if (drawto == NULL || font_initial[NUM_INITIAL_FONTS - 1].bitmap == NULL)
    return;

  int font_1 = FC_RED;
  int font_2 = FC_YELLOW;
  int font_3 = FC_BLUE;
  int font_width = getFontWidth(font_2);
  int font_height = getFontHeight(font_2);
  int sx = SX;
  int sy = SY;
  int sxsize = WIN_XSIZE - 2 * sx;
  int sysize = WIN_YSIZE - 2 * sy;
  int line_length = sxsize / font_width;
  int max_lines = sysize / font_height;
  int num_lines_printed;

  gfx.sx = sx;
  gfx.sy = sy;
  gfx.sxsize = sxsize;
  gfx.sysize = sysize;

  sy = 20;

  ClearRectangle(drawto, 0, 0, WIN_XSIZE, WIN_YSIZE);

  DrawTextSCentered(sy, font_1, "Fatal error:");
  sy += 3 * font_height;;

  num_lines_printed =
    DrawTextBufferVA(sx, sy, format, ap, font_2,
		     line_length, line_length, max_lines,
		     0, BLIT_ON_BACKGROUND, TRUE, TRUE, FALSE);
  sy += (num_lines_printed + 3) * font_height;

  DrawTextSCentered(sy, font_1, "For details, see the following error file:");
  sy += 3 * font_height;

  num_lines_printed =
    DrawTextBuffer(sx, sy, program.log_filename[LOG_ERR_ID], font_2,
		   line_length, line_length, max_lines,
		   0, BLIT_ON_BACKGROUND, TRUE, TRUE, FALSE);

  DrawTextSCentered(SYSIZE - 20, font_3, "Press any key or button to exit");

  redraw_mask = REDRAW_ALL;

  /* force drawing exit message even if screen updates are currently limited */
  LimitScreenUpdates(FALSE);

  BackToFront();

  /* deactivate toons on error message screen */
  setup.toons = FALSE;

  WaitForEventToContinue();
}


/* ========================================================================= */
/* OpenAll()                                                                 */
/* ========================================================================= */

void OpenAll()
{
  print_timestamp_init("OpenAll");

  SetGameStatus(GAME_MODE_LOADING);

  InitCounter();

  InitGlobal();			/* initialize some global variables */

  print_timestamp_time("[init global stuff]");

  InitSetup();

  print_timestamp_time("[init setup/config stuff (1)]");

  InitScoresInfo();

  if (options.execute_command)
    Execute_Command(options.execute_command);

  if (options.serveronly)
  {
#if defined(PLATFORM_UNIX)
    NetworkServer(options.server_port, options.serveronly);
#else
    Error(ERR_WARN, "networking only supported in Unix version");
#endif

    exit(0);			/* never reached, server loops forever */
  }

  InitGameInfo();
  print_timestamp_time("[init setup/config stuff (2)]");
  InitPlayerInfo();
  print_timestamp_time("[init setup/config stuff (3)]");
  InitArtworkInfo();		/* needed before loading gfx, sound & music */
  print_timestamp_time("[init setup/config stuff (4)]");
  InitArtworkConfig();		/* needed before forking sound child process */
  print_timestamp_time("[init setup/config stuff (5)]");
  InitMixer();
  print_timestamp_time("[init setup/config stuff (6)]");

  InitRND(NEW_RANDOMIZE);
  InitSimpleRandom(NEW_RANDOMIZE);

  InitJoysticks();

  print_timestamp_time("[init setup/config stuff]");

  InitVideoDefaults();
  InitVideoDisplay();
  InitVideoBuffer(WIN_XSIZE, WIN_YSIZE, DEFAULT_DEPTH, setup.fullscreen);

  InitTileCursorInfo();
  InitOverlayInfo();

  print_timestamp_time("[init video stuff]");

  InitElementPropertiesStatic();
  InitElementPropertiesEngine(GAME_VERSION_ACTUAL);
  InitElementPropertiesGfxElement();

  print_timestamp_time("[init element properties stuff]");

  InitGfx();

  print_timestamp_time("InitGfx");

  InitLevelInfo();
  print_timestamp_time("InitLevelInfo");

  InitLevelArtworkInfo();
  print_timestamp_time("InitLevelArtworkInfo");

  InitOverrideArtwork();	/* needs to know current level directory */
  print_timestamp_time("InitOverrideArtwork");

  InitImages();			/* needs to know current level directory */
  print_timestamp_time("InitImages");

  InitSound(NULL);		/* needs to know current level directory */
  print_timestamp_time("InitSound");

  InitMusic(NULL);		/* needs to know current level directory */
  print_timestamp_time("InitMusic");

  InitArtworkDone();

  InitGfxBackground();

  em_open_all();
  sp_open_all();
  mm_open_all();

  if (global.autoplay_leveldir)
  {
    AutoPlayTape();
    return;
  }
  else if (global.convert_leveldir)
  {
    ConvertLevels();
    return;
  }
  else if (global.create_images_dir)
  {
    CreateLevelSketchImages();
    return;
  }

  SetGameStatus(GAME_MODE_MAIN);

  FadeSetEnterScreen();
  if (!(fading.fade_mode & FADE_TYPE_TRANSFORM))
    FadeSkipNextFadeOut();

  print_timestamp_time("[post-artwork]");

  print_timestamp_done("OpenAll");

  DrawMainMenu();

  InitNetworkServer();

#if 0
  Error(ERR_DEBUG, "::: SDL_GetBasePath() == '%s'",
	SDL_GetBasePath());
  Error(ERR_DEBUG, "::: SDL_GetPrefPath() == '%s'",
	SDL_GetPrefPath("artsoft", "rocksndiamonds"));
#if defined(PLATFORM_ANDROID)
  Error(ERR_DEBUG, "::: SDL_AndroidGetInternalStoragePath() == '%s'",
	SDL_AndroidGetInternalStoragePath());
  Error(ERR_DEBUG, "::: SDL_AndroidGetExternalStoragePath() == '%s'",
	SDL_AndroidGetExternalStoragePath());
  Error(ERR_DEBUG, "::: SDL_AndroidGetExternalStorageState() == '%s'",
	(SDL_AndroidGetExternalStorageState() &
	 SDL_ANDROID_EXTERNAL_STORAGE_WRITE ? "writable" :
	 SDL_AndroidGetExternalStorageState() &
	 SDL_ANDROID_EXTERNAL_STORAGE_READ ? "readable" : "not available"));
#endif
#endif
}

void CloseAllAndExit(int exit_value)
{
  StopSounds();
  FreeAllSounds();
  FreeAllMusic();
  CloseAudio();		/* called after freeing sounds (needed for SDL) */

  em_close_all();
  sp_close_all();

  FreeAllImages();

#if defined(TARGET_SDL)
#if defined(TARGET_SDL2)
  // !!! TODO !!!
  // set a flag to tell the network server thread to quit and wait for it
  // using SDL_WaitThread()
#else
  if (network_server)	/* terminate network server */
    SDL_KillThread(server_thread);
#endif
#endif

  CloseVideoDisplay();
  ClosePlatformDependentStuff();

  if (exit_value != 0 && !options.execute_command)
  {
    /* fall back to default level set (current set may have caused an error) */
    SaveLevelSetup_LastSeries_Deactivate();

    /* tell user where to find error log file which may contain more details */
    // (error notification now directly displayed on screen inside R'n'D
    // NotifyUserAboutErrorFile();	/* currently only works for Windows */
  }

  exit(exit_value);
}
