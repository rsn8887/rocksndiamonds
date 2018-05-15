// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_files.c
// ============================================================================

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>

#include "main_mm.h"

#include "mm_main.h"

#define CHUNK_ID_LEN		4	/* IFF style chunk id length */
#define CHUNK_SIZE_UNDEFINED	0	/* undefined chunk size == 0  */
#define CHUNK_SIZE_NONE		-1	/* do not write chunk size    */
#define FILE_VERS_CHUNK_SIZE	8	/* size of file version chunk */
#define LEVEL_HEADER_SIZE	80	/* size of level file header */
#define LEVEL_HEADER_UNUSED	19	/* unused level header bytes */

/* file identifier strings */
#define LEVEL_COOKIE_TMPL	"MIRRORMAGIC_LEVEL_FILE_VERSION_x.x"
#define SCORE_COOKIE		"MIRRORMAGIC_SCORE_FILE_VERSION_1.4"


int default_score[LEVEL_SCORE_ELEMENTS] =
{
  [SC_COLLECTIBLE]	= 10,
  [SC_PACMAN]		= 50,
  [SC_KEY]		= 10,
  [SC_TIME_BONUS]	= 1,
  [SC_LIGHTBALL]	= 10,
};


/* ========================================================================= */
/* level file functions                                                      */
/* ========================================================================= */

static void ReadChunk_MM_VERS(File *file, int *file_version, int *game_version)
{
  int file_version_major, file_version_minor, file_version_patch;
  int game_version_major, game_version_minor, game_version_patch;

  file_version_major = getFile8Bit(file);
  file_version_minor = getFile8Bit(file);
  file_version_patch = getFile8Bit(file);
  getFile8Bit(file);		/* not used */

  game_version_major = getFile8Bit(file);
  game_version_minor = getFile8Bit(file);
  game_version_patch = getFile8Bit(file);
  getFile8Bit(file);		/* not used */

  *file_version = MM_VERSION_IDENT(file_version_major,
				   file_version_minor,
				   file_version_patch);

  *game_version = MM_VERSION_IDENT(game_version_major,
				   game_version_minor,
				   game_version_patch);
}

static void WriteChunk_MM_VERS(FILE *file, int file_version, int game_version)
{
  int file_version_major = MM_VERSION_MAJOR(file_version);
  int file_version_minor = MM_VERSION_MINOR(file_version);
  int file_version_patch = MM_VERSION_PATCH(file_version);
  int game_version_major = MM_VERSION_MAJOR(game_version);
  int game_version_minor = MM_VERSION_MINOR(game_version);
  int game_version_patch = MM_VERSION_PATCH(game_version);

  fputc(file_version_major, file);
  fputc(file_version_minor, file);
  fputc(file_version_patch, file);
  fputc(0, file);	/* not used */

  fputc(game_version_major, file);
  fputc(game_version_minor, file);
  fputc(game_version_patch, file);
  fputc(0, file);	/* not used */
}

void setLevelInfoToDefaults_MM()
{
  int i, x, y;

  native_mm_level.file_version = MM_FILE_VERSION_ACTUAL;
  native_mm_level.game_version = MM_GAME_VERSION_ACTUAL;

  native_mm_level.encoding_16bit_field = FALSE;	/* default: only 8-bit elements */

  lev_fieldx = native_mm_level.fieldx = STD_LEV_FIELDX;
  lev_fieldy = native_mm_level.fieldy = STD_LEV_FIELDY;

  for (x = 0; x < MAX_LEV_FIELDX; x++)
    for (y = 0; y < MAX_LEV_FIELDY; y++)
      native_mm_level.field[x][y] = Feld[x][y] = Ur[x][y] = EL_EMPTY;

  native_mm_level.time = 100;
  native_mm_level.kettles_needed = 0;
  native_mm_level.auto_count_kettles = TRUE;
  native_mm_level.amoeba_speed = 0;
  native_mm_level.time_fuse = 25;
  native_mm_level.time_bomb = 75;
  native_mm_level.time_ball = 75;
  native_mm_level.time_block = 75;
  native_mm_level.laser_red = FALSE;
  native_mm_level.laser_green = FALSE;
  native_mm_level.laser_blue = TRUE;

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    native_mm_level.name[i] = '\0';
  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    native_mm_level.author[i] = '\0';

  strcpy(native_mm_level.name, NAMELESS_LEVEL_NAME);
  strcpy(native_mm_level.author, ANONYMOUS_NAME);

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    native_mm_level.score[i] = 10;

  native_mm_level.field[0][0] = Feld[0][0] = Ur[0][0] = EL_MCDUFFIN_RIGHT;
  native_mm_level.field[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] =
    Feld[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] =
    Ur[STD_LEV_FIELDX-1][STD_LEV_FIELDY-1] = EL_EXIT_CLOSED;
}

static int checkLevelElement(int element)
{
  if (element >= EL_FIRST_RUNTIME_EL)
  {
    Error(ERR_WARN, "invalid level element %d", element);
    element = EL_CHAR_FRAGE;
  }

  return element;
}

