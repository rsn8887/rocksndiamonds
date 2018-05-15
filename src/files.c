// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// files.c
// ============================================================================

#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>

#include "libgame/libgame.h"

#include "files.h"
#include "init.h"
#include "tools.h"
#include "tape.h"
#include "config.h"

#define ENABLE_UNUSED_CODE	0	/* currently unused functions */
#define ENABLE_HISTORIC_CHUNKS	0	/* only for historic reference */
#define ENABLE_RESERVED_CODE	0	/* reserved for later use */

#define CHUNK_ID_LEN		4	/* IFF style chunk id length  */
#define CHUNK_SIZE_UNDEFINED	0	/* undefined chunk size == 0  */
#define CHUNK_SIZE_NONE		-1	/* do not write chunk size    */

#define LEVEL_CHUNK_NAME_SIZE	MAX_LEVEL_NAME_LEN
#define LEVEL_CHUNK_AUTH_SIZE	MAX_LEVEL_AUTHOR_LEN

#define LEVEL_CHUNK_VERS_SIZE	8	/* size of file version chunk */
#define LEVEL_CHUNK_DATE_SIZE	4	/* size of file date chunk    */
#define LEVEL_CHUNK_HEAD_SIZE	80	/* size of level file header  */
#define LEVEL_CHUNK_HEAD_UNUSED	0	/* unused level header bytes  */
#define LEVEL_CHUNK_CNT2_SIZE	160	/* size of level CNT2 chunk   */
#define LEVEL_CHUNK_CNT2_UNUSED	11	/* unused CNT2 chunk bytes    */
#define LEVEL_CHUNK_CNT3_HEADER	16	/* size of level CNT3 header  */
#define LEVEL_CHUNK_CNT3_UNUSED	10	/* unused CNT3 chunk bytes    */
#define LEVEL_CPART_CUS3_SIZE	134	/* size of CUS3 chunk part    */
#define LEVEL_CPART_CUS3_UNUSED	15	/* unused CUS3 bytes / part   */
#define LEVEL_CHUNK_GRP1_SIZE	74	/* size of level GRP1 chunk   */

/* (element number, number of change pages, change page number) */
#define LEVEL_CHUNK_CUSX_UNCHANGED	(2 + (1 + 1) + (1 + 1))

/* (element number only) */
#define LEVEL_CHUNK_GRPX_UNCHANGED	2
#define LEVEL_CHUNK_NOTE_UNCHANGED	2

/* (nothing at all if unchanged) */
#define LEVEL_CHUNK_ELEM_UNCHANGED	0

#define TAPE_CHUNK_VERS_SIZE	8	/* size of file version chunk */
#define TAPE_CHUNK_HEAD_SIZE	20	/* size of tape file header   */
#define TAPE_CHUNK_HEAD_UNUSED	2	/* unused tape header bytes   */

#define LEVEL_CHUNK_CNT3_SIZE(x)	 (LEVEL_CHUNK_CNT3_HEADER + (x))
#define LEVEL_CHUNK_CUS3_SIZE(x)	 (2 + (x) * LEVEL_CPART_CUS3_SIZE)
#define LEVEL_CHUNK_CUS4_SIZE(x)	 (96 + (x) * 48)

/* file identifier strings */
#define LEVEL_COOKIE_TMPL		"ROCKSNDIAMONDS_LEVEL_FILE_VERSION_x.x"
#define TAPE_COOKIE_TMPL		"ROCKSNDIAMONDS_TAPE_FILE_VERSION_x.x"
#define SCORE_COOKIE			"ROCKSNDIAMONDS_SCORE_FILE_VERSION_1.2"

/* values for deciding when (not) to save configuration data */
#define SAVE_CONF_NEVER			0
#define SAVE_CONF_ALWAYS		1
#define SAVE_CONF_WHEN_CHANGED		-1

/* values for chunks using micro chunks */
#define CONF_MASK_1_BYTE		0x00
#define CONF_MASK_2_BYTE		0x40
#define CONF_MASK_4_BYTE		0x80
#define CONF_MASK_MULTI_BYTES		0xc0

#define CONF_MASK_BYTES			0xc0
#define CONF_MASK_TOKEN			0x3f

#define CONF_VALUE_1_BYTE(x)		(CONF_MASK_1_BYTE	| (x))
#define CONF_VALUE_2_BYTE(x)		(CONF_MASK_2_BYTE	| (x))
#define CONF_VALUE_4_BYTE(x)		(CONF_MASK_4_BYTE	| (x))
#define CONF_VALUE_MULTI_BYTES(x)	(CONF_MASK_MULTI_BYTES	| (x))

/* these definitions are just for convenience of use and readability */
#define CONF_VALUE_8_BIT(x)		CONF_VALUE_1_BYTE(x)
#define CONF_VALUE_16_BIT(x)		CONF_VALUE_2_BYTE(x)
#define CONF_VALUE_32_BIT(x)		CONF_VALUE_4_BYTE(x)
#define CONF_VALUE_BYTES(x)		CONF_VALUE_MULTI_BYTES(x)

#define CONF_VALUE_NUM_BYTES(x)		((x) == CONF_MASK_1_BYTE ? 1 :	\
					 (x) == CONF_MASK_2_BYTE ? 2 :	\
					 (x) == CONF_MASK_4_BYTE ? 4 : 0)

#define CONF_CONTENT_NUM_ELEMENTS	(3 * 3)
#define CONF_CONTENT_NUM_BYTES		(CONF_CONTENT_NUM_ELEMENTS * 2)
#define CONF_ELEMENT_NUM_BYTES		(2)

#define CONF_ENTITY_NUM_BYTES(t)	((t) == TYPE_ELEMENT ||		\
					 (t) == TYPE_ELEMENT_LIST ?	\
					 CONF_ELEMENT_NUM_BYTES :	\
					 (t) == TYPE_CONTENT ||		\
					 (t) == TYPE_CONTENT_LIST ?	\
					 CONF_CONTENT_NUM_BYTES : 1)

#define CONF_ELEMENT_BYTE_POS(i)	((i) * CONF_ELEMENT_NUM_BYTES)
#define CONF_ELEMENTS_ELEMENT(b,i)     ((b[CONF_ELEMENT_BYTE_POS(i)] << 8) |  \
					(b[CONF_ELEMENT_BYTE_POS(i) + 1]))

#define CONF_CONTENT_ELEMENT_POS(c,x,y)	((c) * CONF_CONTENT_NUM_ELEMENTS +    \
					 (y) * 3 + (x))
#define CONF_CONTENT_BYTE_POS(c,x,y)	(CONF_CONTENT_ELEMENT_POS(c,x,y) *    \
					 CONF_ELEMENT_NUM_BYTES)
#define CONF_CONTENTS_ELEMENT(b,c,x,y) ((b[CONF_CONTENT_BYTE_POS(c,x,y)]<< 8)|\
					(b[CONF_CONTENT_BYTE_POS(c,x,y) + 1]))

/* temporary variables used to store pointers to structure members */
static struct LevelInfo li;
static struct ElementInfo xx_ei, yy_ei;
static struct ElementChangeInfo xx_change;
static struct ElementGroupInfo xx_group;
static struct EnvelopeInfo xx_envelope;
static unsigned int xx_event_bits[NUM_CE_BITFIELDS];
static char xx_default_description[MAX_ELEMENT_NAME_LEN + 1];
static int xx_num_contents;
static int xx_current_change_page;
static char xx_default_string_empty[1] = "";
static int xx_string_length_unused;

struct LevelFileConfigInfo
{
  int element;			/* element for which data is to be stored */
  int save_type;		/* save data always, never or when changed */
  int data_type;		/* data type (used internally, not stored) */
  int conf_type;		/* micro chunk identifier (stored in file) */

  /* (mandatory) */
  void *value;			/* variable that holds the data to be stored */
  int default_value;		/* initial default value for this variable */

  /* (optional) */
  void *value_copy;		/* variable that holds the data to be copied */
  void *num_entities;		/* number of entities for multi-byte data */
  int default_num_entities;	/* default number of entities for this data */
  int max_num_entities;		/* maximal number of entities for this data */
  char *default_string;		/* optional default string for string data */
};

static struct LevelFileConfigInfo chunk_config_INFO[] =
{
  /* ---------- values not related to single elements ----------------------- */

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.game_engine_type,		GAME_ENGINE_TYPE_RND
  },

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.fieldx,				STD_LEV_FIELDX
  },
  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.fieldy,				STD_LEV_FIELDY
  },

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(3),
    &li.time,				100
  },

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(4),
    &li.gems_needed,			0
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_32_BIT(2),
    &li.random_seed,			0
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.use_step_counter,		FALSE
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(4),
    &li.wind_direction_initial,		MV_NONE
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(5),
    &li.em_slippery_gems,		FALSE
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(6),
    &li.use_custom_template,		FALSE
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(1),
    &li.can_move_into_acid_bits,	~0	/* default: everything can */
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(7),
    &li.dont_collide_with_bits,		~0	/* default: always deadly */
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.em_explodes_by_fire,		FALSE
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(5),
    &li.score[SC_TIME_BONUS],		1
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.auto_exit_sokoban,		FALSE
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.auto_count_gems,		FALSE
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_ELEM[] =
{
  /* (these values are the same for each player) */
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.block_last_field,		FALSE	/* default case for EM levels */
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.sp_block_last_field,		TRUE	/* default case for SP levels */
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.instant_relocation,		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &li.can_pass_to_walkable,		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(5),
    &li.block_snap_field,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(6),
    &li.continuous_snapping,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(12),
    &li.shifted_relocation,		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(15),
    &li.lazy_relocation,		FALSE
  },

  /* (these values are different for each player) */
  {
    EL_PLAYER_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[0],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[0],	1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[0][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[0],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  {
    EL_PLAYER_2,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[1],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[1],	FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[1],		FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[1],		EL_PLAYER_2
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[1],		FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[1],		EL_PLAYER_2
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[1],	FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[1],		EL_PLAYER_2
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[1],	FALSE
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[1],	1
  },
  {
    EL_PLAYER_2,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[1][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[1],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  {
    EL_PLAYER_3,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[2],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[2],	FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[2],		FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[2],		EL_PLAYER_3
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[2],		FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[2],		EL_PLAYER_3
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[2],	FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[2],		EL_PLAYER_3
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[2],	FALSE
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[2],	1
  },
  {
    EL_PLAYER_3,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[2][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[2],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  {
    EL_PLAYER_4,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &li.initial_player_stepsize[3],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &li.initial_player_gravity[3],	FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.use_start_element[3],		FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[3],		EL_PLAYER_4
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_artwork_element[3],		FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[3],		EL_PLAYER_4
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_explosion_element[3],	FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[3],		EL_PLAYER_4
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.use_initial_inventory[3],	FALSE
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(14),
    &li.initial_inventory_size[3],	1
  },
  {
    EL_PLAYER_4,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.initial_inventory_content[3][0],EL_EMPTY, NULL,
    &li.initial_inventory_size[3],	1, MAX_INITIAL_INVENTORY_SIZE
  },

  {
    EL_EMERALD,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_EMERALD],		10
  },

  {
    EL_DIAMOND,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_DIAMOND],		10
  },

  {
    EL_BUG,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_BUG],			10
  },

  {
    EL_SPACESHIP,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_SPACESHIP],		10
  },

  {
    EL_PACMAN,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_PACMAN],		10
  },

  {
    EL_NUT,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_NUT],			10
  },

  {
    EL_DYNAMITE,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_DYNAMITE],		10
  },

  {
    EL_KEY_1,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_KEY],			10
  },

  {
    EL_PEARL,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_PEARL],		10
  },

  {
    EL_CRYSTAL,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_CRYSTAL],		10
  },

  {
    EL_BD_AMOEBA,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.amoeba_content,			EL_DIAMOND
  },
  {
    EL_BD_AMOEBA,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.amoeba_speed,			10
  },
  {
    EL_BD_AMOEBA,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.grow_into_diggable,		TRUE
  },

  {
    EL_YAMYAM,				-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(1),
    &li.yamyam_content,			EL_ROCK, NULL,
    &li.num_yamyam_contents,		4, MAX_ELEMENT_CONTENTS
  },
  {
    EL_YAMYAM,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_YAMYAM],		10
  },

  {
    EL_ROBOT,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_ROBOT],		10
  },
  {
    EL_ROBOT,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.slurp_score,			10
  },

  {
    EL_ROBOT_WHEEL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_wheel,			10
  },

  {
    EL_MAGIC_WALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_magic_wall,		10
  },

  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.game_of_life[0],		2
  },
  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.game_of_life[1],		3
  },
  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.game_of_life[2],		3
  },
  {
    EL_GAME_OF_LIFE,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(4),
    &li.game_of_life[3],		3
  },

  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.biomaze[0],			2
  },
  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &li.biomaze[1],			3
  },
  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(3),
    &li.biomaze[2],			3
  },
  {
    EL_BIOMAZE,				-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(4),
    &li.biomaze[3],			3
  },

  {
    EL_TIMEGATE_SWITCH,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_timegate,			10
  },

  {
    EL_LIGHT_SWITCH_ACTIVE,		-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_light,			10
  },

  {
    EL_SHIELD_NORMAL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.shield_normal_time,		10
  },
  {
    EL_SHIELD_NORMAL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.score[SC_SHIELD],		10
  },

  {
    EL_SHIELD_DEADLY,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.shield_deadly_time,		10
  },
  {
    EL_SHIELD_DEADLY,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.score[SC_SHIELD],		10
  },

  {
    EL_EXTRA_TIME,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.extra_time,			10
  },
  {
    EL_EXTRA_TIME,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.extra_time_score,		10
  },

  {
    EL_TIME_ORB_FULL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.time_orb_time,			10
  },
  {
    EL_TIME_ORB_FULL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.use_time_orb_bug,		FALSE
  },

  {
    EL_SPRING,				-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.use_spring_bug,			FALSE
  },

  {
    EL_EMC_ANDROID,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.android_move_time,		10
  },
  {
    EL_EMC_ANDROID,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.android_clone_time,		10
  },
  {
    EL_EMC_ANDROID,			-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(1),
    &li.android_clone_element[0],	EL_EMPTY, NULL,
    &li.num_android_clone_elements,	1, MAX_ANDROID_ELEMENTS
  },

  {
    EL_EMC_LENSES,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.lenses_score,			10
  },
  {
    EL_EMC_LENSES,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.lenses_time,			10
  },

  {
    EL_EMC_MAGNIFIER,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.magnify_score,			10
  },
  {
    EL_EMC_MAGNIFIER,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &li.magnify_time,			10
  },

  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.ball_time,			10
  },
  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.ball_random,			FALSE
  },
  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.ball_state_initial,		FALSE
  },
  {
    EL_EMC_MAGIC_BALL,			-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(1),
    &li.ball_content,			EL_EMPTY, NULL,
    &li.num_ball_contents,		4, MAX_ELEMENT_CONTENTS
  },

  {
    EL_MM_MCDUFFIN,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.mm_laser_red,			FALSE
  },
  {
    EL_MM_MCDUFFIN,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.mm_laser_green,			FALSE
  },
  {
    EL_MM_MCDUFFIN,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.mm_laser_blue,			TRUE
  },

  {
    EL_DF_LASER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &li.df_laser_red,			TRUE
  },
  {
    EL_DF_LASER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &li.df_laser_green,			TRUE
  },
  {
    EL_DF_LASER,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &li.df_laser_blue,			FALSE
  },

  {
    EL_MM_FUSE_ACTIVE,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_fuse,			25
  },
  {
    EL_MM_BOMB,				-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_bomb,			75
  },
  {
    EL_MM_GRAY_BALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_ball,			75
  },
  {
    EL_MM_STEEL_BLOCK,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.mm_time_block,			75
  },
  {
    EL_MM_LIGHTBALL,			-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_ELEM_BONUS],		10
  },

  /* ---------- unused values ----------------------------------------------- */

  {
    EL_UNKNOWN,				SAVE_CONF_NEVER,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(1),
    &li.score[SC_UNKNOWN_15],		10
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_NOTE[] =
{
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &xx_envelope.xsize,			MAX_ENVELOPE_XSIZE,
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &xx_envelope.ysize,			MAX_ENVELOPE_YSIZE,
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &xx_envelope.autowrap,		FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(4),
    &xx_envelope.centered,		FALSE
  },

  {
    -1,					-1,
    TYPE_STRING,			CONF_VALUE_BYTES(1),
    &xx_envelope.text,			-1, NULL,
    &xx_string_length_unused,		-1, MAX_ENVELOPE_TEXT_LEN,
    &xx_default_string_empty[0]
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_CUSX_base[] =
{
  {
    -1,					-1,
    TYPE_STRING,			CONF_VALUE_BYTES(1),
    &xx_ei.description[0],		-1,
    &yy_ei.description[0],
    &xx_string_length_unused,		-1, MAX_ELEMENT_NAME_LEN,
    &xx_default_description[0]
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(1),
    &xx_ei.properties[EP_BITFIELD_BASE_NR], EP_BITMASK_BASE_DEFAULT,
    &yy_ei.properties[EP_BITFIELD_BASE_NR]
  },
#if ENABLE_RESERVED_CODE
  /* (reserved for later use) */
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(2),
    &xx_ei.properties[EP_BITFIELD_BASE_NR + 1], EP_BITMASK_DEFAULT,
    &yy_ei.properties[EP_BITFIELD_BASE_NR + 1]
  },
#endif

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &xx_ei.use_gfx_element,		FALSE,
    &yy_ei.use_gfx_element
  },
  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &xx_ei.gfx_element_initial,		EL_EMPTY_SPACE,
    &yy_ei.gfx_element_initial
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(2),
    &xx_ei.access_direction,		MV_ALL_DIRECTIONS,
    &yy_ei.access_direction
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &xx_ei.collect_score_initial,	10,
    &yy_ei.collect_score_initial
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(3),
    &xx_ei.collect_count_initial,	1,
    &yy_ei.collect_count_initial
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(4),
    &xx_ei.ce_value_fixed_initial,	0,
    &yy_ei.ce_value_fixed_initial
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(5),
    &xx_ei.ce_value_random_initial,	0,
    &yy_ei.ce_value_random_initial
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(3),
    &xx_ei.use_last_ce_value,		FALSE,
    &yy_ei.use_last_ce_value
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(6),
    &xx_ei.push_delay_fixed,		8,
    &yy_ei.push_delay_fixed
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(7),
    &xx_ei.push_delay_random,		8,
    &yy_ei.push_delay_random
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(8),
    &xx_ei.drop_delay_fixed,		0,
    &yy_ei.drop_delay_fixed
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(9),
    &xx_ei.drop_delay_random,		0,
    &yy_ei.drop_delay_random
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(10),
    &xx_ei.move_delay_fixed,		0,
    &yy_ei.move_delay_fixed
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(11),
    &xx_ei.move_delay_random,		0,
    &yy_ei.move_delay_random
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(3),
    &xx_ei.move_pattern,		MV_ALL_DIRECTIONS,
    &yy_ei.move_pattern
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(4),
    &xx_ei.move_direction_initial,	MV_START_AUTOMATIC,
    &yy_ei.move_direction_initial
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(5),
    &xx_ei.move_stepsize,		TILEX / 8,
    &yy_ei.move_stepsize
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(12),
    &xx_ei.move_enter_element,		EL_EMPTY_SPACE,
    &yy_ei.move_enter_element
  },
  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(13),
    &xx_ei.move_leave_element,		EL_EMPTY_SPACE,
    &yy_ei.move_leave_element
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(6),
    &xx_ei.move_leave_type,		LEAVE_TYPE_UNLIMITED,
    &yy_ei.move_leave_type
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(7),
    &xx_ei.slippery_type,		SLIPPERY_ANY_RANDOM,
    &yy_ei.slippery_type
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(8),
    &xx_ei.explosion_type,		EXPLODES_3X3,
    &yy_ei.explosion_type
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(14),
    &xx_ei.explosion_delay,		16,
    &yy_ei.explosion_delay
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(15),
    &xx_ei.ignition_delay,		8,
    &yy_ei.ignition_delay
  },

  {
    -1,					-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(2),
    &xx_ei.content,			EL_EMPTY_SPACE,
    &yy_ei.content,
    &xx_num_contents,			1, 1
  },

  /* ---------- "num_change_pages" must be the last entry ------------------- */

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(9),
    &xx_ei.num_change_pages,		1,
    &yy_ei.num_change_pages
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1,
    NULL
  }
};

static struct LevelFileConfigInfo chunk_config_CUSX_change[] =
{
  /* ---------- "current_change_page" must be the first entry --------------- */

  {
    -1,					SAVE_CONF_ALWAYS,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &xx_current_change_page,		-1
  },

  /* ---------- (the remaining entries can be in any order) ----------------- */

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(2),
    &xx_change.can_change,		FALSE
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(1),
    &xx_event_bits[0],			0
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(2),
    &xx_event_bits[1],			0
  },

  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(3),
    &xx_change.trigger_player,		CH_PLAYER_ANY
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_8_BIT(4),
    &xx_change.trigger_side,		CH_SIDE_ANY
  },
  {
    -1,					-1,
    TYPE_BITFIELD,			CONF_VALUE_32_BIT(3),
    &xx_change.trigger_page,		CH_PAGE_ANY
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &xx_change.target_element,		EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(2),
    &xx_change.delay_fixed,		0
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(3),
    &xx_change.delay_random,		0
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(4),
    &xx_change.delay_frames,		FRAMES_PER_SECOND
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(5),
    &xx_change.initial_trigger_element,	EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(6),
    &xx_change.explode,			FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(7),
    &xx_change.use_target_content,	FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(8),
    &xx_change.only_if_complete,	FALSE
  },
  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &xx_change.use_random_replace,	FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(10),
    &xx_change.random_percentage,	100
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(11),
    &xx_change.replace_when,		CP_WHEN_EMPTY
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(12),
    &xx_change.has_action,		FALSE
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(13),
    &xx_change.action_type,		CA_NO_ACTION
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(14),
    &xx_change.action_mode,		CA_MODE_UNDEFINED
  },
  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_16_BIT(6),
    &xx_change.action_arg,		CA_ARG_UNDEFINED
  },

  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(7),
    &xx_change.action_element,		EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_CONTENT_LIST,			CONF_VALUE_BYTES(1),
    &xx_change.target_content,		EL_EMPTY_SPACE, NULL,
    &xx_num_contents,			1, 1
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_GRPX[] =
{
  {
    -1,					-1,
    TYPE_STRING,			CONF_VALUE_BYTES(1),
    &xx_ei.description[0],		-1, NULL,
    &xx_string_length_unused,		-1, MAX_ELEMENT_NAME_LEN,
    &xx_default_description[0]
  },

  {
    -1,					-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(1),
    &xx_ei.use_gfx_element,		FALSE
  },
  {
    -1,					-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &xx_ei.gfx_element_initial,		EL_EMPTY_SPACE
  },

  {
    -1,					-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(2),
    &xx_group.choice_mode,		ANIM_RANDOM
  },

  {
    -1,					-1,
    TYPE_ELEMENT_LIST,			CONF_VALUE_BYTES(2),
    &xx_group.element[0],		EL_EMPTY_SPACE, NULL,
    &xx_group.num_elements,		1, MAX_ELEMENTS_IN_GROUP
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct LevelFileConfigInfo chunk_config_CONF[] =		/* (OBSOLETE) */
{
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(9),
    &li.block_snap_field,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(13),
    &li.continuous_snapping,		TRUE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_INTEGER,			CONF_VALUE_8_BIT(1),
    &li.initial_player_stepsize[0],	STEPSIZE_NORMAL
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(10),
    &li.use_start_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(1),
    &li.start_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(11),
    &li.use_artwork_element[0],		FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(2),
    &li.artwork_element[0],		EL_PLAYER_1
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_BOOLEAN,			CONF_VALUE_8_BIT(12),
    &li.use_explosion_element[0],	FALSE
  },
  {
    EL_PLAYER_1,			-1,
    TYPE_ELEMENT,			CONF_VALUE_16_BIT(3),
    &li.explosion_element[0],		EL_PLAYER_1
  },

  {
    -1,					-1,
    -1,					-1,
    NULL,				-1
  }
};

static struct
{
  int filetype;
  char *id;
}
filetype_id_list[] =
{
  { LEVEL_FILE_TYPE_RND,	"RND"	},
  { LEVEL_FILE_TYPE_BD,		"BD"	},
  { LEVEL_FILE_TYPE_EM,		"EM"	},
  { LEVEL_FILE_TYPE_SP,		"SP"	},
  { LEVEL_FILE_TYPE_DX,		"DX"	},
  { LEVEL_FILE_TYPE_SB,		"SB"	},
  { LEVEL_FILE_TYPE_DC,		"DC"	},
  { LEVEL_FILE_TYPE_MM,		"MM"	},
  { LEVEL_FILE_TYPE_MM,		"DF"	},
  { -1,				NULL	},
};


/* ========================================================================= */
/* level file functions                                                      */
/* ========================================================================= */

static boolean check_special_flags(char *flag)
{
  if (strEqual(options.special_flags, flag) ||
      strEqual(leveldir_current->special_flags, flag))
    return TRUE;

  return FALSE;
}

static struct DateInfo getCurrentDate()
{
  time_t epoch_seconds = time(NULL);
  struct tm *now = localtime(&epoch_seconds);
  struct DateInfo date;

  date.year  = now->tm_year + 1900;
  date.month = now->tm_mon  + 1;
  date.day   = now->tm_mday;

  date.src   = DATE_SRC_CLOCK;

  return date;
}

static void resetEventFlags(struct ElementChangeInfo *change)
{
  int i;

  for (i = 0; i < NUM_CHANGE_EVENTS; i++)
    change->has_event[i] = FALSE;
}

static void resetEventBits()
{
  int i;

  for (i = 0; i < NUM_CE_BITFIELDS; i++)
    xx_event_bits[i] = 0;
}

static void setEventFlagsFromEventBits(struct ElementChangeInfo *change)
{
  int i;

  /* important: only change event flag if corresponding event bit is set
     (this is because all xx_event_bits[] values are loaded separately,
     and all xx_event_bits[] values are set back to zero before loading
     another value xx_event_bits[x] (each value representing 32 flags)) */

  for (i = 0; i < NUM_CHANGE_EVENTS; i++)
    if (xx_event_bits[CH_EVENT_BITFIELD_NR(i)] & CH_EVENT_BIT(i))
      change->has_event[i] = TRUE;
}

static void setEventBitsFromEventFlags(struct ElementChangeInfo *change)
{
  int i;

  /* in contrast to the above function setEventFlagsFromEventBits(), it
     would also be possible to set all bits in xx_event_bits[] to 0 or 1
     depending on the corresponding change->has_event[i] values here, as
     all xx_event_bits[] values are reset in resetEventBits() before */

  for (i = 0; i < NUM_CHANGE_EVENTS; i++)
    if (change->has_event[i])
      xx_event_bits[CH_EVENT_BITFIELD_NR(i)] |= CH_EVENT_BIT(i);
}

static char *getDefaultElementDescription(struct ElementInfo *ei)
{
  static char description[MAX_ELEMENT_NAME_LEN + 1];
  char *default_description = (ei->custom_description != NULL ?
			       ei->custom_description :
			       ei->editor_description);
  int i;

  /* always start with reliable default values */
  for (i = 0; i < MAX_ELEMENT_NAME_LEN + 1; i++)
    description[i] = '\0';

  /* truncate element description to MAX_ELEMENT_NAME_LEN bytes */
  strncpy(description, default_description, MAX_ELEMENT_NAME_LEN);

  return &description[0];
}

static void setElementDescriptionToDefault(struct ElementInfo *ei)
{
  char *default_description = getDefaultElementDescription(ei);
  int i;

  for (i = 0; i < MAX_ELEMENT_NAME_LEN + 1; i++)
    ei->description[i] = default_description[i];
}

static void setConfigToDefaultsFromConfigList(struct LevelFileConfigInfo *conf)
{
  int i;

  for (i = 0; conf[i].data_type != -1; i++)
  {
    int default_value = conf[i].default_value;
    int data_type = conf[i].data_type;
    int conf_type = conf[i].conf_type;
    int byte_mask = conf_type & CONF_MASK_BYTES;

    if (byte_mask == CONF_MASK_MULTI_BYTES)
    {
      int default_num_entities = conf[i].default_num_entities;
      int max_num_entities = conf[i].max_num_entities;

      *(int *)(conf[i].num_entities) = default_num_entities;

      if (data_type == TYPE_STRING)
      {
	char *default_string = conf[i].default_string;
	char *string = (char *)(conf[i].value);

	strncpy(string, default_string, max_num_entities);
      }
      else if (data_type == TYPE_ELEMENT_LIST)
      {
	int *element_array = (int *)(conf[i].value);
	int j;

	for (j = 0; j < max_num_entities; j++)
	  element_array[j] = default_value;
      }
      else if (data_type == TYPE_CONTENT_LIST)
      {
	struct Content *content = (struct Content *)(conf[i].value);
	int c, x, y;

	for (c = 0; c < max_num_entities; c++)
	  for (y = 0; y < 3; y++)
	    for (x = 0; x < 3; x++)
	      content[c].e[x][y] = default_value;
      }
    }
    else	/* constant size configuration data (1, 2 or 4 bytes) */
    {
      if (data_type == TYPE_BOOLEAN)
	*(boolean *)(conf[i].value) = default_value;
      else
	*(int *)    (conf[i].value) = default_value;
    }
  }
}

static void copyConfigFromConfigList(struct LevelFileConfigInfo *conf)
{
  int i;

  for (i = 0; conf[i].data_type != -1; i++)
  {
    int data_type = conf[i].data_type;
    int conf_type = conf[i].conf_type;
    int byte_mask = conf_type & CONF_MASK_BYTES;

    if (byte_mask == CONF_MASK_MULTI_BYTES)
    {
      int max_num_entities = conf[i].max_num_entities;

      if (data_type == TYPE_STRING)
      {
	char *string      = (char *)(conf[i].value);
	char *string_copy = (char *)(conf[i].value_copy);

	strncpy(string_copy, string, max_num_entities);
      }
      else if (data_type == TYPE_ELEMENT_LIST)
      {
	int *element_array      = (int *)(conf[i].value);
	int *element_array_copy = (int *)(conf[i].value_copy);
	int j;

	for (j = 0; j < max_num_entities; j++)
	  element_array_copy[j] = element_array[j];
      }
      else if (data_type == TYPE_CONTENT_LIST)
      {
	struct Content *content      = (struct Content *)(conf[i].value);
	struct Content *content_copy = (struct Content *)(conf[i].value_copy);
	int c, x, y;

	for (c = 0; c < max_num_entities; c++)
	  for (y = 0; y < 3; y++)
	    for (x = 0; x < 3; x++)
	      content_copy[c].e[x][y] = content[c].e[x][y];
      }
    }
    else	/* constant size configuration data (1, 2 or 4 bytes) */
    {
      if (data_type == TYPE_BOOLEAN)
	*(boolean *)(conf[i].value_copy) = *(boolean *)(conf[i].value);
      else
	*(int *)    (conf[i].value_copy) = *(int *)    (conf[i].value);
    }
  }
}

void copyElementInfo(struct ElementInfo *ei_from, struct ElementInfo *ei_to)
{
  int i;

  xx_ei = *ei_from;	/* copy element data into temporary buffer */
  yy_ei = *ei_to;	/* copy element data into temporary buffer */

  copyConfigFromConfigList(chunk_config_CUSX_base);

  *ei_from = xx_ei;
  *ei_to   = yy_ei;

  /* ---------- reinitialize and copy change pages ---------- */

  ei_to->num_change_pages = ei_from->num_change_pages;
  ei_to->current_change_page = ei_from->current_change_page;

  setElementChangePages(ei_to, ei_to->num_change_pages);

  for (i = 0; i < ei_to->num_change_pages; i++)
    ei_to->change_page[i] = ei_from->change_page[i];

  /* ---------- copy group element info ---------- */
  if (ei_from->group != NULL && ei_to->group != NULL)	/* group or internal */
    *ei_to->group = *ei_from->group;

  /* mark this custom element as modified */
  ei_to->modified_settings = TRUE;
}

void setElementChangePages(struct ElementInfo *ei, int change_pages)
{
  int change_page_size = sizeof(struct ElementChangeInfo);

  ei->num_change_pages = MAX(1, change_pages);

  ei->change_page =
    checked_realloc(ei->change_page, ei->num_change_pages * change_page_size);

  if (ei->current_change_page >= ei->num_change_pages)
    ei->current_change_page = ei->num_change_pages - 1;

  ei->change = &ei->change_page[ei->current_change_page];
}

void setElementChangeInfoToDefaults(struct ElementChangeInfo *change)
{
  xx_change = *change;		/* copy change data into temporary buffer */

  setConfigToDefaultsFromConfigList(chunk_config_CUSX_change);

  *change = xx_change;

  resetEventFlags(change);

  change->direct_action = 0;
  change->other_action = 0;

  change->pre_change_function = NULL;
  change->change_function = NULL;
  change->post_change_function = NULL;
}

static void setLevelInfoToDefaults_Level(struct LevelInfo *level)
{
  int i, x, y;

  li = *level;		/* copy level data into temporary buffer */
  setConfigToDefaultsFromConfigList(chunk_config_INFO);
  *level = li;		/* copy temporary buffer back to level data */

  setLevelInfoToDefaults_EM();
  setLevelInfoToDefaults_SP();
  setLevelInfoToDefaults_MM();

  level->native_em_level = &native_em_level;
  level->native_sp_level = &native_sp_level;
  level->native_mm_level = &native_mm_level;

  level->file_version = FILE_VERSION_ACTUAL;
  level->game_version = GAME_VERSION_ACTUAL;

  level->creation_date = getCurrentDate();

  level->encoding_16bit_field  = TRUE;
  level->encoding_16bit_yamyam = TRUE;
  level->encoding_16bit_amoeba = TRUE;

  /* clear level name and level author string buffers */
  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = '\0';
  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = '\0';

  /* set level name and level author to default values */
  strcpy(level->name, NAMELESS_LEVEL_NAME);
  strcpy(level->author, ANONYMOUS_NAME);

  /* set level playfield to playable default level with player and exit */
  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      level->field[x][y] = EL_SAND;

  level->field[0][0] = EL_PLAYER_1;
  level->field[STD_LEV_FIELDX - 1][STD_LEV_FIELDY - 1] = EL_EXIT_CLOSED;

  BorderElement = EL_STEELWALL;

  /* detect custom elements when loading them */
  level->file_has_custom_elements = FALSE;

  /* set all bug compatibility flags to "false" => do not emulate this bug */
  level->use_action_after_change_bug = FALSE;

  if (leveldir_current)
  {
    /* try to determine better author name than 'anonymous' */
    if (!strEqual(leveldir_current->author, ANONYMOUS_NAME))
    {
      strncpy(level->author, leveldir_current->author, MAX_LEVEL_AUTHOR_LEN);
      level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
    }
    else
    {
      switch (LEVELCLASS(leveldir_current))
      {
	case LEVELCLASS_TUTORIAL:
	  strcpy(level->author, PROGRAM_AUTHOR_STRING);
	  break;

        case LEVELCLASS_CONTRIB:
	  strncpy(level->author, leveldir_current->name, MAX_LEVEL_AUTHOR_LEN);
	  level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	  break;

        case LEVELCLASS_PRIVATE:
	  strncpy(level->author, getRealName(), MAX_LEVEL_AUTHOR_LEN);
	  level->author[MAX_LEVEL_AUTHOR_LEN] = '\0';
	  break;

        default:
	  /* keep default value */
	  break;
      }
    }
  }
}

static void setLevelInfoToDefaults_Elements(struct LevelInfo *level)
{
  static boolean clipboard_elements_initialized = FALSE;
  int i;

  InitElementPropertiesStatic();

  li = *level;		/* copy level data into temporary buffer */
  setConfigToDefaultsFromConfigList(chunk_config_ELEM);
  *level = li;		/* copy temporary buffer back to level data */

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
  {
    int element = i;
    struct ElementInfo *ei = &element_info[element];

    /* never initialize clipboard elements after the very first time */
    /* (to be able to use clipboard elements between several levels) */
    if (IS_CLIPBOARD_ELEMENT(element) && clipboard_elements_initialized)
      continue;

    if (IS_ENVELOPE(element))
    {
      int envelope_nr = element - EL_ENVELOPE_1;

      setConfigToDefaultsFromConfigList(chunk_config_NOTE);

      level->envelope[envelope_nr] = xx_envelope;
    }

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_GROUP_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      xx_ei = *ei;	/* copy element data into temporary buffer */

      setConfigToDefaultsFromConfigList(chunk_config_CUSX_base);

      *ei = xx_ei;
    }

    setElementChangePages(ei, 1);
    setElementChangeInfoToDefaults(ei->change);

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_GROUP_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      setElementDescriptionToDefault(ei);

      ei->modified_settings = FALSE;
    }

    if (IS_CUSTOM_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      /* internal values used in level editor */

      ei->access_type = 0;
      ei->access_layer = 0;
      ei->access_protected = 0;
      ei->walk_to_action = 0;
      ei->smash_targets = 0;
      ei->deadliness = 0;

      ei->can_explode_by_fire = FALSE;
      ei->can_explode_smashed = FALSE;
      ei->can_explode_impact = FALSE;

      ei->current_change_page = 0;
    }

    if (IS_GROUP_ELEMENT(element) ||
	IS_INTERNAL_ELEMENT(element))
    {
      struct ElementGroupInfo *group;

      /* initialize memory for list of elements in group */
      if (ei->group == NULL)
	ei->group = checked_malloc(sizeof(struct ElementGroupInfo));

      group = ei->group;

      xx_group = *group;	/* copy group data into temporary buffer */

      setConfigToDefaultsFromConfigList(chunk_config_GRPX);

      *group = xx_group;
    }
  }

  clipboard_elements_initialized = TRUE;
}

static void setLevelInfoToDefaults(struct LevelInfo *level,
				   boolean level_info_only,
				   boolean reset_file_status)
{
  setLevelInfoToDefaults_Level(level);

  if (!level_info_only)
    setLevelInfoToDefaults_Elements(level);

  if (reset_file_status)
  {
    level->no_valid_file = FALSE;
    level->no_level_file = FALSE;
  }

  level->changed = FALSE;
}

static void setFileInfoToDefaults(struct LevelFileInfo *level_file_info)
{
  level_file_info->nr = 0;
  level_file_info->type = LEVEL_FILE_TYPE_UNKNOWN;
  level_file_info->packed = FALSE;
  level_file_info->basename = NULL;
  level_file_info->filename = NULL;
}

int getMappedElement_SB(int, boolean);

static void ActivateLevelTemplate()
{
  int x, y;

  if (check_special_flags("load_xsb_to_ces"))
  {
    /* fill smaller playfields with padding "beyond border wall" elements */
    if (level.fieldx < level_template.fieldx ||
	level.fieldy < level_template.fieldy)
    {
      short field[level.fieldx][level.fieldy];
      int new_fieldx = MAX(level.fieldx, level_template.fieldx);
      int new_fieldy = MAX(level.fieldy, level_template.fieldy);
      int pos_fieldx = (new_fieldx - level.fieldx) / 2;
      int pos_fieldy = (new_fieldy - level.fieldy) / 2;

      /* copy old playfield (which is smaller than the visible area) */
      for (y = 0; y < level.fieldy; y++) for (x = 0; x < level.fieldx; x++)
	field[x][y] = level.field[x][y];

      /* fill new, larger playfield with "beyond border wall" elements */
      for (y = 0; y < new_fieldy; y++) for (x = 0; x < new_fieldx; x++)
	level.field[x][y] = getMappedElement_SB('_', TRUE);

      /* copy the old playfield to the middle of the new playfield */
      for (y = 0; y < level.fieldy; y++) for (x = 0; x < level.fieldx; x++)
	level.field[pos_fieldx + x][pos_fieldy + y] = field[x][y];

      level.fieldx = new_fieldx;
      level.fieldy = new_fieldy;
    }
  }

  /* Currently there is no special action needed to activate the template
     data, because 'element_info' property settings overwrite the original
     level data, while all other variables do not change. */

  /* Exception: 'from_level_template' elements in the original level playfield
     are overwritten with the corresponding elements at the same position in
     playfield from the level template. */

  for (x = 0; x < level.fieldx; x++)
    for (y = 0; y < level.fieldy; y++)
      if (level.field[x][y] == EL_FROM_LEVEL_TEMPLATE)
	level.field[x][y] = level_template.field[x][y];

  if (check_special_flags("load_xsb_to_ces"))
  {
    struct LevelInfo level_backup = level;

    /* overwrite all individual level settings from template level settings */
    level = level_template;

    /* restore playfield size */
    level.fieldx = level_backup.fieldx;
    level.fieldy = level_backup.fieldy;

    /* restore playfield content */
    for (x = 0; x < level.fieldx; x++)
      for (y = 0; y < level.fieldy; y++)
	level.field[x][y] = level_backup.field[x][y];

    /* restore name and author from individual level */
    strcpy(level.name,   level_backup.name);
    strcpy(level.author, level_backup.author);

    /* restore flag "use_custom_template" */
    level.use_custom_template = level_backup.use_custom_template;
  }
}

static char *getLevelFilenameFromBasename(char *basename)
{
  static char *filename[2] = { NULL, NULL };
  int pos = (strEqual(basename, LEVELTEMPLATE_FILENAME) ? 0 : 1);

  checked_free(filename[pos]);

  filename[pos] = getPath2(getCurrentLevelDir(), basename);

  return filename[pos];
}

static int getFileTypeFromBasename(char *basename)
{
  /* !!! ALSO SEE COMMENT IN checkForPackageFromBasename() !!! */

  static char *filename = NULL;
  struct stat file_status;

  /* ---------- try to determine file type from filename ---------- */

  /* check for typical filename of a Supaplex level package file */
  if (strlen(basename) == 10 && strPrefixLower(basename, "levels.d"))
    return LEVEL_FILE_TYPE_SP;

  /* check for typical filename of a Diamond Caves II level package file */
  if (strSuffixLower(basename, ".dc") ||
      strSuffixLower(basename, ".dc2"))
    return LEVEL_FILE_TYPE_DC;

  /* check for typical filename of a Sokoban level package file */
  if (strSuffixLower(basename, ".xsb") &&
      strchr(basename, '%') == NULL)
    return LEVEL_FILE_TYPE_SB;

  /* ---------- try to determine file type from filesize ---------- */

  checked_free(filename);
  filename = getPath2(getCurrentLevelDir(), basename);

  if (stat(filename, &file_status) == 0)
  {
    /* check for typical filesize of a Supaplex level package file */
    if (file_status.st_size == 170496)
      return LEVEL_FILE_TYPE_SP;
  }

  return LEVEL_FILE_TYPE_UNKNOWN;
}

static int getFileTypeFromMagicBytes(char *filename, int type)
{
  File *file;

  if ((file = openFile(filename, MODE_READ)))
  {
    char chunk_name[CHUNK_ID_LEN + 1];

    getFileChunkBE(file, chunk_name, NULL);

    if (strEqual(chunk_name, "MMII") ||
	strEqual(chunk_name, "MIRR"))
      type = LEVEL_FILE_TYPE_MM;

    closeFile(file);
  }

  return type;
}

static boolean checkForPackageFromBasename(char *basename)
{
  /* !!! WON'T WORK ANYMORE IF getFileTypeFromBasename() ALSO DETECTS !!!
     !!! SINGLE LEVELS (CURRENTLY ONLY DETECTS LEVEL PACKAGES         !!! */

  return (getFileTypeFromBasename(basename) != LEVEL_FILE_TYPE_UNKNOWN);
}

static char *getSingleLevelBasenameExt(int nr, char *extension)
{
  static char basename[MAX_FILENAME_LEN];

  if (nr < 0)
    sprintf(basename, "%s", LEVELTEMPLATE_FILENAME);
  else
    sprintf(basename, "%03d.%s", nr, extension);

  return basename;
}

static char *getSingleLevelBasename(int nr)
{
  return getSingleLevelBasenameExt(nr, LEVELFILE_EXTENSION);
}

static char *getPackedLevelBasename(int type)
{
  static char basename[MAX_FILENAME_LEN];
  char *directory = getCurrentLevelDir();
  Directory *dir;
  DirectoryEntry *dir_entry;

  strcpy(basename, UNDEFINED_FILENAME);		/* default: undefined file */

  if ((dir = openDirectory(directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read current level directory '%s'", directory);

    return basename;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	/* loop all entries */
  {
    char *entry_basename = dir_entry->basename;
    int entry_type = getFileTypeFromBasename(entry_basename);

    if (entry_type != LEVEL_FILE_TYPE_UNKNOWN)	/* found valid level package */
    {
      if (type == LEVEL_FILE_TYPE_UNKNOWN ||
	  type == entry_type)
      {
	strcpy(basename, entry_basename);

	break;
      }
    }
  }

  closeDirectory(dir);

  return basename;
}

static char *getSingleLevelFilename(int nr)
{
  return getLevelFilenameFromBasename(getSingleLevelBasename(nr));
}

#if ENABLE_UNUSED_CODE
static char *getPackedLevelFilename(int type)
{
  return getLevelFilenameFromBasename(getPackedLevelBasename(type));
}
#endif

char *getDefaultLevelFilename(int nr)
{
  return getSingleLevelFilename(nr);
}

#if ENABLE_UNUSED_CODE
static void setLevelFileInfo_SingleLevelFilename(struct LevelFileInfo *lfi,
						 int type)
{
  lfi->type = type;
  lfi->packed = FALSE;
  lfi->basename = getSingleLevelBasename(lfi->nr, lfi->type);
  lfi->filename = getLevelFilenameFromBasename(lfi->basename);
}
#endif

static void setLevelFileInfo_FormatLevelFilename(struct LevelFileInfo *lfi,
						 int type, char *format, ...)
{
  static char basename[MAX_FILENAME_LEN];
  va_list ap;

  va_start(ap, format);
  vsprintf(basename, format, ap);
  va_end(ap);

  lfi->type = type;
  lfi->packed = FALSE;
  lfi->basename = basename;
  lfi->filename = getLevelFilenameFromBasename(lfi->basename);
}

static void setLevelFileInfo_PackedLevelFilename(struct LevelFileInfo *lfi,
						 int type)
{
  lfi->type = type;
  lfi->packed = TRUE;
  lfi->basename = getPackedLevelBasename(lfi->type);
  lfi->filename = getLevelFilenameFromBasename(lfi->basename);
}

static int getFiletypeFromID(char *filetype_id)
{
  char *filetype_id_lower;
  int filetype = LEVEL_FILE_TYPE_UNKNOWN;
  int i;

  if (filetype_id == NULL)
    return LEVEL_FILE_TYPE_UNKNOWN;

  filetype_id_lower = getStringToLower(filetype_id);

  for (i = 0; filetype_id_list[i].id != NULL; i++)
  {
    char *id_lower = getStringToLower(filetype_id_list[i].id);
    
    if (strEqual(filetype_id_lower, id_lower))
      filetype = filetype_id_list[i].filetype;

    free(id_lower);

    if (filetype != LEVEL_FILE_TYPE_UNKNOWN)
      break;
  }

  free(filetype_id_lower);

  return filetype;
}

char *getLocalLevelTemplateFilename()
{
  return getDefaultLevelFilename(-1);
}

char *getGlobalLevelTemplateFilename()
{
  /* global variable "leveldir_current" must be modified in the loop below */
  LevelDirTree *leveldir_current_last = leveldir_current;
  char *filename = NULL;

  /* check for template level in path from current to topmost tree node */

  while (leveldir_current != NULL)
  {
    filename = getDefaultLevelFilename(-1);

    if (fileExists(filename))
      break;

    leveldir_current = leveldir_current->node_parent;
  }

  /* restore global variable "leveldir_current" modified in above loop */
  leveldir_current = leveldir_current_last;

  return filename;
}

static void determineLevelFileInfo_Filename(struct LevelFileInfo *lfi)
{
  int nr = lfi->nr;

  /* special case: level number is negative => check for level template file */
  if (nr < 0)
  {
    setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_RND,
					 getSingleLevelBasename(-1));

    /* replace local level template filename with global template filename */
    lfi->filename = getGlobalLevelTemplateFilename();

    /* no fallback if template file not existing */
    return;
  }

  /* special case: check for file name/pattern specified in "levelinfo.conf" */
  if (leveldir_current->level_filename != NULL)
  {
    int filetype = getFiletypeFromID(leveldir_current->level_filetype);

    setLevelFileInfo_FormatLevelFilename(lfi, filetype,
					 leveldir_current->level_filename, nr);

    lfi->packed = checkForPackageFromBasename(leveldir_current->level_filename);

    if (fileExists(lfi->filename))
      return;
  }
  else if (leveldir_current->level_filetype != NULL)
  {
    int filetype = getFiletypeFromID(leveldir_current->level_filetype);

    /* check for specified native level file with standard file name */
    setLevelFileInfo_FormatLevelFilename(lfi, filetype,
					 "%03d.%s", nr, LEVELFILE_EXTENSION);
    if (fileExists(lfi->filename))
      return;
  }

  /* check for native Rocks'n'Diamonds level file */
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_RND,
				       "%03d.%s", nr, LEVELFILE_EXTENSION);
  if (fileExists(lfi->filename))
    return;

  /* check for Emerald Mine level file (V1) */
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "a%c%c",
				       'a' + (nr / 10) % 26, '0' + nr % 10);
  if (fileExists(lfi->filename))
    return;
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "A%c%c",
				       'A' + (nr / 10) % 26, '0' + nr % 10);
  if (fileExists(lfi->filename))
    return;

  /* check for Emerald Mine level file (V2 to V5) */
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%d", nr);
  if (fileExists(lfi->filename))
    return;

  /* check for Emerald Mine level file (V6 / single mode) */
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02ds", nr);
  if (fileExists(lfi->filename))
    return;
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02dS", nr);
  if (fileExists(lfi->filename))
    return;

  /* check for Emerald Mine level file (V6 / teamwork mode) */
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02dt", nr);
  if (fileExists(lfi->filename))
    return;
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_EM, "%02dT", nr);
  if (fileExists(lfi->filename))
    return;

  /* check for various packed level file formats */
  setLevelFileInfo_PackedLevelFilename(lfi, LEVEL_FILE_TYPE_UNKNOWN);
  if (fileExists(lfi->filename))
    return;

  /* no known level file found -- use default values (and fail later) */
  setLevelFileInfo_FormatLevelFilename(lfi, LEVEL_FILE_TYPE_RND,
				       "%03d.%s", nr, LEVELFILE_EXTENSION);
}

static void determineLevelFileInfo_Filetype(struct LevelFileInfo *lfi)
{
  if (lfi->type == LEVEL_FILE_TYPE_UNKNOWN)
    lfi->type = getFileTypeFromBasename(lfi->basename);

  if (lfi->type == LEVEL_FILE_TYPE_RND)
    lfi->type = getFileTypeFromMagicBytes(lfi->filename, lfi->type);
}

static void setLevelFileInfo(struct LevelFileInfo *level_file_info, int nr)
{
  /* always start with reliable default values */
  setFileInfoToDefaults(level_file_info);

  level_file_info->nr = nr;	/* set requested level number */

  determineLevelFileInfo_Filename(level_file_info);
  determineLevelFileInfo_Filetype(level_file_info);
}

/* ------------------------------------------------------------------------- */
/* functions for loading R'n'D level                                         */
/* ------------------------------------------------------------------------- */

int getMappedElement(int element)
{
  /* remap some (historic, now obsolete) elements */

  switch (element)
  {
    case EL_PLAYER_OBSOLETE:
      element = EL_PLAYER_1;
      break;

    case EL_KEY_OBSOLETE:
      element = EL_KEY_1;
      break;

    case EL_EM_KEY_1_FILE_OBSOLETE:
      element = EL_EM_KEY_1;
      break;

    case EL_EM_KEY_2_FILE_OBSOLETE:
      element = EL_EM_KEY_2;
      break;

    case EL_EM_KEY_3_FILE_OBSOLETE:
      element = EL_EM_KEY_3;
      break;

    case EL_EM_KEY_4_FILE_OBSOLETE:
      element = EL_EM_KEY_4;
      break;

    case EL_ENVELOPE_OBSOLETE:
      element = EL_ENVELOPE_1;
      break;

    case EL_SP_EMPTY:
      element = EL_EMPTY;
      break;

    default:
      if (element >= NUM_FILE_ELEMENTS)
      {
	Error(ERR_WARN, "invalid level element %d", element);

	element = EL_UNKNOWN;
      }
      break;
  }

  return element;
}

int getMappedElementByVersion(int element, int game_version)
{
  /* remap some elements due to certain game version */

  if (game_version <= VERSION_IDENT(2,2,0,0))
  {
    /* map game font elements */
    element = (element == EL_CHAR('[')  ? EL_CHAR_AUMLAUT :
	       element == EL_CHAR('\\') ? EL_CHAR_OUMLAUT :
	       element == EL_CHAR(']')  ? EL_CHAR_UUMLAUT :
	       element == EL_CHAR('^')  ? EL_CHAR_COPYRIGHT : element);
  }

  if (game_version < VERSION_IDENT(3,0,0,0))
  {
    /* map Supaplex gravity tube elements */
    element = (element == EL_SP_GRAVITY_PORT_LEFT  ? EL_SP_PORT_LEFT  :
	       element == EL_SP_GRAVITY_PORT_RIGHT ? EL_SP_PORT_RIGHT :
	       element == EL_SP_GRAVITY_PORT_UP    ? EL_SP_PORT_UP    :
	       element == EL_SP_GRAVITY_PORT_DOWN  ? EL_SP_PORT_DOWN  :
	       element);
  }

  return element;
}

static int LoadLevel_VERS(File *file, int chunk_size, struct LevelInfo *level)
{
  level->file_version = getFileVersion(file);
  level->game_version = getFileVersion(file);

  return chunk_size;
}

static int LoadLevel_DATE(File *file, int chunk_size, struct LevelInfo *level)
{
  level->creation_date.year  = getFile16BitBE(file);
  level->creation_date.month = getFile8Bit(file);
  level->creation_date.day   = getFile8Bit(file);

  level->creation_date.src   = DATE_SRC_LEVELFILE;

  return chunk_size;
}

static int LoadLevel_HEAD(File *file, int chunk_size, struct LevelInfo *level)
{
  int initial_player_stepsize;
  int initial_player_gravity;
  int i, x, y;

  level->fieldx = getFile8Bit(file);
  level->fieldy = getFile8Bit(file);

  level->time		= getFile16BitBE(file);
  level->gems_needed	= getFile16BitBE(file);

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = getFile8Bit(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = getFile8Bit(file);

  level->num_yamyam_contents = STD_ELEMENT_CONTENTS;
  for (i = 0; i < STD_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i].e[x][y] = getMappedElement(getFile8Bit(file));

  level->amoeba_speed		= getFile8Bit(file);
  level->time_magic_wall	= getFile8Bit(file);
  level->time_wheel		= getFile8Bit(file);
  level->amoeba_content		= getMappedElement(getFile8Bit(file));

  initial_player_stepsize	= (getFile8Bit(file) == 1 ? STEPSIZE_FAST :
				   STEPSIZE_NORMAL);

  for (i = 0; i < MAX_PLAYERS; i++)
    level->initial_player_stepsize[i] = initial_player_stepsize;

  initial_player_gravity	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  for (i = 0; i < MAX_PLAYERS; i++)
    level->initial_player_gravity[i] = initial_player_gravity;

  level->encoding_16bit_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->em_slippery_gems	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->use_custom_template	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->block_last_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->sp_block_last_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->can_move_into_acid_bits = getFile32BitBE(file);
  level->dont_collide_with_bits = getFile8Bit(file);

  level->use_spring_bug		= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->use_step_counter	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->instant_relocation	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->can_pass_to_walkable	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->grow_into_diggable	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  level->game_engine_type	= getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_HEAD_UNUSED);

  return chunk_size;
}

static int LoadLevel_NAME(File *file, int chunk_size, struct LevelInfo *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = getFile8Bit(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  return chunk_size;
}

static int LoadLevel_AUTH(File *file, int chunk_size, struct LevelInfo *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = getFile8Bit(file);
  level->author[MAX_LEVEL_AUTHOR_LEN] = 0;

  return chunk_size;
}

static int LoadLevel_BODY(File *file, int chunk_size, struct LevelInfo *level)
{
  int x, y;
  int chunk_size_expected = level->fieldx * level->fieldy;

  /* Note: "chunk_size" was wrong before version 2.0 when elements are
     stored with 16-bit encoding (and should be twice as big then).
     Even worse, playfield data was stored 16-bit when only yamyam content
     contained 16-bit elements and vice versa. */

  if (level->encoding_16bit_field && level->file_version >= FILE_VERSION_2_0)
    chunk_size_expected *= 2;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      level->field[x][y] =
	getMappedElement(level->encoding_16bit_field ? getFile16BitBE(file) :
			 getFile8Bit(file));
  return chunk_size;
}

static int LoadLevel_CONT(File *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int header_size = 4;
  int content_size = MAX_ELEMENT_CONTENTS * 3 * 3;
  int chunk_size_expected = header_size + content_size;

  /* Note: "chunk_size" was wrong before version 2.0 when elements are
     stored with 16-bit encoding (and should be twice as big then).
     Even worse, playfield data was stored 16-bit when only yamyam content
     contained 16-bit elements and vice versa. */

  if (level->encoding_16bit_field && level->file_version >= FILE_VERSION_2_0)
    chunk_size_expected += content_size;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  getFile8Bit(file);
  level->num_yamyam_contents = getFile8Bit(file);
  getFile8Bit(file);
  getFile8Bit(file);

  /* correct invalid number of content fields -- should never happen */
  if (level->num_yamyam_contents < 1 ||
      level->num_yamyam_contents > MAX_ELEMENT_CONTENTS)
    level->num_yamyam_contents = STD_ELEMENT_CONTENTS;

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i].e[x][y] =
	  getMappedElement(level->encoding_16bit_field ?
			   getFile16BitBE(file) : getFile8Bit(file));
  return chunk_size;
}

static int LoadLevel_CNT2(File *file, int chunk_size, struct LevelInfo *level)
{
  int i, x, y;
  int element;
  int num_contents;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  element = getMappedElement(getFile16BitBE(file));
  num_contents = getFile8Bit(file);

  getFile8Bit(file);	/* content x size (unused) */
  getFile8Bit(file);	/* content y size (unused) */

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	content_array[i][x][y] = getMappedElement(getFile16BitBE(file));

  /* correct invalid number of content fields -- should never happen */
  if (num_contents < 1 || num_contents > MAX_ELEMENT_CONTENTS)
    num_contents = STD_ELEMENT_CONTENTS;

  if (element == EL_YAMYAM)
  {
    level->num_yamyam_contents = num_contents;

    for (i = 0; i < num_contents; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  level->yamyam_content[i].e[x][y] = content_array[i][x][y];
  }
  else if (element == EL_BD_AMOEBA)
  {
    level->amoeba_content = content_array[0][0][0];
  }
  else
  {
    Error(ERR_WARN, "cannot load content for element '%d'", element);
  }

  return chunk_size;
}

static int LoadLevel_CNT3(File *file, int chunk_size, struct LevelInfo *level)
{
  int i;
  int element;
  int envelope_nr;
  int envelope_len;
  int chunk_size_expected;

  element = getMappedElement(getFile16BitBE(file));
  if (!IS_ENVELOPE(element))
    element = EL_ENVELOPE_1;

  envelope_nr = element - EL_ENVELOPE_1;

  envelope_len = getFile16BitBE(file);

  level->envelope[envelope_nr].xsize = getFile8Bit(file);
  level->envelope[envelope_nr].ysize = getFile8Bit(file);

  ReadUnusedBytesFromFile(file, LEVEL_CHUNK_CNT3_UNUSED);

  chunk_size_expected = LEVEL_CHUNK_CNT3_SIZE(envelope_len);
  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - LEVEL_CHUNK_CNT3_HEADER);
    return chunk_size_expected;
  }

  for (i = 0; i < envelope_len; i++)
    level->envelope[envelope_nr].text[i] = getFile8Bit(file);

  return chunk_size;
}

static int LoadLevel_CUS1(File *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = 2 + num_changed_custom_elements * 6;
  int i;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getMappedElement(getFile16BitBE(file));
    int properties = getFile32BitBE(file);

    if (IS_CUSTOM_ELEMENT(element))
      element_info[element].properties[EP_BITFIELD_BASE_NR] = properties;
    else
      Error(ERR_WARN, "invalid custom element number %d", element);

    /* older game versions that wrote level files with CUS1 chunks used
       different default push delay values (not yet stored in level file) */
    element_info[element].push_delay_fixed = 2;
    element_info[element].push_delay_random = 8;
  }

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_CUS2(File *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = 2 + num_changed_custom_elements * 4;
  int i;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getMappedElement(getFile16BitBE(file));
    int custom_target_element = getMappedElement(getFile16BitBE(file));

    if (IS_CUSTOM_ELEMENT(element))
      element_info[element].change->target_element = custom_target_element;
    else
      Error(ERR_WARN, "invalid custom element number %d", element);
  }

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_CUS3(File *file, int chunk_size, struct LevelInfo *level)
{
  int num_changed_custom_elements = getFile16BitBE(file);
  int chunk_size_expected = LEVEL_CHUNK_CUS3_SIZE(num_changed_custom_elements);
  int i, j, x, y;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size_expected;
  }

  for (i = 0; i < num_changed_custom_elements; i++)
  {
    int element = getMappedElement(getFile16BitBE(file));
    struct ElementInfo *ei = &element_info[element];
    unsigned int event_bits;

    if (!IS_CUSTOM_ELEMENT(element))
    {
      Error(ERR_WARN, "invalid custom element number %d", element);

      element = EL_INTERNAL_DUMMY;
    }

    for (j = 0; j < MAX_ELEMENT_NAME_LEN; j++)
      ei->description[j] = getFile8Bit(file);
    ei->description[MAX_ELEMENT_NAME_LEN] = 0;

    ei->properties[EP_BITFIELD_BASE_NR] = getFile32BitBE(file);

    /* some free bytes for future properties and padding */
    ReadUnusedBytesFromFile(file, 7);

    ei->use_gfx_element = getFile8Bit(file);
    ei->gfx_element_initial = getMappedElement(getFile16BitBE(file));

    ei->collect_score_initial = getFile8Bit(file);
    ei->collect_count_initial = getFile8Bit(file);

    ei->push_delay_fixed = getFile16BitBE(file);
    ei->push_delay_random = getFile16BitBE(file);
    ei->move_delay_fixed = getFile16BitBE(file);
    ei->move_delay_random = getFile16BitBE(file);

    ei->move_pattern = getFile16BitBE(file);
    ei->move_direction_initial = getFile8Bit(file);
    ei->move_stepsize = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	ei->content.e[x][y] = getMappedElement(getFile16BitBE(file));

    event_bits = getFile32BitBE(file);
    for (j = 0; j < NUM_CHANGE_EVENTS; j++)
      if (event_bits & (1 << j))
	ei->change->has_event[j] = TRUE;

    ei->change->target_element = getMappedElement(getFile16BitBE(file));

    ei->change->delay_fixed = getFile16BitBE(file);
    ei->change->delay_random = getFile16BitBE(file);
    ei->change->delay_frames = getFile16BitBE(file);

    ei->change->initial_trigger_element= getMappedElement(getFile16BitBE(file));

    ei->change->explode = getFile8Bit(file);
    ei->change->use_target_content = getFile8Bit(file);
    ei->change->only_if_complete = getFile8Bit(file);
    ei->change->use_random_replace = getFile8Bit(file);

    ei->change->random_percentage = getFile8Bit(file);
    ei->change->replace_when = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	ei->change->target_content.e[x][y] =
	  getMappedElement(getFile16BitBE(file));

    ei->slippery_type = getFile8Bit(file);

    /* some free bytes for future properties and padding */
    ReadUnusedBytesFromFile(file, LEVEL_CPART_CUS3_UNUSED);

    /* mark that this custom element has been modified */
    ei->modified_settings = TRUE;
  }

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_CUS4(File *file, int chunk_size, struct LevelInfo *level)
{
  struct ElementInfo *ei;
  int chunk_size_expected;
  int element;
  int i, j, x, y;

  /* ---------- custom element base property values (96 bytes) ------------- */

  element = getMappedElement(getFile16BitBE(file));

  if (!IS_CUSTOM_ELEMENT(element))
  {
    Error(ERR_WARN, "invalid custom element number %d", element);

    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size;
  }

  ei = &element_info[element];

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    ei->description[i] = getFile8Bit(file);
  ei->description[MAX_ELEMENT_NAME_LEN] = 0;

  ei->properties[EP_BITFIELD_BASE_NR] = getFile32BitBE(file);

  ReadUnusedBytesFromFile(file, 4);	/* reserved for more base properties */

  ei->num_change_pages = getFile8Bit(file);

  chunk_size_expected = LEVEL_CHUNK_CUS4_SIZE(ei->num_change_pages);
  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size - 43);
    return chunk_size_expected;
  }

  ei->ce_value_fixed_initial = getFile16BitBE(file);
  ei->ce_value_random_initial = getFile16BitBE(file);
  ei->use_last_ce_value = getFile8Bit(file);

  ei->use_gfx_element = getFile8Bit(file);
  ei->gfx_element_initial = getMappedElement(getFile16BitBE(file));

  ei->collect_score_initial = getFile8Bit(file);
  ei->collect_count_initial = getFile8Bit(file);

  ei->drop_delay_fixed = getFile8Bit(file);
  ei->push_delay_fixed = getFile8Bit(file);
  ei->drop_delay_random = getFile8Bit(file);
  ei->push_delay_random = getFile8Bit(file);
  ei->move_delay_fixed = getFile16BitBE(file);
  ei->move_delay_random = getFile16BitBE(file);

  /* bits 0 - 15 of "move_pattern" ... */
  ei->move_pattern = getFile16BitBE(file);
  ei->move_direction_initial = getFile8Bit(file);
  ei->move_stepsize = getFile8Bit(file);

  ei->slippery_type = getFile8Bit(file);

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      ei->content.e[x][y] = getMappedElement(getFile16BitBE(file));

  ei->move_enter_element = getMappedElement(getFile16BitBE(file));
  ei->move_leave_element = getMappedElement(getFile16BitBE(file));
  ei->move_leave_type = getFile8Bit(file);

  /* ... bits 16 - 31 of "move_pattern" (not nice, but downward compatible) */
  ei->move_pattern |= (getFile16BitBE(file) << 16);

  ei->access_direction = getFile8Bit(file);

  ei->explosion_delay = getFile8Bit(file);
  ei->ignition_delay = getFile8Bit(file);
  ei->explosion_type = getFile8Bit(file);

  /* some free bytes for future custom property values and padding */
  ReadUnusedBytesFromFile(file, 1);

  /* ---------- change page property values (48 bytes) --------------------- */

  setElementChangePages(ei, ei->num_change_pages);

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];
    unsigned int event_bits;

    /* always start with reliable default values */
    setElementChangeInfoToDefaults(change);

    /* bits 0 - 31 of "has_event[]" ... */
    event_bits = getFile32BitBE(file);
    for (j = 0; j < MIN(NUM_CHANGE_EVENTS, 32); j++)
      if (event_bits & (1 << j))
	change->has_event[j] = TRUE;

    change->target_element = getMappedElement(getFile16BitBE(file));

    change->delay_fixed = getFile16BitBE(file);
    change->delay_random = getFile16BitBE(file);
    change->delay_frames = getFile16BitBE(file);

    change->initial_trigger_element = getMappedElement(getFile16BitBE(file));

    change->explode = getFile8Bit(file);
    change->use_target_content = getFile8Bit(file);
    change->only_if_complete = getFile8Bit(file);
    change->use_random_replace = getFile8Bit(file);

    change->random_percentage = getFile8Bit(file);
    change->replace_when = getFile8Bit(file);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	change->target_content.e[x][y]= getMappedElement(getFile16BitBE(file));

    change->can_change = getFile8Bit(file);

    change->trigger_side = getFile8Bit(file);

    change->trigger_player = getFile8Bit(file);
    change->trigger_page = getFile8Bit(file);

    change->trigger_page = (change->trigger_page == CH_PAGE_ANY_FILE ?
			    CH_PAGE_ANY : (1 << change->trigger_page));

    change->has_action = getFile8Bit(file);
    change->action_type = getFile8Bit(file);
    change->action_mode = getFile8Bit(file);
    change->action_arg = getFile16BitBE(file);

    /* ... bits 32 - 39 of "has_event[]" (not nice, but downward compatible) */
    event_bits = getFile8Bit(file);
    for (j = 32; j < NUM_CHANGE_EVENTS; j++)
      if (event_bits & (1 << (j - 32)))
	change->has_event[j] = TRUE;
  }

  /* mark this custom element as modified */
  ei->modified_settings = TRUE;

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_GRP1(File *file, int chunk_size, struct LevelInfo *level)
{
  struct ElementInfo *ei;
  struct ElementGroupInfo *group;
  int element;
  int i;

  element = getMappedElement(getFile16BitBE(file));

  if (!IS_GROUP_ELEMENT(element))
  {
    Error(ERR_WARN, "invalid group element number %d", element);

    ReadUnusedBytesFromFile(file, chunk_size - 2);
    return chunk_size;
  }

  ei = &element_info[element];

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    ei->description[i] = getFile8Bit(file);
  ei->description[MAX_ELEMENT_NAME_LEN] = 0;

  group = element_info[element].group;

  group->num_elements = getFile8Bit(file);

  ei->use_gfx_element = getFile8Bit(file);
  ei->gfx_element_initial = getMappedElement(getFile16BitBE(file));

  group->choice_mode = getFile8Bit(file);

  /* some free bytes for future values and padding */
  ReadUnusedBytesFromFile(file, 3);

  for (i = 0; i < MAX_ELEMENTS_IN_GROUP; i++)
    group->element[i] = getMappedElement(getFile16BitBE(file));

  /* mark this group element as modified */
  element_info[element].modified_settings = TRUE;

  level->file_has_custom_elements = TRUE;

  return chunk_size;
}

static int LoadLevel_MicroChunk(File *file, struct LevelFileConfigInfo *conf,
				int element, int real_element)
{
  int micro_chunk_size = 0;
  int conf_type = getFile8Bit(file);
  int byte_mask = conf_type & CONF_MASK_BYTES;
  boolean element_found = FALSE;
  int i;

  micro_chunk_size += 1;

  if (byte_mask == CONF_MASK_MULTI_BYTES)
  {
    int num_bytes = getFile16BitBE(file);
    byte *buffer = checked_malloc(num_bytes);

    ReadBytesFromFile(file, buffer, num_bytes);

    for (i = 0; conf[i].data_type != -1; i++)
    {
      if (conf[i].element == element &&
	  conf[i].conf_type == conf_type)
      {
	int data_type = conf[i].data_type;
	int num_entities = num_bytes / CONF_ENTITY_NUM_BYTES(data_type);
	int max_num_entities = conf[i].max_num_entities;

	if (num_entities > max_num_entities)
	{
	  Error(ERR_WARN,
		"truncating number of entities for element %d from %d to %d",
		element, num_entities, max_num_entities);

	  num_entities = max_num_entities;
	}

	if (num_entities == 0 && (data_type == TYPE_ELEMENT_LIST ||
				  data_type == TYPE_CONTENT_LIST))
	{
	  /* for element and content lists, zero entities are not allowed */
	  Error(ERR_WARN, "found empty list of entities for element %d",
		element);

	  /* do not set "num_entities" here to prevent reading behind buffer */

	  *(int *)(conf[i].num_entities) = 1;	/* at least one is required */
	}
	else
	{
	  *(int *)(conf[i].num_entities) = num_entities;
	}

	element_found = TRUE;

	if (data_type == TYPE_STRING)
	{
	  char *string = (char *)(conf[i].value);
	  int j;

	  for (j = 0; j < max_num_entities; j++)
	    string[j] = (j < num_entities ? buffer[j] : '\0');
	}
	else if (data_type == TYPE_ELEMENT_LIST)
	{
	  int *element_array = (int *)(conf[i].value);
	  int j;

	  for (j = 0; j < num_entities; j++)
	    element_array[j] =
	      getMappedElement(CONF_ELEMENTS_ELEMENT(buffer, j));
	}
	else if (data_type == TYPE_CONTENT_LIST)
	{
	  struct Content *content= (struct Content *)(conf[i].value);
	  int c, x, y;

	  for (c = 0; c < num_entities; c++)
	    for (y = 0; y < 3; y++)
	      for (x = 0; x < 3; x++)
		content[c].e[x][y] =
		  getMappedElement(CONF_CONTENTS_ELEMENT(buffer, c, x, y));
	}
	else
	  element_found = FALSE;

	break;
      }
    }

    checked_free(buffer);

    micro_chunk_size += 2 + num_bytes;
  }
  else		/* constant size configuration data (1, 2 or 4 bytes) */
  {
    int value = (byte_mask == CONF_MASK_1_BYTE ? getFile8Bit   (file) :
		 byte_mask == CONF_MASK_2_BYTE ? getFile16BitBE(file) :
		 byte_mask == CONF_MASK_4_BYTE ? getFile32BitBE(file) : 0);

    for (i = 0; conf[i].data_type != -1; i++)
    {
      if (conf[i].element == element &&
	  conf[i].conf_type == conf_type)
      {
	int data_type = conf[i].data_type;

	if (data_type == TYPE_ELEMENT)
	  value = getMappedElement(value);

	if (data_type == TYPE_BOOLEAN)
	  *(boolean *)(conf[i].value) = value;
	else
	  *(int *)    (conf[i].value) = value;

	element_found = TRUE;

	break;
      }
    }

    micro_chunk_size += CONF_VALUE_NUM_BYTES(byte_mask);
  }

  if (!element_found)
  {
    char *error_conf_chunk_bytes =
      (byte_mask == CONF_MASK_1_BYTE ? "CONF_VALUE_8_BIT" :
       byte_mask == CONF_MASK_2_BYTE ? "CONF_VALUE_16_BIT" :
       byte_mask == CONF_MASK_4_BYTE ? "CONF_VALUE_32_BIT" :"CONF_VALUE_BYTES");
    int error_conf_chunk_token = conf_type & CONF_MASK_TOKEN;
    int error_element = real_element;

    Error(ERR_WARN, "cannot load micro chunk '%s(%d)' value for element %d ['%s']",
	  error_conf_chunk_bytes, error_conf_chunk_token,
	  error_element, EL_NAME(error_element));
  }

  return micro_chunk_size;
}

static int LoadLevel_INFO(File *file, int chunk_size, struct LevelInfo *level)
{
  int real_chunk_size = 0;

  li = *level;		/* copy level data into temporary buffer */

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_INFO, -1, -1);

    if (real_chunk_size >= chunk_size)
      break;
  }

  *level = li;		/* copy temporary buffer back to level data */

  return real_chunk_size;
}

static int LoadLevel_CONF(File *file, int chunk_size, struct LevelInfo *level)
{
  int real_chunk_size = 0;

  li = *level;		/* copy level data into temporary buffer */

  while (!checkEndOfFile(file))
  {
    int element = getMappedElement(getFile16BitBE(file));

    real_chunk_size += 2;
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_CONF,
					    element, element);
    if (real_chunk_size >= chunk_size)
      break;
  }

  *level = li;		/* copy temporary buffer back to level data */

  return real_chunk_size;
}

static int LoadLevel_ELEM(File *file, int chunk_size, struct LevelInfo *level)
{
  int real_chunk_size = 0;

  li = *level;		/* copy level data into temporary buffer */

  while (!checkEndOfFile(file))
  {
    int element = getMappedElement(getFile16BitBE(file));

    real_chunk_size += 2;
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_ELEM,
					    element, element);
    if (real_chunk_size >= chunk_size)
      break;
  }

  *level = li;		/* copy temporary buffer back to level data */

  return real_chunk_size;
}

static int LoadLevel_NOTE(File *file, int chunk_size, struct LevelInfo *level)
{
  int element = getMappedElement(getFile16BitBE(file));
  int envelope_nr = element - EL_ENVELOPE_1;
  int real_chunk_size = 2;

  xx_envelope = level->envelope[envelope_nr];	/* copy into temporary buffer */

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_NOTE,
					    -1, element);

    if (real_chunk_size >= chunk_size)
      break;
  }

  level->envelope[envelope_nr] = xx_envelope;	/* copy from temporary buffer */

  return real_chunk_size;
}

static int LoadLevel_CUSX(File *file, int chunk_size, struct LevelInfo *level)
{
  int element = getMappedElement(getFile16BitBE(file));
  int real_chunk_size = 2;
  struct ElementInfo *ei = &element_info[element];
  int i;

  xx_ei = *ei;		/* copy element data into temporary buffer */

  xx_ei.num_change_pages = -1;

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_CUSX_base,
					    -1, element);
    if (xx_ei.num_change_pages != -1)
      break;

    if (real_chunk_size >= chunk_size)
      break;
  }

  *ei = xx_ei;

  if (ei->num_change_pages == -1)
  {
    Error(ERR_WARN, "LoadLevel_CUSX(): missing 'num_change_pages' for '%s'",
	  EL_NAME(element));

    ei->num_change_pages = 1;

    setElementChangePages(ei, 1);
    setElementChangeInfoToDefaults(ei->change);

    return real_chunk_size;
  }

  /* initialize number of change pages stored for this custom element */
  setElementChangePages(ei, ei->num_change_pages);
  for (i = 0; i < ei->num_change_pages; i++)
    setElementChangeInfoToDefaults(&ei->change_page[i]);

  /* start with reading properties for the first change page */
  xx_current_change_page = 0;

  while (!checkEndOfFile(file))
  {
    struct ElementChangeInfo *change = &ei->change_page[xx_current_change_page];

    xx_change = *change;	/* copy change data into temporary buffer */

    resetEventBits();		/* reset bits; change page might have changed */

    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_CUSX_change,
					    -1, element);

    *change = xx_change;

    setEventFlagsFromEventBits(change);

    if (real_chunk_size >= chunk_size)
      break;
  }

  level->file_has_custom_elements = TRUE;

  return real_chunk_size;
}

static int LoadLevel_GRPX(File *file, int chunk_size, struct LevelInfo *level)
{
  int element = getMappedElement(getFile16BitBE(file));
  int real_chunk_size = 2;
  struct ElementInfo *ei = &element_info[element];
  struct ElementGroupInfo *group = ei->group;

  xx_ei = *ei;		/* copy element data into temporary buffer */
  xx_group = *group;	/* copy group data into temporary buffer */

  while (!checkEndOfFile(file))
  {
    real_chunk_size += LoadLevel_MicroChunk(file, chunk_config_GRPX,
					    -1, element);

    if (real_chunk_size >= chunk_size)
      break;
  }

  *ei = xx_ei;
  *group = xx_group;

  level->file_has_custom_elements = TRUE;

  return real_chunk_size;
}

static void LoadLevelFromFileInfo_RND(struct LevelInfo *level,
				      struct LevelFileInfo *level_file_info,
				      boolean level_info_only)
{
  char *filename = level_file_info->filename;
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  File *file;

  if (!(file = openFile(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;
    level->no_level_file = TRUE;

    if (level_info_only)
      return;

    Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    if (!setup.editor.use_template_for_new_levels)
      return;

    /* if level file not found, try to initialize level data from template */
    filename = getGlobalLevelTemplateFilename();

    if (!(file = openFile(filename, MODE_READ)))
      return;

    /* default: for empty levels, use level template for custom elements */
    level->use_custom_template = TRUE;

    level->no_valid_file = FALSE;
  }

  getFileChunkBE(file, chunk_name, NULL);
  if (strEqual(chunk_name, "RND1"))
  {
    getFile32BitBE(file);		/* not used */

    getFileChunkBE(file, chunk_name, NULL);
    if (!strEqual(chunk_name, "CAVE"))
    {
      level->no_valid_file = TRUE;

      Error(ERR_WARN, "unknown format of level file '%s'", filename);

      closeFile(file);

      return;
    }
  }
  else	/* check for pre-2.0 file format with cookie string */
  {
    strcpy(cookie, chunk_name);
    if (getStringFromFile(file, &cookie[4], MAX_LINE_LEN - 4) == NULL)
      cookie[4] = '\0';
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, LEVEL_COOKIE_TMPL))
    {
      level->no_valid_file = TRUE;

      Error(ERR_WARN, "unknown format of level file '%s'", filename);

      closeFile(file);

      return;
    }

    if ((level->file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      level->no_valid_file = TRUE;

      Error(ERR_WARN, "unsupported version of level file '%s'", filename);

      closeFile(file);

      return;
    }

    /* pre-2.0 level files have no game version, so use file version here */
    level->game_version = level->file_version;
  }

  if (level->file_version < FILE_VERSION_1_2)
  {
    /* level files from versions before 1.2.0 without chunk structure */
    LoadLevel_HEAD(file, LEVEL_CHUNK_HEAD_SIZE,         level);
    LoadLevel_BODY(file, level->fieldx * level->fieldy, level);
  }
  else
  {
    static struct
    {
      char *name;
      int size;
      int (*loader)(File *, int, struct LevelInfo *);
    }
    chunk_info[] =
    {
      { "VERS", LEVEL_CHUNK_VERS_SIZE,	LoadLevel_VERS },
      { "DATE", LEVEL_CHUNK_DATE_SIZE,	LoadLevel_DATE },
      { "HEAD", LEVEL_CHUNK_HEAD_SIZE,	LoadLevel_HEAD },
      { "NAME", LEVEL_CHUNK_NAME_SIZE,	LoadLevel_NAME },
      { "AUTH", LEVEL_CHUNK_AUTH_SIZE,	LoadLevel_AUTH },
      { "INFO", -1,			LoadLevel_INFO },
      { "BODY", -1,			LoadLevel_BODY },
      { "CONT", -1,			LoadLevel_CONT },
      { "CNT2", LEVEL_CHUNK_CNT2_SIZE,	LoadLevel_CNT2 },
      { "CNT3", -1,			LoadLevel_CNT3 },
      { "CUS1", -1,			LoadLevel_CUS1 },
      { "CUS2", -1,			LoadLevel_CUS2 },
      { "CUS3", -1,			LoadLevel_CUS3 },
      { "CUS4", -1,			LoadLevel_CUS4 },
      { "GRP1", -1,			LoadLevel_GRP1 },
      { "CONF", -1,			LoadLevel_CONF },
      { "ELEM", -1,			LoadLevel_ELEM },
      { "NOTE", -1,			LoadLevel_NOTE },
      { "CUSX", -1,			LoadLevel_CUSX },
      { "GRPX", -1,			LoadLevel_GRPX },

      {  NULL,  0,			NULL }
    };

    while (getFileChunkBE(file, chunk_name, &chunk_size))
    {
      int i = 0;

      while (chunk_info[i].name != NULL &&
	     !strEqual(chunk_name, chunk_info[i].name))
	i++;

      if (chunk_info[i].name == NULL)
      {
	Error(ERR_WARN, "unknown chunk '%s' in level file '%s'",
	      chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else if (chunk_info[i].size != -1 &&
	       chunk_info[i].size != chunk_size)
      {
	Error(ERR_WARN, "wrong size (%d) of chunk '%s' in level file '%s'",
	      chunk_size, chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else
      {
	/* call function to load this level chunk */
	int chunk_size_expected =
	  (chunk_info[i].loader)(file, chunk_size, level);

	/* the size of some chunks cannot be checked before reading other
	   chunks first (like "HEAD" and "BODY") that contain some header
	   information, so check them here */
	if (chunk_size_expected != chunk_size)
	{
	  Error(ERR_WARN, "wrong size (%d) of chunk '%s' in level file '%s'",
		chunk_size, chunk_name, filename);
	}
      }
    }
  }

  closeFile(file);
}


/* ------------------------------------------------------------------------- */
/* functions for loading EM level                                            */
/* ------------------------------------------------------------------------- */

void CopyNativeLevel_RND_to_EM(struct LevelInfo *level)
{
  static int ball_xy[8][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 2, 0 },
    { 0, 1 },
    { 2, 1 },
    { 0, 2 },
    { 1, 2 },
    { 2, 2 },
  };
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct LEVEL *lev = level_em->lev;
  struct PLAYER **ply = level_em->ply;
  int i, j, x, y;

  lev->width  = MIN(level->fieldx, EM_MAX_CAVE_WIDTH);
  lev->height = MIN(level->fieldy, EM_MAX_CAVE_HEIGHT);

  lev->time_seconds     = level->time;
  lev->required_initial = level->gems_needed;

  lev->emerald_score	= level->score[SC_EMERALD];
  lev->diamond_score	= level->score[SC_DIAMOND];
  lev->alien_score	= level->score[SC_ROBOT];
  lev->tank_score	= level->score[SC_SPACESHIP];
  lev->bug_score	= level->score[SC_BUG];
  lev->eater_score	= level->score[SC_YAMYAM];
  lev->nut_score	= level->score[SC_NUT];
  lev->dynamite_score	= level->score[SC_DYNAMITE];
  lev->key_score	= level->score[SC_KEY];
  lev->exit_score	= level->score[SC_TIME_BONUS];

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	lev->eater_array[i][y * 3 + x] =
	  map_element_RND_to_EM(level->yamyam_content[i].e[x][y]);

  lev->amoeba_time		= level->amoeba_speed;
  lev->wonderwall_time_initial	= level->time_magic_wall;
  lev->wheel_time		= level->time_wheel;

  lev->android_move_time	= level->android_move_time;
  lev->android_clone_time	= level->android_clone_time;
  lev->ball_random		= level->ball_random;
  lev->ball_state_initial	= level->ball_state_initial;
  lev->ball_time		= level->ball_time;
  lev->num_ball_arrays		= level->num_ball_contents;

  lev->lenses_score		= level->lenses_score;
  lev->magnify_score		= level->magnify_score;
  lev->slurp_score		= level->slurp_score;

  lev->lenses_time		= level->lenses_time;
  lev->magnify_time		= level->magnify_time;

  lev->wind_direction_initial =
    map_direction_RND_to_EM(level->wind_direction_initial);
  lev->wind_cnt_initial = (level->wind_direction_initial != MV_NONE ?
			   lev->wind_time : 0);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (j = 0; j < 8; j++)
      lev->ball_array[i][j] =
	map_element_RND_to_EM(level->
			      ball_content[i].e[ball_xy[j][0]][ball_xy[j][1]]);

  map_android_clone_elements_RND_to_EM(level);

  /* first fill the complete playfield with the default border element */
  for (y = 0; y < EM_MAX_CAVE_HEIGHT; y++)
    for (x = 0; x < EM_MAX_CAVE_WIDTH; x++)
      level_em->cave[x][y] = ZBORDER;

  if (BorderElement == EL_STEELWALL)
  {
    for (y = 0; y < lev->height + 2; y++)
      for (x = 0; x < lev->width + 2; x++)
	level_em->cave[x + 1][y + 1] = map_element_RND_to_EM(EL_STEELWALL);
  }

  /* then copy the real level contents from level file into the playfield */
  for (y = 0; y < lev->height; y++) for (x = 0; x < lev->width; x++)
  {
    int new_element = map_element_RND_to_EM(level->field[x][y]);
    int offset = (BorderElement == EL_STEELWALL ? 1 : 0);
    int xx = x + 1 + offset;
    int yy = y + 1 + offset;

    if (level->field[x][y] == EL_AMOEBA_DEAD)
      new_element = map_element_RND_to_EM(EL_AMOEBA_WET);

    level_em->cave[xx][yy] = new_element;
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i]->x_initial = 0;
    ply[i]->y_initial = 0;
  }

  /* initialize player positions and delete players from the playfield */
  for (y = 0; y < lev->height; y++) for (x = 0; x < lev->width; x++)
  {
    if (ELEM_IS_PLAYER(level->field[x][y]))
    {
      int player_nr = GET_PLAYER_NR(level->field[x][y]);
      int offset = (BorderElement == EL_STEELWALL ? 1 : 0);
      int xx = x + 1 + offset;
      int yy = y + 1 + offset;

      ply[player_nr]->x_initial = xx;
      ply[player_nr]->y_initial = yy;

      level_em->cave[xx][yy] = map_element_RND_to_EM(EL_EMPTY);
    }
  }

  if (BorderElement == EL_STEELWALL)
  {
    lev->width  += 2;
    lev->height += 2;
  }
}

void CopyNativeLevel_EM_to_RND(struct LevelInfo *level)
{
  static int ball_xy[8][2] =
  {
    { 0, 0 },
    { 1, 0 },
    { 2, 0 },
    { 0, 1 },
    { 2, 1 },
    { 0, 2 },
    { 1, 2 },
    { 2, 2 },
  };
  struct LevelInfo_EM *level_em = level->native_em_level;
  struct LEVEL *lev = level_em->lev;
  struct PLAYER **ply = level_em->ply;
  int i, j, x, y;

  level->fieldx = MIN(lev->width,  MAX_LEV_FIELDX);
  level->fieldy = MIN(lev->height, MAX_LEV_FIELDY);

  level->time        = lev->time_seconds;
  level->gems_needed = lev->required_initial;

  sprintf(level->name, "Level %d", level->file_info.nr);

  level->score[SC_EMERALD]	= lev->emerald_score;
  level->score[SC_DIAMOND]	= lev->diamond_score;
  level->score[SC_ROBOT]	= lev->alien_score;
  level->score[SC_SPACESHIP]	= lev->tank_score;
  level->score[SC_BUG]		= lev->bug_score;
  level->score[SC_YAMYAM]	= lev->eater_score;
  level->score[SC_NUT]		= lev->nut_score;
  level->score[SC_DYNAMITE]	= lev->dynamite_score;
  level->score[SC_KEY]		= lev->key_score;
  level->score[SC_TIME_BONUS]	= lev->exit_score;

  level->num_yamyam_contents = MAX_ELEMENT_CONTENTS;

  for (i = 0; i < level->num_yamyam_contents; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	level->yamyam_content[i].e[x][y] =
	  map_element_EM_to_RND(lev->eater_array[i][y * 3 + x]);

  level->amoeba_speed		= lev->amoeba_time;
  level->time_magic_wall	= lev->wonderwall_time_initial;
  level->time_wheel		= lev->wheel_time;

  level->android_move_time	= lev->android_move_time;
  level->android_clone_time	= lev->android_clone_time;
  level->ball_random		= lev->ball_random;
  level->ball_state_initial	= lev->ball_state_initial;
  level->ball_time		= lev->ball_time;
  level->num_ball_contents	= lev->num_ball_arrays;

  level->lenses_score		= lev->lenses_score;
  level->magnify_score		= lev->magnify_score;
  level->slurp_score		= lev->slurp_score;

  level->lenses_time		= lev->lenses_time;
  level->magnify_time		= lev->magnify_time;

  level->wind_direction_initial =
    map_direction_EM_to_RND(lev->wind_direction_initial);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (j = 0; j < 8; j++)
      level->ball_content[i].e[ball_xy[j][0]][ball_xy[j][1]] =
	map_element_EM_to_RND(lev->ball_array[i][j]);

  map_android_clone_elements_EM_to_RND(level);

  /* convert the playfield (some elements need special treatment) */
  for (y = 0; y < level->fieldy; y++) for (x = 0; x < level->fieldx; x++)
  {
    int new_element = map_element_EM_to_RND(level_em->cave[x + 1][y + 1]);

    if (new_element == EL_AMOEBA_WET && level->amoeba_speed == 0)
      new_element = EL_AMOEBA_DEAD;

    level->field[x][y] = new_element;
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    /* in case of all players set to the same field, use the first player */
    int nr = MAX_PLAYERS - i - 1;
    int jx = ply[nr]->x_initial - 1;
    int jy = ply[nr]->y_initial - 1;

    if (jx != -1 && jy != -1)
      level->field[jx][jy] = EL_PLAYER_1 + nr;
  }
}


/* ------------------------------------------------------------------------- */
/* functions for loading SP level                                            */
/* ------------------------------------------------------------------------- */

void CopyNativeLevel_RND_to_SP(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  LevelInfoType *header = &level_sp->header;
  int i, x, y;

  level_sp->width  = level->fieldx;
  level_sp->height = level->fieldy;

  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      level_sp->playfield[x][y] = map_element_RND_to_SP(level->field[x][y]);

  header->InitialGravity = (level->initial_player_gravity[0] ? 1 : 0);

  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    header->LevelTitle[i] = level->name[i];
  /* !!! NO STRING TERMINATION IN SUPAPLEX VB CODE YET -- FIX THIS !!! */

  header->InfotronsNeeded = level->gems_needed;

  header->SpecialPortCount = 0;

  for (x = 0; x < level->fieldx; x++) for (y = 0; y < level->fieldy; y++)
  {
    boolean gravity_port_found = FALSE;
    boolean gravity_port_valid = FALSE;
    int gravity_port_flag;
    int gravity_port_base_element;
    int element = level->field[x][y];

    if (element >= EL_SP_GRAVITY_ON_PORT_RIGHT &&
	element <= EL_SP_GRAVITY_ON_PORT_UP)
    {
      gravity_port_found = TRUE;
      gravity_port_valid = TRUE;
      gravity_port_flag = 1;
      gravity_port_base_element = EL_SP_GRAVITY_ON_PORT_RIGHT;
    }
    else if (element >= EL_SP_GRAVITY_OFF_PORT_RIGHT &&
	     element <= EL_SP_GRAVITY_OFF_PORT_UP)
    {
      gravity_port_found = TRUE;
      gravity_port_valid = TRUE;
      gravity_port_flag = 0;
      gravity_port_base_element = EL_SP_GRAVITY_OFF_PORT_RIGHT;
    }
    else if (element >= EL_SP_GRAVITY_PORT_RIGHT &&
	     element <= EL_SP_GRAVITY_PORT_UP)
    {
      /* change R'n'D style gravity inverting special port to normal port
	 (there are no gravity inverting ports in native Supaplex engine) */

      gravity_port_found = TRUE;
      gravity_port_valid = FALSE;
      gravity_port_base_element = EL_SP_GRAVITY_PORT_RIGHT;
    }

    if (gravity_port_found)
    {
      if (gravity_port_valid &&
	  header->SpecialPortCount < SP_MAX_SPECIAL_PORTS)
      {
	SpecialPortType *port = &header->SpecialPort[header->SpecialPortCount];

	port->PortLocation = (y * level->fieldx + x) * 2;
	port->Gravity = gravity_port_flag;

	element += EL_SP_GRAVITY_PORT_RIGHT - gravity_port_base_element;

	header->SpecialPortCount++;
      }
      else
      {
	/* change special gravity port to normal port */

	element += EL_SP_PORT_RIGHT - gravity_port_base_element;
      }

      level_sp->playfield[x][y] = element - EL_SP_START;
    }
  }
}

void CopyNativeLevel_SP_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  LevelInfoType *header = &level_sp->header;
  boolean num_invalid_elements = 0;
  int i, j, x, y;

  level->fieldx = level_sp->width;
  level->fieldy = level_sp->height;

  for (x = 0; x < level->fieldx; x++)
  {
    for (y = 0; y < level->fieldy; y++)
    {
      int element_old = level_sp->playfield[x][y];
      int element_new = getMappedElement(map_element_SP_to_RND(element_old));

      if (element_new == EL_UNKNOWN)
      {
	num_invalid_elements++;

	Error(ERR_DEBUG, "invalid element %d at position %d, %d",
	      element_old, x, y);
      }

      level->field[x][y] = element_new;
    }
  }

  if (num_invalid_elements > 0)
    Error(ERR_WARN, "found %d invalid elements%s", num_invalid_elements,
	  (!options.debug ? " (use '--debug' for more details)" : ""));

  for (i = 0; i < MAX_PLAYERS; i++)
    level->initial_player_gravity[i] =
      (header->InitialGravity == 1 ? TRUE : FALSE);

  /* skip leading spaces */
  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    if (header->LevelTitle[i] != ' ')
      break;

  /* copy level title */
  for (j = 0; i < SP_LEVEL_NAME_LEN; i++, j++)
    level->name[j] = header->LevelTitle[i];
  level->name[j] = '\0';

  /* cut trailing spaces */
  for (; j > 0; j--)
    if (level->name[j - 1] == ' ' && level->name[j] == '\0')
      level->name[j - 1] = '\0';

  level->gems_needed = header->InfotronsNeeded;

  for (i = 0; i < header->SpecialPortCount; i++)
  {
    SpecialPortType *port = &header->SpecialPort[i];
    int port_location = port->PortLocation;
    int gravity = port->Gravity;
    int port_x, port_y, port_element;

    port_x = (port_location / 2) % level->fieldx;
    port_y = (port_location / 2) / level->fieldx;

    if (port_x < 0 || port_x >= level->fieldx ||
	port_y < 0 || port_y >= level->fieldy)
    {
      Error(ERR_WARN, "special port position (%d, %d) out of bounds",
	    port_x, port_y);

      continue;
    }

    port_element = level->field[port_x][port_y];

    if (port_element < EL_SP_GRAVITY_PORT_RIGHT ||
	port_element > EL_SP_GRAVITY_PORT_UP)
    {
      Error(ERR_WARN, "no special port at position (%d, %d)", port_x, port_y);

      continue;
    }

    /* change previous (wrong) gravity inverting special port to either
       gravity enabling special port or gravity disabling special port */
    level->field[port_x][port_y] +=
      (gravity == 1 ? EL_SP_GRAVITY_ON_PORT_RIGHT :
       EL_SP_GRAVITY_OFF_PORT_RIGHT) - EL_SP_GRAVITY_PORT_RIGHT;
  }

  /* change special gravity ports without database entries to normal ports */
  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      if (level->field[x][y] >= EL_SP_GRAVITY_PORT_RIGHT &&
	  level->field[x][y] <= EL_SP_GRAVITY_PORT_UP)
	level->field[x][y] += EL_SP_PORT_RIGHT - EL_SP_GRAVITY_PORT_RIGHT;

  level->time = 0;			/* no time limit */
  level->amoeba_speed = 0;
  level->time_magic_wall = 0;
  level->time_wheel = 0;
  level->amoeba_content = EL_EMPTY;

#if 1
  /* original Supaplex does not use score values -- use default values */
#else
  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = 0;
#endif

  /* there are no yamyams in supaplex levels */
  for (i = 0; i < level->num_yamyam_contents; i++)
    for (x = 0; x < 3; x++)
      for (y = 0; y < 3; y++)
	level->yamyam_content[i].e[x][y] = EL_EMPTY;
}

static void CopyNativeTape_RND_to_SP(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  struct DemoInfo_SP *demo = &level_sp->demo;
  int i, j;

  /* always start with reliable default values */
  demo->is_available = FALSE;
  demo->length = 0;

  if (TAPE_IS_EMPTY(tape))
    return;

  demo->level_nr = tape.level_nr;	/* (currently not used) */

  level_sp->header.DemoRandomSeed = tape.random_seed;

  demo->length = 0;

  for (i = 0; i < tape.length; i++)
  {
    int demo_action = map_key_RND_to_SP(tape.pos[i].action[0]);
    int demo_repeat = tape.pos[i].delay;
    int demo_entries = (demo_repeat + 15) / 16;

    if (demo->length + demo_entries >= SP_MAX_TAPE_LEN)
    {
      Error(ERR_WARN, "tape truncated: size exceeds maximum SP demo size %d",
	    SP_MAX_TAPE_LEN);

      break;
    }

    for (j = 0; j < demo_repeat / 16; j++)
      demo->data[demo->length++] = 0xf0 | demo_action;

    if (demo_repeat % 16)
      demo->data[demo->length++] = ((demo_repeat % 16 - 1) << 4) | demo_action;
  }

  demo->is_available = TRUE;
}

static void setTapeInfoToDefaults();

static void CopyNativeTape_SP_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_SP *level_sp = level->native_sp_level;
  struct DemoInfo_SP *demo = &level_sp->demo;
  char *filename = level->file_info.filename;
  int i;

  /* always start with reliable default values */
  setTapeInfoToDefaults();

  if (!demo->is_available)
    return;

  tape.level_nr = demo->level_nr;	/* (currently not used) */
  tape.random_seed = level_sp->header.DemoRandomSeed;

  TapeSetDateFromEpochSeconds(getFileTimestampEpochSeconds(filename));

  tape.counter = 0;
  tape.pos[tape.counter].delay = 0;

  for (i = 0; i < demo->length; i++)
  {
    int demo_action = demo->data[i] & 0x0f;
    int demo_repeat = (demo->data[i] & 0xf0) >> 4;
    int tape_action = map_key_SP_to_RND(demo_action);
    int tape_repeat = demo_repeat + 1;
    byte action[MAX_PLAYERS] = { tape_action, 0, 0, 0 };
    boolean success = 0;
    int j;

    for (j = 0; j < tape_repeat; j++)
      success = TapeAddAction(action);

    if (!success)
    {
      Error(ERR_WARN, "SP demo truncated: size exceeds maximum tape size %d",
	    MAX_TAPE_LEN);

      break;
    }
  }

  TapeHaltRecording();
}


/* ------------------------------------------------------------------------- */
/* functions for loading MM level                                            */
/* ------------------------------------------------------------------------- */

void CopyNativeLevel_RND_to_MM(struct LevelInfo *level)
{
  struct LevelInfo_MM *level_mm = level->native_mm_level;
  int x, y;

  level_mm->fieldx = MIN(level->fieldx, MM_MAX_PLAYFIELD_WIDTH);
  level_mm->fieldy = MIN(level->fieldy, MM_MAX_PLAYFIELD_HEIGHT);

  level_mm->time = level->time;
  level_mm->kettles_needed = level->gems_needed;
  level_mm->auto_count_kettles = level->auto_count_gems;

  level_mm->laser_red = level->mm_laser_red;
  level_mm->laser_green = level->mm_laser_green;
  level_mm->laser_blue = level->mm_laser_blue;

  strcpy(level_mm->name, level->name);
  strcpy(level_mm->author, level->author);

  level_mm->score[SC_EMERALD]    = level->score[SC_EMERALD];
  level_mm->score[SC_PACMAN]     = level->score[SC_PACMAN];
  level_mm->score[SC_KEY]        = level->score[SC_KEY];
  level_mm->score[SC_TIME_BONUS] = level->score[SC_TIME_BONUS];
  level_mm->score[SC_ELEM_BONUS] = level->score[SC_ELEM_BONUS];

  level_mm->amoeba_speed = level->amoeba_speed;
  level_mm->time_fuse    = level->mm_time_fuse;
  level_mm->time_bomb    = level->mm_time_bomb;
  level_mm->time_ball    = level->mm_time_ball;
  level_mm->time_block   = level->mm_time_block;

  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      Ur[x][y] =
	level_mm->field[x][y] = map_element_RND_to_MM(level->field[x][y]);
}

void CopyNativeLevel_MM_to_RND(struct LevelInfo *level)
{
  struct LevelInfo_MM *level_mm = level->native_mm_level;
  int x, y;

  level->fieldx = MIN(level_mm->fieldx, MAX_LEV_FIELDX);
  level->fieldy = MIN(level_mm->fieldy, MAX_LEV_FIELDY);

  level->time = level_mm->time;
  level->gems_needed = level_mm->kettles_needed;
  level->auto_count_gems = level_mm->auto_count_kettles;

  level->mm_laser_red = level_mm->laser_red;
  level->mm_laser_green = level_mm->laser_green;
  level->mm_laser_blue = level_mm->laser_blue;

  strcpy(level->name, level_mm->name);

  /* only overwrite author from 'levelinfo.conf' if author defined in level */
  if (!strEqual(level_mm->author, ANONYMOUS_NAME))
    strcpy(level->author, level_mm->author);

  level->score[SC_EMERALD]    = level_mm->score[SC_EMERALD];
  level->score[SC_PACMAN]     = level_mm->score[SC_PACMAN];
  level->score[SC_KEY]        = level_mm->score[SC_KEY];
  level->score[SC_TIME_BONUS] = level_mm->score[SC_TIME_BONUS];
  level->score[SC_ELEM_BONUS] = level_mm->score[SC_ELEM_BONUS];

  level->amoeba_speed  = level_mm->amoeba_speed;
  level->mm_time_fuse  = level_mm->time_fuse;
  level->mm_time_bomb  = level_mm->time_bomb;
  level->mm_time_ball  = level_mm->time_ball;
  level->mm_time_block = level_mm->time_block;

  for (x = 0; x < level->fieldx; x++)
    for (y = 0; y < level->fieldy; y++)
      level->field[x][y] = map_element_MM_to_RND(level_mm->field[x][y]);
}


/* ------------------------------------------------------------------------- */
/* functions for loading DC level                                            */
/* ------------------------------------------------------------------------- */

#define DC_LEVEL_HEADER_SIZE		344

unsigned short getDecodedWord_DC(unsigned short data_encoded, boolean init)
{
  static int last_data_encoded;
  static int offset1;
  static int offset2;
  int diff;
  int diff_hi, diff_lo;
  int data_hi, data_lo;
  unsigned short data_decoded;

  if (init)
  {
    last_data_encoded = 0;
    offset1 = -1;
    offset2 = 0;

    return 0;
  }

  diff = data_encoded - last_data_encoded;
  diff_hi = diff & ~0xff;
  diff_lo = diff &  0xff;

  offset2 += diff_lo;

  data_hi = diff_hi - (offset1 << 8) + (offset2 & 0xff00);
  data_lo = (diff_lo + (data_hi >> 16)) & 0x00ff;
  data_hi = data_hi & 0xff00;

  data_decoded = data_hi | data_lo;

  last_data_encoded = data_encoded;

  offset1 = (offset1 + 1) % 31;
  offset2 = offset2 & 0xff;

  return data_decoded;
}

int getMappedElement_DC(int element)
{
  switch (element)
  {
    case 0x0000:
      element = EL_ROCK;
      break;

      /* 0x0117 - 0x036e: (?) */
      /* EL_DIAMOND */

      /* 0x042d - 0x0684: (?) */
      /* EL_EMERALD */

    case 0x06f1:
      element = EL_NUT;
      break;

    case 0x074c:
      element = EL_BOMB;
      break;

    case 0x07a4:
      element = EL_PEARL;
      break;

    case 0x0823:
      element = EL_CRYSTAL;
      break;

    case 0x0e77:	/* quicksand (boulder) */
      element = EL_QUICKSAND_FAST_FULL;
      break;

    case 0x0e99:	/* slow quicksand (boulder) */
      element = EL_QUICKSAND_FULL;
      break;

    case 0x0ed2:
      element = EL_EM_EXIT_OPEN;
      break;

    case 0x0ee3:
      element = EL_EM_EXIT_CLOSED;
      break;

    case 0x0eeb:
      element = EL_EM_STEEL_EXIT_OPEN;
      break;

    case 0x0efc:
      element = EL_EM_STEEL_EXIT_CLOSED;
      break;

    case 0x0f4f:	/* dynamite (lit 1) */
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f57:	/* dynamite (lit 2) */
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f5f:	/* dynamite (lit 3) */
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f67:	/* dynamite (lit 4) */
      element = EL_EM_DYNAMITE_ACTIVE;
      break;

    case 0x0f81:
    case 0x0f82:
    case 0x0f83:
    case 0x0f84:
      element = EL_AMOEBA_WET;
      break;

    case 0x0f85:
      element = EL_AMOEBA_DROP;
      break;

    case 0x0fb9:
      element = EL_DC_MAGIC_WALL;
      break;

    case 0x0fd0:
      element = EL_SPACESHIP_UP;
      break;

    case 0x0fd9:
      element = EL_SPACESHIP_DOWN;
      break;

    case 0x0ff1:
      element = EL_SPACESHIP_LEFT;
      break;

    case 0x0ff9:
      element = EL_SPACESHIP_RIGHT;
      break;

    case 0x1057:
      element = EL_BUG_UP;
      break;

    case 0x1060:
      element = EL_BUG_DOWN;
      break;

    case 0x1078:
      element = EL_BUG_LEFT;
      break;

    case 0x1080:
      element = EL_BUG_RIGHT;
      break;

    case 0x10de:
      element = EL_MOLE_UP;
      break;

    case 0x10e7:
      element = EL_MOLE_DOWN;
      break;

    case 0x10ff:
      element = EL_MOLE_LEFT;
      break;

    case 0x1107:
      element = EL_MOLE_RIGHT;
      break;

    case 0x11c0:
      element = EL_ROBOT;
      break;

    case 0x13f5:
      element = EL_YAMYAM;
      break;

    case 0x1425:
      element = EL_SWITCHGATE_OPEN;
      break;

    case 0x1426:
      element = EL_SWITCHGATE_CLOSED;
      break;

    case 0x1437:
      element = EL_DC_SWITCHGATE_SWITCH_UP;
      break;

    case 0x143a:
      element = EL_TIMEGATE_CLOSED;
      break;

    case 0x144c:	/* conveyor belt switch (green) */
      element = EL_CONVEYOR_BELT_3_SWITCH_MIDDLE;
      break;

    case 0x144f:	/* conveyor belt switch (red) */
      element = EL_CONVEYOR_BELT_1_SWITCH_MIDDLE;
      break;

    case 0x1452:	/* conveyor belt switch (blue) */
      element = EL_CONVEYOR_BELT_4_SWITCH_MIDDLE;
      break;

    case 0x145b:
      element = EL_CONVEYOR_BELT_3_MIDDLE;
      break;

    case 0x1463:
      element = EL_CONVEYOR_BELT_3_LEFT;
      break;

    case 0x146b:
      element = EL_CONVEYOR_BELT_3_RIGHT;
      break;

    case 0x1473:
      element = EL_CONVEYOR_BELT_1_MIDDLE;
      break;

    case 0x147b:
      element = EL_CONVEYOR_BELT_1_LEFT;
      break;

    case 0x1483:
      element = EL_CONVEYOR_BELT_1_RIGHT;
      break;

    case 0x148b:
      element = EL_CONVEYOR_BELT_4_MIDDLE;
      break;

    case 0x1493:
      element = EL_CONVEYOR_BELT_4_LEFT;
      break;

    case 0x149b:
      element = EL_CONVEYOR_BELT_4_RIGHT;
      break;

    case 0x14ac:
      element = EL_EXPANDABLE_WALL_HORIZONTAL;
      break;

    case 0x14bd:
      element = EL_EXPANDABLE_WALL_VERTICAL;
      break;

    case 0x14c6:
      element = EL_EXPANDABLE_WALL_ANY;
      break;

    case 0x14ce:	/* growing steel wall (left/right) */
      element = EL_EXPANDABLE_STEELWALL_HORIZONTAL;
      break;

    case 0x14df:	/* growing steel wall (up/down) */
      element = EL_EXPANDABLE_STEELWALL_VERTICAL;
      break;

    case 0x14e8:	/* growing steel wall (up/down/left/right) */
      element = EL_EXPANDABLE_STEELWALL_ANY;
      break;

    case 0x14e9:
      element = EL_SHIELD_DEADLY;
      break;

    case 0x1501:
      element = EL_EXTRA_TIME;
      break;

    case 0x154f:
      element = EL_ACID;
      break;

    case 0x1577:
      element = EL_EMPTY_SPACE;
      break;

    case 0x1578:	/* quicksand (empty) */
      element = EL_QUICKSAND_FAST_EMPTY;
      break;

    case 0x1579:	/* slow quicksand (empty) */
      element = EL_QUICKSAND_EMPTY;
      break;

      /* 0x157c - 0x158b: */
      /* EL_SAND */

      /* 0x1590 - 0x159f: */
      /* EL_DC_LANDMINE */

    case 0x15a0:
      element = EL_EM_DYNAMITE;
      break;

    case 0x15a1:	/* key (red) */
      element = EL_EM_KEY_1;
      break;

    case 0x15a2:	/* key (yellow) */
      element = EL_EM_KEY_2;
      break;

    case 0x15a3:	/* key (blue) */
      element = EL_EM_KEY_4;
      break;

    case 0x15a4:	/* key (green) */
      element = EL_EM_KEY_3;
      break;

    case 0x15a5:	/* key (white) */
      element = EL_DC_KEY_WHITE;
      break;

    case 0x15a6:
      element = EL_WALL_SLIPPERY;
      break;

    case 0x15a7:
      element = EL_WALL;
      break;

    case 0x15a8:	/* wall (not round) */
      element = EL_WALL;
      break;

    case 0x15a9:	/* (blue) */
      element = EL_CHAR_A;
      break;

    case 0x15aa:	/* (blue) */
      element = EL_CHAR_B;
      break;

    case 0x15ab:	/* (blue) */
      element = EL_CHAR_C;
      break;

    case 0x15ac:	/* (blue) */
      element = EL_CHAR_D;
      break;

    case 0x15ad:	/* (blue) */
      element = EL_CHAR_E;
      break;

    case 0x15ae:	/* (blue) */
      element = EL_CHAR_F;
      break;

    case 0x15af:	/* (blue) */
      element = EL_CHAR_G;
      break;

    case 0x15b0:	/* (blue) */
      element = EL_CHAR_H;
      break;

    case 0x15b1:	/* (blue) */
      element = EL_CHAR_I;
      break;

    case 0x15b2:	/* (blue) */
      element = EL_CHAR_J;
      break;

    case 0x15b3:	/* (blue) */
      element = EL_CHAR_K;
      break;

    case 0x15b4:	/* (blue) */
      element = EL_CHAR_L;
      break;

    case 0x15b5:	/* (blue) */
      element = EL_CHAR_M;
      break;

    case 0x15b6:	/* (blue) */
      element = EL_CHAR_N;
      break;

    case 0x15b7:	/* (blue) */
      element = EL_CHAR_O;
      break;

    case 0x15b8:	/* (blue) */
      element = EL_CHAR_P;
      break;

    case 0x15b9:	/* (blue) */
      element = EL_CHAR_Q;
      break;

    case 0x15ba:	/* (blue) */
      element = EL_CHAR_R;
      break;

    case 0x15bb:	/* (blue) */
      element = EL_CHAR_S;
      break;

    case 0x15bc:	/* (blue) */
      element = EL_CHAR_T;
      break;

    case 0x15bd:	/* (blue) */
      element = EL_CHAR_U;
      break;

    case 0x15be:	/* (blue) */
      element = EL_CHAR_V;
      break;

    case 0x15bf:	/* (blue) */
      element = EL_CHAR_W;
      break;

    case 0x15c0:	/* (blue) */
      element = EL_CHAR_X;
      break;

    case 0x15c1:	/* (blue) */
      element = EL_CHAR_Y;
      break;

    case 0x15c2:	/* (blue) */
      element = EL_CHAR_Z;
      break;

    case 0x15c3:	/* (blue) */
      element = EL_CHAR_AUMLAUT;
      break;

    case 0x15c4:	/* (blue) */
      element = EL_CHAR_OUMLAUT;
      break;

    case 0x15c5:	/* (blue) */
      element = EL_CHAR_UUMLAUT;
      break;

    case 0x15c6:	/* (blue) */
      element = EL_CHAR_0;
      break;

    case 0x15c7:	/* (blue) */
      element = EL_CHAR_1;
      break;

    case 0x15c8:	/* (blue) */
      element = EL_CHAR_2;
      break;

    case 0x15c9:	/* (blue) */
      element = EL_CHAR_3;
      break;

    case 0x15ca:	/* (blue) */
      element = EL_CHAR_4;
      break;

    case 0x15cb:	/* (blue) */
      element = EL_CHAR_5;
      break;

    case 0x15cc:	/* (blue) */
      element = EL_CHAR_6;
      break;

    case 0x15cd:	/* (blue) */
      element = EL_CHAR_7;
      break;

    case 0x15ce:	/* (blue) */
      element = EL_CHAR_8;
      break;

    case 0x15cf:	/* (blue) */
      element = EL_CHAR_9;
      break;

    case 0x15d0:	/* (blue) */
      element = EL_CHAR_PERIOD;
      break;

    case 0x15d1:	/* (blue) */
      element = EL_CHAR_EXCLAM;
      break;

    case 0x15d2:	/* (blue) */
      element = EL_CHAR_COLON;
      break;

    case 0x15d3:	/* (blue) */
      element = EL_CHAR_LESS;
      break;

    case 0x15d4:	/* (blue) */
      element = EL_CHAR_GREATER;
      break;

    case 0x15d5:	/* (blue) */
      element = EL_CHAR_QUESTION;
      break;

    case 0x15d6:	/* (blue) */
      element = EL_CHAR_COPYRIGHT;
      break;

    case 0x15d7:	/* (blue) */
      element = EL_CHAR_UP;
      break;

    case 0x15d8:	/* (blue) */
      element = EL_CHAR_DOWN;
      break;

    case 0x15d9:	/* (blue) */
      element = EL_CHAR_BUTTON;
      break;

    case 0x15da:	/* (blue) */
      element = EL_CHAR_PLUS;
      break;

    case 0x15db:	/* (blue) */
      element = EL_CHAR_MINUS;
      break;

    case 0x15dc:	/* (blue) */
      element = EL_CHAR_APOSTROPHE;
      break;

    case 0x15dd:	/* (blue) */
      element = EL_CHAR_PARENLEFT;
      break;

    case 0x15de:	/* (blue) */
      element = EL_CHAR_PARENRIGHT;
      break;

    case 0x15df:	/* (green) */
      element = EL_CHAR_A;
      break;

    case 0x15e0:	/* (green) */
      element = EL_CHAR_B;
      break;

    case 0x15e1:	/* (green) */
      element = EL_CHAR_C;
      break;

    case 0x15e2:	/* (green) */
      element = EL_CHAR_D;
      break;

    case 0x15e3:	/* (green) */
      element = EL_CHAR_E;
      break;

    case 0x15e4:	/* (green) */
      element = EL_CHAR_F;
      break;

    case 0x15e5:	/* (green) */
      element = EL_CHAR_G;
      break;

    case 0x15e6:	/* (green) */
      element = EL_CHAR_H;
      break;

    case 0x15e7:	/* (green) */
      element = EL_CHAR_I;
      break;

    case 0x15e8:	/* (green) */
      element = EL_CHAR_J;
      break;

    case 0x15e9:	/* (green) */
      element = EL_CHAR_K;
      break;

    case 0x15ea:	/* (green) */
      element = EL_CHAR_L;
      break;

    case 0x15eb:	/* (green) */
      element = EL_CHAR_M;
      break;

    case 0x15ec:	/* (green) */
      element = EL_CHAR_N;
      break;

    case 0x15ed:	/* (green) */
      element = EL_CHAR_O;
      break;

    case 0x15ee:	/* (green) */
      element = EL_CHAR_P;
      break;

    case 0x15ef:	/* (green) */
      element = EL_CHAR_Q;
      break;

    case 0x15f0:	/* (green) */
      element = EL_CHAR_R;
      break;

    case 0x15f1:	/* (green) */
      element = EL_CHAR_S;
      break;

    case 0x15f2:	/* (green) */
      element = EL_CHAR_T;
      break;

    case 0x15f3:	/* (green) */
      element = EL_CHAR_U;
      break;

    case 0x15f4:	/* (green) */
      element = EL_CHAR_V;
      break;

    case 0x15f5:	/* (green) */
      element = EL_CHAR_W;
      break;

    case 0x15f6:	/* (green) */
      element = EL_CHAR_X;
      break;

    case 0x15f7:	/* (green) */
      element = EL_CHAR_Y;
      break;

    case 0x15f8:	/* (green) */
      element = EL_CHAR_Z;
      break;

    case 0x15f9:	/* (green) */
      element = EL_CHAR_AUMLAUT;
      break;

    case 0x15fa:	/* (green) */
      element = EL_CHAR_OUMLAUT;
      break;

    case 0x15fb:	/* (green) */
      element = EL_CHAR_UUMLAUT;
      break;

    case 0x15fc:	/* (green) */
      element = EL_CHAR_0;
      break;

    case 0x15fd:	/* (green) */
      element = EL_CHAR_1;
      break;

    case 0x15fe:	/* (green) */
      element = EL_CHAR_2;
      break;

    case 0x15ff:	/* (green) */
      element = EL_CHAR_3;
      break;

    case 0x1600:	/* (green) */
      element = EL_CHAR_4;
      break;

    case 0x1601:	/* (green) */
      element = EL_CHAR_5;
      break;

    case 0x1602:	/* (green) */
      element = EL_CHAR_6;
      break;

    case 0x1603:	/* (green) */
      element = EL_CHAR_7;
      break;

    case 0x1604:	/* (green) */
      element = EL_CHAR_8;
      break;

    case 0x1605:	/* (green) */
      element = EL_CHAR_9;
      break;

    case 0x1606:	/* (green) */
      element = EL_CHAR_PERIOD;
      break;

    case 0x1607:	/* (green) */
      element = EL_CHAR_EXCLAM;
      break;

    case 0x1608:	/* (green) */
      element = EL_CHAR_COLON;
      break;

    case 0x1609:	/* (green) */
      element = EL_CHAR_LESS;
      break;

    case 0x160a:	/* (green) */
      element = EL_CHAR_GREATER;
      break;

    case 0x160b:	/* (green) */
      element = EL_CHAR_QUESTION;
      break;

    case 0x160c:	/* (green) */
      element = EL_CHAR_COPYRIGHT;
      break;

    case 0x160d:	/* (green) */
      element = EL_CHAR_UP;
      break;

    case 0x160e:	/* (green) */
      element = EL_CHAR_DOWN;
      break;

    case 0x160f:	/* (green) */
      element = EL_CHAR_BUTTON;
      break;

    case 0x1610:	/* (green) */
      element = EL_CHAR_PLUS;
      break;

    case 0x1611:	/* (green) */
      element = EL_CHAR_MINUS;
      break;

    case 0x1612:	/* (green) */
      element = EL_CHAR_APOSTROPHE;
      break;

    case 0x1613:	/* (green) */
      element = EL_CHAR_PARENLEFT;
      break;

    case 0x1614:	/* (green) */
      element = EL_CHAR_PARENRIGHT;
      break;

    case 0x1615:	/* (blue steel) */
      element = EL_STEEL_CHAR_A;
      break;

    case 0x1616:	/* (blue steel) */
      element = EL_STEEL_CHAR_B;
      break;

    case 0x1617:	/* (blue steel) */
      element = EL_STEEL_CHAR_C;
      break;

    case 0x1618:	/* (blue steel) */
      element = EL_STEEL_CHAR_D;
      break;

    case 0x1619:	/* (blue steel) */
      element = EL_STEEL_CHAR_E;
      break;

    case 0x161a:	/* (blue steel) */
      element = EL_STEEL_CHAR_F;
      break;

    case 0x161b:	/* (blue steel) */
      element = EL_STEEL_CHAR_G;
      break;

    case 0x161c:	/* (blue steel) */
      element = EL_STEEL_CHAR_H;
      break;

    case 0x161d:	/* (blue steel) */
      element = EL_STEEL_CHAR_I;
      break;

    case 0x161e:	/* (blue steel) */
      element = EL_STEEL_CHAR_J;
      break;

    case 0x161f:	/* (blue steel) */
      element = EL_STEEL_CHAR_K;
      break;

    case 0x1620:	/* (blue steel) */
      element = EL_STEEL_CHAR_L;
      break;

    case 0x1621:	/* (blue steel) */
      element = EL_STEEL_CHAR_M;
      break;

    case 0x1622:	/* (blue steel) */
      element = EL_STEEL_CHAR_N;
      break;

    case 0x1623:	/* (blue steel) */
      element = EL_STEEL_CHAR_O;
      break;

    case 0x1624:	/* (blue steel) */
      element = EL_STEEL_CHAR_P;
      break;

    case 0x1625:	/* (blue steel) */
      element = EL_STEEL_CHAR_Q;
      break;

    case 0x1626:	/* (blue steel) */
      element = EL_STEEL_CHAR_R;
      break;

    case 0x1627:	/* (blue steel) */
      element = EL_STEEL_CHAR_S;
      break;

    case 0x1628:	/* (blue steel) */
      element = EL_STEEL_CHAR_T;
      break;

    case 0x1629:	/* (blue steel) */
      element = EL_STEEL_CHAR_U;
      break;

    case 0x162a:	/* (blue steel) */
      element = EL_STEEL_CHAR_V;
      break;

    case 0x162b:	/* (blue steel) */
      element = EL_STEEL_CHAR_W;
      break;

    case 0x162c:	/* (blue steel) */
      element = EL_STEEL_CHAR_X;
      break;

    case 0x162d:	/* (blue steel) */
      element = EL_STEEL_CHAR_Y;
      break;

    case 0x162e:	/* (blue steel) */
      element = EL_STEEL_CHAR_Z;
      break;

    case 0x162f:	/* (blue steel) */
      element = EL_STEEL_CHAR_AUMLAUT;
      break;

    case 0x1630:	/* (blue steel) */
      element = EL_STEEL_CHAR_OUMLAUT;
      break;

    case 0x1631:	/* (blue steel) */
      element = EL_STEEL_CHAR_UUMLAUT;
      break;

    case 0x1632:	/* (blue steel) */
      element = EL_STEEL_CHAR_0;
      break;

    case 0x1633:	/* (blue steel) */
      element = EL_STEEL_CHAR_1;
      break;

    case 0x1634:	/* (blue steel) */
      element = EL_STEEL_CHAR_2;
      break;

    case 0x1635:	/* (blue steel) */
      element = EL_STEEL_CHAR_3;
      break;

    case 0x1636:	/* (blue steel) */
      element = EL_STEEL_CHAR_4;
      break;

    case 0x1637:	/* (blue steel) */
      element = EL_STEEL_CHAR_5;
      break;

    case 0x1638:	/* (blue steel) */
      element = EL_STEEL_CHAR_6;
      break;

    case 0x1639:	/* (blue steel) */
      element = EL_STEEL_CHAR_7;
      break;

    case 0x163a:	/* (blue steel) */
      element = EL_STEEL_CHAR_8;
      break;

    case 0x163b:	/* (blue steel) */
      element = EL_STEEL_CHAR_9;
      break;

    case 0x163c:	/* (blue steel) */
      element = EL_STEEL_CHAR_PERIOD;
      break;

    case 0x163d:	/* (blue steel) */
      element = EL_STEEL_CHAR_EXCLAM;
      break;

    case 0x163e:	/* (blue steel) */
      element = EL_STEEL_CHAR_COLON;
      break;

    case 0x163f:	/* (blue steel) */
      element = EL_STEEL_CHAR_LESS;
      break;

    case 0x1640:	/* (blue steel) */
      element = EL_STEEL_CHAR_GREATER;
      break;

    case 0x1641:	/* (blue steel) */
      element = EL_STEEL_CHAR_QUESTION;
      break;

    case 0x1642:	/* (blue steel) */
      element = EL_STEEL_CHAR_COPYRIGHT;
      break;

    case 0x1643:	/* (blue steel) */
      element = EL_STEEL_CHAR_UP;
      break;

    case 0x1644:	/* (blue steel) */
      element = EL_STEEL_CHAR_DOWN;
      break;

    case 0x1645:	/* (blue steel) */
      element = EL_STEEL_CHAR_BUTTON;
      break;

    case 0x1646:	/* (blue steel) */
      element = EL_STEEL_CHAR_PLUS;
      break;

    case 0x1647:	/* (blue steel) */
      element = EL_STEEL_CHAR_MINUS;
      break;

    case 0x1648:	/* (blue steel) */
      element = EL_STEEL_CHAR_APOSTROPHE;
      break;

    case 0x1649:	/* (blue steel) */
      element = EL_STEEL_CHAR_PARENLEFT;
      break;

    case 0x164a:	/* (blue steel) */
      element = EL_STEEL_CHAR_PARENRIGHT;
      break;

    case 0x164b:	/* (green steel) */
      element = EL_STEEL_CHAR_A;
      break;

    case 0x164c:	/* (green steel) */
      element = EL_STEEL_CHAR_B;
      break;

    case 0x164d:	/* (green steel) */
      element = EL_STEEL_CHAR_C;
      break;

    case 0x164e:	/* (green steel) */
      element = EL_STEEL_CHAR_D;
      break;

    case 0x164f:	/* (green steel) */
      element = EL_STEEL_CHAR_E;
      break;

    case 0x1650:	/* (green steel) */
      element = EL_STEEL_CHAR_F;
      break;

    case 0x1651:	/* (green steel) */
      element = EL_STEEL_CHAR_G;
      break;

    case 0x1652:	/* (green steel) */
      element = EL_STEEL_CHAR_H;
      break;

    case 0x1653:	/* (green steel) */
      element = EL_STEEL_CHAR_I;
      break;

    case 0x1654:	/* (green steel) */
      element = EL_STEEL_CHAR_J;
      break;

    case 0x1655:	/* (green steel) */
      element = EL_STEEL_CHAR_K;
      break;

    case 0x1656:	/* (green steel) */
      element = EL_STEEL_CHAR_L;
      break;

    case 0x1657:	/* (green steel) */
      element = EL_STEEL_CHAR_M;
      break;

    case 0x1658:	/* (green steel) */
      element = EL_STEEL_CHAR_N;
      break;

    case 0x1659:	/* (green steel) */
      element = EL_STEEL_CHAR_O;
      break;

    case 0x165a:	/* (green steel) */
      element = EL_STEEL_CHAR_P;
      break;

    case 0x165b:	/* (green steel) */
      element = EL_STEEL_CHAR_Q;
      break;

    case 0x165c:	/* (green steel) */
      element = EL_STEEL_CHAR_R;
      break;

    case 0x165d:	/* (green steel) */
      element = EL_STEEL_CHAR_S;
      break;

    case 0x165e:	/* (green steel) */
      element = EL_STEEL_CHAR_T;
      break;

    case 0x165f:	/* (green steel) */
      element = EL_STEEL_CHAR_U;
      break;

    case 0x1660:	/* (green steel) */
      element = EL_STEEL_CHAR_V;
      break;

    case 0x1661:	/* (green steel) */
      element = EL_STEEL_CHAR_W;
      break;

    case 0x1662:	/* (green steel) */
      element = EL_STEEL_CHAR_X;
      break;

    case 0x1663:	/* (green steel) */
      element = EL_STEEL_CHAR_Y;
      break;

    case 0x1664:	/* (green steel) */
      element = EL_STEEL_CHAR_Z;
      break;

    case 0x1665:	/* (green steel) */
      element = EL_STEEL_CHAR_AUMLAUT;
      break;

    case 0x1666:	/* (green steel) */
      element = EL_STEEL_CHAR_OUMLAUT;
      break;

    case 0x1667:	/* (green steel) */
      element = EL_STEEL_CHAR_UUMLAUT;
      break;

    case 0x1668:	/* (green steel) */
      element = EL_STEEL_CHAR_0;
      break;

    case 0x1669:	/* (green steel) */
      element = EL_STEEL_CHAR_1;
      break;

    case 0x166a:	/* (green steel) */
      element = EL_STEEL_CHAR_2;
      break;

    case 0x166b:	/* (green steel) */
      element = EL_STEEL_CHAR_3;
      break;

    case 0x166c:	/* (green steel) */
      element = EL_STEEL_CHAR_4;
      break;

    case 0x166d:	/* (green steel) */
      element = EL_STEEL_CHAR_5;
      break;

    case 0x166e:	/* (green steel) */
      element = EL_STEEL_CHAR_6;
      break;

    case 0x166f:	/* (green steel) */
      element = EL_STEEL_CHAR_7;
      break;

    case 0x1670:	/* (green steel) */
      element = EL_STEEL_CHAR_8;
      break;

    case 0x1671:	/* (green steel) */
      element = EL_STEEL_CHAR_9;
      break;

    case 0x1672:	/* (green steel) */
      element = EL_STEEL_CHAR_PERIOD;
      break;

    case 0x1673:	/* (green steel) */
      element = EL_STEEL_CHAR_EXCLAM;
      break;

    case 0x1674:	/* (green steel) */
      element = EL_STEEL_CHAR_COLON;
      break;

    case 0x1675:	/* (green steel) */
      element = EL_STEEL_CHAR_LESS;
      break;

    case 0x1676:	/* (green steel) */
      element = EL_STEEL_CHAR_GREATER;
      break;

    case 0x1677:	/* (green steel) */
      element = EL_STEEL_CHAR_QUESTION;
      break;

    case 0x1678:	/* (green steel) */
      element = EL_STEEL_CHAR_COPYRIGHT;
      break;

    case 0x1679:	/* (green steel) */
      element = EL_STEEL_CHAR_UP;
      break;

    case 0x167a:	/* (green steel) */
      element = EL_STEEL_CHAR_DOWN;
      break;

    case 0x167b:	/* (green steel) */
      element = EL_STEEL_CHAR_BUTTON;
      break;

    case 0x167c:	/* (green steel) */
      element = EL_STEEL_CHAR_PLUS;
      break;

    case 0x167d:	/* (green steel) */
      element = EL_STEEL_CHAR_MINUS;
      break;

    case 0x167e:	/* (green steel) */
      element = EL_STEEL_CHAR_APOSTROPHE;
      break;

    case 0x167f:	/* (green steel) */
      element = EL_STEEL_CHAR_PARENLEFT;
      break;

    case 0x1680:	/* (green steel) */
      element = EL_STEEL_CHAR_PARENRIGHT;
      break;

    case 0x1681:	/* gate (red) */
      element = EL_EM_GATE_1;
      break;

    case 0x1682:	/* secret gate (red) */
      element = EL_GATE_1_GRAY;
      break;

    case 0x1683:	/* gate (yellow) */
      element = EL_EM_GATE_2;
      break;

    case 0x1684:	/* secret gate (yellow) */
      element = EL_GATE_2_GRAY;
      break;

    case 0x1685:	/* gate (blue) */
      element = EL_EM_GATE_4;
      break;

    case 0x1686:	/* secret gate (blue) */
      element = EL_GATE_4_GRAY;
      break;

    case 0x1687:	/* gate (green) */
      element = EL_EM_GATE_3;
      break;

    case 0x1688:	/* secret gate (green) */
      element = EL_GATE_3_GRAY;
      break;

    case 0x1689:	/* gate (white) */
      element = EL_DC_GATE_WHITE;
      break;

    case 0x168a:	/* secret gate (white) */
      element = EL_DC_GATE_WHITE_GRAY;
      break;

    case 0x168b:	/* secret gate (no key) */
      element = EL_DC_GATE_FAKE_GRAY;
      break;

    case 0x168c:
      element = EL_ROBOT_WHEEL;
      break;

    case 0x168d:
      element = EL_DC_TIMEGATE_SWITCH;
      break;

    case 0x168e:
      element = EL_ACID_POOL_BOTTOM;
      break;

    case 0x168f:
      element = EL_ACID_POOL_TOPLEFT;
      break;

    case 0x1690:
      element = EL_ACID_POOL_TOPRIGHT;
      break;

    case 0x1691:
      element = EL_ACID_POOL_BOTTOMLEFT;
      break;

    case 0x1692:
      element = EL_ACID_POOL_BOTTOMRIGHT;
      break;

    case 0x1693:
      element = EL_STEELWALL;
      break;

    case 0x1694:
      element = EL_STEELWALL_SLIPPERY;
      break;

    case 0x1695:	/* steel wall (not round) */
      element = EL_STEELWALL;
      break;

    case 0x1696:	/* steel wall (left) */
      element = EL_DC_STEELWALL_1_LEFT;
      break;

    case 0x1697:	/* steel wall (bottom) */
      element = EL_DC_STEELWALL_1_BOTTOM;
      break;

    case 0x1698:	/* steel wall (right) */
      element = EL_DC_STEELWALL_1_RIGHT;
      break;

    case 0x1699:	/* steel wall (top) */
      element = EL_DC_STEELWALL_1_TOP;
      break;

    case 0x169a:	/* steel wall (left/bottom) */
      element = EL_DC_STEELWALL_1_BOTTOMLEFT;
      break;

    case 0x169b:	/* steel wall (right/bottom) */
      element = EL_DC_STEELWALL_1_BOTTOMRIGHT;
      break;

    case 0x169c:	/* steel wall (right/top) */
      element = EL_DC_STEELWALL_1_TOPRIGHT;
      break;

    case 0x169d:	/* steel wall (left/top) */
      element = EL_DC_STEELWALL_1_TOPLEFT;
      break;

    case 0x169e:	/* steel wall (right/bottom small) */
      element = EL_DC_STEELWALL_1_BOTTOMRIGHT_2;
      break;

    case 0x169f:	/* steel wall (left/bottom small) */
      element = EL_DC_STEELWALL_1_BOTTOMLEFT_2;
      break;

    case 0x16a0:	/* steel wall (right/top small) */
      element = EL_DC_STEELWALL_1_TOPRIGHT_2;
      break;

    case 0x16a1:	/* steel wall (left/top small) */
      element = EL_DC_STEELWALL_1_TOPLEFT_2;
      break;

    case 0x16a2:	/* steel wall (left/right) */
      element = EL_DC_STEELWALL_1_VERTICAL;
      break;

    case 0x16a3:	/* steel wall (top/bottom) */
      element = EL_DC_STEELWALL_1_HORIZONTAL;
      break;

    case 0x16a4:	/* steel wall 2 (left end) */
      element = EL_DC_STEELWALL_2_LEFT;
      break;

    case 0x16a5:	/* steel wall 2 (right end) */
      element = EL_DC_STEELWALL_2_RIGHT;
      break;

    case 0x16a6:	/* steel wall 2 (top end) */
      element = EL_DC_STEELWALL_2_TOP;
      break;

    case 0x16a7:	/* steel wall 2 (bottom end) */
      element = EL_DC_STEELWALL_2_BOTTOM;
      break;

    case 0x16a8:	/* steel wall 2 (left/right) */
      element = EL_DC_STEELWALL_2_HORIZONTAL;
      break;

    case 0x16a9:	/* steel wall 2 (up/down) */
      element = EL_DC_STEELWALL_2_VERTICAL;
      break;

    case 0x16aa:	/* steel wall 2 (mid) */
      element = EL_DC_STEELWALL_2_MIDDLE;
      break;

    case 0x16ab:
      element = EL_SIGN_EXCLAMATION;
      break;

    case 0x16ac:
      element = EL_SIGN_RADIOACTIVITY;
      break;

    case 0x16ad:
      element = EL_SIGN_STOP;
      break;

    case 0x16ae:
      element = EL_SIGN_WHEELCHAIR;
      break;

    case 0x16af:
      element = EL_SIGN_PARKING;
      break;

    case 0x16b0:
      element = EL_SIGN_NO_ENTRY;
      break;

    case 0x16b1:
      element = EL_SIGN_HEART;
      break;

    case 0x16b2:
      element = EL_SIGN_GIVE_WAY;
      break;

    case 0x16b3:
      element = EL_SIGN_ENTRY_FORBIDDEN;
      break;

    case 0x16b4:
      element = EL_SIGN_EMERGENCY_EXIT;
      break;

    case 0x16b5:
      element = EL_SIGN_YIN_YANG;
      break;

    case 0x16b6:
      element = EL_WALL_EMERALD;
      break;

    case 0x16b7:
      element = EL_WALL_DIAMOND;
      break;

    case 0x16b8:
      element = EL_WALL_PEARL;
      break;

    case 0x16b9:
      element = EL_WALL_CRYSTAL;
      break;

    case 0x16ba:
      element = EL_INVISIBLE_WALL;
      break;

    case 0x16bb:
      element = EL_INVISIBLE_STEELWALL;
      break;

      /* 0x16bc - 0x16cb: */
      /* EL_INVISIBLE_SAND */

    case 0x16cc:
      element = EL_LIGHT_SWITCH;
      break;

    case 0x16cd:
      element = EL_ENVELOPE_1;
      break;

    default:
      if (element >= 0x0117 && element <= 0x036e)	/* (?) */
	element = EL_DIAMOND;
      else if (element >= 0x042d && element <= 0x0684)	/* (?) */
	element = EL_EMERALD;
      else if (element >= 0x157c && element <= 0x158b)
	element = EL_SAND;
      else if (element >= 0x1590 && element <= 0x159f)
	element = EL_DC_LANDMINE;
      else if (element >= 0x16bc && element <= 0x16cb)
	element = EL_INVISIBLE_SAND;
      else
      {
	Error(ERR_WARN, "unknown Diamond Caves element 0x%04x", element);
	element = EL_UNKNOWN;
      }
      break;
  }

  return getMappedElement(element);
}

static void LoadLevelFromFileStream_DC(File *file, struct LevelInfo *level,
				       int nr)
{
  byte header[DC_LEVEL_HEADER_SIZE];
  int envelope_size;
  int envelope_header_pos = 62;
  int envelope_content_pos = 94;
  int level_name_pos = 251;
  int level_author_pos = 292;
  int envelope_header_len;
  int envelope_content_len;
  int level_name_len;
  int level_author_len;
  int fieldx, fieldy;
  int num_yamyam_contents;
  int i, x, y;

  getDecodedWord_DC(0, TRUE);		/* initialize DC2 decoding engine */

  for (i = 0; i < DC_LEVEL_HEADER_SIZE / 2; i++)
  {
    unsigned short header_word = getDecodedWord_DC(getFile16BitBE(file), FALSE);

    header[i * 2 + 0] = header_word >> 8;
    header[i * 2 + 1] = header_word & 0xff;
  }

  /* read some values from level header to check level decoding integrity */
  fieldx = header[6] | (header[7] << 8);
  fieldy = header[8] | (header[9] << 8);
  num_yamyam_contents = header[60] | (header[61] << 8);

  /* do some simple sanity checks to ensure that level was correctly decoded */
  if (fieldx < 1 || fieldx > 256 ||
      fieldy < 1 || fieldy > 256 ||
      num_yamyam_contents < 1 || num_yamyam_contents > 8)
  {
    level->no_valid_file = TRUE;

    Error(ERR_WARN, "cannot decode level from stream -- using empty level");

    return;
  }

  /* maximum envelope header size is 31 bytes */
  envelope_header_len	= header[envelope_header_pos];
  /* maximum envelope content size is 110 (156?) bytes */
  envelope_content_len	= header[envelope_content_pos];

  /* maximum level title size is 40 bytes */
  level_name_len	= MIN(header[level_name_pos],   MAX_LEVEL_NAME_LEN);
  /* maximum level author size is 30 (51?) bytes */
  level_author_len	= MIN(header[level_author_pos], MAX_LEVEL_AUTHOR_LEN);

  envelope_size = 0;

  for (i = 0; i < envelope_header_len; i++)
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] =
	header[envelope_header_pos + 1 + i];

  if (envelope_header_len > 0 && envelope_content_len > 0)
  {
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] = '\n';
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] = '\n';
  }

  for (i = 0; i < envelope_content_len; i++)
    if (envelope_size < MAX_ENVELOPE_TEXT_LEN)
      level->envelope[0].text[envelope_size++] =
	header[envelope_content_pos + 1 + i];

  level->envelope[0].text[envelope_size] = '\0';

  level->envelope[0].xsize = MAX_ENVELOPE_XSIZE;
  level->envelope[0].ysize = 10;
  level->envelope[0].autowrap = TRUE;
  level->envelope[0].centered = TRUE;

  for (i = 0; i < level_name_len; i++)
    level->name[i] = header[level_name_pos + 1 + i];
  level->name[level_name_len] = '\0';

  for (i = 0; i < level_author_len; i++)
    level->author[i] = header[level_author_pos + 1 + i];
  level->author[level_author_len] = '\0';

  num_yamyam_contents = header[60] | (header[61] << 8);
  level->num_yamyam_contents =
    MIN(MAX(MIN_ELEMENT_CONTENTS, num_yamyam_contents), MAX_ELEMENT_CONTENTS);

  for (i = 0; i < num_yamyam_contents; i++)
  {
    for (y = 0; y < 3; y++) for (x = 0; x < 3; x++)
    {
      unsigned short word = getDecodedWord_DC(getFile16BitBE(file), FALSE);
      int element_dc = ((word & 0xff) << 8) | ((word >> 8) & 0xff);

      if (i < MAX_ELEMENT_CONTENTS)
	level->yamyam_content[i].e[x][y] = getMappedElement_DC(element_dc);
    }
  }

  fieldx = header[6] | (header[7] << 8);
  fieldy = header[8] | (header[9] << 8);
  level->fieldx = MIN(MAX(MIN_LEV_FIELDX, fieldx), MAX_LEV_FIELDX);
  level->fieldy = MIN(MAX(MIN_LEV_FIELDY, fieldy), MAX_LEV_FIELDY);

  for (y = 0; y < fieldy; y++) for (x = 0; x < fieldx; x++)
  {
    unsigned short word = getDecodedWord_DC(getFile16BitBE(file), FALSE);
    int element_dc = ((word & 0xff) << 8) | ((word >> 8) & 0xff);

    if (x < MAX_LEV_FIELDX && y < MAX_LEV_FIELDY)
      level->field[x][y] = getMappedElement_DC(element_dc);
  }

  x = MIN(MAX(0, (header[10] | (header[11] << 8)) - 1), MAX_LEV_FIELDX - 1);
  y = MIN(MAX(0, (header[12] | (header[13] << 8)) - 1), MAX_LEV_FIELDY - 1);
  level->field[x][y] = EL_PLAYER_1;

  x = MIN(MAX(0, (header[14] | (header[15] << 8)) - 1), MAX_LEV_FIELDX - 1);
  y = MIN(MAX(0, (header[16] | (header[17] << 8)) - 1), MAX_LEV_FIELDY - 1);
  level->field[x][y] = EL_PLAYER_2;

  level->gems_needed		= header[18] | (header[19] << 8);

  level->score[SC_EMERALD]	= header[20] | (header[21] << 8);
  level->score[SC_DIAMOND]	= header[22] | (header[23] << 8);
  level->score[SC_PEARL]	= header[24] | (header[25] << 8);
  level->score[SC_CRYSTAL]	= header[26] | (header[27] << 8);
  level->score[SC_NUT]		= header[28] | (header[29] << 8);
  level->score[SC_ROBOT]	= header[30] | (header[31] << 8);
  level->score[SC_SPACESHIP]	= header[32] | (header[33] << 8);
  level->score[SC_BUG]		= header[34] | (header[35] << 8);
  level->score[SC_YAMYAM]	= header[36] | (header[37] << 8);
  level->score[SC_DYNAMITE]	= header[38] | (header[39] << 8);
  level->score[SC_KEY]		= header[40] | (header[41] << 8);
  level->score[SC_TIME_BONUS]	= header[42] | (header[43] << 8);

  level->time			= header[44] | (header[45] << 8);

  level->amoeba_speed		= header[46] | (header[47] << 8);
  level->time_light		= header[48] | (header[49] << 8);
  level->time_timegate		= header[50] | (header[51] << 8);
  level->time_wheel		= header[52] | (header[53] << 8);
  level->time_magic_wall	= header[54] | (header[55] << 8);
  level->extra_time		= header[56] | (header[57] << 8);
  level->shield_normal_time	= header[58] | (header[59] << 8);

  /* Diamond Caves has the same (strange) behaviour as Emerald Mine that gems
     can slip down from flat walls, like normal walls and steel walls */
  level->em_slippery_gems = TRUE;
}

static void LoadLevelFromFileInfo_DC(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  char *filename = level_file_info->filename;
  File *file;
  int num_magic_bytes = 8;
  char magic_bytes[num_magic_bytes + 1];
  int num_levels_to_skip = level_file_info->nr - leveldir_current->first_level;

  if (!(file = openFile(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;

    if (!level_info_only)
      Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return;
  }

  // fseek(file, 0x0000, SEEK_SET);

  if (level_file_info->packed)
  {
    /* read "magic bytes" from start of file */
    if (getStringFromFile(file, magic_bytes, num_magic_bytes + 1) == NULL)
      magic_bytes[0] = '\0';

    /* check "magic bytes" for correct file format */
    if (!strPrefix(magic_bytes, "DC2"))
    {
      level->no_valid_file = TRUE;

      Error(ERR_WARN, "unknown DC level file '%s' -- using empty level",
	    filename);

      return;
    }

    if (strPrefix(magic_bytes, "DC2Win95") ||
	strPrefix(magic_bytes, "DC2Win98"))
    {
      int position_first_level = 0x00fa;
      int extra_bytes = 4;
      int skip_bytes;

      /* advance file stream to first level inside the level package */
      skip_bytes = position_first_level - num_magic_bytes - extra_bytes;

      /* each block of level data is followed by block of non-level data */
      num_levels_to_skip *= 2;

      /* at least skip header bytes, therefore use ">= 0" instead of "> 0" */
      while (num_levels_to_skip >= 0)
      {
	/* advance file stream to next level inside the level package */
	if (seekFile(file, skip_bytes, SEEK_CUR) != 0)
	{
	  level->no_valid_file = TRUE;

	  Error(ERR_WARN, "cannot fseek in file '%s' -- using empty level",
		filename);

	  return;
	}

	/* skip apparently unused extra bytes following each level */
	ReadUnusedBytesFromFile(file, extra_bytes);

	/* read size of next level in level package */
	skip_bytes = getFile32BitLE(file);

	num_levels_to_skip--;
      }
    }
    else
    {
      level->no_valid_file = TRUE;

      Error(ERR_WARN, "unknown DC2 level file '%s' -- using empty level",
	    filename);

      return;
    }
  }

  LoadLevelFromFileStream_DC(file, level, level_file_info->nr);

  closeFile(file);
}


/* ------------------------------------------------------------------------- */
/* functions for loading SB level                                            */
/* ------------------------------------------------------------------------- */

int getMappedElement_SB(int element_ascii, boolean use_ces)
{
  static struct
  {
    int ascii;
    int sb;
    int ce;
  }
  sb_element_mapping[] =
  {
    { ' ', EL_EMPTY,                EL_CUSTOM_1 },  /* floor (space) */
    { '#', EL_STEELWALL,            EL_CUSTOM_2 },  /* wall */
    { '@', EL_PLAYER_1,             EL_CUSTOM_3 },  /* player */
    { '$', EL_SOKOBAN_OBJECT,       EL_CUSTOM_4 },  /* box */
    { '.', EL_SOKOBAN_FIELD_EMPTY,  EL_CUSTOM_5 },  /* goal square */
    { '*', EL_SOKOBAN_FIELD_FULL,   EL_CUSTOM_6 },  /* box on goal square */
    { '+', EL_SOKOBAN_FIELD_PLAYER, EL_CUSTOM_7 },  /* player on goal square */
    { '_', EL_INVISIBLE_STEELWALL,  EL_FROM_LEVEL_TEMPLATE },  /* floor beyond border */

    { 0,   -1,                      -1          },
  };

  int i;

  for (i = 0; sb_element_mapping[i].ascii != 0; i++)
    if (element_ascii == sb_element_mapping[i].ascii)
      return (use_ces ? sb_element_mapping[i].ce : sb_element_mapping[i].sb);

  return EL_UNDEFINED;
}

static void LoadLevelFromFileInfo_SB(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  char *filename = level_file_info->filename;
  char line[MAX_LINE_LEN], line_raw[MAX_LINE_LEN], previous_line[MAX_LINE_LEN];
  char last_comment[MAX_LINE_LEN];
  char level_name[MAX_LINE_LEN];
  char *line_ptr;
  File *file;
  int num_levels_to_skip = level_file_info->nr - leveldir_current->first_level;
  boolean read_continued_line = FALSE;
  boolean reading_playfield = FALSE;
  boolean got_valid_playfield_line = FALSE;
  boolean invalid_playfield_char = FALSE;
  boolean load_xsb_to_ces = check_special_flags("load_xsb_to_ces");
  int file_level_nr = 0;
  int line_nr = 0;
  int x = 0, y = 0;		/* initialized to make compilers happy */

  last_comment[0] = '\0';
  level_name[0] = '\0';

  if (!(file = openFile(filename, MODE_READ)))
  {
    level->no_valid_file = TRUE;

    if (!level_info_only)
      Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return;
  }

  while (!checkEndOfFile(file))
  {
    /* level successfully read, but next level may follow here */
    if (!got_valid_playfield_line && reading_playfield)
    {
      /* read playfield from single level file -- skip remaining file */
      if (!level_file_info->packed)
	break;

      if (file_level_nr >= num_levels_to_skip)
	break;

      file_level_nr++;

      last_comment[0] = '\0';
      level_name[0] = '\0';

      reading_playfield = FALSE;
    }

    got_valid_playfield_line = FALSE;

    /* read next line of input file */
    if (!getStringFromFile(file, line, MAX_LINE_LEN))
      break;

    /* check if line was completely read and is terminated by line break */
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
      line_nr++;

    /* cut trailing line break (this can be newline and/or carriage return) */
    for (line_ptr = &line[strlen(line)]; line_ptr >= line; line_ptr--)
      if ((*line_ptr == '\n' || *line_ptr == '\r') && *(line_ptr + 1) == '\0')
        *line_ptr = '\0';

    /* copy raw input line for later use (mainly debugging output) */
    strcpy(line_raw, line);

    if (read_continued_line)
    {
      /* append new line to existing line, if there is enough space */
      if (strlen(previous_line) + strlen(line_ptr) < MAX_LINE_LEN)
        strcat(previous_line, line_ptr);

      strcpy(line, previous_line);      /* copy storage buffer to line */

      read_continued_line = FALSE;
    }

    /* if the last character is '\', continue at next line */
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\\')
    {
      line[strlen(line) - 1] = '\0';    /* cut off trailing backslash */
      strcpy(previous_line, line);      /* copy line to storage buffer */

      read_continued_line = TRUE;

      continue;
    }

    /* skip empty lines */
    if (line[0] == '\0')
      continue;

    /* extract comment text from comment line */
    if (line[0] == ';')
    {
      for (line_ptr = line; *line_ptr; line_ptr++)
        if (*line_ptr != ' ' && *line_ptr != '\t' && *line_ptr != ';')
          break;

      strcpy(last_comment, line_ptr);

      continue;
    }

    /* extract level title text from line containing level title */
    if (line[0] == '\'')
    {
      strcpy(level_name, &line[1]);

      if (strlen(level_name) > 0 && level_name[strlen(level_name) - 1] == '\'')
	level_name[strlen(level_name) - 1] = '\0';

      continue;
    }

    /* skip lines containing only spaces (or empty lines) */
    for (line_ptr = line; *line_ptr; line_ptr++)
      if (*line_ptr != ' ')
	break;
    if (*line_ptr == '\0')
      continue;

    /* at this point, we have found a line containing part of a playfield */

    got_valid_playfield_line = TRUE;

    if (!reading_playfield)
    {
      reading_playfield = TRUE;
      invalid_playfield_char = FALSE;

      for (x = 0; x < MAX_LEV_FIELDX; x++)
	for (y = 0; y < MAX_LEV_FIELDY; y++)
	  level->field[x][y] = getMappedElement_SB(' ', load_xsb_to_ces);

      level->fieldx = 0;
      level->fieldy = 0;

      /* start with topmost tile row */
      y = 0;
    }

    /* skip playfield line if larger row than allowed */
    if (y >= MAX_LEV_FIELDY)
      continue;

    /* start with leftmost tile column */
    x = 0;

    /* read playfield elements from line */
    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      int mapped_sb_element = getMappedElement_SB(*line_ptr, load_xsb_to_ces);

      /* stop parsing playfield line if larger column than allowed */
      if (x >= MAX_LEV_FIELDX)
	break;

      if (mapped_sb_element == EL_UNDEFINED)
      {
	invalid_playfield_char = TRUE;

	break;
      }

      level->field[x][y] = mapped_sb_element;

      /* continue with next tile column */
      x++;

      level->fieldx = MAX(x, level->fieldx);
    }

    if (invalid_playfield_char)
    {
      /* if first playfield line, treat invalid lines as comment lines */
      if (y == 0)
	reading_playfield = FALSE;

      continue;
    }

    /* continue with next tile row */
    y++;
  }

  closeFile(file);

  level->fieldy = y;

  level->fieldx = MIN(MAX(MIN_LEV_FIELDX, level->fieldx), MAX_LEV_FIELDX);
  level->fieldy = MIN(MAX(MIN_LEV_FIELDY, level->fieldy), MAX_LEV_FIELDY);

  if (!reading_playfield)
  {
    level->no_valid_file = TRUE;

    Error(ERR_WARN, "cannot read level '%s' -- using empty level", filename);

    return;
  }

  if (*level_name != '\0')
  {
    strncpy(level->name, level_name, MAX_LEVEL_NAME_LEN);
    level->name[MAX_LEVEL_NAME_LEN] = '\0';
  }
  else if (*last_comment != '\0')
  {
    strncpy(level->name, last_comment, MAX_LEVEL_NAME_LEN);
    level->name[MAX_LEVEL_NAME_LEN] = '\0';
  }
  else
  {
    sprintf(level->name, "--> Level %d <--", level_file_info->nr);
  }

  /* set all empty fields beyond the border walls to invisible steel wall */
  for (y = 0; y < level->fieldy; y++) for (x = 0; x < level->fieldx; x++)
  {
    if ((x == 0 || x == level->fieldx - 1 ||
	 y == 0 || y == level->fieldy - 1) &&
	level->field[x][y] == getMappedElement_SB(' ', load_xsb_to_ces))
      FloodFillLevel(x, y, getMappedElement_SB('_', load_xsb_to_ces),
		     level->field, level->fieldx, level->fieldy);
  }

  /* set special level settings for Sokoban levels */

  level->time = 0;
  level->use_step_counter = TRUE;

  if (load_xsb_to_ces)
  {
    /* special global settings can now be set in level template */

    level->use_custom_template = TRUE;
  }
}


/* ------------------------------------------------------------------------- */
/* functions for handling native levels                                      */
/* ------------------------------------------------------------------------- */

static void LoadLevelFromFileInfo_EM(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  if (!LoadNativeLevel_EM(level_file_info->filename, level_info_only))
    level->no_valid_file = TRUE;
}

static void LoadLevelFromFileInfo_SP(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  int pos = 0;

  /* determine position of requested level inside level package */
  if (level_file_info->packed)
    pos = level_file_info->nr - leveldir_current->first_level;

  if (!LoadNativeLevel_SP(level_file_info->filename, pos, level_info_only))
    level->no_valid_file = TRUE;
}

static void LoadLevelFromFileInfo_MM(struct LevelInfo *level,
				     struct LevelFileInfo *level_file_info,
				     boolean level_info_only)
{
  if (!LoadNativeLevel_MM(level_file_info->filename, level_info_only))
    level->no_valid_file = TRUE;
}

void CopyNativeLevel_RND_to_Native(struct LevelInfo *level)
{
  if (level->game_engine_type == GAME_ENGINE_TYPE_EM)
    CopyNativeLevel_RND_to_EM(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_SP)
    CopyNativeLevel_RND_to_SP(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_MM)
    CopyNativeLevel_RND_to_MM(level);
}

void CopyNativeLevel_Native_to_RND(struct LevelInfo *level)
{
  if (level->game_engine_type == GAME_ENGINE_TYPE_EM)
    CopyNativeLevel_EM_to_RND(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_SP)
    CopyNativeLevel_SP_to_RND(level);
  else if (level->game_engine_type == GAME_ENGINE_TYPE_MM)
    CopyNativeLevel_MM_to_RND(level);
}

void SaveNativeLevel(struct LevelInfo *level)
{
  if (level->game_engine_type == GAME_ENGINE_TYPE_SP)
  {
    char *basename = getSingleLevelBasenameExt(level->file_info.nr, "sp");
    char *filename = getLevelFilenameFromBasename(basename);

    CopyNativeLevel_RND_to_SP(level);
    CopyNativeTape_RND_to_SP(level);

    SaveNativeLevel_SP(filename);
  }
}


/* ------------------------------------------------------------------------- */
/* functions for loading generic level                                       */
/* ------------------------------------------------------------------------- */

static void LoadLevelFromFileInfo(struct LevelInfo *level,
				  struct LevelFileInfo *level_file_info,
				  boolean level_info_only)
{
  /* always start with reliable default values */
  setLevelInfoToDefaults(level, level_info_only, TRUE);

  switch (level_file_info->type)
  {
    case LEVEL_FILE_TYPE_RND:
      LoadLevelFromFileInfo_RND(level, level_file_info, level_info_only);
      break;

    case LEVEL_FILE_TYPE_EM:
      LoadLevelFromFileInfo_EM(level, level_file_info, level_info_only);
      level->game_engine_type = GAME_ENGINE_TYPE_EM;
      break;

    case LEVEL_FILE_TYPE_SP:
      LoadLevelFromFileInfo_SP(level, level_file_info, level_info_only);
      level->game_engine_type = GAME_ENGINE_TYPE_SP;
      break;

    case LEVEL_FILE_TYPE_MM:
      LoadLevelFromFileInfo_MM(level, level_file_info, level_info_only);
      level->game_engine_type = GAME_ENGINE_TYPE_MM;
      break;

    case LEVEL_FILE_TYPE_DC:
      LoadLevelFromFileInfo_DC(level, level_file_info, level_info_only);
      break;

    case LEVEL_FILE_TYPE_SB:
      LoadLevelFromFileInfo_SB(level, level_file_info, level_info_only);
      break;

    default:
      LoadLevelFromFileInfo_RND(level, level_file_info, level_info_only);
      break;
  }

  /* if level file is invalid, restore level structure to default values */
  if (level->no_valid_file)
    setLevelInfoToDefaults(level, level_info_only, FALSE);

  if (level->game_engine_type == GAME_ENGINE_TYPE_UNKNOWN)
    level->game_engine_type = GAME_ENGINE_TYPE_RND;

  if (level_file_info->type != LEVEL_FILE_TYPE_RND)
    CopyNativeLevel_Native_to_RND(level);
}

void LoadLevelFromFilename(struct LevelInfo *level, char *filename)
{
  static struct LevelFileInfo level_file_info;

  /* always start with reliable default values */
  setFileInfoToDefaults(&level_file_info);

  level_file_info.nr = 0;			/* unknown level number */
  level_file_info.type = LEVEL_FILE_TYPE_RND;	/* no others supported yet */
  level_file_info.filename = filename;

  LoadLevelFromFileInfo(level, &level_file_info, FALSE);
}

static void LoadLevel_InitVersion(struct LevelInfo *level, char *filename)
{
  int i, j;

  if (leveldir_current == NULL)		/* only when dumping level */
    return;

  /* all engine modifications also valid for levels which use latest engine */
  if (level->game_version < VERSION_IDENT(3,2,0,5))
  {
    /* time bonus score was given for 10 s instead of 1 s before 3.2.0-5 */
    level->score[SC_TIME_BONUS] /= 10;
  }

  if (leveldir_current->latest_engine)
  {
    /* ---------- use latest game engine ----------------------------------- */

    /* For all levels which are forced to use the latest game engine version
       (normally all but user contributed, private and undefined levels), set
       the game engine version to the actual version; this allows for actual
       corrections in the game engine to take effect for existing, converted
       levels (from "classic" or other existing games) to make the emulation
       of the corresponding game more accurate, while (hopefully) not breaking
       existing levels created from other players. */

    level->game_version = GAME_VERSION_ACTUAL;

    /* Set special EM style gems behaviour: EM style gems slip down from
       normal, steel and growing wall. As this is a more fundamental change,
       it seems better to set the default behaviour to "off" (as it is more
       natural) and make it configurable in the level editor (as a property
       of gem style elements). Already existing converted levels (neither
       private nor contributed levels) are changed to the new behaviour. */

    if (level->file_version < FILE_VERSION_2_0)
      level->em_slippery_gems = TRUE;

    return;
  }

  /* ---------- use game engine the level was created with ----------------- */

  /* For all levels which are not forced to use the latest game engine
     version (normally user contributed, private and undefined levels),
     use the version of the game engine the levels were created for.

     Since 2.0.1, the game engine version is now directly stored
     in the level file (chunk "VERS"), so there is no need anymore
     to set the game version from the file version (except for old,
     pre-2.0 levels, where the game version is still taken from the
     file format version used to store the level -- see above). */

  /* player was faster than enemies in 1.0.0 and before */
  if (level->file_version == FILE_VERSION_1_0)
    for (i = 0; i < MAX_PLAYERS; i++)
      level->initial_player_stepsize[i] = STEPSIZE_FAST;

  /* default behaviour for EM style gems was "slippery" only in 2.0.1 */
  if (level->game_version == VERSION_IDENT(2,0,1,0))
    level->em_slippery_gems = TRUE;

  /* springs could be pushed over pits before (pre-release version) 2.2.0 */
  if (level->game_version < VERSION_IDENT(2,2,0,0))
    level->use_spring_bug = TRUE;

  if (level->game_version < VERSION_IDENT(3,2,0,5))
  {
    /* time orb caused limited time in endless time levels before 3.2.0-5 */
    level->use_time_orb_bug = TRUE;

    /* default behaviour for snapping was "no snap delay" before 3.2.0-5 */
    level->block_snap_field = FALSE;

    /* extra time score was same value as time left score before 3.2.0-5 */
    level->extra_time_score = level->score[SC_TIME_BONUS];
  }

  if (level->game_version < VERSION_IDENT(3,2,0,7))
  {
    /* default behaviour for snapping was "not continuous" before 3.2.0-7 */
    level->continuous_snapping = FALSE;
  }

  /* only few elements were able to actively move into acid before 3.1.0 */
  /* trigger settings did not exist before 3.1.0; set to default "any" */
  if (level->game_version < VERSION_IDENT(3,1,0,0))
  {
    /* correct "can move into acid" settings (all zero in old levels) */

    level->can_move_into_acid_bits = 0; /* nothing can move into acid */
    level->dont_collide_with_bits = 0; /* nothing is deadly when colliding */

    setMoveIntoAcidProperty(level, EL_ROBOT,     TRUE);
    setMoveIntoAcidProperty(level, EL_SATELLITE, TRUE);
    setMoveIntoAcidProperty(level, EL_PENGUIN,   TRUE);
    setMoveIntoAcidProperty(level, EL_BALLOON,   TRUE);

    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
      SET_PROPERTY(EL_CUSTOM_START + i, EP_CAN_MOVE_INTO_ACID, TRUE);

    /* correct trigger settings (stored as zero == "none" in old levels) */

    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;
      struct ElementInfo *ei = &element_info[element];

      for (j = 0; j < ei->num_change_pages; j++)
      {
	struct ElementChangeInfo *change = &ei->change_page[j];

	change->trigger_player = CH_PLAYER_ANY;
	change->trigger_page = CH_PAGE_ANY;
      }
    }
  }

  /* try to detect and fix "Snake Bite" levels, which are broken with 3.2.0 */
  {
    int element = EL_CUSTOM_256;
    struct ElementInfo *ei = &element_info[element];
    struct ElementChangeInfo *change = &ei->change_page[0];

    /* This is needed to fix a problem that was caused by a bugfix in function
       game.c/CreateFieldExt() introduced with 3.2.0 that corrects the behaviour
       when a custom element changes to EL_SOKOBAN_FIELD_PLAYER (before, it did
       not replace walkable elements, but instead just placed the player on it,
       without placing the Sokoban field under the player). Unfortunately, this
       breaks "Snake Bite" style levels when the snake is halfway through a door
       that just closes (the snake head is still alive and can be moved in this
       case). This can be fixed by replacing the EL_SOKOBAN_FIELD_PLAYER by the
       player (without Sokoban element) which then gets killed as designed). */

    if ((strncmp(leveldir_current->identifier, "snake_bite", 10) == 0 ||
	 strncmp(ei->description, "pause b4 death", 14) == 0) &&
	change->target_element == EL_SOKOBAN_FIELD_PLAYER)
      change->target_element = EL_PLAYER_1;
  }

  /* try to detect and fix "Zelda" style levels, which are broken with 3.2.5 */
  if (level->game_version < VERSION_IDENT(3,2,5,0))
  {
    /* This is needed to fix a problem that was caused by a bugfix in function
       game.c/CheckTriggeredElementChangeExt() introduced with 3.2.5 that
       corrects the behaviour when a custom element changes to another custom
       element with a higher element number that has change actions defined.
       Normally, only one change per frame is allowed for custom elements.
       Therefore, it is checked if a custom element already changed in the
       current frame; if it did, subsequent changes are suppressed.
       Unfortunately, this is only checked for element changes, but not for
       change actions, which are still executed. As the function above loops
       through all custom elements from lower to higher, an element change
       resulting in a lower CE number won't be checked again, while a target
       element with a higher number will also be checked, and potential change
       actions will get executed for this CE, too (which is wrong), while
       further changes are ignored (which is correct). As this bugfix breaks
       Zelda II (and introduces graphical bugs to Zelda I, and also breaks a
       few other levels like Alan Bond's "FMV"), allow the previous, incorrect
       behaviour for existing levels and tapes that make use of this bug */

    level->use_action_after_change_bug = TRUE;
  }

  /* not centering level after relocating player was default only in 3.2.3 */
  if (level->game_version == VERSION_IDENT(3,2,3,0))	/* (no pre-releases) */
    level->shifted_relocation = TRUE;

  /* EM style elements always chain-exploded in R'n'D engine before 3.2.6 */
  if (level->game_version < VERSION_IDENT(3,2,6,0))
    level->em_explodes_by_fire = TRUE;
}

static void LoadLevel_InitStandardElements(struct LevelInfo *level)
{
  int i, x, y;

  /* map elements that have changed in newer versions */
  level->amoeba_content = getMappedElementByVersion(level->amoeba_content,
						    level->game_version);
  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (x = 0; x < 3; x++)
      for (y = 0; y < 3; y++)
	level->yamyam_content[i].e[x][y] =
	  getMappedElementByVersion(level->yamyam_content[i].e[x][y],
				    level->game_version);

}

static void LoadLevel_InitCustomElements(struct LevelInfo *level)
{
  int i, j;

  /* map custom element change events that have changed in newer versions
     (these following values were accidentally changed in version 3.0.1)
     (this seems to be needed only for 'ab_levelset3' and 'ab_levelset4') */
  if (level->game_version <= VERSION_IDENT(3,0,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      /* order of checking and copying events to be mapped is important */
      /* (do not change the start and end value -- they are constant) */
      for (j = CE_BY_OTHER_ACTION; j >= CE_VALUE_GETS_ZERO; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 2))
	{
	  SET_CHANGE_EVENT(element, j - 2, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }

      /* order of checking and copying events to be mapped is important */
      /* (do not change the start and end value -- they are constant) */
      for (j = CE_PLAYER_COLLECTS_X; j >= CE_HITTING_SOMETHING; j--)
      {
	if (HAS_CHANGE_EVENT(element, j - 1))
	{
	  SET_CHANGE_EVENT(element, j - 1, FALSE);
	  SET_CHANGE_EVENT(element, j, TRUE);
	}
      }
    }
  }

  /* initialize "can_change" field for old levels with only one change page */
  if (level->game_version <= VERSION_IDENT(3,0,2,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (CAN_CHANGE(element))
	element_info[element].change->can_change = TRUE;
    }
  }

  /* correct custom element values (for old levels without these options) */
  if (level->game_version < VERSION_IDENT(3,1,1,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;
      struct ElementInfo *ei = &element_info[element];

      if (ei->access_direction == MV_NO_DIRECTION)
	ei->access_direction = MV_ALL_DIRECTIONS;
    }
  }

  /* correct custom element values (fix invalid values for all versions) */
  if (1)
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;
      struct ElementInfo *ei = &element_info[element];

      for (j = 0; j < ei->num_change_pages; j++)
      {
	struct ElementChangeInfo *change = &ei->change_page[j];

	if (change->trigger_player == CH_PLAYER_NONE)
	  change->trigger_player = CH_PLAYER_ANY;

	if (change->trigger_side == CH_SIDE_NONE)
	  change->trigger_side = CH_SIDE_ANY;
      }
    }
  }

  /* initialize "can_explode" field for old levels which did not store this */
  /* !!! CHECK THIS -- "<= 3,1,0,0" IS PROBABLY WRONG !!! */
  if (level->game_version <= VERSION_IDENT(3,1,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (EXPLODES_1X1_OLD(element))
	element_info[element].explosion_type = EXPLODES_1X1;

      SET_PROPERTY(element, EP_CAN_EXPLODE, (EXPLODES_BY_FIRE(element) ||
					     EXPLODES_SMASHED(element) ||
					     EXPLODES_IMPACT(element)));
    }
  }

  /* correct previously hard-coded move delay values for maze runner style */
  if (level->game_version < VERSION_IDENT(3,1,1,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      if (element_info[element].move_pattern & MV_MAZE_RUNNER_STYLE)
      {
	/* previously hard-coded and therefore ignored */
	element_info[element].move_delay_fixed = 9;
	element_info[element].move_delay_random = 0;
      }
    }
  }

  /* set some other uninitialized values of custom elements in older levels */
  if (level->game_version < VERSION_IDENT(3,1,0,0))
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      element_info[element].access_direction = MV_ALL_DIRECTIONS;

      element_info[element].explosion_delay = 17;
      element_info[element].ignition_delay = 8;
    }
  }
}

static void LoadLevel_InitElements(struct LevelInfo *level, char *filename)
{
  LoadLevel_InitStandardElements(level);

  if (level->file_has_custom_elements)
    LoadLevel_InitCustomElements(level);

  /* initialize element properties for level editor etc. */
  InitElementPropertiesEngine(level->game_version);
  InitElementPropertiesGfxElement();
}

static void LoadLevel_InitPlayfield(struct LevelInfo *level, char *filename)
{
  int x, y;

  /* map elements that have changed in newer versions */
  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      level->field[x][y] = getMappedElementByVersion(level->field[x][y],
						     level->game_version);

  /* clear unused playfield data (nicer if level gets resized in editor) */
  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      if (x >= level->fieldx || y >= level->fieldy)
	level->field[x][y] = EL_EMPTY;

  /* copy elements to runtime playfield array */
  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      Feld[x][y] = level->field[x][y];

  /* initialize level size variables for faster access */
  lev_fieldx = level->fieldx;
  lev_fieldy = level->fieldy;

  /* determine border element for this level */
  if (level->file_info.type == LEVEL_FILE_TYPE_DC)
    BorderElement = EL_EMPTY;	/* (in editor, SetBorderElement() is used) */
  else
    SetBorderElement();
}

static void LoadLevel_InitNativeEngines(struct LevelInfo *level,char *filename)
{
  struct LevelFileInfo *level_file_info = &level->file_info;

  if (level_file_info->type == LEVEL_FILE_TYPE_RND)
    CopyNativeLevel_RND_to_Native(level);
}

void LoadLevelTemplate(int nr)
{
  char *filename;

  setLevelFileInfo(&level_template.file_info, nr);
  filename = level_template.file_info.filename;

  LoadLevelFromFileInfo(&level_template, &level_template.file_info, FALSE);

  LoadLevel_InitVersion(&level_template, filename);
  LoadLevel_InitElements(&level_template, filename);

  ActivateLevelTemplate();
}

void LoadLevel(int nr)
{
  char *filename;

  setLevelFileInfo(&level.file_info, nr);
  filename = level.file_info.filename;

  LoadLevelFromFileInfo(&level, &level.file_info, FALSE);

  if (level.use_custom_template)
    LoadLevelTemplate(-1);

  LoadLevel_InitVersion(&level, filename);
  LoadLevel_InitElements(&level, filename);
  LoadLevel_InitPlayfield(&level, filename);

  LoadLevel_InitNativeEngines(&level, filename);
}

void LoadLevelInfoOnly(int nr)
{
  setLevelFileInfo(&level.file_info, nr);

  LoadLevelFromFileInfo(&level, &level.file_info, TRUE);
}

static int SaveLevel_VERS(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;

  chunk_size += putFileVersion(file, level->file_version);
  chunk_size += putFileVersion(file, level->game_version);

  return chunk_size;
}

static int SaveLevel_DATE(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;

  chunk_size += putFile16BitBE(file, level->creation_date.year);
  chunk_size += putFile8Bit(file,    level->creation_date.month);
  chunk_size += putFile8Bit(file,    level->creation_date.day);

  return chunk_size;
}

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_HEAD(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  putFile8Bit(file, level->fieldx);
  putFile8Bit(file, level->fieldy);

  putFile16BitBE(file, level->time);
  putFile16BitBE(file, level->gems_needed);

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    putFile8Bit(file, level->name[i]);

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    putFile8Bit(file, level->score[i]);

  for (i = 0; i < STD_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile8Bit(file, (level->encoding_16bit_yamyam ? EL_EMPTY :
			   level->yamyam_content[i].e[x][y]));
  putFile8Bit(file, level->amoeba_speed);
  putFile8Bit(file, level->time_magic_wall);
  putFile8Bit(file, level->time_wheel);
  putFile8Bit(file, (level->encoding_16bit_amoeba ? EL_EMPTY :
		     level->amoeba_content));
  putFile8Bit(file, (level->initial_player_stepsize == STEPSIZE_FAST ? 1 : 0));
  putFile8Bit(file, (level->initial_gravity ? 1 : 0));
  putFile8Bit(file, (level->encoding_16bit_field ? 1 : 0));
  putFile8Bit(file, (level->em_slippery_gems ? 1 : 0));

  putFile8Bit(file, (level->use_custom_template ? 1 : 0));

  putFile8Bit(file, (level->block_last_field ? 1 : 0));
  putFile8Bit(file, (level->sp_block_last_field ? 1 : 0));
  putFile32BitBE(file, level->can_move_into_acid_bits);
  putFile8Bit(file, level->dont_collide_with_bits);

  putFile8Bit(file, (level->use_spring_bug ? 1 : 0));
  putFile8Bit(file, (level->use_step_counter ? 1 : 0));

  putFile8Bit(file, (level->instant_relocation ? 1 : 0));
  putFile8Bit(file, (level->can_pass_to_walkable ? 1 : 0));
  putFile8Bit(file, (level->grow_into_diggable ? 1 : 0));

  putFile8Bit(file, level->game_engine_type);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_HEAD_UNUSED);
}
#endif

static int SaveLevel_NAME(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    chunk_size += putFile8Bit(file, level->name[i]);

  return chunk_size;
}

static int SaveLevel_AUTH(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    chunk_size += putFile8Bit(file, level->author[i]);

  return chunk_size;
}

#if ENABLE_HISTORIC_CHUNKS
static int SaveLevel_BODY(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int x, y;

  for (y = 0; y < level->fieldy; y++) 
    for (x = 0; x < level->fieldx; x++) 
      if (level->encoding_16bit_field)
	chunk_size += putFile16BitBE(file, level->field[x][y]);
      else
	chunk_size += putFile8Bit(file, level->field[x][y]);

  return chunk_size;
}
#endif

static int SaveLevel_BODY(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int x, y;

  for (y = 0; y < level->fieldy; y++) 
    for (x = 0; x < level->fieldx; x++) 
      chunk_size += putFile16BitBE(file, level->field[x][y]);

  return chunk_size;
}

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CONT(FILE *file, struct LevelInfo *level)
{
  int i, x, y;

  putFile8Bit(file, EL_YAMYAM);
  putFile8Bit(file, level->num_yamyam_contents);
  putFile8Bit(file, 0);
  putFile8Bit(file, 0);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	if (level->encoding_16bit_field)
	  putFile16BitBE(file, level->yamyam_content[i].e[x][y]);
	else
	  putFile8Bit(file, level->yamyam_content[i].e[x][y]);
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CNT2(FILE *file, struct LevelInfo *level, int element)
{
  int i, x, y;
  int num_contents, content_xsize, content_ysize;
  int content_array[MAX_ELEMENT_CONTENTS][3][3];

  if (element == EL_YAMYAM)
  {
    num_contents = level->num_yamyam_contents;
    content_xsize = 3;
    content_ysize = 3;

    for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  content_array[i][x][y] = level->yamyam_content[i].e[x][y];
  }
  else if (element == EL_BD_AMOEBA)
  {
    num_contents = 1;
    content_xsize = 1;
    content_ysize = 1;

    for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  content_array[i][x][y] = EL_EMPTY;
    content_array[0][0][0] = level->amoeba_content;
  }
  else
  {
    /* chunk header already written -- write empty chunk data */
    WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_SIZE);

    Error(ERR_WARN, "cannot save content for element '%d'", element);
    return;
  }

  putFile16BitBE(file, element);
  putFile8Bit(file, num_contents);
  putFile8Bit(file, content_xsize);
  putFile8Bit(file, content_ysize);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT2_UNUSED);

  for (i = 0; i < MAX_ELEMENT_CONTENTS; i++)
    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile16BitBE(file, content_array[i][x][y]);
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static int SaveLevel_CNT3(FILE *file, struct LevelInfo *level, int element)
{
  int envelope_nr = element - EL_ENVELOPE_1;
  int envelope_len = strlen(level->envelope_text[envelope_nr]) + 1;
  int chunk_size = 0;
  int i;

  chunk_size += putFile16BitBE(file, element);
  chunk_size += putFile16BitBE(file, envelope_len);
  chunk_size += putFile8Bit(file, level->envelope_xsize[envelope_nr]);
  chunk_size += putFile8Bit(file, level->envelope_ysize[envelope_nr]);

  WriteUnusedBytesToFile(file, LEVEL_CHUNK_CNT3_UNUSED);
  chunk_size += LEVEL_CHUNK_CNT3_UNUSED;

  for (i = 0; i < envelope_len; i++)
    chunk_size += putFile8Bit(file, level->envelope_text[envelope_nr][i]);

  return chunk_size;
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS1(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    struct ElementInfo *ei = &element_info[element];

    if (ei->properties[EP_BITFIELD_BASE_NR] != EP_BITMASK_DEFAULT)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);
	putFile32BitBE(file, ei->properties[EP_BITFIELD_BASE_NR]);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	/* should not happen */
    Error(ERR_WARN, "inconsistent number of custom element properties");
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS2(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;

    if (element_info[element].change->target_element != EL_EMPTY_SPACE)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);
	putFile16BitBE(file, element_info[element].change->target_element);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	/* should not happen */
    Error(ERR_WARN, "inconsistent number of custom target elements");
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS3(FILE *file, struct LevelInfo *level,
			   int num_changed_custom_elements)
{
  int i, j, x, y, check = 0;

  putFile16BitBE(file, num_changed_custom_elements);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int element = EL_CUSTOM_START + i;
    struct ElementInfo *ei = &element_info[element];

    if (ei->modified_settings)
    {
      if (check < num_changed_custom_elements)
      {
	putFile16BitBE(file, element);

	for (j = 0; j < MAX_ELEMENT_NAME_LEN; j++)
	  putFile8Bit(file, ei->description[j]);

	putFile32BitBE(file, ei->properties[EP_BITFIELD_BASE_NR]);

	/* some free bytes for future properties and padding */
	WriteUnusedBytesToFile(file, 7);

	putFile8Bit(file, ei->use_gfx_element);
	putFile16BitBE(file, ei->gfx_element_initial);

	putFile8Bit(file, ei->collect_score_initial);
	putFile8Bit(file, ei->collect_count_initial);

	putFile16BitBE(file, ei->push_delay_fixed);
	putFile16BitBE(file, ei->push_delay_random);
	putFile16BitBE(file, ei->move_delay_fixed);
	putFile16BitBE(file, ei->move_delay_random);

	putFile16BitBE(file, ei->move_pattern);
	putFile8Bit(file, ei->move_direction_initial);
	putFile8Bit(file, ei->move_stepsize);

	for (y = 0; y < 3; y++)
	  for (x = 0; x < 3; x++)
	    putFile16BitBE(file, ei->content.e[x][y]);

	putFile32BitBE(file, ei->change->events);

	putFile16BitBE(file, ei->change->target_element);

	putFile16BitBE(file, ei->change->delay_fixed);
	putFile16BitBE(file, ei->change->delay_random);
	putFile16BitBE(file, ei->change->delay_frames);

	putFile16BitBE(file, ei->change->initial_trigger_element);

	putFile8Bit(file, ei->change->explode);
	putFile8Bit(file, ei->change->use_target_content);
	putFile8Bit(file, ei->change->only_if_complete);
	putFile8Bit(file, ei->change->use_random_replace);

	putFile8Bit(file, ei->change->random_percentage);
	putFile8Bit(file, ei->change->replace_when);

	for (y = 0; y < 3; y++)
	  for (x = 0; x < 3; x++)
	    putFile16BitBE(file, ei->change->content.e[x][y]);

	putFile8Bit(file, ei->slippery_type);

	/* some free bytes for future properties and padding */
	WriteUnusedBytesToFile(file, LEVEL_CPART_CUS3_UNUSED);
      }

      check++;
    }
  }

  if (check != num_changed_custom_elements)	/* should not happen */
    Error(ERR_WARN, "inconsistent number of custom element properties");
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_CUS4(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  int i, j, x, y;

  /* ---------- custom element base property values (96 bytes) ------------- */

  putFile16BitBE(file, element);

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    putFile8Bit(file, ei->description[i]);

  putFile32BitBE(file, ei->properties[EP_BITFIELD_BASE_NR]);

  WriteUnusedBytesToFile(file, 4);	/* reserved for more base properties */

  putFile8Bit(file, ei->num_change_pages);

  putFile16BitBE(file, ei->ce_value_fixed_initial);
  putFile16BitBE(file, ei->ce_value_random_initial);
  putFile8Bit(file, ei->use_last_ce_value);

  putFile8Bit(file, ei->use_gfx_element);
  putFile16BitBE(file, ei->gfx_element_initial);

  putFile8Bit(file, ei->collect_score_initial);
  putFile8Bit(file, ei->collect_count_initial);

  putFile8Bit(file, ei->drop_delay_fixed);
  putFile8Bit(file, ei->push_delay_fixed);
  putFile8Bit(file, ei->drop_delay_random);
  putFile8Bit(file, ei->push_delay_random);
  putFile16BitBE(file, ei->move_delay_fixed);
  putFile16BitBE(file, ei->move_delay_random);

  /* bits 0 - 15 of "move_pattern" ... */
  putFile16BitBE(file, ei->move_pattern & 0xffff);
  putFile8Bit(file, ei->move_direction_initial);
  putFile8Bit(file, ei->move_stepsize);

  putFile8Bit(file, ei->slippery_type);

  for (y = 0; y < 3; y++)
    for (x = 0; x < 3; x++)
      putFile16BitBE(file, ei->content.e[x][y]);

  putFile16BitBE(file, ei->move_enter_element);
  putFile16BitBE(file, ei->move_leave_element);
  putFile8Bit(file, ei->move_leave_type);

  /* ... bits 16 - 31 of "move_pattern" (not nice, but downward compatible) */
  putFile16BitBE(file, (ei->move_pattern >> 16) & 0xffff);

  putFile8Bit(file, ei->access_direction);

  putFile8Bit(file, ei->explosion_delay);
  putFile8Bit(file, ei->ignition_delay);
  putFile8Bit(file, ei->explosion_type);

  /* some free bytes for future custom property values and padding */
  WriteUnusedBytesToFile(file, 1);

  /* ---------- change page property values (48 bytes) --------------------- */

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];
    unsigned int event_bits;

    /* bits 0 - 31 of "has_event[]" ... */
    event_bits = 0;
    for (j = 0; j < MIN(NUM_CHANGE_EVENTS, 32); j++)
      if (change->has_event[j])
	event_bits |= (1 << j);
    putFile32BitBE(file, event_bits);

    putFile16BitBE(file, change->target_element);

    putFile16BitBE(file, change->delay_fixed);
    putFile16BitBE(file, change->delay_random);
    putFile16BitBE(file, change->delay_frames);

    putFile16BitBE(file, change->initial_trigger_element);

    putFile8Bit(file, change->explode);
    putFile8Bit(file, change->use_target_content);
    putFile8Bit(file, change->only_if_complete);
    putFile8Bit(file, change->use_random_replace);

    putFile8Bit(file, change->random_percentage);
    putFile8Bit(file, change->replace_when);

    for (y = 0; y < 3; y++)
      for (x = 0; x < 3; x++)
	putFile16BitBE(file, change->target_content.e[x][y]);

    putFile8Bit(file, change->can_change);

    putFile8Bit(file, change->trigger_side);

    putFile8Bit(file, change->trigger_player);
    putFile8Bit(file, (change->trigger_page == CH_PAGE_ANY ? CH_PAGE_ANY_FILE :
		       log_2(change->trigger_page)));

    putFile8Bit(file, change->has_action);
    putFile8Bit(file, change->action_type);
    putFile8Bit(file, change->action_mode);
    putFile16BitBE(file, change->action_arg);

    /* ... bits 32 - 39 of "has_event[]" (not nice, but downward compatible) */
    event_bits = 0;
    for (j = 32; j < NUM_CHANGE_EVENTS; j++)
      if (change->has_event[j])
	event_bits |= (1 << (j - 32));
    putFile8Bit(file, event_bits);
  }
}
#endif

#if ENABLE_HISTORIC_CHUNKS
static void SaveLevel_GRP1(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  struct ElementGroupInfo *group = ei->group;
  int i;

  putFile16BitBE(file, element);

  for (i = 0; i < MAX_ELEMENT_NAME_LEN; i++)
    putFile8Bit(file, ei->description[i]);

  putFile8Bit(file, group->num_elements);

  putFile8Bit(file, ei->use_gfx_element);
  putFile16BitBE(file, ei->gfx_element_initial);

  putFile8Bit(file, group->choice_mode);

  /* some free bytes for future values and padding */
  WriteUnusedBytesToFile(file, 3);

  for (i = 0; i < MAX_ELEMENTS_IN_GROUP; i++)
    putFile16BitBE(file, group->element[i]);
}
#endif

static int SaveLevel_MicroChunk(FILE *file, struct LevelFileConfigInfo *entry,
				boolean write_element)
{
  int save_type = entry->save_type;
  int data_type = entry->data_type;
  int conf_type = entry->conf_type;
  int byte_mask = conf_type & CONF_MASK_BYTES;
  int element = entry->element;
  int default_value = entry->default_value;
  int num_bytes = 0;
  boolean modified = FALSE;

  if (byte_mask != CONF_MASK_MULTI_BYTES)
  {
    void *value_ptr = entry->value;
    int value = (data_type == TYPE_BOOLEAN ? *(boolean *)value_ptr :
		 *(int *)value_ptr);

    /* check if any settings have been modified before saving them */
    if (value != default_value)
      modified = TRUE;

    /* do not save if explicitly told or if unmodified default settings */
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += (byte_mask == CONF_MASK_1_BYTE ? putFile8Bit   (file, value) :
		  byte_mask == CONF_MASK_2_BYTE ? putFile16BitBE(file, value) :
		  byte_mask == CONF_MASK_4_BYTE ? putFile32BitBE(file, value) :
		  0);
  }
  else if (data_type == TYPE_STRING)
  {
    char *default_string = entry->default_string;
    char *string = (char *)(entry->value);
    int string_length = strlen(string);
    int i;

    /* check if any settings have been modified before saving them */
    if (!strEqual(string, default_string))
      modified = TRUE;

    /* do not save if explicitly told or if unmodified default settings */
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += putFile16BitBE(file, string_length);

    for (i = 0; i < string_length; i++)
      num_bytes += putFile8Bit(file, string[i]);
  }
  else if (data_type == TYPE_ELEMENT_LIST)
  {
    int *element_array = (int *)(entry->value);
    int num_elements = *(int *)(entry->num_entities);
    int i;

    /* check if any settings have been modified before saving them */
    for (i = 0; i < num_elements; i++)
      if (element_array[i] != default_value)
	modified = TRUE;

    /* do not save if explicitly told or if unmodified default settings */
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += putFile16BitBE(file, num_elements * CONF_ELEMENT_NUM_BYTES);

    for (i = 0; i < num_elements; i++)
      num_bytes += putFile16BitBE(file, element_array[i]);
  }
  else if (data_type == TYPE_CONTENT_LIST)
  {
    struct Content *content = (struct Content *)(entry->value);
    int num_contents = *(int *)(entry->num_entities);
    int i, x, y;

    /* check if any settings have been modified before saving them */
    for (i = 0; i < num_contents; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  if (content[i].e[x][y] != default_value)
	    modified = TRUE;

    /* do not save if explicitly told or if unmodified default settings */
    if ((save_type == SAVE_CONF_NEVER) ||
	(save_type == SAVE_CONF_WHEN_CHANGED && !modified))
      return 0;

    if (write_element)
      num_bytes += putFile16BitBE(file, element);

    num_bytes += putFile8Bit(file, conf_type);
    num_bytes += putFile16BitBE(file, num_contents * CONF_CONTENT_NUM_BYTES);

    for (i = 0; i < num_contents; i++)
      for (y = 0; y < 3; y++)
	for (x = 0; x < 3; x++)
	  num_bytes += putFile16BitBE(file, content[i].e[x][y]);
  }

  return num_bytes;
}

static int SaveLevel_INFO(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  li = *level;		/* copy level data into temporary buffer */

  for (i = 0; chunk_config_INFO[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_INFO[i], FALSE);

  return chunk_size;
}

static int SaveLevel_ELEM(FILE *file, struct LevelInfo *level)
{
  int chunk_size = 0;
  int i;

  li = *level;		/* copy level data into temporary buffer */

  for (i = 0; chunk_config_ELEM[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_ELEM[i], TRUE);

  return chunk_size;
}

static int SaveLevel_NOTE(FILE *file, struct LevelInfo *level, int element)
{
  int envelope_nr = element - EL_ENVELOPE_1;
  int chunk_size = 0;
  int i;

  chunk_size += putFile16BitBE(file, element);

  /* copy envelope data into temporary buffer */
  xx_envelope = level->envelope[envelope_nr];

  for (i = 0; chunk_config_NOTE[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_NOTE[i], FALSE);

  return chunk_size;
}

static int SaveLevel_CUSX(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  int chunk_size = 0;
  int i, j;

  chunk_size += putFile16BitBE(file, element);

  xx_ei = *ei;		/* copy element data into temporary buffer */

  /* set default description string for this specific element */
  strcpy(xx_default_description, getDefaultElementDescription(ei));

  for (i = 0; chunk_config_CUSX_base[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_CUSX_base[i], FALSE);

  for (i = 0; i < ei->num_change_pages; i++)
  {
    struct ElementChangeInfo *change = &ei->change_page[i];

    xx_current_change_page = i;

    xx_change = *change;	/* copy change data into temporary buffer */

    resetEventBits();
    setEventBitsFromEventFlags(change);

    for (j = 0; chunk_config_CUSX_change[j].data_type != -1; j++)
      chunk_size += SaveLevel_MicroChunk(file, &chunk_config_CUSX_change[j],
					 FALSE);
  }

  return chunk_size;
}

static int SaveLevel_GRPX(FILE *file, struct LevelInfo *level, int element)
{
  struct ElementInfo *ei = &element_info[element];
  struct ElementGroupInfo *group = ei->group;
  int chunk_size = 0;
  int i;

  chunk_size += putFile16BitBE(file, element);

  xx_ei = *ei;		/* copy element data into temporary buffer */
  xx_group = *group;	/* copy group data into temporary buffer */

  /* set default description string for this specific element */
  strcpy(xx_default_description, getDefaultElementDescription(ei));

  for (i = 0; chunk_config_GRPX[i].data_type != -1; i++)
    chunk_size += SaveLevel_MicroChunk(file, &chunk_config_GRPX[i], FALSE);

  return chunk_size;
}

static void SaveLevelFromFilename(struct LevelInfo *level, char *filename,
				  boolean save_as_template)
{
  int chunk_size;
  int i;
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level file '%s'", filename);
    return;
  }

  level->file_version = FILE_VERSION_ACTUAL;
  level->game_version = GAME_VERSION_ACTUAL;

  level->creation_date = getCurrentDate();

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "CAVE", CHUNK_SIZE_NONE);

  chunk_size = SaveLevel_VERS(NULL, level);
  putFileChunkBE(file, "VERS", chunk_size);
  SaveLevel_VERS(file, level);

  chunk_size = SaveLevel_DATE(NULL, level);
  putFileChunkBE(file, "DATE", chunk_size);
  SaveLevel_DATE(file, level);

  chunk_size = SaveLevel_NAME(NULL, level);
  putFileChunkBE(file, "NAME", chunk_size);
  SaveLevel_NAME(file, level);

  chunk_size = SaveLevel_AUTH(NULL, level);
  putFileChunkBE(file, "AUTH", chunk_size);
  SaveLevel_AUTH(file, level);

  chunk_size = SaveLevel_INFO(NULL, level);
  putFileChunkBE(file, "INFO", chunk_size);
  SaveLevel_INFO(file, level);

  chunk_size = SaveLevel_BODY(NULL, level);
  putFileChunkBE(file, "BODY", chunk_size);
  SaveLevel_BODY(file, level);

  chunk_size = SaveLevel_ELEM(NULL, level);
  if (chunk_size > LEVEL_CHUNK_ELEM_UNCHANGED)		/* save if changed */
  {
    putFileChunkBE(file, "ELEM", chunk_size);
    SaveLevel_ELEM(file, level);
  }

  for (i = 0; i < NUM_ENVELOPES; i++)
  {
    int element = EL_ENVELOPE_1 + i;

    chunk_size = SaveLevel_NOTE(NULL, level, element);
    if (chunk_size > LEVEL_CHUNK_NOTE_UNCHANGED)	/* save if changed */
    {
      putFileChunkBE(file, "NOTE", chunk_size);
      SaveLevel_NOTE(file, level, element);
    }
  }

  /* if not using template level, check for non-default custom/group elements */
  if (!level->use_custom_template || save_as_template)
  {
    for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
    {
      int element = EL_CUSTOM_START + i;

      chunk_size = SaveLevel_CUSX(NULL, level, element);
      if (chunk_size > LEVEL_CHUNK_CUSX_UNCHANGED)	/* save if changed */
      {
	putFileChunkBE(file, "CUSX", chunk_size);
	SaveLevel_CUSX(file, level, element);
      }
    }

    for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
    {
      int element = EL_GROUP_START + i;

      chunk_size = SaveLevel_GRPX(NULL, level, element);
      if (chunk_size > LEVEL_CHUNK_GRPX_UNCHANGED)	/* save if changed */
      {
	putFileChunkBE(file, "GRPX", chunk_size);
	SaveLevel_GRPX(file, level, element);
      }
    }
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void SaveLevel(int nr)
{
  char *filename = getDefaultLevelFilename(nr);

  SaveLevelFromFilename(&level, filename, FALSE);
}

void SaveLevelTemplate()
{
  char *filename = getLocalLevelTemplateFilename();

  SaveLevelFromFilename(&level, filename, TRUE);
}

boolean SaveLevelChecked(int nr)
{
  char *filename = getDefaultLevelFilename(nr);
  boolean new_level = !fileExists(filename);
  boolean level_saved = FALSE;

  if (new_level || Request("Save this level and kill the old?", REQ_ASK))
  {
    SaveLevel(nr);

    if (new_level)
      Request("Level saved!", REQ_CONFIRM);

    level_saved = TRUE;
  }

  return level_saved;
}

void DumpLevel(struct LevelInfo *level)
{
  if (level->no_level_file || level->no_valid_file)
  {
    Error(ERR_WARN, "cannot dump -- no valid level file found");

    return;
  }

  PrintLine("-", 79);
  Print("Level xxx (file version %08d, game version %08d)\n",
	level->file_version, level->game_version);
  PrintLine("-", 79);

  Print("Level author: '%s'\n", level->author);
  Print("Level title:  '%s'\n", level->name);
  Print("\n");
  Print("Playfield size: %d x %d\n", level->fieldx, level->fieldy);
  Print("\n");
  Print("Level time:  %d seconds\n", level->time);
  Print("Gems needed: %d\n", level->gems_needed);
  Print("\n");
  Print("Time for magic wall: %d seconds\n", level->time_magic_wall);
  Print("Time for wheel:      %d seconds\n", level->time_wheel);
  Print("Time for light:      %d seconds\n", level->time_light);
  Print("Time for timegate:   %d seconds\n", level->time_timegate);
  Print("\n");
  Print("Amoeba speed: %d\n", level->amoeba_speed);
  Print("\n");

  Print("EM style slippery gems:      %s\n", (level->em_slippery_gems ? "yes" : "no"));
  Print("Player blocks last field:    %s\n", (level->block_last_field ? "yes" : "no"));
  Print("SP player blocks last field: %s\n", (level->sp_block_last_field ? "yes" : "no"));
  Print("use spring bug: %s\n", (level->use_spring_bug ? "yes" : "no"));
  Print("use step counter: %s\n", (level->use_step_counter ? "yes" : "no"));

  PrintLine("-", 79);
}


/* ========================================================================= */
/* tape file functions                                                       */
/* ========================================================================= */

static void setTapeInfoToDefaults()
{
  int i;

  /* always start with reliable default values (empty tape) */
  TapeErase();

  /* default values (also for pre-1.2 tapes) with only the first player */
  tape.player_participates[0] = TRUE;
  for (i = 1; i < MAX_PLAYERS; i++)
    tape.player_participates[i] = FALSE;

  /* at least one (default: the first) player participates in every tape */
  tape.num_participating_players = 1;

  tape.level_nr = level_nr;
  tape.counter = 0;
  tape.changed = FALSE;

  tape.recording = FALSE;
  tape.playing = FALSE;
  tape.pausing = FALSE;

  tape.no_valid_file = FALSE;
}

static int LoadTape_VERS(File *file, int chunk_size, struct TapeInfo *tape)
{
  tape->file_version = getFileVersion(file);
  tape->game_version = getFileVersion(file);

  return chunk_size;
}

static int LoadTape_HEAD(File *file, int chunk_size, struct TapeInfo *tape)
{
  int i;

  tape->random_seed = getFile32BitBE(file);
  tape->date        = getFile32BitBE(file);
  tape->length      = getFile32BitBE(file);

  /* read header fields that are new since version 1.2 */
  if (tape->file_version >= FILE_VERSION_1_2)
  {
    byte store_participating_players = getFile8Bit(file);
    int engine_version;

    /* since version 1.2, tapes store which players participate in the tape */
    tape->num_participating_players = 0;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
      tape->player_participates[i] = FALSE;

      if (store_participating_players & (1 << i))
      {
	tape->player_participates[i] = TRUE;
	tape->num_participating_players++;
      }
    }

    tape->use_mouse = (getFile8Bit(file) == 1 ? TRUE : FALSE);

    ReadUnusedBytesFromFile(file, TAPE_CHUNK_HEAD_UNUSED);

    engine_version = getFileVersion(file);
    if (engine_version > 0)
      tape->engine_version = engine_version;
    else
      tape->engine_version = tape->game_version;
  }

  return chunk_size;
}

static int LoadTape_INFO(File *file, int chunk_size, struct TapeInfo *tape)
{
  int level_identifier_size;
  int i;

  level_identifier_size = getFile16BitBE(file);

  tape->level_identifier =
    checked_realloc(tape->level_identifier, level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    tape->level_identifier[i] = getFile8Bit(file);

  tape->level_nr = getFile16BitBE(file);

  chunk_size = 2 + level_identifier_size + 2;

  return chunk_size;
}

static int LoadTape_BODY(File *file, int chunk_size, struct TapeInfo *tape)
{
  int i, j;
  int tape_pos_size =
    (tape->use_mouse ? 3 : tape->num_participating_players) + 1;
  int chunk_size_expected = tape_pos_size * tape->length;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);
    return chunk_size_expected;
  }

  for (i = 0; i < tape->length; i++)
  {
    if (i >= MAX_TAPE_LEN)
    {
      Error(ERR_WARN, "tape truncated -- size exceeds maximum tape size %d",
	    MAX_TAPE_LEN);

      // tape too large; read and ignore remaining tape data from this chunk
      for (;i < tape->length; i++)
	ReadUnusedBytesFromFile(file, tape->num_participating_players + 1);

      break;
    }

    if (tape->use_mouse)
    {
      tape->pos[i].action[TAPE_ACTION_LX]     = getFile8Bit(file);
      tape->pos[i].action[TAPE_ACTION_LY]     = getFile8Bit(file);
      tape->pos[i].action[TAPE_ACTION_BUTTON] = getFile8Bit(file);

      tape->pos[i].action[TAPE_ACTION_UNUSED] = 0;
    }
    else
    {
      for (j = 0; j < MAX_PLAYERS; j++)
      {
	tape->pos[i].action[j] = MV_NONE;

	if (tape->player_participates[j])
	  tape->pos[i].action[j] = getFile8Bit(file);
      }
    }

    tape->pos[i].delay = getFile8Bit(file);

    if (tape->file_version == FILE_VERSION_1_0)
    {
      /* eliminate possible diagonal moves in old tapes */
      /* this is only for backward compatibility */

      byte joy_dir[4] = { JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN };
      byte action = tape->pos[i].action[0];
      int k, num_moves = 0;

      for (k = 0; k<4; k++)
      {
	if (action & joy_dir[k])
	{
	  tape->pos[i + num_moves].action[0] = joy_dir[k];
	  if (num_moves > 0)
	    tape->pos[i + num_moves].delay = 0;
	  num_moves++;
	}
      }

      if (num_moves > 1)
      {
	num_moves--;
	i += num_moves;
	tape->length += num_moves;
      }
    }
    else if (tape->file_version < FILE_VERSION_2_0)
    {
      /* convert pre-2.0 tapes to new tape format */

      if (tape->pos[i].delay > 1)
      {
	/* action part */
	tape->pos[i + 1] = tape->pos[i];
	tape->pos[i + 1].delay = 1;

	/* delay part */
	for (j = 0; j < MAX_PLAYERS; j++)
	  tape->pos[i].action[j] = MV_NONE;
	tape->pos[i].delay--;

	i++;
	tape->length++;
      }
    }

    if (checkEndOfFile(file))
      break;
  }

  if (i != tape->length)
    chunk_size = tape_pos_size * i;

  return chunk_size;
}

void LoadTape_SokobanSolution(char *filename)
{
  File *file;
  int move_delay = TILESIZE / level.initial_player_stepsize[0];

  if (!(file = openFile(filename, MODE_READ)))
  {
    tape.no_valid_file = TRUE;

    return;
  }

  while (!checkEndOfFile(file))
  {
    unsigned char c = getByteFromFile(file);

    if (checkEndOfFile(file))
      break;

    switch (c)
    {
      case 'u':
      case 'U':
	tape.pos[tape.length].action[0] = MV_UP;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case 'd':
      case 'D':
	tape.pos[tape.length].action[0] = MV_DOWN;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case 'l':
      case 'L':
	tape.pos[tape.length].action[0] = MV_LEFT;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case 'r':
      case 'R':
	tape.pos[tape.length].action[0] = MV_RIGHT;
	tape.pos[tape.length].delay = move_delay + (c < 'a' ? 2 : 0);
	tape.length++;
	break;

      case '\n':
      case '\r':
      case '\t':
      case ' ':
	/* ignore white-space characters */
	break;

      default:
	tape.no_valid_file = TRUE;

	Error(ERR_WARN, "unsupported Sokoban solution file '%s' ['%d']", filename, c);

	break;
    }
  }

  closeFile(file);

  if (tape.no_valid_file)
    return;

  tape.length_frames  = GetTapeLengthFrames();
  tape.length_seconds = GetTapeLengthSeconds();
}

void LoadTapeFromFilename(char *filename)
{
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  File *file;
  int chunk_size;

  /* always start with reliable default values */
  setTapeInfoToDefaults();

  if (strSuffix(filename, ".sln"))
  {
    LoadTape_SokobanSolution(filename);

    return;
  }

  if (!(file = openFile(filename, MODE_READ)))
  {
    tape.no_valid_file = TRUE;

    return;
  }

  getFileChunkBE(file, chunk_name, NULL);
  if (strEqual(chunk_name, "RND1"))
  {
    getFile32BitBE(file);		/* not used */

    getFileChunkBE(file, chunk_name, NULL);
    if (!strEqual(chunk_name, "TAPE"))
    {
      tape.no_valid_file = TRUE;

      Error(ERR_WARN, "unknown format of tape file '%s'", filename);

      closeFile(file);

      return;
    }
  }
  else	/* check for pre-2.0 file format with cookie string */
  {
    strcpy(cookie, chunk_name);
    if (getStringFromFile(file, &cookie[4], MAX_LINE_LEN - 4) == NULL)
      cookie[4] = '\0';
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, TAPE_COOKIE_TMPL))
    {
      tape.no_valid_file = TRUE;

      Error(ERR_WARN, "unknown format of tape file '%s'", filename);

      closeFile(file);

      return;
    }

    if ((tape.file_version = getFileVersionFromCookieString(cookie)) == -1)
    {
      tape.no_valid_file = TRUE;

      Error(ERR_WARN, "unsupported version of tape file '%s'", filename);

      closeFile(file);

      return;
    }

    /* pre-2.0 tape files have no game version, so use file version here */
    tape.game_version = tape.file_version;
  }

  if (tape.file_version < FILE_VERSION_1_2)
  {
    /* tape files from versions before 1.2.0 without chunk structure */
    LoadTape_HEAD(file, TAPE_CHUNK_HEAD_SIZE, &tape);
    LoadTape_BODY(file, 2 * tape.length,      &tape);
  }
  else
  {
    static struct
    {
      char *name;
      int size;
      int (*loader)(File *, int, struct TapeInfo *);
    }
    chunk_info[] =
    {
      { "VERS", TAPE_CHUNK_VERS_SIZE,	LoadTape_VERS },
      { "HEAD", TAPE_CHUNK_HEAD_SIZE,	LoadTape_HEAD },
      { "INFO", -1,			LoadTape_INFO },
      { "BODY", -1,			LoadTape_BODY },
      {  NULL,  0,			NULL }
    };

    while (getFileChunkBE(file, chunk_name, &chunk_size))
    {
      int i = 0;

      while (chunk_info[i].name != NULL &&
	     !strEqual(chunk_name, chunk_info[i].name))
	i++;

      if (chunk_info[i].name == NULL)
      {
	Error(ERR_WARN, "unknown chunk '%s' in tape file '%s'",
	      chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else if (chunk_info[i].size != -1 &&
	       chunk_info[i].size != chunk_size)
      {
	Error(ERR_WARN, "wrong size (%d) of chunk '%s' in tape file '%s'",
	      chunk_size, chunk_name, filename);
	ReadUnusedBytesFromFile(file, chunk_size);
      }
      else
      {
	/* call function to load this tape chunk */
	int chunk_size_expected =
	  (chunk_info[i].loader)(file, chunk_size, &tape);

	/* the size of some chunks cannot be checked before reading other
	   chunks first (like "HEAD" and "BODY") that contain some header
	   information, so check them here */
	if (chunk_size_expected != chunk_size)
	{
	  Error(ERR_WARN, "wrong size (%d) of chunk '%s' in tape file '%s'",
		chunk_size, chunk_name, filename);
	}
      }
    }
  }

  closeFile(file);

  tape.length_frames  = GetTapeLengthFrames();
  tape.length_seconds = GetTapeLengthSeconds();

#if 0
  printf("::: tape file version: %d\n",   tape.file_version);
  printf("::: tape game version: %d\n",   tape.game_version);
  printf("::: tape engine version: %d\n", tape.engine_version);
#endif
}

void LoadTape(int nr)
{
  char *filename = getTapeFilename(nr);

  LoadTapeFromFilename(filename);
}

void LoadSolutionTape(int nr)
{
  char *filename = getSolutionTapeFilename(nr);

  LoadTapeFromFilename(filename);

  if (TAPE_IS_EMPTY(tape) &&
      level.game_engine_type == GAME_ENGINE_TYPE_SP &&
      level.native_sp_level->demo.is_available)
    CopyNativeTape_SP_to_RND(&level);
}

static void SaveTape_VERS(FILE *file, struct TapeInfo *tape)
{
  putFileVersion(file, tape->file_version);
  putFileVersion(file, tape->game_version);
}

static void SaveTape_HEAD(FILE *file, struct TapeInfo *tape)
{
  int i;
  byte store_participating_players = 0;

  /* set bits for participating players for compact storage */
  for (i = 0; i < MAX_PLAYERS; i++)
    if (tape->player_participates[i])
      store_participating_players |= (1 << i);

  putFile32BitBE(file, tape->random_seed);
  putFile32BitBE(file, tape->date);
  putFile32BitBE(file, tape->length);

  putFile8Bit(file, store_participating_players);

  putFile8Bit(file, (tape->use_mouse ? 1 : 0));

  /* unused bytes not at the end here for 4-byte alignment of engine_version */
  WriteUnusedBytesToFile(file, TAPE_CHUNK_HEAD_UNUSED);

  putFileVersion(file, tape->engine_version);
}

static void SaveTape_INFO(FILE *file, struct TapeInfo *tape)
{
  int level_identifier_size = strlen(tape->level_identifier) + 1;
  int i;

  putFile16BitBE(file, level_identifier_size);

  for (i = 0; i < level_identifier_size; i++)
    putFile8Bit(file, tape->level_identifier[i]);

  putFile16BitBE(file, tape->level_nr);
}

static void SaveTape_BODY(FILE *file, struct TapeInfo *tape)
{
  int i, j;

  for (i = 0; i < tape->length; i++)
  {
    if (tape->use_mouse)
    {
      putFile8Bit(file, tape->pos[i].action[TAPE_ACTION_LX]);
      putFile8Bit(file, tape->pos[i].action[TAPE_ACTION_LY]);
      putFile8Bit(file, tape->pos[i].action[TAPE_ACTION_BUTTON]);
    }
    else
    {
      for (j = 0; j < MAX_PLAYERS; j++)
	if (tape->player_participates[j])
	  putFile8Bit(file, tape->pos[i].action[j]);
    }

    putFile8Bit(file, tape->pos[i].delay);
  }
}

void SaveTape(int nr)
{
  char *filename = getTapeFilename(nr);
  FILE *file;
  int num_participating_players = 0;
  int tape_pos_size;
  int info_chunk_size;
  int body_chunk_size;
  int i;

  InitTapeDirectory(leveldir_current->subdir);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level recording file '%s'", filename);
    return;
  }

  tape.file_version = FILE_VERSION_ACTUAL;
  tape.game_version = GAME_VERSION_ACTUAL;

  /* count number of participating players  */
  for (i = 0; i < MAX_PLAYERS; i++)
    if (tape.player_participates[i])
      num_participating_players++;

  tape_pos_size = (tape.use_mouse ? 3 : num_participating_players) + 1;

  info_chunk_size = 2 + (strlen(tape.level_identifier) + 1) + 2;
  body_chunk_size = tape_pos_size * tape.length;

  putFileChunkBE(file, "RND1", CHUNK_SIZE_UNDEFINED);
  putFileChunkBE(file, "TAPE", CHUNK_SIZE_NONE);

  putFileChunkBE(file, "VERS", TAPE_CHUNK_VERS_SIZE);
  SaveTape_VERS(file, &tape);

  putFileChunkBE(file, "HEAD", TAPE_CHUNK_HEAD_SIZE);
  SaveTape_HEAD(file, &tape);

  putFileChunkBE(file, "INFO", info_chunk_size);
  SaveTape_INFO(file, &tape);

  putFileChunkBE(file, "BODY", body_chunk_size);
  SaveTape_BODY(file, &tape);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  tape.changed = FALSE;
}

static boolean SaveTapeCheckedExt(int nr, char *msg_replace, char *msg_saved)
{
  char *filename = getTapeFilename(nr);
  boolean new_tape = !fileExists(filename);
  boolean tape_saved = FALSE;

  if (new_tape || Request(msg_replace, REQ_ASK))
  {
    SaveTape(nr);

    if (new_tape)
      Request(msg_saved, REQ_CONFIRM);

    tape_saved = TRUE;
  }

  return tape_saved;
}

boolean SaveTapeChecked(int nr)
{
  return SaveTapeCheckedExt(nr, "Replace old tape?", "Tape saved!");
}

boolean SaveTapeChecked_LevelSolved(int nr)
{
  return SaveTapeCheckedExt(nr, "Level solved! Replace old tape?",
			        "Level solved! Tape saved!");
}

void DumpTape(struct TapeInfo *tape)
{
  int tape_frame_counter;
  int i, j;

  if (tape->no_valid_file)
  {
    Error(ERR_WARN, "cannot dump -- no valid tape file found");

    return;
  }

  PrintLine("-", 79);
  Print("Tape of Level %03d (file version %08d, game version %08d)\n",
	tape->level_nr, tape->file_version, tape->game_version);
  Print("                  (effective engine version %08d)\n",
	tape->engine_version);
  Print("Level series identifier: '%s'\n", tape->level_identifier);
  PrintLine("-", 79);

  tape_frame_counter = 0;

  for (i = 0; i < tape->length; i++)
  {
    if (i >= MAX_TAPE_LEN)
      break;

    Print("%04d: ", i);

    for (j = 0; j < MAX_PLAYERS; j++)
    {
      if (tape->player_participates[j])
      {
	int action = tape->pos[i].action[j];

	Print("%d:%02x ", j, action);
	Print("[%c%c%c%c|%c%c] - ",
	      (action & JOY_LEFT ? '<' : ' '),
	      (action & JOY_RIGHT ? '>' : ' '),
	      (action & JOY_UP ? '^' : ' '),
	      (action & JOY_DOWN ? 'v' : ' '),
	      (action & JOY_BUTTON_1 ? '1' : ' '),
	      (action & JOY_BUTTON_2 ? '2' : ' '));
      }
    }

    Print("(%03d) ", tape->pos[i].delay);
    Print("[%05d]\n", tape_frame_counter);

    tape_frame_counter += tape->pos[i].delay;
  }

  PrintLine("-", 79);
}


/* ========================================================================= */
/* score file functions                                                      */
/* ========================================================================= */

void LoadScore(int nr)
{
  int i;
  char *filename = getScoreFilename(nr);
  char cookie[MAX_LINE_LEN];
  char line[MAX_LINE_LEN];
  char *line_ptr;
  FILE *file;

  /* always start with reliable default values */
  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    strcpy(highscore[i].Name, EMPTY_PLAYER_NAME);
    highscore[i].Score = 0;
  }

  if (!(file = fopen(filename, MODE_READ)))
    return;

  /* check file identifier */
  if (fgets(cookie, MAX_LINE_LEN, file) == NULL)
    cookie[0] = '\0';
  if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
    cookie[strlen(cookie) - 1] = '\0';

  if (!checkCookieString(cookie, SCORE_COOKIE))
  {
    Error(ERR_WARN, "unknown format of score file '%s'", filename);
    fclose(file);
    return;
  }

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
  {
    if (fscanf(file, "%d", &highscore[i].Score) == EOF)
      Error(ERR_WARN, "fscanf() failed; %s", strerror(errno));
    if (fgets(line, MAX_LINE_LEN, file) == NULL)
      line[0] = '\0';

    if (strlen(line) > 0 && line[strlen(line) - 1] == '\n')
      line[strlen(line) - 1] = '\0';

    for (line_ptr = line; *line_ptr; line_ptr++)
    {
      if (*line_ptr != ' ' && *line_ptr != '\t' && *line_ptr != '\0')
      {
	strncpy(highscore[i].Name, line_ptr, MAX_PLAYER_NAME_LEN);
	highscore[i].Name[MAX_PLAYER_NAME_LEN] = '\0';
	break;
      }
    }
  }

  fclose(file);
}

void SaveScore(int nr)
{
  int i;
  int permissions = (program.global_scores ? PERMS_PUBLIC : PERMS_PRIVATE);
  char *filename = getScoreFilename(nr);
  FILE *file;

  InitScoreDirectory(leveldir_current->subdir);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save score for level %d", nr);
    return;
  }

  fprintf(file, "%s\n\n", SCORE_COOKIE);

  for (i = 0; i < MAX_SCORE_ENTRIES; i++)
    fprintf(file, "%d %s\n", highscore[i].Score, highscore[i].Name);

  fclose(file);

  SetFilePermissions(filename, permissions);
}


/* ========================================================================= */
/* setup file functions                                                      */
/* ========================================================================= */

#define TOKEN_STR_PLAYER_PREFIX			"player_"

/* global setup */
#define SETUP_TOKEN_PLAYER_NAME			0
#define SETUP_TOKEN_SOUND			1
#define SETUP_TOKEN_SOUND_LOOPS			2
#define SETUP_TOKEN_SOUND_MUSIC			3
#define SETUP_TOKEN_SOUND_SIMPLE		4
#define SETUP_TOKEN_TOONS			5
#define SETUP_TOKEN_SCROLL_DELAY		6
#define SETUP_TOKEN_SCROLL_DELAY_VALUE		7
#define SETUP_TOKEN_ENGINE_SNAPSHOT_MODE	8
#define SETUP_TOKEN_ENGINE_SNAPSHOT_MEMORY	9
#define SETUP_TOKEN_FADE_SCREENS		10
#define SETUP_TOKEN_AUTORECORD			11
#define SETUP_TOKEN_SHOW_TITLESCREEN		12
#define SETUP_TOKEN_QUICK_DOORS			13
#define SETUP_TOKEN_TEAM_MODE			14
#define SETUP_TOKEN_HANDICAP			15
#define SETUP_TOKEN_SKIP_LEVELS			16
#define SETUP_TOKEN_INCREMENT_LEVELS		17
#define SETUP_TOKEN_TIME_LIMIT			18
#define SETUP_TOKEN_FULLSCREEN			19
#define SETUP_TOKEN_WINDOW_SCALING_PERCENT	20
#define SETUP_TOKEN_WINDOW_SCALING_QUALITY	21
#define SETUP_TOKEN_SCREEN_RENDERING_MODE	22
#define SETUP_TOKEN_ASK_ON_ESCAPE		23
#define SETUP_TOKEN_ASK_ON_ESCAPE_EDITOR	24
#define SETUP_TOKEN_QUICK_SWITCH		25
#define SETUP_TOKEN_INPUT_ON_FOCUS		26
#define SETUP_TOKEN_PREFER_AGA_GRAPHICS		27
#define SETUP_TOKEN_GAME_FRAME_DELAY		28
#define SETUP_TOKEN_SP_SHOW_BORDER_ELEMENTS	29
#define SETUP_TOKEN_SMALL_GAME_GRAPHICS		30
#define SETUP_TOKEN_SHOW_SNAPSHOT_BUTTONS	31
#define SETUP_TOKEN_GRAPHICS_SET		32
#define SETUP_TOKEN_SOUNDS_SET			33
#define SETUP_TOKEN_MUSIC_SET			34
#define SETUP_TOKEN_OVERRIDE_LEVEL_GRAPHICS	35
#define SETUP_TOKEN_OVERRIDE_LEVEL_SOUNDS	36
#define SETUP_TOKEN_OVERRIDE_LEVEL_MUSIC	37
#define SETUP_TOKEN_VOLUME_SIMPLE		38
#define SETUP_TOKEN_VOLUME_LOOPS		39
#define SETUP_TOKEN_VOLUME_MUSIC		40
#define SETUP_TOKEN_TOUCH_CONTROL_TYPE		41
#define SETUP_TOKEN_TOUCH_MOVE_DISTANCE		42
#define SETUP_TOKEN_TOUCH_DROP_DISTANCE		43

#define NUM_GLOBAL_SETUP_TOKENS			44

/* auto setup */
#define SETUP_TOKEN_AUTO_EDITOR_ZOOM_TILESIZE	0

#define NUM_AUTO_SETUP_TOKENS			1

/* editor setup */
#define SETUP_TOKEN_EDITOR_EL_CLASSIC		0
#define SETUP_TOKEN_EDITOR_EL_CUSTOM		1
#define SETUP_TOKEN_EDITOR_EL_USER_DEFINED	2
#define SETUP_TOKEN_EDITOR_EL_DYNAMIC		3
#define SETUP_TOKEN_EDITOR_EL_HEADLINES		4
#define SETUP_TOKEN_EDITOR_SHOW_ELEMENT_TOKEN	5

#define NUM_EDITOR_SETUP_TOKENS			6

/* editor cascade setup */
#define SETUP_TOKEN_EDITOR_CASCADE_BD		0
#define SETUP_TOKEN_EDITOR_CASCADE_EM		1
#define SETUP_TOKEN_EDITOR_CASCADE_EMC		2
#define SETUP_TOKEN_EDITOR_CASCADE_RND		3
#define SETUP_TOKEN_EDITOR_CASCADE_SB		4
#define SETUP_TOKEN_EDITOR_CASCADE_SP		5
#define SETUP_TOKEN_EDITOR_CASCADE_DC		6
#define SETUP_TOKEN_EDITOR_CASCADE_DX		7
#define SETUP_TOKEN_EDITOR_CASCADE_TEXT		8
#define SETUP_TOKEN_EDITOR_CASCADE_STEELTEXT	9
#define SETUP_TOKEN_EDITOR_CASCADE_CE		10
#define SETUP_TOKEN_EDITOR_CASCADE_GE		11
#define SETUP_TOKEN_EDITOR_CASCADE_REF		12
#define SETUP_TOKEN_EDITOR_CASCADE_USER		13
#define SETUP_TOKEN_EDITOR_CASCADE_DYNAMIC	14

#define NUM_EDITOR_CASCADE_SETUP_TOKENS		15

/* shortcut setup */
#define SETUP_TOKEN_SHORTCUT_SAVE_GAME		0
#define SETUP_TOKEN_SHORTCUT_LOAD_GAME		1
#define SETUP_TOKEN_SHORTCUT_TOGGLE_PAUSE	2
#define SETUP_TOKEN_SHORTCUT_FOCUS_PLAYER_1	3
#define SETUP_TOKEN_SHORTCUT_FOCUS_PLAYER_2	4
#define SETUP_TOKEN_SHORTCUT_FOCUS_PLAYER_3	5
#define SETUP_TOKEN_SHORTCUT_FOCUS_PLAYER_4	6
#define SETUP_TOKEN_SHORTCUT_FOCUS_PLAYER_ALL	7
#define SETUP_TOKEN_SHORTCUT_TAPE_EJECT		8
#define SETUP_TOKEN_SHORTCUT_TAPE_EXTRA		9
#define SETUP_TOKEN_SHORTCUT_TAPE_STOP		10
#define SETUP_TOKEN_SHORTCUT_TAPE_PAUSE		11
#define SETUP_TOKEN_SHORTCUT_TAPE_RECORD	12
#define SETUP_TOKEN_SHORTCUT_TAPE_PLAY		13
#define SETUP_TOKEN_SHORTCUT_SOUND_SIMPLE	14
#define SETUP_TOKEN_SHORTCUT_SOUND_LOOPS	15
#define SETUP_TOKEN_SHORTCUT_SOUND_MUSIC	16
#define SETUP_TOKEN_SHORTCUT_SNAP_LEFT		17
#define SETUP_TOKEN_SHORTCUT_SNAP_RIGHT		18
#define SETUP_TOKEN_SHORTCUT_SNAP_UP		19
#define SETUP_TOKEN_SHORTCUT_SNAP_DOWN		20

#define NUM_SHORTCUT_SETUP_TOKENS		21

/* player setup */
#define SETUP_TOKEN_PLAYER_USE_JOYSTICK		0
#define SETUP_TOKEN_PLAYER_JOY_DEVICE_NAME	1
#define SETUP_TOKEN_PLAYER_JOY_XLEFT		2
#define SETUP_TOKEN_PLAYER_JOY_XMIDDLE		3
#define SETUP_TOKEN_PLAYER_JOY_XRIGHT		4
#define SETUP_TOKEN_PLAYER_JOY_YUPPER		5
#define SETUP_TOKEN_PLAYER_JOY_YMIDDLE		6
#define SETUP_TOKEN_PLAYER_JOY_YLOWER		7
#define SETUP_TOKEN_PLAYER_JOY_SNAP		8
#define SETUP_TOKEN_PLAYER_JOY_DROP		9
#define SETUP_TOKEN_PLAYER_KEY_LEFT		10
#define SETUP_TOKEN_PLAYER_KEY_RIGHT		11
#define SETUP_TOKEN_PLAYER_KEY_UP		12
#define SETUP_TOKEN_PLAYER_KEY_DOWN		13
#define SETUP_TOKEN_PLAYER_KEY_SNAP		14
#define SETUP_TOKEN_PLAYER_KEY_DROP		15

#define NUM_PLAYER_SETUP_TOKENS			16

/* system setup */
#define SETUP_TOKEN_SYSTEM_SDL_VIDEODRIVER	0
#define SETUP_TOKEN_SYSTEM_SDL_AUDIODRIVER	1
#define SETUP_TOKEN_SYSTEM_AUDIO_FRAGMENT_SIZE	2

#define NUM_SYSTEM_SETUP_TOKENS			3

/* internal setup */
#define SETUP_TOKEN_INT_PROGRAM_TITLE		0
#define SETUP_TOKEN_INT_PROGRAM_VERSION		1
#define SETUP_TOKEN_INT_PROGRAM_AUTHOR		2
#define SETUP_TOKEN_INT_PROGRAM_EMAIL		3
#define SETUP_TOKEN_INT_PROGRAM_WEBSITE		4
#define SETUP_TOKEN_INT_PROGRAM_COPYRIGHT	5
#define SETUP_TOKEN_INT_PROGRAM_COMPANY		6
#define SETUP_TOKEN_INT_PROGRAM_ICON_FILE	7
#define SETUP_TOKEN_INT_DEFAULT_GRAPHICS_SET	8
#define SETUP_TOKEN_INT_DEFAULT_SOUNDS_SET	9
#define SETUP_TOKEN_INT_DEFAULT_MUSIC_SET	10
#define SETUP_TOKEN_INT_FALLBACK_GRAPHICS_FILE	11
#define SETUP_TOKEN_INT_FALLBACK_SOUNDS_FILE	12
#define SETUP_TOKEN_INT_FALLBACK_MUSIC_FILE	13
#define SETUP_TOKEN_INT_DEFAULT_LEVEL_SERIES	14
#define SETUP_TOKEN_INT_CHOOSE_FROM_TOP_LEVELDIR 15
#define SETUP_TOKEN_INT_SHOW_SCALING_IN_TITLE	16
#define SETUP_TOKEN_INT_DEFAULT_WINDOW_WIDTH	17
#define SETUP_TOKEN_INT_DEFAULT_WINDOW_HEIGHT	18

#define NUM_INTERNAL_SETUP_TOKENS		19

/* debug setup */
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_0		0
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_1		1
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_2		2
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_3		3
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_4		4
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_5		5
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_6		6
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_7		7
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_8		8
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_9		9
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_0	10
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_1	11
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_2	12
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_3	13
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_4	14
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_5	15
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_6	16
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_7	17
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_8	18
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_KEY_9	19
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_USE_MOD_KEY 20
#define SETUP_TOKEN_DEBUG_FRAME_DELAY_GAME_ONLY	21
#define SETUP_TOKEN_DEBUG_SHOW_FRAMES_PER_SECOND 22

#define NUM_DEBUG_SETUP_TOKENS			23

/* options setup */
#define SETUP_TOKEN_OPTIONS_VERBOSE		0

#define NUM_OPTIONS_SETUP_TOKENS		1


static struct SetupInfo si;
static struct SetupAutoSetupInfo sasi;
static struct SetupEditorInfo sei;
static struct SetupEditorCascadeInfo seci;
static struct SetupShortcutInfo ssi;
static struct SetupInputInfo sii;
static struct SetupSystemInfo syi;
static struct SetupInternalInfo sxi;
static struct SetupDebugInfo sdi;
static struct OptionInfo soi;

static struct TokenInfo global_setup_tokens[] =
{
  { TYPE_STRING, &si.player_name,             "player_name"		},
  { TYPE_SWITCH, &si.sound,                   "sound"			},
  { TYPE_SWITCH, &si.sound_loops,             "repeating_sound_loops"	},
  { TYPE_SWITCH, &si.sound_music,             "background_music"	},
  { TYPE_SWITCH, &si.sound_simple,            "simple_sound_effects"	},
  { TYPE_SWITCH, &si.toons,                   "toons"			},
  { TYPE_SWITCH, &si.scroll_delay,            "scroll_delay"		},
  { TYPE_INTEGER,&si.scroll_delay_value,      "scroll_delay_value"	},
  { TYPE_STRING, &si.engine_snapshot_mode,    "engine_snapshot_mode"	},
  { TYPE_INTEGER,&si.engine_snapshot_memory,  "engine_snapshot_memory"	},
  { TYPE_SWITCH, &si.fade_screens,            "fade_screens"		},
  { TYPE_SWITCH, &si.autorecord,              "automatic_tape_recording"},
  { TYPE_SWITCH, &si.show_titlescreen,        "show_titlescreen"	},
  { TYPE_SWITCH, &si.quick_doors,             "quick_doors"		},
  { TYPE_SWITCH, &si.team_mode,               "team_mode"		},
  { TYPE_SWITCH, &si.handicap,                "handicap"		},
  { TYPE_SWITCH, &si.skip_levels,             "skip_levels"		},
  { TYPE_SWITCH, &si.increment_levels,        "increment_levels"	},
  { TYPE_SWITCH, &si.time_limit,              "time_limit"		},
  { TYPE_SWITCH, &si.fullscreen,              "fullscreen"		},
  { TYPE_INTEGER,&si.window_scaling_percent,  "window_scaling_percent"	},
  { TYPE_STRING, &si.window_scaling_quality,  "window_scaling_quality"	},
  { TYPE_STRING, &si.screen_rendering_mode,   "screen_rendering_mode"	},
  { TYPE_SWITCH, &si.ask_on_escape,           "ask_on_escape"		},
  { TYPE_SWITCH, &si.ask_on_escape_editor,    "ask_on_escape_editor"	},
  { TYPE_SWITCH, &si.quick_switch,            "quick_player_switch"	},
  { TYPE_SWITCH, &si.input_on_focus,          "input_on_focus"		},
  { TYPE_SWITCH, &si.prefer_aga_graphics,     "prefer_aga_graphics"	},
  { TYPE_INTEGER,&si.game_frame_delay,        "game_frame_delay"	},
  { TYPE_SWITCH, &si.sp_show_border_elements, "sp_show_border_elements"	},
  { TYPE_SWITCH, &si.small_game_graphics,     "small_game_graphics"	},
  { TYPE_SWITCH, &si.show_snapshot_buttons,   "show_snapshot_buttons"	},
  { TYPE_STRING, &si.graphics_set,            "graphics_set"		},
  { TYPE_STRING, &si.sounds_set,              "sounds_set"		},
  { TYPE_STRING, &si.music_set,               "music_set"		},
  { TYPE_SWITCH3,&si.override_level_graphics, "override_level_graphics"	},
  { TYPE_SWITCH3,&si.override_level_sounds,   "override_level_sounds"	},
  { TYPE_SWITCH3,&si.override_level_music,    "override_level_music"	},
  { TYPE_INTEGER,&si.volume_simple,           "volume_simple"		},
  { TYPE_INTEGER,&si.volume_loops,            "volume_loops"		},
  { TYPE_INTEGER,&si.volume_music,            "volume_music"		},
  { TYPE_STRING, &si.touch.control_type,      "touch.control_type"	},
  { TYPE_INTEGER,&si.touch.move_distance,     "touch.move_distance"	},
  { TYPE_INTEGER,&si.touch.drop_distance,     "touch.drop_distance"	},
};

static struct TokenInfo auto_setup_tokens[] =
{
  { TYPE_INTEGER,&sasi.editor_zoom_tilesize,	"editor.zoom_tilesize"	},
};

static struct TokenInfo editor_setup_tokens[] =
{
  { TYPE_SWITCH, &sei.el_classic,	"editor.el_classic"		},
  { TYPE_SWITCH, &sei.el_custom,	"editor.el_custom"		},
  { TYPE_SWITCH, &sei.el_user_defined,	"editor.el_user_defined"	},
  { TYPE_SWITCH, &sei.el_dynamic,	"editor.el_dynamic"		},
  { TYPE_SWITCH, &sei.el_headlines,	"editor.el_headlines"		},
  { TYPE_SWITCH, &sei.show_element_token,"editor.show_element_token"	},
};

static struct TokenInfo editor_cascade_setup_tokens[] =
{
  { TYPE_SWITCH, &seci.el_bd,		"editor.cascade.el_bd"		},
  { TYPE_SWITCH, &seci.el_em,		"editor.cascade.el_em"		},
  { TYPE_SWITCH, &seci.el_emc,		"editor.cascade.el_emc"		},
  { TYPE_SWITCH, &seci.el_rnd,		"editor.cascade.el_rnd"		},
  { TYPE_SWITCH, &seci.el_sb,		"editor.cascade.el_sb"		},
  { TYPE_SWITCH, &seci.el_sp,		"editor.cascade.el_sp"		},
  { TYPE_SWITCH, &seci.el_dc,		"editor.cascade.el_dc"		},
  { TYPE_SWITCH, &seci.el_dx,		"editor.cascade.el_dx"		},
  { TYPE_SWITCH, &seci.el_mm,		"editor.cascade.el_mm"		},
  { TYPE_SWITCH, &seci.el_df,		"editor.cascade.el_df"		},
  { TYPE_SWITCH, &seci.el_chars,	"editor.cascade.el_chars"	},
  { TYPE_SWITCH, &seci.el_steel_chars,	"editor.cascade.el_steel_chars"	},
  { TYPE_SWITCH, &seci.el_ce,		"editor.cascade.el_ce"		},
  { TYPE_SWITCH, &seci.el_ge,		"editor.cascade.el_ge"		},
  { TYPE_SWITCH, &seci.el_ref,		"editor.cascade.el_ref"		},
  { TYPE_SWITCH, &seci.el_user,		"editor.cascade.el_user"	},
  { TYPE_SWITCH, &seci.el_dynamic,	"editor.cascade.el_dynamic"	},
};

static struct TokenInfo shortcut_setup_tokens[] =
{
  { TYPE_KEY_X11, &ssi.save_game,	"shortcut.save_game"		},
  { TYPE_KEY_X11, &ssi.load_game,	"shortcut.load_game"		},
  { TYPE_KEY_X11, &ssi.toggle_pause,	"shortcut.toggle_pause"		},
  { TYPE_KEY_X11, &ssi.focus_player[0],	"shortcut.focus_player_1"	},
  { TYPE_KEY_X11, &ssi.focus_player[1],	"shortcut.focus_player_2"	},
  { TYPE_KEY_X11, &ssi.focus_player[2],	"shortcut.focus_player_3"	},
  { TYPE_KEY_X11, &ssi.focus_player[3],	"shortcut.focus_player_4"	},
  { TYPE_KEY_X11, &ssi.focus_player_all,"shortcut.focus_player_all"	},
  { TYPE_KEY_X11, &ssi.tape_eject,	"shortcut.tape_eject"		},
  { TYPE_KEY_X11, &ssi.tape_extra,	"shortcut.tape_extra"		},
  { TYPE_KEY_X11, &ssi.tape_stop,	"shortcut.tape_stop"		},
  { TYPE_KEY_X11, &ssi.tape_pause,	"shortcut.tape_pause"		},
  { TYPE_KEY_X11, &ssi.tape_record,	"shortcut.tape_record"		},
  { TYPE_KEY_X11, &ssi.tape_play,	"shortcut.tape_play"		},
  { TYPE_KEY_X11, &ssi.sound_simple,	"shortcut.sound_simple"		},
  { TYPE_KEY_X11, &ssi.sound_loops,	"shortcut.sound_loops"		},
  { TYPE_KEY_X11, &ssi.sound_music,	"shortcut.sound_music"		},
  { TYPE_KEY_X11, &ssi.snap_left,	"shortcut.snap_left"		},
  { TYPE_KEY_X11, &ssi.snap_right,	"shortcut.snap_right"		},
  { TYPE_KEY_X11, &ssi.snap_up,		"shortcut.snap_up"		},
  { TYPE_KEY_X11, &ssi.snap_down,	"shortcut.snap_down"		},
};

static struct TokenInfo player_setup_tokens[] =
{
  { TYPE_BOOLEAN, &sii.use_joystick,	".use_joystick"			},
  { TYPE_STRING,  &sii.joy.device_name,	".joy.device_name"		},
  { TYPE_INTEGER, &sii.joy.xleft,	".joy.xleft"			},
  { TYPE_INTEGER, &sii.joy.xmiddle,	".joy.xmiddle"			},
  { TYPE_INTEGER, &sii.joy.xright,	".joy.xright"			},
  { TYPE_INTEGER, &sii.joy.yupper,	".joy.yupper"			},
  { TYPE_INTEGER, &sii.joy.ymiddle,	".joy.ymiddle"			},
  { TYPE_INTEGER, &sii.joy.ylower,	".joy.ylower"			},
  { TYPE_INTEGER, &sii.joy.snap,	".joy.snap_field"		},
  { TYPE_INTEGER, &sii.joy.drop,	".joy.place_bomb"		},
  { TYPE_KEY_X11, &sii.key.left,	".key.move_left"		},
  { TYPE_KEY_X11, &sii.key.right,	".key.move_right"		},
  { TYPE_KEY_X11, &sii.key.up,		".key.move_up"			},
  { TYPE_KEY_X11, &sii.key.down,	".key.move_down"		},
  { TYPE_KEY_X11, &sii.key.snap,	".key.snap_field"		},
  { TYPE_KEY_X11, &sii.key.drop,	".key.place_bomb"		},
};

static struct TokenInfo system_setup_tokens[] =
{
  { TYPE_STRING,  &syi.sdl_videodriver,    "system.sdl_videodriver"	},
  { TYPE_STRING,  &syi.sdl_audiodriver,	   "system.sdl_audiodriver"	},
  { TYPE_INTEGER, &syi.audio_fragment_size,"system.audio_fragment_size"	},
};

static struct TokenInfo internal_setup_tokens[] =
{
  { TYPE_STRING, &sxi.program_title,		"program_title"		},
  { TYPE_STRING, &sxi.program_version,		"program_version"	},
  { TYPE_STRING, &sxi.program_author,		"program_author"	},
  { TYPE_STRING, &sxi.program_email,		"program_email"		},
  { TYPE_STRING, &sxi.program_website,		"program_website"	},
  { TYPE_STRING, &sxi.program_copyright,	"program_copyright"	},
  { TYPE_STRING, &sxi.program_company,		"program_company"	},
  { TYPE_STRING, &sxi.program_icon_file,	"program_icon_file"	},
  { TYPE_STRING, &sxi.default_graphics_set,	"default_graphics_set"	},
  { TYPE_STRING, &sxi.default_sounds_set,	"default_sounds_set"	},
  { TYPE_STRING, &sxi.default_music_set,	"default_music_set"	},
  { TYPE_STRING, &sxi.fallback_graphics_file,	"fallback_graphics_file"},
  { TYPE_STRING, &sxi.fallback_sounds_file,	"fallback_sounds_file"	},
  { TYPE_STRING, &sxi.fallback_music_file,	"fallback_music_file"	},
  { TYPE_STRING, &sxi.default_level_series,	"default_level_series"	},
  { TYPE_BOOLEAN,&sxi.choose_from_top_leveldir,	"choose_from_top_leveldir" },
  { TYPE_BOOLEAN,&sxi.show_scaling_in_title,	"show_scaling_in_title" },
  { TYPE_INTEGER,&sxi.default_window_width,	"default_window_width"	},
  { TYPE_INTEGER,&sxi.default_window_height,	"default_window_height"	},
};

static struct TokenInfo debug_setup_tokens[] =
{
  { TYPE_INTEGER, &sdi.frame_delay[0],		"debug.frame_delay_0"	},
  { TYPE_INTEGER, &sdi.frame_delay[1],		"debug.frame_delay_1"	},
  { TYPE_INTEGER, &sdi.frame_delay[2],		"debug.frame_delay_2"	},
  { TYPE_INTEGER, &sdi.frame_delay[3],		"debug.frame_delay_3"	},
  { TYPE_INTEGER, &sdi.frame_delay[4],		"debug.frame_delay_4"	},
  { TYPE_INTEGER, &sdi.frame_delay[5],		"debug.frame_delay_5"	},
  { TYPE_INTEGER, &sdi.frame_delay[6],		"debug.frame_delay_6"	},
  { TYPE_INTEGER, &sdi.frame_delay[7],		"debug.frame_delay_7"	},
  { TYPE_INTEGER, &sdi.frame_delay[8],		"debug.frame_delay_8"	},
  { TYPE_INTEGER, &sdi.frame_delay[9],		"debug.frame_delay_9"	},
  { TYPE_KEY_X11, &sdi.frame_delay_key[0],	"debug.key.frame_delay_0" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[1],	"debug.key.frame_delay_1" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[2],	"debug.key.frame_delay_2" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[3],	"debug.key.frame_delay_3" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[4],	"debug.key.frame_delay_4" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[5],	"debug.key.frame_delay_5" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[6],	"debug.key.frame_delay_6" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[7],	"debug.key.frame_delay_7" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[8],	"debug.key.frame_delay_8" },
  { TYPE_KEY_X11, &sdi.frame_delay_key[9],	"debug.key.frame_delay_9" },
  { TYPE_BOOLEAN, &sdi.frame_delay_use_mod_key,"debug.frame_delay.use_mod_key"},
  { TYPE_BOOLEAN, &sdi.frame_delay_game_only,  "debug.frame_delay.game_only" },
  { TYPE_BOOLEAN, &sdi.show_frames_per_second, "debug.show_frames_per_second" },
};

static struct TokenInfo options_setup_tokens[] =
{
  { TYPE_BOOLEAN, &soi.verbose,		"options.verbose"		},
};

static char *get_corrected_login_name(char *login_name)
{
  /* needed because player name must be a fixed length string */
  char *login_name_new = checked_malloc(MAX_PLAYER_NAME_LEN + 1);

  strncpy(login_name_new, login_name, MAX_PLAYER_NAME_LEN);
  login_name_new[MAX_PLAYER_NAME_LEN] = '\0';

  if (strlen(login_name) > MAX_PLAYER_NAME_LEN)		/* name has been cut */
    if (strchr(login_name_new, ' '))
      *strchr(login_name_new, ' ') = '\0';

  return login_name_new;
}

static void setSetupInfoToDefaults(struct SetupInfo *si)
{
  int i;

  si->player_name = get_corrected_login_name(getLoginName());

  si->sound = TRUE;
  si->sound_loops = TRUE;
  si->sound_music = TRUE;
  si->sound_simple = TRUE;
  si->toons = TRUE;
  si->scroll_delay = TRUE;
  si->scroll_delay_value = STD_SCROLL_DELAY;
  si->engine_snapshot_mode = getStringCopy(STR_SNAPSHOT_MODE_DEFAULT);
  si->engine_snapshot_memory = SNAPSHOT_MEMORY_DEFAULT;
  si->fade_screens = TRUE;
  si->autorecord = TRUE;
  si->show_titlescreen = TRUE;
  si->quick_doors = FALSE;
  si->team_mode = FALSE;
  si->handicap = TRUE;
  si->skip_levels = TRUE;
  si->increment_levels = TRUE;
  si->time_limit = TRUE;
  si->fullscreen = FALSE;
  si->window_scaling_percent = STD_WINDOW_SCALING_PERCENT;
  si->window_scaling_quality = getStringCopy(SCALING_QUALITY_DEFAULT);
  si->screen_rendering_mode = getStringCopy(STR_SPECIAL_RENDERING_DEFAULT);
  si->ask_on_escape = TRUE;
  si->ask_on_escape_editor = TRUE;
  si->quick_switch = FALSE;
  si->input_on_focus = FALSE;
  si->prefer_aga_graphics = TRUE;
  si->game_frame_delay = GAME_FRAME_DELAY;
  si->sp_show_border_elements = FALSE;
  si->small_game_graphics = FALSE;
  si->show_snapshot_buttons = FALSE;

  si->graphics_set = getStringCopy(GFX_CLASSIC_SUBDIR);
  si->sounds_set   = getStringCopy(SND_CLASSIC_SUBDIR);
  si->music_set    = getStringCopy(MUS_CLASSIC_SUBDIR);

  si->override_level_graphics = FALSE;
  si->override_level_sounds = FALSE;
  si->override_level_music = FALSE;

  si->volume_simple = 100;		/* percent */
  si->volume_loops = 100;		/* percent */
  si->volume_music = 100;		/* percent */

  si->touch.control_type = getStringCopy(TOUCH_CONTROL_DEFAULT);
  si->touch.move_distance = TOUCH_MOVE_DISTANCE_DEFAULT;	/* percent */
  si->touch.drop_distance = TOUCH_DROP_DISTANCE_DEFAULT;	/* percent */

  si->editor.el_boulderdash		= TRUE;
  si->editor.el_emerald_mine		= TRUE;
  si->editor.el_emerald_mine_club	= TRUE;
  si->editor.el_more			= TRUE;
  si->editor.el_sokoban			= TRUE;
  si->editor.el_supaplex		= TRUE;
  si->editor.el_diamond_caves		= TRUE;
  si->editor.el_dx_boulderdash		= TRUE;

  si->editor.el_mirror_magic		= TRUE;
  si->editor.el_deflektor		= TRUE;

  si->editor.el_chars			= TRUE;
  si->editor.el_steel_chars		= TRUE;

  si->editor.el_classic			= TRUE;
  si->editor.el_custom			= TRUE;

  si->editor.el_user_defined		= FALSE;
  si->editor.el_dynamic			= TRUE;

  si->editor.el_headlines		= TRUE;

  si->editor.show_element_token		= FALSE;

  si->editor.use_template_for_new_levels = TRUE;

  si->shortcut.save_game	= DEFAULT_KEY_SAVE_GAME;
  si->shortcut.load_game	= DEFAULT_KEY_LOAD_GAME;
  si->shortcut.toggle_pause	= DEFAULT_KEY_TOGGLE_PAUSE;

  si->shortcut.focus_player[0]	= DEFAULT_KEY_FOCUS_PLAYER_1;
  si->shortcut.focus_player[1]	= DEFAULT_KEY_FOCUS_PLAYER_2;
  si->shortcut.focus_player[2]	= DEFAULT_KEY_FOCUS_PLAYER_3;
  si->shortcut.focus_player[3]	= DEFAULT_KEY_FOCUS_PLAYER_4;
  si->shortcut.focus_player_all	= DEFAULT_KEY_FOCUS_PLAYER_ALL;

  si->shortcut.tape_eject	= DEFAULT_KEY_TAPE_EJECT;
  si->shortcut.tape_extra	= DEFAULT_KEY_TAPE_EXTRA;
  si->shortcut.tape_stop	= DEFAULT_KEY_TAPE_STOP;
  si->shortcut.tape_pause	= DEFAULT_KEY_TAPE_PAUSE;
  si->shortcut.tape_record	= DEFAULT_KEY_TAPE_RECORD;
  si->shortcut.tape_play	= DEFAULT_KEY_TAPE_PLAY;

  si->shortcut.sound_simple	= DEFAULT_KEY_SOUND_SIMPLE;
  si->shortcut.sound_loops	= DEFAULT_KEY_SOUND_LOOPS;
  si->shortcut.sound_music	= DEFAULT_KEY_SOUND_MUSIC;

  si->shortcut.snap_left	= DEFAULT_KEY_SNAP_LEFT;
  si->shortcut.snap_right	= DEFAULT_KEY_SNAP_RIGHT;
  si->shortcut.snap_up		= DEFAULT_KEY_SNAP_UP;
  si->shortcut.snap_down	= DEFAULT_KEY_SNAP_DOWN;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    si->input[i].use_joystick = FALSE;
    si->input[i].joy.device_name=getStringCopy(getDeviceNameFromJoystickNr(i));
    si->input[i].joy.xleft   = JOYSTICK_XLEFT;
    si->input[i].joy.xmiddle = JOYSTICK_XMIDDLE;
    si->input[i].joy.xright  = JOYSTICK_XRIGHT;
    si->input[i].joy.yupper  = JOYSTICK_YUPPER;
    si->input[i].joy.ymiddle = JOYSTICK_YMIDDLE;
    si->input[i].joy.ylower  = JOYSTICK_YLOWER;
    si->input[i].joy.snap  = (i == 0 ? JOY_BUTTON_1 : 0);
    si->input[i].joy.drop  = (i == 0 ? JOY_BUTTON_2 : 0);
    si->input[i].key.left  = (i == 0 ? DEFAULT_KEY_LEFT  : KSYM_UNDEFINED);
    si->input[i].key.right = (i == 0 ? DEFAULT_KEY_RIGHT : KSYM_UNDEFINED);
    si->input[i].key.up    = (i == 0 ? DEFAULT_KEY_UP    : KSYM_UNDEFINED);
    si->input[i].key.down  = (i == 0 ? DEFAULT_KEY_DOWN  : KSYM_UNDEFINED);
    si->input[i].key.snap  = (i == 0 ? DEFAULT_KEY_SNAP  : KSYM_UNDEFINED);
    si->input[i].key.drop  = (i == 0 ? DEFAULT_KEY_DROP  : KSYM_UNDEFINED);
  }

  si->system.sdl_videodriver = getStringCopy(ARG_DEFAULT);
  si->system.sdl_audiodriver = getStringCopy(ARG_DEFAULT);
  si->system.audio_fragment_size = DEFAULT_AUDIO_FRAGMENT_SIZE;

  si->internal.program_title     = getStringCopy(PROGRAM_TITLE_STRING);
  si->internal.program_version   = getStringCopy(getProgramRealVersionString());
  si->internal.program_author    = getStringCopy(PROGRAM_AUTHOR_STRING);
  si->internal.program_email     = getStringCopy(PROGRAM_EMAIL_STRING);
  si->internal.program_website   = getStringCopy(PROGRAM_WEBSITE_STRING);
  si->internal.program_copyright = getStringCopy(PROGRAM_COPYRIGHT_STRING);
  si->internal.program_company   = getStringCopy(PROGRAM_COMPANY_STRING);

  si->internal.program_icon_file = getStringCopy(PROGRAM_ICON_FILENAME);

  si->internal.default_graphics_set = getStringCopy(GFX_CLASSIC_SUBDIR);
  si->internal.default_sounds_set   = getStringCopy(SND_CLASSIC_SUBDIR);
  si->internal.default_music_set    = getStringCopy(MUS_CLASSIC_SUBDIR);

  si->internal.fallback_graphics_file = getStringCopy(UNDEFINED_FILENAME);
  si->internal.fallback_sounds_file   = getStringCopy(UNDEFINED_FILENAME);
  si->internal.fallback_music_file    = getStringCopy(UNDEFINED_FILENAME);

  si->internal.default_level_series = getStringCopy(UNDEFINED_LEVELSET);
  si->internal.choose_from_top_leveldir = FALSE;
  si->internal.show_scaling_in_title = TRUE;

  si->internal.default_window_width  = WIN_XSIZE_DEFAULT;
  si->internal.default_window_height = WIN_YSIZE_DEFAULT;

  si->debug.frame_delay[0] = DEFAULT_FRAME_DELAY_0;
  si->debug.frame_delay[1] = DEFAULT_FRAME_DELAY_1;
  si->debug.frame_delay[2] = DEFAULT_FRAME_DELAY_2;
  si->debug.frame_delay[3] = DEFAULT_FRAME_DELAY_3;
  si->debug.frame_delay[4] = DEFAULT_FRAME_DELAY_4;
  si->debug.frame_delay[5] = DEFAULT_FRAME_DELAY_5;
  si->debug.frame_delay[6] = DEFAULT_FRAME_DELAY_6;
  si->debug.frame_delay[7] = DEFAULT_FRAME_DELAY_7;
  si->debug.frame_delay[8] = DEFAULT_FRAME_DELAY_8;
  si->debug.frame_delay[9] = DEFAULT_FRAME_DELAY_9;

  si->debug.frame_delay_key[0] = DEFAULT_KEY_FRAME_DELAY_0;
  si->debug.frame_delay_key[1] = DEFAULT_KEY_FRAME_DELAY_1;
  si->debug.frame_delay_key[2] = DEFAULT_KEY_FRAME_DELAY_2;
  si->debug.frame_delay_key[3] = DEFAULT_KEY_FRAME_DELAY_3;
  si->debug.frame_delay_key[4] = DEFAULT_KEY_FRAME_DELAY_4;
  si->debug.frame_delay_key[5] = DEFAULT_KEY_FRAME_DELAY_5;
  si->debug.frame_delay_key[6] = DEFAULT_KEY_FRAME_DELAY_6;
  si->debug.frame_delay_key[7] = DEFAULT_KEY_FRAME_DELAY_7;
  si->debug.frame_delay_key[8] = DEFAULT_KEY_FRAME_DELAY_8;
  si->debug.frame_delay_key[9] = DEFAULT_KEY_FRAME_DELAY_9;

  si->debug.frame_delay_use_mod_key = DEFAULT_FRAME_DELAY_USE_MOD_KEY;
  si->debug.frame_delay_game_only   = DEFAULT_FRAME_DELAY_GAME_ONLY;

  si->debug.show_frames_per_second = FALSE;

  si->options.verbose = FALSE;

#if defined(PLATFORM_ANDROID)
  si->fullscreen = TRUE;
#endif
}

static void setSetupInfoToDefaults_AutoSetup(struct SetupInfo *si)
{
  si->auto_setup.editor_zoom_tilesize = MINI_TILESIZE;
}

static void setSetupInfoToDefaults_EditorCascade(struct SetupInfo *si)
{
  si->editor_cascade.el_bd		= TRUE;
  si->editor_cascade.el_em		= TRUE;
  si->editor_cascade.el_emc		= TRUE;
  si->editor_cascade.el_rnd		= TRUE;
  si->editor_cascade.el_sb		= TRUE;
  si->editor_cascade.el_sp		= TRUE;
  si->editor_cascade.el_dc		= TRUE;
  si->editor_cascade.el_dx		= TRUE;

  si->editor_cascade.el_mm		= TRUE;
  si->editor_cascade.el_df		= TRUE;

  si->editor_cascade.el_chars		= FALSE;
  si->editor_cascade.el_steel_chars	= FALSE;
  si->editor_cascade.el_ce		= FALSE;
  si->editor_cascade.el_ge		= FALSE;
  si->editor_cascade.el_ref		= FALSE;
  si->editor_cascade.el_user		= FALSE;
  si->editor_cascade.el_dynamic		= FALSE;
}

#define MAX_HIDE_SETUP_TOKEN_SIZE		20

static char *getHideSetupToken(void *setup_value)
{
  static char hide_setup_token[MAX_HIDE_SETUP_TOKEN_SIZE];

  if (setup_value != NULL)
    snprintf(hide_setup_token, MAX_HIDE_SETUP_TOKEN_SIZE, "%p", setup_value);

  return hide_setup_token;
}

static void setHideSetupEntry(void *setup_value_raw)
{
  /* !!! DIRTY WORKAROUND; TO BE FIXED AFTER THE MM ENGINE RELEASE !!! */
  void *setup_value = setup_value_raw - (void *)&si + (void *)&setup;

  char *hide_setup_token = getHideSetupToken(setup_value);

  if (setup_value != NULL)
    setHashEntry(hide_setup_hash, hide_setup_token, "");
}

boolean hideSetupEntry(void *setup_value)
{
  char *hide_setup_token = getHideSetupToken(setup_value);

  return (setup_value != NULL &&
	  getHashEntry(hide_setup_hash, hide_setup_token) != NULL);
}

static void setSetupInfoFromTokenText(SetupFileHash *setup_file_hash,
				      struct TokenInfo *token_info,
				      int token_nr, char *token_text)
{
  char *token_hide_text = getStringCat2(token_text, ".hide");
  char *token_hide_value = getHashEntry(setup_file_hash, token_hide_text);

  /* set the value of this setup option in the setup option structure */
  setSetupInfo(token_info, token_nr, getHashEntry(setup_file_hash, token_text));

  /* check if this setup option should be hidden in the setup menu */
  if (token_hide_value != NULL && get_boolean_from_string(token_hide_value))
    setHideSetupEntry(token_info[token_nr].value);
}

static void setSetupInfoFromTokenInfo(SetupFileHash *setup_file_hash,
				      struct TokenInfo *token_info,
				      int token_nr)
{
  setSetupInfoFromTokenText(setup_file_hash, token_info, token_nr,
			    token_info[token_nr].text);
}

static void decodeSetupFileHash(SetupFileHash *setup_file_hash)
{
  int i, pnr;

  if (!setup_file_hash)
    return;

  if (hide_setup_hash == NULL)
    hide_setup_hash = newSetupFileHash();

  /* global setup */
  si = setup;
  for (i = 0; i < NUM_GLOBAL_SETUP_TOKENS; i++)
    setSetupInfoFromTokenInfo(setup_file_hash, global_setup_tokens, i);
  setup = si;

  /* editor setup */
  sei = setup.editor;
  for (i = 0; i < NUM_EDITOR_SETUP_TOKENS; i++)
    setSetupInfoFromTokenInfo(setup_file_hash, editor_setup_tokens, i);
  setup.editor = sei;

  /* shortcut setup */
  ssi = setup.shortcut;
  for (i = 0; i < NUM_SHORTCUT_SETUP_TOKENS; i++)
    setSetupInfoFromTokenInfo(setup_file_hash, shortcut_setup_tokens, i);
  setup.shortcut = ssi;

  /* player setup */
  for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);

    sii = setup.input[pnr];
    for (i = 0; i < NUM_PLAYER_SETUP_TOKENS; i++)
    {
      char full_token[100];

      sprintf(full_token, "%s%s", prefix, player_setup_tokens[i].text);
      setSetupInfoFromTokenText(setup_file_hash, player_setup_tokens, i,
				full_token);
    }
    setup.input[pnr] = sii;
  }

  /* system setup */
  syi = setup.system;
  for (i = 0; i < NUM_SYSTEM_SETUP_TOKENS; i++)
    setSetupInfoFromTokenInfo(setup_file_hash, system_setup_tokens, i);
  setup.system = syi;

  /* internal setup */
  sxi = setup.internal;
  for (i = 0; i < NUM_INTERNAL_SETUP_TOKENS; i++)
    setSetupInfoFromTokenInfo(setup_file_hash, internal_setup_tokens, i);
  setup.internal = sxi;

  /* debug setup */
  sdi = setup.debug;
  for (i = 0; i < NUM_DEBUG_SETUP_TOKENS; i++)
    setSetupInfoFromTokenInfo(setup_file_hash, debug_setup_tokens, i);
  setup.debug = sdi;

  /* options setup */
  soi = setup.options;
  for (i = 0; i < NUM_OPTIONS_SETUP_TOKENS; i++)
    setSetupInfoFromTokenInfo(setup_file_hash, options_setup_tokens, i);
  setup.options = soi;
}

static void decodeSetupFileHash_AutoSetup(SetupFileHash *setup_file_hash)
{
  int i;

  if (!setup_file_hash)
    return;

  /* auto setup */
  sasi = setup.auto_setup;
  for (i = 0; i < NUM_AUTO_SETUP_TOKENS; i++)
    setSetupInfo(auto_setup_tokens, i,
		 getHashEntry(setup_file_hash,
			      auto_setup_tokens[i].text));
  setup.auto_setup = sasi;
}

static void decodeSetupFileHash_EditorCascade(SetupFileHash *setup_file_hash)
{
  int i;

  if (!setup_file_hash)
    return;

  /* editor cascade setup */
  seci = setup.editor_cascade;
  for (i = 0; i < NUM_EDITOR_CASCADE_SETUP_TOKENS; i++)
    setSetupInfo(editor_cascade_setup_tokens, i,
		 getHashEntry(setup_file_hash,
			      editor_cascade_setup_tokens[i].text));
  setup.editor_cascade = seci;
}

void LoadSetupFromFilename(char *filename)
{
  SetupFileHash *setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    decodeSetupFileHash(setup_file_hash);

    freeSetupFileHash(setup_file_hash);
  }
  else
  {
    Error(ERR_DEBUG, "using default setup values");
  }
}

static void LoadSetup_SpecialPostProcessing()
{
  char *player_name_new;

  /* needed to work around problems with fixed length strings */
  player_name_new = get_corrected_login_name(setup.player_name);
  free(setup.player_name);
  setup.player_name = player_name_new;

  /* "scroll_delay: on(3) / off(0)" was replaced by scroll delay value */
  if (setup.scroll_delay == FALSE)
  {
    setup.scroll_delay_value = MIN_SCROLL_DELAY;
    setup.scroll_delay = TRUE;			/* now always "on" */
  }

  /* make sure that scroll delay value stays inside valid range */
  setup.scroll_delay_value =
    MIN(MAX(MIN_SCROLL_DELAY, setup.scroll_delay_value), MAX_SCROLL_DELAY);
}

void LoadSetup()
{
  char *filename;

  /* always start with reliable default values */
  setSetupInfoToDefaults(&setup);

  /* try to load setup values from default setup file */
  filename = getDefaultSetupFilename();

  if (fileExists(filename))
    LoadSetupFromFilename(filename);

  /* try to load setup values from user setup file */
  filename = getSetupFilename();

  LoadSetupFromFilename(filename);

  LoadSetup_SpecialPostProcessing();
}

void LoadSetup_AutoSetup()
{
  char *filename = getPath2(getSetupDir(), AUTOSETUP_FILENAME);
  SetupFileHash *setup_file_hash = NULL;

  /* always start with reliable default values */
  setSetupInfoToDefaults_AutoSetup(&setup);

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    decodeSetupFileHash_AutoSetup(setup_file_hash);

    freeSetupFileHash(setup_file_hash);
  }

  free(filename);
}

void LoadSetup_EditorCascade()
{
  char *filename = getPath2(getSetupDir(), EDITORCASCADE_FILENAME);
  SetupFileHash *setup_file_hash = NULL;

  /* always start with reliable default values */
  setSetupInfoToDefaults_EditorCascade(&setup);

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash)
  {
    decodeSetupFileHash_EditorCascade(setup_file_hash);

    freeSetupFileHash(setup_file_hash);
  }

  free(filename);
}

static void addGameControllerMappingToHash(SetupFileHash *mappings_hash,
					   char *mapping_line)
{
  char mapping_guid[MAX_LINE_LEN];
  char *mapping_start, *mapping_end;

  // get GUID from game controller mapping line: copy complete line
  strncpy(mapping_guid, mapping_line, MAX_LINE_LEN - 1);
  mapping_guid[MAX_LINE_LEN - 1] = '\0';

  // get GUID from game controller mapping line: cut after GUID part
  mapping_start = strchr(mapping_guid, ',');
  if (mapping_start != NULL)
    *mapping_start = '\0';

  // cut newline from game controller mapping line
  mapping_end = strchr(mapping_line, '\n');
  if (mapping_end != NULL)
    *mapping_end = '\0';

  // add mapping entry to game controller mappings hash
  setHashEntry(mappings_hash, mapping_guid, mapping_line);
}

static void LoadSetup_ReadGameControllerMappings(SetupFileHash *mappings_hash,
						 char *filename)
{
  FILE *file;

  if (!(file = fopen(filename, MODE_READ)))
  {
    Error(ERR_WARN, "cannot read game controller mappings file '%s'", filename);

    return;
  }

  while (!feof(file))
  {
    char line[MAX_LINE_LEN];

    if (!fgets(line, MAX_LINE_LEN, file))
      break;

    addGameControllerMappingToHash(mappings_hash, line);
  }

  fclose(file);
}

void SaveSetup()
{
  char *filename = getSetupFilename();
  FILE *file;
  int i, pnr;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    return;
  }

  fprintFileHeader(file, SETUP_FILENAME);

  /* global setup */
  si = setup;
  for (i = 0; i < NUM_GLOBAL_SETUP_TOKENS; i++)
  {
    /* just to make things nicer :) */
    if (i == SETUP_TOKEN_PLAYER_NAME + 1 ||
	i == SETUP_TOKEN_GRAPHICS_SET ||
	i == SETUP_TOKEN_VOLUME_SIMPLE ||
	i == SETUP_TOKEN_TOUCH_CONTROL_TYPE)
      fprintf(file, "\n");

    fprintf(file, "%s\n", getSetupLine(global_setup_tokens, "", i));
  }

  /* editor setup */
  sei = setup.editor;
  fprintf(file, "\n");
  for (i = 0; i < NUM_EDITOR_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(editor_setup_tokens, "", i));

  /* shortcut setup */
  ssi = setup.shortcut;
  fprintf(file, "\n");
  for (i = 0; i < NUM_SHORTCUT_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(shortcut_setup_tokens, "", i));

  /* player setup */
  for (pnr = 0; pnr < MAX_PLAYERS; pnr++)
  {
    char prefix[30];

    sprintf(prefix, "%s%d", TOKEN_STR_PLAYER_PREFIX, pnr + 1);
    fprintf(file, "\n");

    sii = setup.input[pnr];
    for (i = 0; i < NUM_PLAYER_SETUP_TOKENS; i++)
      fprintf(file, "%s\n", getSetupLine(player_setup_tokens, prefix, i));
  }

  /* system setup */
  syi = setup.system;
  fprintf(file, "\n");
  for (i = 0; i < NUM_SYSTEM_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(system_setup_tokens, "", i));

  /* internal setup */
  /* (internal setup values not saved to user setup file) */

  /* debug setup */
  sdi = setup.debug;
  fprintf(file, "\n");
  for (i = 0; i < NUM_DEBUG_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(debug_setup_tokens, "", i));

  /* options setup */
  soi = setup.options;
  fprintf(file, "\n");
  for (i = 0; i < NUM_OPTIONS_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(options_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}

void SaveSetup_AutoSetup()
{
  char *filename = getPath2(getSetupDir(), AUTOSETUP_FILENAME);
  FILE *file;
  int i;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write auto setup file '%s'", filename);
    free(filename);
    return;
  }

  fprintFileHeader(file, AUTOSETUP_FILENAME);

  sasi = setup.auto_setup;
  for (i = 0; i < NUM_AUTO_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(auto_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

void SaveSetup_EditorCascade()
{
  char *filename = getPath2(getSetupDir(), EDITORCASCADE_FILENAME);
  FILE *file;
  int i;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write editor cascade state file '%s'", filename);
    free(filename);
    return;
  }

  fprintFileHeader(file, EDITORCASCADE_FILENAME);

  seci = setup.editor_cascade;
  for (i = 0; i < NUM_EDITOR_CASCADE_SETUP_TOKENS; i++)
    fprintf(file, "%s\n", getSetupLine(editor_cascade_setup_tokens, "", i));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

static void SaveSetup_WriteGameControllerMappings(SetupFileHash *mappings_hash,
						  char *filename)
{
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write game controller mappings file '%s'",filename);

    return;
  }

  BEGIN_HASH_ITERATION(mappings_hash, itr)
  {
    fprintf(file, "%s\n", HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(mappings_hash, itr)

  fclose(file);
}

void SaveSetup_AddGameControllerMapping(char *mapping)
{
  char *filename = getPath2(getSetupDir(), GAMECONTROLLER_BASENAME);
  SetupFileHash *mappings_hash = newSetupFileHash();

  InitUserDataDirectory();

  // load existing personal game controller mappings
  LoadSetup_ReadGameControllerMappings(mappings_hash, filename);

  // add new mapping to personal game controller mappings
  addGameControllerMappingToHash(mappings_hash, mapping);

  // save updated personal game controller mappings
  SaveSetup_WriteGameControllerMappings(mappings_hash, filename);

  freeSetupFileHash(mappings_hash);
  free(filename);
}

void LoadCustomElementDescriptions()
{
  char *filename = getCustomArtworkConfigFilename(ARTWORK_TYPE_GRAPHICS);
  SetupFileHash *setup_file_hash;
  int i;

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    if (element_info[i].custom_description != NULL)
    {
      free(element_info[i].custom_description);
      element_info[i].custom_description = NULL;
    }
  }

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    char *token = getStringCat2(element_info[i].token_name, ".name");
    char *value = getHashEntry(setup_file_hash, token);

    if (value != NULL)
      element_info[i].custom_description = getStringCopy(value);

    free(token);
  }

  freeSetupFileHash(setup_file_hash);
}

static int getElementFromToken(char *token)
{
  char *value = getHashEntry(element_token_hash, token);

  if (value != NULL)
    return atoi(value);

  Error(ERR_WARN, "unknown element token '%s'", token);

  return EL_UNDEFINED;
}

static int get_token_parameter_value(char *token, char *value_raw)
{
  char *suffix;

  if (token == NULL || value_raw == NULL)
    return ARG_UNDEFINED_VALUE;

  suffix = strrchr(token, '.');
  if (suffix == NULL)
    suffix = token;

  if (strEqual(suffix, ".element"))
    return getElementFromToken(value_raw);

  /* !!! USE CORRECT VALUE TYPE (currently works also for TYPE_BOOLEAN) !!! */
  return get_parameter_value(value_raw, suffix, TYPE_INTEGER);
}

void InitMenuDesignSettings_Static()
{
  int i;

  /* always start with reliable default values from static default config */
  for (i = 0; image_config_vars[i].token != NULL; i++)
  {
    char *value = getHashEntry(image_config_hash, image_config_vars[i].token);

    if (value != NULL)
      *image_config_vars[i].value =
	get_token_parameter_value(image_config_vars[i].token, value);
  }
}

static void InitMenuDesignSettings_SpecialPreProcessing()
{
  int i;

  /* the following initializes hierarchical values from static configuration */

  /* special case: initialize "ARG_DEFAULT" values in static default config */
  /* (e.g., initialize "[titlemessage].fade_mode" from "[title].fade_mode") */
  titlescreen_initial_first_default.fade_mode  =
    title_initial_first_default.fade_mode;
  titlescreen_initial_first_default.fade_delay =
    title_initial_first_default.fade_delay;
  titlescreen_initial_first_default.post_delay =
    title_initial_first_default.post_delay;
  titlescreen_initial_first_default.auto_delay =
    title_initial_first_default.auto_delay;
  titlescreen_first_default.fade_mode  = title_first_default.fade_mode;
  titlescreen_first_default.fade_delay = title_first_default.fade_delay;
  titlescreen_first_default.post_delay = title_first_default.post_delay;
  titlescreen_first_default.auto_delay = title_first_default.auto_delay;
  titlemessage_initial_first_default.fade_mode  =
    title_initial_first_default.fade_mode;
  titlemessage_initial_first_default.fade_delay =
    title_initial_first_default.fade_delay;
  titlemessage_initial_first_default.post_delay =
    title_initial_first_default.post_delay;
  titlemessage_initial_first_default.auto_delay =
    title_initial_first_default.auto_delay;
  titlemessage_first_default.fade_mode  = title_first_default.fade_mode;
  titlemessage_first_default.fade_delay = title_first_default.fade_delay;
  titlemessage_first_default.post_delay = title_first_default.post_delay;
  titlemessage_first_default.auto_delay = title_first_default.auto_delay;

  titlescreen_initial_default.fade_mode  = title_initial_default.fade_mode;
  titlescreen_initial_default.fade_delay = title_initial_default.fade_delay;
  titlescreen_initial_default.post_delay = title_initial_default.post_delay;
  titlescreen_initial_default.auto_delay = title_initial_default.auto_delay;
  titlescreen_default.fade_mode  = title_default.fade_mode;
  titlescreen_default.fade_delay = title_default.fade_delay;
  titlescreen_default.post_delay = title_default.post_delay;
  titlescreen_default.auto_delay = title_default.auto_delay;
  titlemessage_initial_default.fade_mode  = title_initial_default.fade_mode;
  titlemessage_initial_default.fade_delay = title_initial_default.fade_delay;
  titlemessage_initial_default.post_delay = title_initial_default.post_delay;
  titlemessage_initial_default.auto_delay = title_initial_default.auto_delay;
  titlemessage_default.fade_mode  = title_default.fade_mode;
  titlemessage_default.fade_delay = title_default.fade_delay;
  titlemessage_default.post_delay = title_default.post_delay;
  titlemessage_default.auto_delay = title_default.auto_delay;

  /* special case: initialize "ARG_DEFAULT" values in static default config */
  /* (e.g., init "titlemessage_1.fade_mode" from "[titlemessage].fade_mode") */
  for (i = 0; i < MAX_NUM_TITLE_MESSAGES; i++)
  {
    titlescreen_initial_first[i] = titlescreen_initial_first_default;
    titlescreen_first[i] = titlescreen_first_default;
    titlemessage_initial_first[i] = titlemessage_initial_first_default;
    titlemessage_first[i] = titlemessage_first_default;

    titlescreen_initial[i] = titlescreen_initial_default;
    titlescreen[i] = titlescreen_default;
    titlemessage_initial[i] = titlemessage_initial_default;
    titlemessage[i] = titlemessage_default;
  }

  /* special case: initialize "ARG_DEFAULT" values in static default config */
  /* (eg, init "menu.enter_screen.SCORES.xyz" from "menu.enter_screen.xyz") */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    if (i == GFX_SPECIAL_ARG_TITLE)	/* title values already initialized */
      continue;

    menu.enter_screen[i] = menu.enter_screen[GFX_SPECIAL_ARG_DEFAULT];
    menu.leave_screen[i] = menu.leave_screen[GFX_SPECIAL_ARG_DEFAULT];
    menu.next_screen[i]  = menu.next_screen[GFX_SPECIAL_ARG_DEFAULT];
  }

  /* special case: initialize "ARG_DEFAULT" values in static default config */
  /* (eg, init "viewport.door_1.MAIN.xyz" from "viewport.door_1.xyz") */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    viewport.window[i]    = viewport.window[GFX_SPECIAL_ARG_DEFAULT];
    viewport.playfield[i] = viewport.playfield[GFX_SPECIAL_ARG_DEFAULT];
    viewport.door_1[i]    = viewport.door_1[GFX_SPECIAL_ARG_DEFAULT];

    if (i == GFX_SPECIAL_ARG_EDITOR)	/* editor values already initialized */
      continue;

    viewport.door_2[i] = viewport.door_2[GFX_SPECIAL_ARG_DEFAULT];
  }
}

static void InitMenuDesignSettings_SpecialPostProcessing()
{
  static struct
  {
    struct XY *dst, *src;
  }
  game_buttons_xy[] =
  {
    { &game.button.save,	&game.button.stop	},
    { &game.button.pause2,	&game.button.pause	},
    { &game.button.load,	&game.button.play	},
    { &game.button.undo,	&game.button.stop	},
    { &game.button.redo,	&game.button.play	},

    { NULL,			NULL			}
  };
  int i;

  /* special case: initialize later added SETUP list size from LEVELS value */
  if (menu.list_size[GAME_MODE_SETUP] == -1)
    menu.list_size[GAME_MODE_SETUP] = menu.list_size[GAME_MODE_LEVELS];

  /* set default position for snapshot buttons to stop/pause/play buttons */
  for (i = 0; game_buttons_xy[i].dst != NULL; i++)
    if ((*game_buttons_xy[i].dst).x == -1 &&
	(*game_buttons_xy[i].dst).y == -1)
      *game_buttons_xy[i].dst = *game_buttons_xy[i].src;
}

static void InitMenuDesignSettings_SpecialPostProcessing_AfterGraphics()
{
  static struct
  {
    struct XYTileSize *dst, *src;
    int graphic;
  }
  editor_buttons_xy[] =
  {
    {
      &editor.button.element_left,	&editor.palette.element_left,
      IMG_GFX_EDITOR_BUTTON_ELEMENT_LEFT
    },
    {
      &editor.button.element_middle,	&editor.palette.element_middle,
      IMG_GFX_EDITOR_BUTTON_ELEMENT_MIDDLE
    },
    {
      &editor.button.element_right,	&editor.palette.element_right,
      IMG_GFX_EDITOR_BUTTON_ELEMENT_RIGHT
    },

    { NULL,			NULL			}
  };
  int i;

  /* set default position for element buttons to element graphics */
  for (i = 0; editor_buttons_xy[i].dst != NULL; i++)
  {
    if ((*editor_buttons_xy[i].dst).x == -1 &&
	(*editor_buttons_xy[i].dst).y == -1)
    {
      struct GraphicInfo *gd = &graphic_info[editor_buttons_xy[i].graphic];

      gd->width = gd->height = editor_buttons_xy[i].src->tile_size;

      *editor_buttons_xy[i].dst = *editor_buttons_xy[i].src;
    }
  }
}

static void LoadMenuDesignSettingsFromFilename(char *filename)
{
  static struct TitleFadingInfo tfi;
  static struct TitleMessageInfo tmi;
  static struct TokenInfo title_tokens[] =
  {
    { TYPE_INTEGER,	&tfi.fade_mode,		".fade_mode"		},
    { TYPE_INTEGER,	&tfi.fade_delay,	".fade_delay"		},
    { TYPE_INTEGER,	&tfi.post_delay,	".post_delay"		},
    { TYPE_INTEGER,	&tfi.auto_delay,	".auto_delay"		},

    { -1,		NULL,			NULL			}
  };
  static struct TokenInfo titlemessage_tokens[] =
  {
    { TYPE_INTEGER,	&tmi.x,			".x"			},
    { TYPE_INTEGER,	&tmi.y,			".y"			},
    { TYPE_INTEGER,	&tmi.width,		".width"		},
    { TYPE_INTEGER,	&tmi.height,		".height"		},
    { TYPE_INTEGER,	&tmi.chars,		".chars"		},
    { TYPE_INTEGER,	&tmi.lines,		".lines"		},
    { TYPE_INTEGER,	&tmi.align,		".align"		},
    { TYPE_INTEGER,	&tmi.valign,		".valign"		},
    { TYPE_INTEGER,	&tmi.font,		".font"			},
    { TYPE_BOOLEAN,	&tmi.autowrap,		".autowrap"		},
    { TYPE_BOOLEAN,	&tmi.centered,		".centered"		},
    { TYPE_BOOLEAN,	&tmi.parse_comments,	".parse_comments"	},
    { TYPE_INTEGER,	&tmi.sort_priority,	".sort_priority"	},
    { TYPE_INTEGER,	&tmi.fade_mode,		".fade_mode"		},
    { TYPE_INTEGER,	&tmi.fade_delay,	".fade_delay"		},
    { TYPE_INTEGER,	&tmi.post_delay,	".post_delay"		},
    { TYPE_INTEGER,	&tmi.auto_delay,	".auto_delay"		},

    { -1,		NULL,			NULL			}
  };
  static struct
  {
    struct TitleFadingInfo *info;
    char *text;
  }
  title_info[] =
  {
    /* initialize first titles from "enter screen" definitions, if defined */
    { &title_initial_first_default,	"menu.enter_screen.TITLE"	},
    { &title_first_default,		"menu.enter_screen.TITLE"	},

    /* initialize title screens from "next screen" definitions, if defined */
    { &title_initial_default,		"menu.next_screen.TITLE"	},
    { &title_default,			"menu.next_screen.TITLE"	},

    { NULL,				NULL				}
  };
  static struct
  {
    struct TitleMessageInfo *array;
    char *text;
  }
  titlemessage_arrays[] =
  {
    /* initialize first titles from "enter screen" definitions, if defined */
    { titlescreen_initial_first,	"menu.enter_screen.TITLE"	},
    { titlescreen_first,		"menu.enter_screen.TITLE"	},
    { titlemessage_initial_first,	"menu.enter_screen.TITLE"	},
    { titlemessage_first,		"menu.enter_screen.TITLE"	},

    /* initialize titles from "next screen" definitions, if defined */
    { titlescreen_initial,		"menu.next_screen.TITLE"	},
    { titlescreen,			"menu.next_screen.TITLE"	},
    { titlemessage_initial,		"menu.next_screen.TITLE"	},
    { titlemessage,			"menu.next_screen.TITLE"	},

    /* overwrite titles with title definitions, if defined */
    { titlescreen_initial_first,	"[title_initial]"		},
    { titlescreen_first,		"[title]"			},
    { titlemessage_initial_first,	"[title_initial]"		},
    { titlemessage_first,		"[title]"			},

    { titlescreen_initial,		"[title_initial]"		},
    { titlescreen,			"[title]"			},
    { titlemessage_initial,		"[title_initial]"		},
    { titlemessage,			"[title]"			},

    /* overwrite titles with title screen/message definitions, if defined */
    { titlescreen_initial_first,	"[titlescreen_initial]"		},
    { titlescreen_first,		"[titlescreen]"			},
    { titlemessage_initial_first,	"[titlemessage_initial]"	},
    { titlemessage_first,		"[titlemessage]"		},

    { titlescreen_initial,		"[titlescreen_initial]"		},
    { titlescreen,			"[titlescreen]"			},
    { titlemessage_initial,		"[titlemessage_initial]"	},
    { titlemessage,			"[titlemessage]"		},

    { NULL,				NULL				}
  };
  SetupFileHash *setup_file_hash;
  int i, j, k;

  if ((setup_file_hash = loadSetupFileHash(filename)) == NULL)
    return;

  /* the following initializes hierarchical values from dynamic configuration */

  /* special case: initialize with default values that may be overwritten */
  /* (e.g., init "menu.draw_xoffset.INFO" from "menu.draw_xoffset") */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    char *value_1 = getHashEntry(setup_file_hash, "menu.draw_xoffset");
    char *value_2 = getHashEntry(setup_file_hash, "menu.draw_yoffset");
    char *value_3 = getHashEntry(setup_file_hash, "menu.list_size");

    if (value_1 != NULL)
      menu.draw_xoffset[i] = get_integer_from_string(value_1);
    if (value_2 != NULL)
      menu.draw_yoffset[i] = get_integer_from_string(value_2);
    if (value_3 != NULL)
      menu.list_size[i] = get_integer_from_string(value_3);
  }

  /* special case: initialize with default values that may be overwritten */
  /* (eg, init "menu.draw_xoffset.INFO[XXX]" from "menu.draw_xoffset.INFO") */
  for (i = 0; i < NUM_SPECIAL_GFX_INFO_ARGS; i++)
  {
    char *value_1 = getHashEntry(setup_file_hash, "menu.draw_xoffset.INFO");
    char *value_2 = getHashEntry(setup_file_hash, "menu.draw_yoffset.INFO");

    if (value_1 != NULL)
      menu.draw_xoffset_info[i] = get_integer_from_string(value_1);
    if (value_2 != NULL)
      menu.draw_yoffset_info[i] = get_integer_from_string(value_2);

    if (i == GFX_SPECIAL_ARG_INFO_ELEMENTS)
    {
      char *value_1 = getHashEntry(setup_file_hash, "menu.list_size.INFO");

      if (value_1 != NULL)
	menu.list_size_info[i] = get_integer_from_string(value_1);
    }
  }

  /* special case: initialize with default values that may be overwritten */
  /* (eg, init "menu.draw_xoffset.SETUP[XXX]" from "menu.draw_xoffset.SETUP") */
  for (i = 0; i < NUM_SPECIAL_GFX_SETUP_ARGS; i++)
  {
    char *value_1 = getHashEntry(setup_file_hash, "menu.draw_xoffset.SETUP");
    char *value_2 = getHashEntry(setup_file_hash, "menu.draw_yoffset.SETUP");

    if (value_1 != NULL)
      menu.draw_xoffset_setup[i] = get_integer_from_string(value_1);
    if (value_2 != NULL)
      menu.draw_yoffset_setup[i] = get_integer_from_string(value_2);
  }

  /* special case: initialize with default values that may be overwritten */
  /* (eg, init "menu.line_spacing.INFO[XXX]" from "menu.line_spacing.INFO") */
  for (i = 0; i < NUM_SPECIAL_GFX_INFO_ARGS; i++)
  {
    char *value_1 = getHashEntry(setup_file_hash,"menu.left_spacing.INFO");
    char *value_2 = getHashEntry(setup_file_hash,"menu.right_spacing.INFO");
    char *value_3 = getHashEntry(setup_file_hash,"menu.top_spacing.INFO");
    char *value_4 = getHashEntry(setup_file_hash,"menu.bottom_spacing.INFO");
    char *value_5 = getHashEntry(setup_file_hash,"menu.paragraph_spacing.INFO");
    char *value_6 = getHashEntry(setup_file_hash,"menu.headline1_spacing.INFO");
    char *value_7 = getHashEntry(setup_file_hash,"menu.headline2_spacing.INFO");
    char *value_8 = getHashEntry(setup_file_hash,"menu.line_spacing.INFO");
    char *value_9 = getHashEntry(setup_file_hash,"menu.extra_spacing.INFO");

    if (value_1 != NULL)
      menu.left_spacing_info[i]      = get_integer_from_string(value_1);
    if (value_2 != NULL)
      menu.right_spacing_info[i]     = get_integer_from_string(value_2);
    if (value_3 != NULL)
      menu.top_spacing_info[i]       = get_integer_from_string(value_3);
    if (value_4 != NULL)
      menu.bottom_spacing_info[i]    = get_integer_from_string(value_4);
    if (value_5 != NULL)
      menu.paragraph_spacing_info[i] = get_integer_from_string(value_5);
    if (value_6 != NULL)
      menu.headline1_spacing_info[i] = get_integer_from_string(value_6);
    if (value_7 != NULL)
      menu.headline2_spacing_info[i] = get_integer_from_string(value_7);
    if (value_8 != NULL)
      menu.line_spacing_info[i]      = get_integer_from_string(value_8);
    if (value_9 != NULL)
      menu.extra_spacing_info[i]     = get_integer_from_string(value_9);
  }

  /* special case: initialize with default values that may be overwritten */
  /* (eg, init "menu.enter_screen.SCORES.xyz" from "menu.enter_screen.xyz") */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    char *token_1 = "menu.enter_screen.fade_mode";
    char *token_2 = "menu.enter_screen.fade_delay";
    char *token_3 = "menu.enter_screen.post_delay";
    char *token_4 = "menu.leave_screen.fade_mode";
    char *token_5 = "menu.leave_screen.fade_delay";
    char *token_6 = "menu.leave_screen.post_delay";
    char *token_7 = "menu.next_screen.fade_mode";
    char *token_8 = "menu.next_screen.fade_delay";
    char *token_9 = "menu.next_screen.post_delay";
    char *value_1 = getHashEntry(setup_file_hash, token_1);
    char *value_2 = getHashEntry(setup_file_hash, token_2);
    char *value_3 = getHashEntry(setup_file_hash, token_3);
    char *value_4 = getHashEntry(setup_file_hash, token_4);
    char *value_5 = getHashEntry(setup_file_hash, token_5);
    char *value_6 = getHashEntry(setup_file_hash, token_6);
    char *value_7 = getHashEntry(setup_file_hash, token_7);
    char *value_8 = getHashEntry(setup_file_hash, token_8);
    char *value_9 = getHashEntry(setup_file_hash, token_9);

    if (value_1 != NULL)
      menu.enter_screen[i].fade_mode = get_token_parameter_value(token_1,
								 value_1);
    if (value_2 != NULL)
      menu.enter_screen[i].fade_delay = get_token_parameter_value(token_2,
								  value_2);
    if (value_3 != NULL)
      menu.enter_screen[i].post_delay = get_token_parameter_value(token_3,
								  value_3);
    if (value_4 != NULL)
      menu.leave_screen[i].fade_mode = get_token_parameter_value(token_4,
								 value_4);
    if (value_5 != NULL)
      menu.leave_screen[i].fade_delay = get_token_parameter_value(token_5,
								  value_5);
    if (value_6 != NULL)
      menu.leave_screen[i].post_delay = get_token_parameter_value(token_6,
								  value_6);
    if (value_7 != NULL)
      menu.next_screen[i].fade_mode = get_token_parameter_value(token_7,
								value_7);
    if (value_8 != NULL)
      menu.next_screen[i].fade_delay = get_token_parameter_value(token_8,
								 value_8);
    if (value_9 != NULL)
      menu.next_screen[i].post_delay = get_token_parameter_value(token_9,
								 value_9);
  }

  /* special case: initialize with default values that may be overwritten */
  /* (eg, init "viewport.door_1.MAIN.xyz" from "viewport.door_1.xyz") */
  for (i = 0; i < NUM_SPECIAL_GFX_ARGS; i++)
  {
    char *token_w1 = "viewport.window.width";
    char *token_w2 = "viewport.window.height";
    char *token_01 = "viewport.playfield.x";
    char *token_02 = "viewport.playfield.y";
    char *token_03 = "viewport.playfield.width";
    char *token_04 = "viewport.playfield.height";
    char *token_05 = "viewport.playfield.border_size";
    char *token_06 = "viewport.door_1.x";
    char *token_07 = "viewport.door_1.y";
    char *token_08 = "viewport.door_1.width";
    char *token_09 = "viewport.door_1.height";
    char *token_10 = "viewport.door_1.border_size";
    char *token_11 = "viewport.door_2.x";
    char *token_12 = "viewport.door_2.y";
    char *token_13 = "viewport.door_2.width";
    char *token_14 = "viewport.door_2.height";
    char *token_15 = "viewport.door_2.border_size";
    char *value_w1 = getHashEntry(setup_file_hash, token_w1);
    char *value_w2 = getHashEntry(setup_file_hash, token_w2);
    char *value_01 = getHashEntry(setup_file_hash, token_01);
    char *value_02 = getHashEntry(setup_file_hash, token_02);
    char *value_03 = getHashEntry(setup_file_hash, token_03);
    char *value_04 = getHashEntry(setup_file_hash, token_04);
    char *value_05 = getHashEntry(setup_file_hash, token_05);
    char *value_06 = getHashEntry(setup_file_hash, token_06);
    char *value_07 = getHashEntry(setup_file_hash, token_07);
    char *value_08 = getHashEntry(setup_file_hash, token_08);
    char *value_09 = getHashEntry(setup_file_hash, token_09);
    char *value_10 = getHashEntry(setup_file_hash, token_10);
    char *value_11 = getHashEntry(setup_file_hash, token_11);
    char *value_12 = getHashEntry(setup_file_hash, token_12);
    char *value_13 = getHashEntry(setup_file_hash, token_13);
    char *value_14 = getHashEntry(setup_file_hash, token_14);
    char *value_15 = getHashEntry(setup_file_hash, token_15);

    if (value_w1 != NULL)
      viewport.window[i].width = get_token_parameter_value(token_w1, value_w1);
    if (value_w2 != NULL)
      viewport.window[i].height = get_token_parameter_value(token_w2, value_w2);
    if (value_01 != NULL)
      viewport.playfield[i].x = get_token_parameter_value(token_01, value_01);
    if (value_02 != NULL)
      viewport.playfield[i].y = get_token_parameter_value(token_02, value_02);
    if (value_03 != NULL)
      viewport.playfield[i].width = get_token_parameter_value(token_03,
							      value_03);
    if (value_04 != NULL)
      viewport.playfield[i].height = get_token_parameter_value(token_04,
							       value_04);
    if (value_05 != NULL)
      viewport.playfield[i].border_size = get_token_parameter_value(token_05,
								    value_05);
    if (value_06 != NULL)
      viewport.door_1[i].x = get_token_parameter_value(token_06, value_06);
    if (value_07 != NULL)
      viewport.door_1[i].y = get_token_parameter_value(token_07, value_07);
    if (value_08 != NULL)
      viewport.door_1[i].width = get_token_parameter_value(token_08, value_08);
    if (value_09 != NULL)
      viewport.door_1[i].height = get_token_parameter_value(token_09, value_09);
    if (value_10 != NULL)
      viewport.door_1[i].border_size = get_token_parameter_value(token_10,
								 value_10);
    if (value_11 != NULL)
      viewport.door_2[i].x = get_token_parameter_value(token_11, value_11);
    if (value_12 != NULL)
      viewport.door_2[i].y = get_token_parameter_value(token_12, value_12);
    if (value_13 != NULL)
      viewport.door_2[i].width = get_token_parameter_value(token_13, value_13);
    if (value_14 != NULL)
      viewport.door_2[i].height = get_token_parameter_value(token_14, value_14);
    if (value_15 != NULL)
      viewport.door_1[i].border_size = get_token_parameter_value(token_15,
								 value_15);
  }

  /* special case: initialize with default values that may be overwritten */
  /* (e.g., init "[title].fade_mode" from "menu.next_screen.TITLE.fade_mode") */
  for (i = 0; title_info[i].info != NULL; i++)
  {
    struct TitleFadingInfo *info = title_info[i].info;
    char *base_token = title_info[i].text;

    for (j = 0; title_tokens[j].type != -1; j++)
    {
      char *token = getStringCat2(base_token, title_tokens[j].text);
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
      {
	int parameter_value = get_token_parameter_value(token, value);

	tfi = *info;

	*(int *)title_tokens[j].value = (int)parameter_value;

	*info = tfi;
      }

      free(token);
    }
  }

  /* special case: initialize with default values that may be overwritten */
  /* (e.g., init "titlemessage_1.fade_mode" from "[titlemessage].fade_mode") */
  for (i = 0; titlemessage_arrays[i].array != NULL; i++)
  {
    struct TitleMessageInfo *array = titlemessage_arrays[i].array;
    char *base_token = titlemessage_arrays[i].text;

    for (j = 0; titlemessage_tokens[j].type != -1; j++)
    {
      char *token = getStringCat2(base_token, titlemessage_tokens[j].text);
      char *value = getHashEntry(setup_file_hash, token);

      if (value != NULL)
      {
	int parameter_value = get_token_parameter_value(token, value);

	for (k = 0; k < MAX_NUM_TITLE_MESSAGES; k++)
	{
	  tmi = array[k];

	  if (titlemessage_tokens[j].type == TYPE_INTEGER)
	    *(int     *)titlemessage_tokens[j].value = (int)parameter_value;
	  else
	    *(boolean *)titlemessage_tokens[j].value = (boolean)parameter_value;

	  array[k] = tmi;
	}
      }

      free(token);
    }
  }

  /* read (and overwrite with) values that may be specified in config file */
  for (i = 0; image_config_vars[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, image_config_vars[i].token);

    /* (ignore definitions set to "[DEFAULT]" which are already initialized) */
    if (value != NULL && !strEqual(value, ARG_DEFAULT))
      *image_config_vars[i].value =
	get_token_parameter_value(image_config_vars[i].token, value);
  }

  freeSetupFileHash(setup_file_hash);
}

void LoadMenuDesignSettings()
{
  char *filename_base = UNDEFINED_FILENAME, *filename_local;

  InitMenuDesignSettings_Static();
  InitMenuDesignSettings_SpecialPreProcessing();

  if (!GFX_OVERRIDE_ARTWORK(ARTWORK_TYPE_GRAPHICS))
  {
    /* first look for special settings configured in level series config */
    filename_base = getCustomArtworkLevelConfigFilename(ARTWORK_TYPE_GRAPHICS);

    if (fileExists(filename_base))
      LoadMenuDesignSettingsFromFilename(filename_base);
  }

  filename_local = getCustomArtworkConfigFilename(ARTWORK_TYPE_GRAPHICS);

  if (filename_local != NULL && !strEqual(filename_base, filename_local))
    LoadMenuDesignSettingsFromFilename(filename_local);

  InitMenuDesignSettings_SpecialPostProcessing();
}

void LoadMenuDesignSettings_AfterGraphics()
{
  InitMenuDesignSettings_SpecialPostProcessing_AfterGraphics();
}

void LoadUserDefinedEditorElementList(int **elements, int *num_elements)
{
  char *filename = getEditorSetupFilename();
  SetupFileList *setup_file_list, *list;
  SetupFileHash *element_hash;
  int num_unknown_tokens = 0;
  int i;

  if ((setup_file_list = loadSetupFileList(filename)) == NULL)
    return;

  element_hash = newSetupFileHash();

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
    setHashEntry(element_hash, element_info[i].token_name, i_to_a(i));

  /* determined size may be larger than needed (due to unknown elements) */
  *num_elements = 0;
  for (list = setup_file_list; list != NULL; list = list->next)
    (*num_elements)++;

  /* add space for up to 3 more elements for padding that may be needed */
  *num_elements += 3;

  /* free memory for old list of elements, if needed */
  checked_free(*elements);

  /* allocate memory for new list of elements */
  *elements = checked_malloc(*num_elements * sizeof(int));

  *num_elements = 0;
  for (list = setup_file_list; list != NULL; list = list->next)
  {
    char *value = getHashEntry(element_hash, list->token);

    if (value == NULL)		/* try to find obsolete token mapping */
    {
      char *mapped_token = get_mapped_token(list->token);

      if (mapped_token != NULL)
      {
	value = getHashEntry(element_hash, mapped_token);

	free(mapped_token);
      }
    }

    if (value != NULL)
    {
      (*elements)[(*num_elements)++] = atoi(value);
    }
    else
    {
      if (num_unknown_tokens == 0)
      {
	Error(ERR_INFO_LINE, "-");
	Error(ERR_INFO, "warning: unknown token(s) found in config file:");
	Error(ERR_INFO, "- config file: '%s'", filename);

	num_unknown_tokens++;
      }

      Error(ERR_INFO, "- token: '%s'", list->token);
    }
  }

  if (num_unknown_tokens > 0)
    Error(ERR_INFO_LINE, "-");

  while (*num_elements % 4)	/* pad with empty elements, if needed */
    (*elements)[(*num_elements)++] = EL_EMPTY;

  freeSetupFileList(setup_file_list);
  freeSetupFileHash(element_hash);

#if 0
  for (i = 0; i < *num_elements; i++)
    printf("editor: element '%s' [%d]\n",
	   element_info[(*elements)[i]].token_name, (*elements)[i]);
#endif
}

static struct MusicFileInfo *get_music_file_info_ext(char *basename, int music,
						     boolean is_sound)
{
  SetupFileHash *setup_file_hash = NULL;
  struct MusicFileInfo tmp_music_file_info, *new_music_file_info;
  char *filename_music, *filename_prefix, *filename_info;
  struct
  {
    char *token;
    char **value_ptr;
  }
  token_to_value_ptr[] =
  {
    { "title_header",	&tmp_music_file_info.title_header	},
    { "artist_header",	&tmp_music_file_info.artist_header	},
    { "album_header",	&tmp_music_file_info.album_header	},
    { "year_header",	&tmp_music_file_info.year_header	},

    { "title",		&tmp_music_file_info.title		},
    { "artist",		&tmp_music_file_info.artist		},
    { "album",		&tmp_music_file_info.album		},
    { "year",		&tmp_music_file_info.year		},

    { NULL,		NULL					},
  };
  int i;

  filename_music = (is_sound ? getCustomSoundFilename(basename) :
		    getCustomMusicFilename(basename));

  if (filename_music == NULL)
    return NULL;

  /* ---------- try to replace file extension ---------- */

  filename_prefix = getStringCopy(filename_music);
  if (strrchr(filename_prefix, '.') != NULL)
    *strrchr(filename_prefix, '.') = '\0';
  filename_info = getStringCat2(filename_prefix, ".txt");

  if (fileExists(filename_info))
    setup_file_hash = loadSetupFileHash(filename_info);

  free(filename_prefix);
  free(filename_info);

  if (setup_file_hash == NULL)
  {
    /* ---------- try to add file extension ---------- */

    filename_prefix = getStringCopy(filename_music);
    filename_info = getStringCat2(filename_prefix, ".txt");

    if (fileExists(filename_info))
      setup_file_hash = loadSetupFileHash(filename_info);

    free(filename_prefix);
    free(filename_info);
  }

  if (setup_file_hash == NULL)
    return NULL;

  /* ---------- music file info found ---------- */

  clear_mem(&tmp_music_file_info, sizeof(struct MusicFileInfo));

  for (i = 0; token_to_value_ptr[i].token != NULL; i++)
  {
    char *value = getHashEntry(setup_file_hash, token_to_value_ptr[i].token);

    *token_to_value_ptr[i].value_ptr =
      getStringCopy(value != NULL && *value != '\0' ? value : UNKNOWN_NAME);
  }

  tmp_music_file_info.basename = getStringCopy(basename);
  tmp_music_file_info.music = music;
  tmp_music_file_info.is_sound = is_sound;

  new_music_file_info = checked_malloc(sizeof(struct MusicFileInfo));
  *new_music_file_info = tmp_music_file_info;

  return new_music_file_info;
}

static struct MusicFileInfo *get_music_file_info(char *basename, int music)
{
  return get_music_file_info_ext(basename, music, FALSE);
}

static struct MusicFileInfo *get_sound_file_info(char *basename, int sound)
{
  return get_music_file_info_ext(basename, sound, TRUE);
}

static boolean music_info_listed_ext(struct MusicFileInfo *list,
				     char *basename, boolean is_sound)
{
  for (; list != NULL; list = list->next)
    if (list->is_sound == is_sound && strEqual(list->basename, basename))
      return TRUE;

  return FALSE;
}

static boolean music_info_listed(struct MusicFileInfo *list, char *basename)
{
  return music_info_listed_ext(list, basename, FALSE);
}

static boolean sound_info_listed(struct MusicFileInfo *list, char *basename)
{
  return music_info_listed_ext(list, basename, TRUE);
}

void LoadMusicInfo()
{
  char *music_directory = getCustomMusicDirectory();
  int num_music = getMusicListSize();
  int num_music_noconf = 0;
  int num_sounds = getSoundListSize();
  Directory *dir;
  DirectoryEntry *dir_entry;
  struct FileInfo *music, *sound;
  struct MusicFileInfo *next, **new;
  int i;

  while (music_file_info != NULL)
  {
    next = music_file_info->next;

    checked_free(music_file_info->basename);

    checked_free(music_file_info->title_header);
    checked_free(music_file_info->artist_header);
    checked_free(music_file_info->album_header);
    checked_free(music_file_info->year_header);

    checked_free(music_file_info->title);
    checked_free(music_file_info->artist);
    checked_free(music_file_info->album);
    checked_free(music_file_info->year);

    free(music_file_info);

    music_file_info = next;
  }

  new = &music_file_info;

  for (i = 0; i < num_music; i++)
  {
    music = getMusicListEntry(i);

    if (music->filename == NULL)
      continue;

    if (strEqual(music->filename, UNDEFINED_FILENAME))
      continue;

    /* a configured file may be not recognized as music */
    if (!FileIsMusic(music->filename))
      continue;

    if (!music_info_listed(music_file_info, music->filename))
    {
      *new = get_music_file_info(music->filename, i);

      if (*new != NULL)
	new = &(*new)->next;
    }
  }

  if ((dir = openDirectory(music_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read music directory '%s'", music_directory);
    return;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	/* loop all entries */
  {
    char *basename = dir_entry->basename;
    boolean music_already_used = FALSE;
    int i;

    /* skip all music files that are configured in music config file */
    for (i = 0; i < num_music; i++)
    {
      music = getMusicListEntry(i);

      if (music->filename == NULL)
	continue;

      if (strEqual(basename, music->filename))
      {
	music_already_used = TRUE;
	break;
      }
    }

    if (music_already_used)
      continue;

    if (!FileIsMusic(dir_entry->filename))
      continue;

    if (!music_info_listed(music_file_info, basename))
    {
      *new = get_music_file_info(basename, MAP_NOCONF_MUSIC(num_music_noconf));

      if (*new != NULL)
	new = &(*new)->next;
    }

    num_music_noconf++;
  }

  closeDirectory(dir);

  for (i = 0; i < num_sounds; i++)
  {
    sound = getSoundListEntry(i);

    if (sound->filename == NULL)
      continue;

    if (strEqual(sound->filename, UNDEFINED_FILENAME))
      continue;

    /* a configured file may be not recognized as sound */
    if (!FileIsSound(sound->filename))
      continue;

    if (!sound_info_listed(music_file_info, sound->filename))
    {
      *new = get_sound_file_info(sound->filename, i);
      if (*new != NULL)
	new = &(*new)->next;
    }
  }
}

void add_helpanim_entry(int element, int action, int direction, int delay,
			int *num_list_entries)
{
  struct HelpAnimInfo *new_list_entry;
  (*num_list_entries)++;

  helpanim_info =
    checked_realloc(helpanim_info,
		    *num_list_entries * sizeof(struct HelpAnimInfo));
  new_list_entry = &helpanim_info[*num_list_entries - 1];

  new_list_entry->element = element;
  new_list_entry->action = action;
  new_list_entry->direction = direction;
  new_list_entry->delay = delay;
}

void print_unknown_token(char *filename, char *token, int token_nr)
{
  if (token_nr == 0)
  {
    Error(ERR_INFO_LINE, "-");
    Error(ERR_INFO, "warning: unknown token(s) found in config file:");
    Error(ERR_INFO, "- config file: '%s'", filename);
  }

  Error(ERR_INFO, "- token: '%s'", token);
}

void print_unknown_token_end(int token_nr)
{
  if (token_nr > 0)
    Error(ERR_INFO_LINE, "-");
}

void LoadHelpAnimInfo()
{
  char *filename = getHelpAnimFilename();
  SetupFileList *setup_file_list = NULL, *list;
  SetupFileHash *element_hash, *action_hash, *direction_hash;
  int num_list_entries = 0;
  int num_unknown_tokens = 0;
  int i;

  if (fileExists(filename))
    setup_file_list = loadSetupFileList(filename);

  if (setup_file_list == NULL)
  {
    /* use reliable default values from static configuration */
    SetupFileList *insert_ptr;

    insert_ptr = setup_file_list =
      newSetupFileList(helpanim_config[0].token,
		       helpanim_config[0].value);

    for (i = 1; helpanim_config[i].token; i++)
      insert_ptr = addListEntry(insert_ptr,
				helpanim_config[i].token,
				helpanim_config[i].value);
  }

  element_hash   = newSetupFileHash();
  action_hash    = newSetupFileHash();
  direction_hash = newSetupFileHash();

  for (i = 0; i < MAX_NUM_ELEMENTS; i++)
    setHashEntry(element_hash, element_info[i].token_name, i_to_a(i));

  for (i = 0; i < NUM_ACTIONS; i++)
    setHashEntry(action_hash, element_action_info[i].suffix,
		 i_to_a(element_action_info[i].value));

  /* do not store direction index (bit) here, but direction value! */
  for (i = 0; i < NUM_DIRECTIONS_FULL; i++)
    setHashEntry(direction_hash, element_direction_info[i].suffix,
		 i_to_a(1 << element_direction_info[i].value));

  for (list = setup_file_list; list != NULL; list = list->next)
  {
    char *element_token, *action_token, *direction_token;
    char *element_value, *action_value, *direction_value;
    int delay = atoi(list->value);

    if (strEqual(list->token, "end"))
    {
      add_helpanim_entry(HELPANIM_LIST_NEXT, -1, -1, -1, &num_list_entries);

      continue;
    }

    /* first try to break element into element/action/direction parts;
       if this does not work, also accept combined "element[.act][.dir]"
       elements (like "dynamite.active"), which are unique elements */

    if (strchr(list->token, '.') == NULL)	/* token contains no '.' */
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
      {
	/* no further suffixes found -- this is not an element */
	print_unknown_token(filename, list->token, num_unknown_tokens++);
      }

      continue;
    }

    /* token has format "<prefix>.<something>" */

    action_token = strchr(list->token, '.');	/* suffix may be action ... */
    direction_token = action_token;		/* ... or direction */

    element_token = getStringCopy(list->token);
    *strchr(element_token, '.') = '\0';

    element_value = getHashEntry(element_hash, element_token);

    if (element_value == NULL)		/* this is no element */
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* combined element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);

      continue;
    }

    action_value = getHashEntry(action_hash, action_token);

    if (action_value != NULL)		/* action found */
    {
      add_helpanim_entry(atoi(element_value), atoi(action_value), -1, delay,
		    &num_list_entries);

      free(element_token);

      continue;
    }

    direction_value = getHashEntry(direction_hash, direction_token);

    if (direction_value != NULL)	/* direction found */
    {
      add_helpanim_entry(atoi(element_value), -1, atoi(direction_value), delay,
			 &num_list_entries);

      free(element_token);

      continue;
    }

    if (strchr(action_token + 1, '.') == NULL)
    {
      /* no further suffixes found -- this is not an action nor direction */

      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* combined element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);

      continue;
    }

    /* token has format "<prefix>.<suffix>.<something>" */

    direction_token = strchr(action_token + 1, '.');

    action_token = getStringCopy(action_token);
    *strchr(action_token + 1, '.') = '\0';

    action_value = getHashEntry(action_hash, action_token);

    if (action_value == NULL)		/* this is no action */
    {
      element_value = getHashEntry(element_hash, list->token);
      if (element_value != NULL)	/* combined element found */
	add_helpanim_entry(atoi(element_value), -1, -1, delay,
			   &num_list_entries);
      else
	print_unknown_token(filename, list->token, num_unknown_tokens++);

      free(element_token);
      free(action_token);

      continue;
    }

    direction_value = getHashEntry(direction_hash, direction_token);

    if (direction_value != NULL)	/* direction found */
    {
      add_helpanim_entry(atoi(element_value), atoi(action_value),
			 atoi(direction_value), delay, &num_list_entries);

      free(element_token);
      free(action_token);

      continue;
    }

    /* this is no direction */

    element_value = getHashEntry(element_hash, list->token);
    if (element_value != NULL)		/* combined element found */
      add_helpanim_entry(atoi(element_value), -1, -1, delay,
			 &num_list_entries);
    else
      print_unknown_token(filename, list->token, num_unknown_tokens++);

    free(element_token);
    free(action_token);
  }

  print_unknown_token_end(num_unknown_tokens);

  add_helpanim_entry(HELPANIM_LIST_NEXT, -1, -1, -1, &num_list_entries);
  add_helpanim_entry(HELPANIM_LIST_END,  -1, -1, -1, &num_list_entries);

  freeSetupFileList(setup_file_list);
  freeSetupFileHash(element_hash);
  freeSetupFileHash(action_hash);
  freeSetupFileHash(direction_hash);

#if 0
  for (i = 0; i < num_list_entries; i++)
    printf("::: '%s': %d, %d, %d => %d\n",
	   EL_NAME(helpanim_info[i].element),
	   helpanim_info[i].element,
	   helpanim_info[i].action,
	   helpanim_info[i].direction,
	   helpanim_info[i].delay);
#endif
}

void LoadHelpTextInfo()
{
  char *filename = getHelpTextFilename();
  int i;

  if (helptext_info != NULL)
  {
    freeSetupFileHash(helptext_info);
    helptext_info = NULL;
  }

  if (fileExists(filename))
    helptext_info = loadSetupFileHash(filename);

  if (helptext_info == NULL)
  {
    /* use reliable default values from static configuration */
    helptext_info = newSetupFileHash();

    for (i = 0; helptext_config[i].token; i++)
      setHashEntry(helptext_info,
		   helptext_config[i].token,
		   helptext_config[i].value);
  }

#if 0
  BEGIN_HASH_ITERATION(helptext_info, itr)
  {
    printf("::: '%s' => '%s'\n",
	   HASH_ITERATION_TOKEN(itr), HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(hash, itr)
#endif
}


/* ------------------------------------------------------------------------- */
/* convert levels                                                            */
/* ------------------------------------------------------------------------- */

#define MAX_NUM_CONVERT_LEVELS		1000

void ConvertLevels()
{
  static LevelDirTree *convert_leveldir = NULL;
  static int convert_level_nr = -1;
  static int num_levels_handled = 0;
  static int num_levels_converted = 0;
  static boolean levels_failed[MAX_NUM_CONVERT_LEVELS];
  int i;

  convert_leveldir = getTreeInfoFromIdentifier(leveldir_first,
					       global.convert_leveldir);

  if (convert_leveldir == NULL)
    Error(ERR_EXIT, "no such level identifier: '%s'",
	  global.convert_leveldir);

  leveldir_current = convert_leveldir;

  if (global.convert_level_nr != -1)
  {
    convert_leveldir->first_level = global.convert_level_nr;
    convert_leveldir->last_level  = global.convert_level_nr;
  }

  convert_level_nr = convert_leveldir->first_level;

  PrintLine("=", 79);
  Print("Converting levels\n");
  PrintLine("-", 79);
  Print("Level series identifier: '%s'\n", convert_leveldir->identifier);
  Print("Level series name:       '%s'\n", convert_leveldir->name);
  Print("Level series author:     '%s'\n", convert_leveldir->author);
  Print("Number of levels:        %d\n",   convert_leveldir->levels);
  PrintLine("=", 79);
  Print("\n");

  for (i = 0; i < MAX_NUM_CONVERT_LEVELS; i++)
    levels_failed[i] = FALSE;

  while (convert_level_nr <= convert_leveldir->last_level)
  {
    char *level_filename;
    boolean new_level;

    level_nr = convert_level_nr++;

    Print("Level %03d: ", level_nr);

    LoadLevel(level_nr);
    if (level.no_level_file || level.no_valid_file)
    {
      Print("(no level)\n");
      continue;
    }

    Print("converting level ... ");

    level_filename = getDefaultLevelFilename(level_nr);
    new_level = !fileExists(level_filename);

    if (new_level)
    {
      SaveLevel(level_nr);

      num_levels_converted++;

      Print("converted.\n");
    }
    else
    {
      if (level_nr >= 0 && level_nr < MAX_NUM_CONVERT_LEVELS)
	levels_failed[level_nr] = TRUE;

      Print("NOT CONVERTED -- LEVEL ALREADY EXISTS.\n");
    }

    num_levels_handled++;
  }

  Print("\n");
  PrintLine("=", 79);
  Print("Number of levels handled: %d\n", num_levels_handled);
  Print("Number of levels converted: %d (%d%%)\n", num_levels_converted,
	 (num_levels_handled ?
	  num_levels_converted * 100 / num_levels_handled : 0));
  PrintLine("-", 79);
  Print("Summary (for automatic parsing by scripts):\n");
  Print("LEVELDIR '%s', CONVERTED %d/%d (%d%%)",
	 convert_leveldir->identifier, num_levels_converted,
	 num_levels_handled,
	 (num_levels_handled ?
	  num_levels_converted * 100 / num_levels_handled : 0));

  if (num_levels_handled != num_levels_converted)
  {
    Print(", FAILED:");
    for (i = 0; i < MAX_NUM_CONVERT_LEVELS; i++)
      if (levels_failed[i])
	Print(" %03d", i);
  }

  Print("\n");
  PrintLine("=", 79);

  CloseAllAndExit(0);
}


/* ------------------------------------------------------------------------- */
/* create and save images for use in level sketches (raw BMP format)         */
/* ------------------------------------------------------------------------- */

void CreateLevelSketchImages()
{
#if defined(TARGET_SDL)
  Bitmap *bitmap1;
  Bitmap *bitmap2;
  int i;

  InitElementPropertiesGfxElement();

  bitmap1 = CreateBitmap(TILEX, TILEY, DEFAULT_DEPTH);
  bitmap2 = CreateBitmap(MINI_TILEX, MINI_TILEY, DEFAULT_DEPTH);

  for (i = 0; i < NUM_FILE_ELEMENTS; i++)
  {
    Bitmap *src_bitmap;
    int src_x, src_y;
    int element = getMappedElement(i);
    int graphic = el2edimg(element);
    char basename1[16];
    char basename2[16];
    char *filename1;
    char *filename2;

    sprintf(basename1, "%03d.bmp", i);
    sprintf(basename2, "%03ds.bmp", i);

    filename1 = getPath2(global.create_images_dir, basename1);
    filename2 = getPath2(global.create_images_dir, basename2);

    getFixedGraphicSource(graphic, 0, &src_bitmap, &src_x, &src_y);
    BlitBitmap(src_bitmap, bitmap1, src_x, src_y, TILEX, TILEY,
	       0, 0);

    if (SDL_SaveBMP(bitmap1->surface, filename1) != 0)
      Error(ERR_EXIT, "cannot save level sketch image file '%s'", filename1);

    getMiniGraphicSource(graphic, &src_bitmap, &src_x, &src_y);
    BlitBitmap(src_bitmap, bitmap2, src_x, src_y, MINI_TILEX, MINI_TILEY, 0, 0);

    if (SDL_SaveBMP(bitmap2->surface, filename2) != 0)
      Error(ERR_EXIT, "cannot save level sketch image file '%s'", filename2);

    free(filename1);
    free(filename2);

    if (options.debug)
      printf("%03d `%03d%c", i, i, (i % 10 < 9 ? ' ' : '\n'));
  }

  FreeBitmap(bitmap1);
  FreeBitmap(bitmap2);

  if (options.debug)
    printf("\n");

  Error(ERR_INFO, "%d normal and small images created", NUM_FILE_ELEMENTS);

  CloseAllAndExit(0);
#endif
}


/* ------------------------------------------------------------------------- */
/* create and save images for custom and group elements (raw BMP format)     */
/* ------------------------------------------------------------------------- */

void CreateCustomElementImages(char *directory)
{
#if defined(TARGET_SDL)
  char *src_basename = "RocksCE-template.ilbm";
  char *dst_basename = "RocksCE.bmp";
  char *src_filename = getPath2(directory, src_basename);
  char *dst_filename = getPath2(directory, dst_basename);
  Bitmap *src_bitmap;
  Bitmap *bitmap;
  int yoffset_ce = 0;
  int yoffset_ge = (TILEY * NUM_CUSTOM_ELEMENTS / 16);
  int i;

  SDLInitVideoDisplay();

  ReCreateBitmap(&backbuffer, video.width, video.height);

  src_bitmap = LoadImage(src_filename);

  bitmap = CreateBitmap(TILEX * 16 * 2,
			TILEY * (NUM_CUSTOM_ELEMENTS + NUM_GROUP_ELEMENTS) / 16,
			DEFAULT_DEPTH);

  for (i = 0; i < NUM_CUSTOM_ELEMENTS; i++)
  {
    int x = i % 16;
    int y = i / 16;
    int ii = i + 1;
    int j;

    BlitBitmap(src_bitmap, bitmap, 0, 0, TILEX, TILEY,
	       TILEX * x, TILEY * y + yoffset_ce);

    BlitBitmap(src_bitmap, bitmap, 0, TILEY,
	       TILEX, TILEY,
	       TILEX * x + TILEX * 16,
	       TILEY * y + yoffset_ce);

    for (j = 2; j >= 0; j--)
    {
      int c = ii % 10;

      BlitBitmap(src_bitmap, bitmap,
		 TILEX + c * 7, 0, 6, 10,
		 TILEX * x + 6 + j * 7,
		 TILEY * y + 11 + yoffset_ce);

      BlitBitmap(src_bitmap, bitmap,
		 TILEX + c * 8, TILEY, 6, 10,
		 TILEX * 16 + TILEX * x + 6 + j * 8,
		 TILEY * y + 10 + yoffset_ce);

      ii /= 10;
    }
  }

  for (i = 0; i < NUM_GROUP_ELEMENTS; i++)
  {
    int x = i % 16;
    int y = i / 16;
    int ii = i + 1;
    int j;

    BlitBitmap(src_bitmap, bitmap, 0, 0, TILEX, TILEY,
	       TILEX * x, TILEY * y + yoffset_ge);

    BlitBitmap(src_bitmap, bitmap, 0, TILEY,
	       TILEX, TILEY,
	       TILEX * x + TILEX * 16,
	       TILEY * y + yoffset_ge);

    for (j = 1; j >= 0; j--)
    {
      int c = ii % 10;

      BlitBitmap(src_bitmap, bitmap, TILEX + c * 10, 11, 10, 10,
		 TILEX * x + 6 + j * 10,
		 TILEY * y + 11 + yoffset_ge);

      BlitBitmap(src_bitmap, bitmap,
		 TILEX + c * 8, TILEY + 12, 6, 10,
		 TILEX * 16 + TILEX * x + 10 + j * 8,
		 TILEY * y + 10 + yoffset_ge);

      ii /= 10;
    }
  }

  if (SDL_SaveBMP(bitmap->surface, dst_filename) != 0)
    Error(ERR_EXIT, "cannot save CE graphics file '%s'", dst_filename);

  FreeBitmap(bitmap);

  CloseAllAndExit(0);
#endif
}