static int LoadLevel_MM_VERS(File *file, int chunk_size,
			     struct LevelInfo_MM *level)
{
  ReadChunk_MM_VERS(file, &level->file_version, &level->game_version);

  return chunk_size;
}

static int LoadLevel_MM_HEAD(File *file, int chunk_size,
			     struct LevelInfo_MM *level)
{
  int i;
  int laser_color;

  lev_fieldx = level->fieldx = getFile8Bit(file);
  lev_fieldy = level->fieldy = getFile8Bit(file);

  level->time           = getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN);
  level->kettles_needed = getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN);

  // one time unit was equivalent to four seconds in level files up to 2.0.x
  if (level->file_version <= MM_FILE_VERSION_2_0)
    level->time *= 4;

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    level->name[i] = getFile8Bit(file);
  level->name[MAX_LEVEL_NAME_LEN] = 0;

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    level->score[i] = getFile8Bit(file);

  // scores were 0 and hardcoded in game engine in level files up to 2.0.x
  if (level->file_version <= MM_FILE_VERSION_2_0)
    for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
      if (level->score[i] == 0)
	level->score[i] = default_score[i];

  level->auto_count_kettles	= (getFile8Bit(file) == 1 ? TRUE : FALSE);
  level->amoeba_speed		= getFile8Bit(file);
  level->time_fuse		= getFile8Bit(file);

  // fuse time was 0 and hardcoded in game engine in level files up to 2.0.x
  if (level->file_version <= MM_FILE_VERSION_2_0)
    level->time_fuse = 25;

  laser_color			= getFile8Bit(file);
  level->laser_red		= (laser_color >> 2) & 0x01;
  level->laser_green		= (laser_color >> 1) & 0x01;
  level->laser_blue		= (laser_color >> 0) & 0x01;

  level->encoding_16bit_field	= (getFile8Bit(file) == 1 ? TRUE : FALSE);

  ReadUnusedBytesFromFile(file, LEVEL_HEADER_UNUSED);

  return chunk_size;
}

static int LoadLevel_MM_AUTH(File *file, int chunk_size,
			     struct LevelInfo_MM *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    level->author[i] = getFile8Bit(file);
  level->author[MAX_LEVEL_NAME_LEN] = 0;

  return chunk_size;
}

static int LoadLevel_MM_BODY(File *file, int chunk_size,
			     struct LevelInfo_MM *level)
{
  int x, y;
  int chunk_size_expected = level->fieldx * level->fieldy;

  /* Note: "chunk_size" was wrong before version 2.0 when elements are
     stored with 16-bit encoding (and should be twice as big then).
     Even worse, playfield data was stored 16-bit when only yamyam content
     contained 16-bit elements and vice versa. */

  if (level->encoding_16bit_field && level->file_version >= MM_FILE_VERSION_2_0)
    chunk_size_expected *= 2;

  if (chunk_size_expected != chunk_size)
  {
    ReadUnusedBytesFromFile(file, chunk_size);

    return chunk_size_expected;
  }

  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      native_mm_level.field[x][y] = Feld[x][y] = Ur[x][y] =
	checkLevelElement(level->encoding_16bit_field ?
			  getFile16BitInteger(file, BYTE_ORDER_BIG_ENDIAN) :
			  getFile8Bit(file));
  return chunk_size;
}

boolean LoadNativeLevel_MM(char *filename, boolean level_info_only)
{
  char cookie[MAX_LINE_LEN];
  char chunk_name[CHUNK_ID_LEN + 1];
  int chunk_size;
  File *file;

  static struct
  {
    char *name;
    int size;
    int (*loader)(File *, int, struct LevelInfo_MM *);
  }
  chunk_info[] =
  {
    { "VERS", FILE_VERS_CHUNK_SIZE,	LoadLevel_MM_VERS },
    { "HEAD", LEVEL_HEADER_SIZE,	LoadLevel_MM_HEAD },
    { "AUTH", MAX_LEVEL_AUTHOR_LEN,	LoadLevel_MM_AUTH },
    { "BODY", -1,			LoadLevel_MM_BODY },
    {  NULL,  0,			NULL }
  };

  /* always start with reliable default values */
  setLevelInfoToDefaults_MM();

  if (!(file = openFile(filename, MODE_READ)))
  {
    if (!level_info_only)
      Error(ERR_WARN, "cannot read level '%s' - creating new level", filename);

    return FALSE;
  }

  getFileChunk(file, chunk_name, NULL, BYTE_ORDER_BIG_ENDIAN);
  if (strcmp(chunk_name, "MMII") == 0)
  {
    getFile32BitInteger(file, BYTE_ORDER_BIG_ENDIAN);	/* not used */

    getFileChunk(file, chunk_name, NULL, BYTE_ORDER_BIG_ENDIAN);
    if (strcmp(chunk_name, "CAVE") != 0)
    {
      Error(ERR_WARN, "unknown format of level file '%s'", filename);

      closeFile(file);

      return FALSE;
    }
  }
  else	/* check for pre-2.0 file format with cookie string */
  {
    strcpy(cookie, chunk_name);
    getStringFromFile(file, &cookie[4], MAX_LINE_LEN - 4);
    if (strlen(cookie) > 0 && cookie[strlen(cookie) - 1] == '\n')
      cookie[strlen(cookie) - 1] = '\0';

    if (!checkCookieString(cookie, LEVEL_COOKIE_TMPL))
    {
      Error(ERR_WARN, "unknown format of level file '%s'", filename);

      closeFile(file);

      return FALSE;
    }

    if ((native_mm_level.file_version = getFileVersionFromCookieString(cookie))
	== -1)
    {
      Error(ERR_WARN, "unsupported version of level file '%s'", filename);

      closeFile(file);

      return FALSE;
    }
  }

  while (getFileChunk(file, chunk_name, &chunk_size, BYTE_ORDER_BIG_ENDIAN))
  {
    int i = 0;

    while (chunk_info[i].name != NULL &&
	   strcmp(chunk_name, chunk_info[i].name) != 0)
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
	(chunk_info[i].loader)(file, chunk_size, &native_mm_level);

      /* the size of some chunks cannot be checked before reading other
	 chunks first (like "HEAD" and "BODY") that contain some header
	 information, so check them here */
      if (chunk_size_expected != chunk_size)
	Error(ERR_WARN, "wrong size (%d) of chunk '%s' in level file '%s'",
	      chunk_size, chunk_name, filename);
    }
  }

  closeFile(file);

  return TRUE;
}

static void SaveLevel_MM_HEAD(FILE *file, struct LevelInfo_MM *level)
{
  int i;
  int laser_color;

  fputc(level->fieldx, file);
  fputc(level->fieldy, file);

  putFile16BitInteger(file, level->time,           BYTE_ORDER_BIG_ENDIAN);
  putFile16BitInteger(file, level->kettles_needed, BYTE_ORDER_BIG_ENDIAN);

  for (i = 0; i < MAX_LEVEL_NAME_LEN; i++)
    fputc(level->name[i], file);

  for (i = 0; i < LEVEL_SCORE_ELEMENTS; i++)
    fputc(level->score[i], file);

  fputc((level->auto_count_kettles ? 1 : 0), file);
  fputc(level->amoeba_speed, file);
  fputc(level->time_fuse, file);

  laser_color = ((level->laser_red   << 2) |
		 (level->laser_green << 1) |
		 (level->laser_blue  << 0));
  fputc(laser_color, file);

  fputc((level->encoding_16bit_field ? 1 : 0), file);

  WriteUnusedBytesToFile(file, LEVEL_HEADER_UNUSED);
}

static void SaveLevel_MM_AUTH(FILE *file, struct LevelInfo_MM *level)
{
  int i;

  for (i = 0; i < MAX_LEVEL_AUTHOR_LEN; i++)
    fputc(level->author[i], file);
}

static void SaveLevel_MM_BODY(FILE *file, struct LevelInfo_MM *level)
{
  int x, y;

  for (y = 0; y < level->fieldy; y++)
    for (x = 0; x < level->fieldx; x++)
      if (level->encoding_16bit_field)
	putFile16BitInteger(file, Ur[x][y], BYTE_ORDER_BIG_ENDIAN);
      else
	fputc(Ur[x][y], file);
}

void SaveNativeLevel_MM(char *filename)
{
  int x, y;
  int body_chunk_size;
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save level file '%s'", filename);

    return;
  }

  /* check level field for 16-bit elements */
  native_mm_level.encoding_16bit_field = FALSE;

  for (y = 0; y < native_mm_level.fieldy; y++)
    for (x = 0; x < native_mm_level.fieldx; x++)
      if (Ur[x][y] > 255)
	native_mm_level.encoding_16bit_field = TRUE;

  body_chunk_size =
    native_mm_level.fieldx * native_mm_level.fieldy *
    (native_mm_level.encoding_16bit_field ? 2 : 1);

  putFileChunk(file, "MMII", CHUNK_SIZE_UNDEFINED, BYTE_ORDER_BIG_ENDIAN);
  putFileChunk(file, "CAVE", CHUNK_SIZE_NONE,      BYTE_ORDER_BIG_ENDIAN);

  putFileChunk(file, "VERS", FILE_VERS_CHUNK_SIZE, BYTE_ORDER_BIG_ENDIAN);
  WriteChunk_MM_VERS(file, MM_FILE_VERSION_ACTUAL, MM_GAME_VERSION_ACTUAL);

  putFileChunk(file, "HEAD", LEVEL_HEADER_SIZE, BYTE_ORDER_BIG_ENDIAN);
  SaveLevel_MM_HEAD(file, &native_mm_level);

  putFileChunk(file, "AUTH", MAX_LEVEL_AUTHOR_LEN, BYTE_ORDER_BIG_ENDIAN);
  SaveLevel_MM_AUTH(file, &native_mm_level);

  putFileChunk(file, "BODY", body_chunk_size, BYTE_ORDER_BIG_ENDIAN);
  SaveLevel_MM_BODY(file, &native_mm_level);

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);
}
