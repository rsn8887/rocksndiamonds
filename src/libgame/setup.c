// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// setup.c
// ============================================================================

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "platform.h"

#if !defined(PLATFORM_WIN32)
#include <pwd.h>
#include <sys/param.h>
#endif

#include "setup.h"
#include "joystick.h"
#include "text.h"
#include "misc.h"
#include "hash.h"


#define ENABLE_UNUSED_CODE	FALSE	/* for currently unused functions */

#define NUM_LEVELCLASS_DESC	8

static char *levelclass_desc[NUM_LEVELCLASS_DESC] =
{
  "Tutorial Levels",
  "Classic Originals",
  "Contributions",
  "Private Levels",
  "Boulderdash",
  "Emerald Mine",
  "Supaplex",
  "DX Boulderdash"
};


#define LEVELCOLOR(n)	(IS_LEVELCLASS_TUTORIAL(n) ?		FC_BLUE :    \
			 IS_LEVELCLASS_CLASSICS(n) ?		FC_RED :     \
			 IS_LEVELCLASS_BD(n) ?			FC_YELLOW :  \
			 IS_LEVELCLASS_EM(n) ?			FC_YELLOW :  \
			 IS_LEVELCLASS_SP(n) ?			FC_YELLOW :  \
			 IS_LEVELCLASS_DX(n) ?			FC_YELLOW :  \
			 IS_LEVELCLASS_SB(n) ?			FC_YELLOW :  \
			 IS_LEVELCLASS_CONTRIB(n) ?		FC_GREEN :   \
			 IS_LEVELCLASS_PRIVATE(n) ?		FC_RED :     \
			 FC_BLUE)

#define LEVELSORTING(n)	(IS_LEVELCLASS_TUTORIAL(n) ?		0 :	\
			 IS_LEVELCLASS_CLASSICS(n) ?		1 :	\
			 IS_LEVELCLASS_BD(n) ?			2 :	\
			 IS_LEVELCLASS_EM(n) ?			3 :	\
			 IS_LEVELCLASS_SP(n) ?			4 :	\
			 IS_LEVELCLASS_DX(n) ?			5 :	\
			 IS_LEVELCLASS_SB(n) ?			6 :	\
			 IS_LEVELCLASS_CONTRIB(n) ?		7 :	\
			 IS_LEVELCLASS_PRIVATE(n) ?		8 :	\
			 9)

#define ARTWORKCOLOR(n)	(IS_ARTWORKCLASS_CLASSICS(n) ?		FC_RED :     \
			 IS_ARTWORKCLASS_CONTRIB(n) ?		FC_GREEN :   \
			 IS_ARTWORKCLASS_PRIVATE(n) ?		FC_RED :     \
			 IS_ARTWORKCLASS_LEVEL(n) ?		FC_YELLOW :  \
			 FC_BLUE)

#define ARTWORKSORTING(n) (IS_ARTWORKCLASS_CLASSICS(n) ?	0 :	\
			   IS_ARTWORKCLASS_LEVEL(n) ?		1 :	\
			   IS_ARTWORKCLASS_CONTRIB(n) ?		2 :	\
			   IS_ARTWORKCLASS_PRIVATE(n) ?		3 :	\
			   9)

#define TOKEN_VALUE_POSITION_SHORT		32
#define TOKEN_VALUE_POSITION_DEFAULT		40
#define TOKEN_COMMENT_POSITION_DEFAULT		60

#define MAX_COOKIE_LEN				256


static void setTreeInfoToDefaults(TreeInfo *, int);
static TreeInfo *getTreeInfoCopy(TreeInfo *ti);
static int compareTreeInfoEntries(const void *, const void *);

static int token_value_position   = TOKEN_VALUE_POSITION_DEFAULT;
static int token_comment_position = TOKEN_COMMENT_POSITION_DEFAULT;

static SetupFileHash *artworkinfo_cache_old = NULL;
static SetupFileHash *artworkinfo_cache_new = NULL;
static boolean use_artworkinfo_cache = TRUE;


/* ------------------------------------------------------------------------- */
/* file functions                                                            */
/* ------------------------------------------------------------------------- */

static char *getLevelClassDescription(TreeInfo *ti)
{
  int position = ti->sort_priority / 100;

  if (position >= 0 && position < NUM_LEVELCLASS_DESC)
    return levelclass_desc[position];
  else
    return "Unknown Level Class";
}

static char *getScoreDir(char *level_subdir)
{
  static char *score_dir = NULL;
  static char *score_level_dir = NULL;
  char *score_subdir = SCORES_DIRECTORY;

  if (score_dir == NULL)
  {
    if (program.global_scores)
      score_dir = getPath2(getCommonDataDir(),   score_subdir);
    else
      score_dir = getPath2(getUserGameDataDir(), score_subdir);
  }

  if (level_subdir != NULL)
  {
    checked_free(score_level_dir);

    score_level_dir = getPath2(score_dir, level_subdir);

    return score_level_dir;
  }

  return score_dir;
}

static char *getLevelSetupDir(char *level_subdir)
{
  static char *levelsetup_dir = NULL;
  char *data_dir = getUserGameDataDir();
  char *levelsetup_subdir = LEVELSETUP_DIRECTORY;

  checked_free(levelsetup_dir);

  if (level_subdir != NULL)
    levelsetup_dir = getPath3(data_dir, levelsetup_subdir, level_subdir);
  else
    levelsetup_dir = getPath2(data_dir, levelsetup_subdir);

  return levelsetup_dir;
}

static char *getCacheDir()
{
  static char *cache_dir = NULL;

  if (cache_dir == NULL)
    cache_dir = getPath2(getUserGameDataDir(), CACHE_DIRECTORY);

  return cache_dir;
}

static char *getLevelDirFromTreeInfo(TreeInfo *node)
{
  static char *level_dir = NULL;

  if (node == NULL)
    return options.level_directory;

  checked_free(level_dir);

  level_dir = getPath2((node->in_user_dir ? getUserLevelDir(NULL) :
			options.level_directory), node->fullpath);

  return level_dir;
}

char *getUserLevelDir(char *level_subdir)
{
  static char *userlevel_dir = NULL;
  char *data_dir = getUserGameDataDir();
  char *userlevel_subdir = LEVELS_DIRECTORY;

  checked_free(userlevel_dir);

  if (level_subdir != NULL)
    userlevel_dir = getPath3(data_dir, userlevel_subdir, level_subdir);
  else
    userlevel_dir = getPath2(data_dir, userlevel_subdir);

  return userlevel_dir;
}

char *getCurrentLevelDir()
{
  return getLevelDirFromTreeInfo(leveldir_current);
}

char *getNewUserLevelSubdir()
{
  static char *new_level_subdir = NULL;
  char *subdir_prefix = getLoginName();
  char subdir_suffix[10];
  int max_suffix_number = 1000;
  int i = 0;

  while (++i < max_suffix_number)
  {
    sprintf(subdir_suffix, "_%d", i);

    checked_free(new_level_subdir);
    new_level_subdir = getStringCat2(subdir_prefix, subdir_suffix);

    if (!directoryExists(getUserLevelDir(new_level_subdir)))
      break;
  }

  return new_level_subdir;
}

static char *getTapeDir(char *level_subdir)
{
  static char *tape_dir = NULL;
  char *data_dir = getUserGameDataDir();
  char *tape_subdir = TAPES_DIRECTORY;

  checked_free(tape_dir);

  if (level_subdir != NULL)
    tape_dir = getPath3(data_dir, tape_subdir, level_subdir);
  else
    tape_dir = getPath2(data_dir, tape_subdir);

  return tape_dir;
}

static char *getSolutionTapeDir()
{
  static char *tape_dir = NULL;
  char *data_dir = getCurrentLevelDir();
  char *tape_subdir = TAPES_DIRECTORY;

  checked_free(tape_dir);

  tape_dir = getPath2(data_dir, tape_subdir);

  return tape_dir;
}

static char *getDefaultGraphicsDir(char *graphics_subdir)
{
  static char *graphics_dir = NULL;

  if (graphics_subdir == NULL)
    return options.graphics_directory;

  checked_free(graphics_dir);

  graphics_dir = getPath2(options.graphics_directory, graphics_subdir);

  return graphics_dir;
}

static char *getDefaultSoundsDir(char *sounds_subdir)
{
  static char *sounds_dir = NULL;

  if (sounds_subdir == NULL)
    return options.sounds_directory;

  checked_free(sounds_dir);

  sounds_dir = getPath2(options.sounds_directory, sounds_subdir);

  return sounds_dir;
}

static char *getDefaultMusicDir(char *music_subdir)
{
  static char *music_dir = NULL;

  if (music_subdir == NULL)
    return options.music_directory;

  checked_free(music_dir);

  music_dir = getPath2(options.music_directory, music_subdir);

  return music_dir;
}

static char *getClassicArtworkSet(int type)
{
  return (type == TREE_TYPE_GRAPHICS_DIR ? GFX_CLASSIC_SUBDIR :
	  type == TREE_TYPE_SOUNDS_DIR   ? SND_CLASSIC_SUBDIR :
	  type == TREE_TYPE_MUSIC_DIR    ? MUS_CLASSIC_SUBDIR : "");
}

static char *getClassicArtworkDir(int type)
{
  return (type == TREE_TYPE_GRAPHICS_DIR ?
	  getDefaultGraphicsDir(GFX_CLASSIC_SUBDIR) :
	  type == TREE_TYPE_SOUNDS_DIR ?
	  getDefaultSoundsDir(SND_CLASSIC_SUBDIR) :
	  type == TREE_TYPE_MUSIC_DIR ?
	  getDefaultMusicDir(MUS_CLASSIC_SUBDIR) : "");
}

static char *getUserGraphicsDir()
{
  static char *usergraphics_dir = NULL;

  if (usergraphics_dir == NULL)
    usergraphics_dir = getPath2(getUserGameDataDir(), GRAPHICS_DIRECTORY);

  return usergraphics_dir;
}

static char *getUserSoundsDir()
{
  static char *usersounds_dir = NULL;

  if (usersounds_dir == NULL)
    usersounds_dir = getPath2(getUserGameDataDir(), SOUNDS_DIRECTORY);

  return usersounds_dir;
}

static char *getUserMusicDir()
{
  static char *usermusic_dir = NULL;

  if (usermusic_dir == NULL)
    usermusic_dir = getPath2(getUserGameDataDir(), MUSIC_DIRECTORY);

  return usermusic_dir;
}

static char *getSetupArtworkDir(TreeInfo *ti)
{
  static char *artwork_dir = NULL;

  if (ti == NULL)
    return NULL;

  checked_free(artwork_dir);

  artwork_dir = getPath2(ti->basepath, ti->fullpath);

  return artwork_dir;
}

char *setLevelArtworkDir(TreeInfo *ti)
{
  char **artwork_path_ptr, **artwork_set_ptr;
  TreeInfo *level_artwork;

  if (ti == NULL || leveldir_current == NULL)
    return NULL;

  artwork_path_ptr = LEVELDIR_ARTWORK_PATH_PTR(leveldir_current, ti->type);
  artwork_set_ptr  = LEVELDIR_ARTWORK_SET_PTR( leveldir_current, ti->type);

  checked_free(*artwork_path_ptr);

  if ((level_artwork = getTreeInfoFromIdentifier(ti, *artwork_set_ptr)))
  {
    *artwork_path_ptr = getStringCopy(getSetupArtworkDir(level_artwork));
  }
  else
  {
    /*
      No (or non-existing) artwork configured in "levelinfo.conf". This would
      normally result in using the artwork configured in the setup menu. But
      if an artwork subdirectory exists (which might contain custom artwork
      or an artwork configuration file), this level artwork must be treated
      as relative to the default "classic" artwork, not to the artwork that
      is currently configured in the setup menu.

      Update: For "special" versions of R'n'D (like "R'n'D jue"), do not use
      the "default" artwork (which would be "jue0" for "R'n'D jue"), but use
      the real "classic" artwork from the original R'n'D (like "gfx_classic").
    */

    char *dir = getPath2(getCurrentLevelDir(), ARTWORK_DIRECTORY(ti->type));

    checked_free(*artwork_set_ptr);

    if (directoryExists(dir))
    {
      *artwork_path_ptr = getStringCopy(getClassicArtworkDir(ti->type));
      *artwork_set_ptr = getStringCopy(getClassicArtworkSet(ti->type));
    }
    else
    {
      *artwork_path_ptr = getStringCopy(UNDEFINED_FILENAME);
      *artwork_set_ptr = NULL;
    }

    free(dir);
  }

  return *artwork_set_ptr;
}

inline static char *getLevelArtworkSet(int type)
{
  if (leveldir_current == NULL)
    return NULL;

  return LEVELDIR_ARTWORK_SET(leveldir_current, type);
}

inline static char *getLevelArtworkDir(int type)
{
  if (leveldir_current == NULL)
    return UNDEFINED_FILENAME;

  return LEVELDIR_ARTWORK_PATH(leveldir_current, type);
}

char *getProgramMainDataPath(char *command_filename, char *base_path)
{
  /* check if the program's main data base directory is configured */
  if (!strEqual(base_path, "."))
    return base_path;

  /* if the program is configured to start from current directory (default),
     determine program package directory from program binary (some versions
     of KDE/Konqueror and Mac OS X (especially "Mavericks") apparently do not
     set the current working directory to the program package directory) */
  char *main_data_path = getBasePath(command_filename);

#if defined(PLATFORM_MACOSX)
  if (strSuffix(main_data_path, MAC_APP_BINARY_SUBDIR))
  {
    char *main_data_path_old = main_data_path;

    // cut relative path to Mac OS X application binary directory from path
    main_data_path[strlen(main_data_path) -
		   strlen(MAC_APP_BINARY_SUBDIR)] = '\0';

    // cut trailing path separator from path (but not if path is root directory)
    if (strSuffix(main_data_path, "/") && !strEqual(main_data_path, "/"))
      main_data_path[strlen(main_data_path) - 1] = '\0';

    // replace empty path with current directory
    if (strEqual(main_data_path, ""))
      main_data_path = ".";

    // add relative path to Mac OS X application resources directory to path
    main_data_path = getPath2(main_data_path, MAC_APP_FILES_SUBDIR);

    free(main_data_path_old);
  }
#endif

  return main_data_path;
}

char *getProgramConfigFilename(char *command_filename)
{
  char *command_filename_1 = getStringCopy(command_filename);

  // strip trailing executable suffix from command filename
  if (strSuffix(command_filename_1, ".exe"))
    command_filename_1[strlen(command_filename_1) - 4] = '\0';

  char *ro_base_path = getProgramMainDataPath(command_filename, RO_BASE_PATH);
  char *conf_directory = getPath2(ro_base_path, CONF_DIRECTORY);

  char *command_basepath = getBasePath(command_filename);
  char *command_basename = getBaseNameNoSuffix(command_filename);
  char *command_filename_2 = getPath2(command_basepath, command_basename);

  char *config_filename_1 = getStringCat2(command_filename_1, ".conf");
  char *config_filename_2 = getStringCat2(command_filename_2, ".conf");
  char *config_filename_3 = getPath2(conf_directory, SETUP_FILENAME);

  // 1st try: look for config file that exactly matches the binary filename
  if (fileExists(config_filename_1))
    return config_filename_1;

  // 2nd try: look for config file that matches binary filename without suffix
  if (fileExists(config_filename_2))
    return config_filename_2;

  // 3rd try: return setup config filename in global program config directory
  return config_filename_3;
}

char *getTapeFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.%s", nr, TAPEFILE_EXTENSION);
  filename = getPath2(getTapeDir(leveldir_current->subdir), basename);

  return filename;
}

char *getSolutionTapeFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.%s", nr, TAPEFILE_EXTENSION);
  filename = getPath2(getSolutionTapeDir(), basename);

  if (!fileExists(filename))
  {
    static char *filename_sln = NULL;

    checked_free(filename_sln);

    sprintf(basename, "%03d.sln", nr);
    filename_sln = getPath2(getSolutionTapeDir(), basename);

    if (fileExists(filename_sln))
      return filename_sln;
  }

  return filename;
}

char *getScoreFilename(int nr)
{
  static char *filename = NULL;
  char basename[MAX_FILENAME_LEN];

  checked_free(filename);

  sprintf(basename, "%03d.%s", nr, SCOREFILE_EXTENSION);
  filename = getPath2(getScoreDir(leveldir_current->subdir), basename);

  return filename;
}

char *getSetupFilename()
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getSetupDir(), SETUP_FILENAME);

  return filename;
}

char *getDefaultSetupFilename()
{
  return program.config_filename;
}

char *getEditorSetupFilename()
{
  static char *filename = NULL;

  checked_free(filename);
  filename = getPath2(getCurrentLevelDir(), EDITORSETUP_FILENAME);

  if (fileExists(filename))
    return filename;

  checked_free(filename);
  filename = getPath2(getSetupDir(), EDITORSETUP_FILENAME);

  return filename;
}

char *getHelpAnimFilename()
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getCurrentLevelDir(), HELPANIM_FILENAME);

  return filename;
}

char *getHelpTextFilename()
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getCurrentLevelDir(), HELPTEXT_FILENAME);

  return filename;
}

char *getLevelSetInfoFilename()
{
  static char *filename = NULL;
  char *basenames[] =
  {
    "README",
    "README.TXT",
    "README.txt",
    "Readme",
    "Readme.txt",
    "readme",
    "readme.txt",

    NULL
  };
  int i;

  for (i = 0; basenames[i] != NULL; i++)
  {
    checked_free(filename);
    filename = getPath2(getCurrentLevelDir(), basenames[i]);

    if (fileExists(filename))
      return filename;
  }

  return NULL;
}

char *getLevelSetTitleMessageBasename(int nr, boolean initial)
{
  static char basename[32];

  sprintf(basename, "%s_%d.txt",
	  (initial ? "titlemessage_initial" : "titlemessage"), nr + 1);

  return basename;
}

char *getLevelSetTitleMessageFilename(int nr, boolean initial)
{
  static char *filename = NULL;
  char *basename;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getLevelSetTitleMessageBasename(nr, initial);

  if (!gfx.override_level_graphics)
  {
    /* 1st try: look for special artwork in current level series directory */
    filename = getPath3(getCurrentLevelDir(), GRAPHICS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    /* 2nd try: look for message file in current level set directory */
    filename = getPath2(getCurrentLevelDir(), basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    /* check if there is special artwork configured in level series config */
    if (getLevelArtworkSet(ARTWORK_TYPE_GRAPHICS) != NULL)
    {
      /* 3rd try: look for special artwork configured in level series config */
      filename = getPath2(getLevelArtworkDir(ARTWORK_TYPE_GRAPHICS), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      /* take missing artwork configured in level set config from default */
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    /* 4th try: look for special artwork in configured artwork directory */
    filename = getPath2(getSetupArtworkDir(artwork.gfx_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  /* 5th try: look for default artwork in new default artwork directory */
  filename = getPath2(getDefaultGraphicsDir(GFX_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  /* 6th try: look for default artwork in old default artwork directory */
  filename = getPath2(options.graphics_directory, basename);
  if (fileExists(filename))
    return filename;

  return NULL;		/* cannot find specified artwork file anywhere */
}

static char *getCorrectedArtworkBasename(char *basename)
{
  return basename;
}

char *getCustomImageFilename(char *basename)
{
  static char *filename = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getCorrectedArtworkBasename(basename);

  if (!gfx.override_level_graphics)
  {
    /* 1st try: look for special artwork in current level series directory */
    filename = getImg3(getCurrentLevelDir(), GRAPHICS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    /* check if there is special artwork configured in level series config */
    if (getLevelArtworkSet(ARTWORK_TYPE_GRAPHICS) != NULL)
    {
      /* 2nd try: look for special artwork configured in level series config */
      filename = getImg2(getLevelArtworkDir(ARTWORK_TYPE_GRAPHICS), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      /* take missing artwork configured in level set config from default */
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    /* 3rd try: look for special artwork in configured artwork directory */
    filename = getImg2(getSetupArtworkDir(artwork.gfx_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  /* 4th try: look for default artwork in new default artwork directory */
  filename = getImg2(getDefaultGraphicsDir(GFX_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  /* 5th try: look for default artwork in old default artwork directory */
  filename = getImg2(options.graphics_directory, basename);
  if (fileExists(filename))
    return filename;

  if (!strEqual(GFX_FALLBACK_FILENAME, UNDEFINED_FILENAME))
  {
    free(filename);

    if (options.debug)
      Error(ERR_WARN, "cannot find artwork file '%s' (using fallback)",
	    basename);

    /* 6th try: look for fallback artwork in old default artwork directory */
    /* (needed to prevent errors when trying to access unused artwork files) */
    filename = getImg2(options.graphics_directory, GFX_FALLBACK_FILENAME);
    if (fileExists(filename))
      return filename;
  }

  return NULL;		/* cannot find specified artwork file anywhere */
}

char *getCustomSoundFilename(char *basename)
{
  static char *filename = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getCorrectedArtworkBasename(basename);

  if (!gfx.override_level_sounds)
  {
    /* 1st try: look for special artwork in current level series directory */
    filename = getPath3(getCurrentLevelDir(), SOUNDS_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    /* check if there is special artwork configured in level series config */
    if (getLevelArtworkSet(ARTWORK_TYPE_SOUNDS) != NULL)
    {
      /* 2nd try: look for special artwork configured in level series config */
      filename = getPath2(getLevelArtworkDir(TREE_TYPE_SOUNDS_DIR), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      /* take missing artwork configured in level set config from default */
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    /* 3rd try: look for special artwork in configured artwork directory */
    filename = getPath2(getSetupArtworkDir(artwork.snd_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  /* 4th try: look for default artwork in new default artwork directory */
  filename = getPath2(getDefaultSoundsDir(SND_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  /* 5th try: look for default artwork in old default artwork directory */
  filename = getPath2(options.sounds_directory, basename);
  if (fileExists(filename))
    return filename;

  if (!strEqual(SND_FALLBACK_FILENAME, UNDEFINED_FILENAME))
  {
    free(filename);

    if (options.debug)
      Error(ERR_WARN, "cannot find artwork file '%s' (using fallback)",
	    basename);

    /* 6th try: look for fallback artwork in old default artwork directory */
    /* (needed to prevent errors when trying to access unused artwork files) */
    filename = getPath2(options.sounds_directory, SND_FALLBACK_FILENAME);
    if (fileExists(filename))
      return filename;
  }

  return NULL;		/* cannot find specified artwork file anywhere */
}

char *getCustomMusicFilename(char *basename)
{
  static char *filename = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(filename);

  basename = getCorrectedArtworkBasename(basename);

  if (!gfx.override_level_music)
  {
    /* 1st try: look for special artwork in current level series directory */
    filename = getPath3(getCurrentLevelDir(), MUSIC_DIRECTORY, basename);
    if (fileExists(filename))
      return filename;

    free(filename);

    /* check if there is special artwork configured in level series config */
    if (getLevelArtworkSet(ARTWORK_TYPE_MUSIC) != NULL)
    {
      /* 2nd try: look for special artwork configured in level series config */
      filename = getPath2(getLevelArtworkDir(TREE_TYPE_MUSIC_DIR), basename);
      if (fileExists(filename))
	return filename;

      free(filename);

      /* take missing artwork configured in level set config from default */
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    /* 3rd try: look for special artwork in configured artwork directory */
    filename = getPath2(getSetupArtworkDir(artwork.mus_current), basename);
    if (fileExists(filename))
      return filename;

    free(filename);
  }

  /* 4th try: look for default artwork in new default artwork directory */
  filename = getPath2(getDefaultMusicDir(MUS_DEFAULT_SUBDIR), basename);
  if (fileExists(filename))
    return filename;

  free(filename);

  /* 5th try: look for default artwork in old default artwork directory */
  filename = getPath2(options.music_directory, basename);
  if (fileExists(filename))
    return filename;

  if (!strEqual(MUS_FALLBACK_FILENAME, UNDEFINED_FILENAME))
  {
    free(filename);

    if (options.debug)
      Error(ERR_WARN, "cannot find artwork file '%s' (using fallback)",
	    basename);

    /* 6th try: look for fallback artwork in old default artwork directory */
    /* (needed to prevent errors when trying to access unused artwork files) */
    filename = getPath2(options.music_directory, MUS_FALLBACK_FILENAME);
    if (fileExists(filename))
      return filename;
  }

  return NULL;		/* cannot find specified artwork file anywhere */
}

char *getCustomArtworkFilename(char *basename, int type)
{
  if (type == ARTWORK_TYPE_GRAPHICS)
    return getCustomImageFilename(basename);
  else if (type == ARTWORK_TYPE_SOUNDS)
    return getCustomSoundFilename(basename);
  else if (type == ARTWORK_TYPE_MUSIC)
    return getCustomMusicFilename(basename);
  else
    return UNDEFINED_FILENAME;
}

char *getCustomArtworkConfigFilename(int type)
{
  return getCustomArtworkFilename(ARTWORKINFO_FILENAME(type), type);
}

char *getCustomArtworkLevelConfigFilename(int type)
{
  static char *filename = NULL;

  checked_free(filename);

  filename = getPath2(getLevelArtworkDir(type), ARTWORKINFO_FILENAME(type));

  return filename;
}

char *getCustomMusicDirectory(void)
{
  static char *directory = NULL;
  boolean skip_setup_artwork = FALSE;

  checked_free(directory);

  if (!gfx.override_level_music)
  {
    /* 1st try: look for special artwork in current level series directory */
    directory = getPath2(getCurrentLevelDir(), MUSIC_DIRECTORY);
    if (directoryExists(directory))
      return directory;

    free(directory);

    /* check if there is special artwork configured in level series config */
    if (getLevelArtworkSet(ARTWORK_TYPE_MUSIC) != NULL)
    {
      /* 2nd try: look for special artwork configured in level series config */
      directory = getStringCopy(getLevelArtworkDir(TREE_TYPE_MUSIC_DIR));
      if (directoryExists(directory))
	return directory;

      free(directory);

      /* take missing artwork configured in level set config from default */
      skip_setup_artwork = TRUE;
    }
  }

  if (!skip_setup_artwork)
  {
    /* 3rd try: look for special artwork in configured artwork directory */
    directory = getStringCopy(getSetupArtworkDir(artwork.mus_current));
    if (directoryExists(directory))
      return directory;

    free(directory);
  }

  /* 4th try: look for default artwork in new default artwork directory */
  directory = getStringCopy(getDefaultMusicDir(MUS_DEFAULT_SUBDIR));
  if (directoryExists(directory))
    return directory;

  free(directory);

  /* 5th try: look for default artwork in old default artwork directory */
  directory = getStringCopy(options.music_directory);
  if (directoryExists(directory))
    return directory;

  return NULL;		/* cannot find specified artwork file anywhere */
}

void InitTapeDirectory(char *level_subdir)
{
  createDirectory(getUserGameDataDir(), "user data", PERMS_PRIVATE);
  createDirectory(getTapeDir(NULL), "main tape", PERMS_PRIVATE);
  createDirectory(getTapeDir(level_subdir), "level tape", PERMS_PRIVATE);
}

void InitScoreDirectory(char *level_subdir)
{
  int permissions = (program.global_scores ? PERMS_PUBLIC : PERMS_PRIVATE);

  if (program.global_scores)
    createDirectory(getCommonDataDir(), "common data", permissions);
  else
    createDirectory(getUserGameDataDir(), "user data", permissions);

  createDirectory(getScoreDir(NULL), "main score", permissions);
  createDirectory(getScoreDir(level_subdir), "level score", permissions);
}

static void SaveUserLevelInfo();

void InitUserLevelDirectory(char *level_subdir)
{
  if (!directoryExists(getUserLevelDir(level_subdir)))
  {
    createDirectory(getUserGameDataDir(), "user data", PERMS_PRIVATE);
    createDirectory(getUserLevelDir(NULL), "main user level", PERMS_PRIVATE);
    createDirectory(getUserLevelDir(level_subdir), "user level", PERMS_PRIVATE);

    SaveUserLevelInfo();
  }
}

void InitLevelSetupDirectory(char *level_subdir)
{
  createDirectory(getUserGameDataDir(), "user data", PERMS_PRIVATE);
  createDirectory(getLevelSetupDir(NULL), "main level setup", PERMS_PRIVATE);
  createDirectory(getLevelSetupDir(level_subdir), "level setup", PERMS_PRIVATE);
}

void InitCacheDirectory()
{
  createDirectory(getUserGameDataDir(), "user data", PERMS_PRIVATE);
  createDirectory(getCacheDir(), "cache data", PERMS_PRIVATE);
}


/* ------------------------------------------------------------------------- */
/* some functions to handle lists of level and artwork directories           */
/* ------------------------------------------------------------------------- */

TreeInfo *newTreeInfo()
{
  return checked_calloc(sizeof(TreeInfo));
}

TreeInfo *newTreeInfo_setDefaults(int type)
{
  TreeInfo *ti = newTreeInfo();

  setTreeInfoToDefaults(ti, type);

  return ti;
}

void pushTreeInfo(TreeInfo **node_first, TreeInfo *node_new)
{
  node_new->next = *node_first;
  *node_first = node_new;
}

int numTreeInfo(TreeInfo *node)
{
  int num = 0;

  while (node)
  {
    num++;
    node = node->next;
  }

  return num;
}

boolean validLevelSeries(TreeInfo *node)
{
  return (node != NULL && !node->node_group && !node->parent_link);
}

TreeInfo *getFirstValidTreeInfoEntry(TreeInfo *node)
{
  if (node == NULL)
    return NULL;

  if (node->node_group)		/* enter level group (step down into tree) */
    return getFirstValidTreeInfoEntry(node->node_group);
  else if (node->parent_link)	/* skip start entry of level group */
  {
    if (node->next)		/* get first real level series entry */
      return getFirstValidTreeInfoEntry(node->next);
    else			/* leave empty level group and go on */
      return getFirstValidTreeInfoEntry(node->node_parent->next);
  }
  else				/* this seems to be a regular level series */
    return node;
}

TreeInfo *getTreeInfoFirstGroupEntry(TreeInfo *node)
{
  if (node == NULL)
    return NULL;

  if (node->node_parent == NULL)		/* top level group */
    return *node->node_top;
  else						/* sub level group */
    return node->node_parent->node_group;
}

int numTreeInfoInGroup(TreeInfo *node)
{
  return numTreeInfo(getTreeInfoFirstGroupEntry(node));
}

int posTreeInfo(TreeInfo *node)
{
  TreeInfo *node_cmp = getTreeInfoFirstGroupEntry(node);
  int pos = 0;

  while (node_cmp)
  {
    if (node_cmp == node)
      return pos;

    pos++;
    node_cmp = node_cmp->next;
  }

  return 0;
}

TreeInfo *getTreeInfoFromPos(TreeInfo *node, int pos)
{
  TreeInfo *node_default = node;
  int pos_cmp = 0;

  while (node)
  {
    if (pos_cmp == pos)
      return node;

    pos_cmp++;
    node = node->next;
  }

  return node_default;
}

TreeInfo *getTreeInfoFromIdentifier(TreeInfo *node, char *identifier)
{
  if (identifier == NULL)
    return NULL;

  while (node)
  {
    if (node->node_group)
    {
      TreeInfo *node_group;

      node_group = getTreeInfoFromIdentifier(node->node_group, identifier);

      if (node_group)
	return node_group;
    }
    else if (!node->parent_link)
    {
      if (strEqual(identifier, node->identifier))
	return node;
    }

    node = node->next;
  }

  return NULL;
}

TreeInfo *cloneTreeNode(TreeInfo **node_top, TreeInfo *node_parent,
			TreeInfo *node, boolean skip_sets_without_levels)
{
  TreeInfo *node_new;

  if (node == NULL)
    return NULL;

  if (!node->parent_link && !node->level_group &&
      skip_sets_without_levels && node->levels == 0)
    return cloneTreeNode(node_top, node_parent, node->next,
			 skip_sets_without_levels);

  node_new = getTreeInfoCopy(node);		/* copy complete node */

  node_new->node_top = node_top;		/* correct top node link */
  node_new->node_parent = node_parent;		/* correct parent node link */

  if (node->level_group)
    node_new->node_group = cloneTreeNode(node_top, node_new, node->node_group,
					 skip_sets_without_levels);

  node_new->next = cloneTreeNode(node_top, node_parent, node->next,
				 skip_sets_without_levels);
  
  return node_new;
}

void cloneTree(TreeInfo **ti_new, TreeInfo *ti, boolean skip_empty_sets)
{
  TreeInfo *ti_cloned = cloneTreeNode(ti_new, NULL, ti, skip_empty_sets);

  *ti_new = ti_cloned;
}

static boolean adjustTreeGraphicsForEMC(TreeInfo *node)
{
  boolean settings_changed = FALSE;

  while (node)
  {
    if (node->graphics_set_ecs && !setup.prefer_aga_graphics &&
	!strEqual(node->graphics_set, node->graphics_set_ecs))
    {
      setString(&node->graphics_set, node->graphics_set_ecs);
      settings_changed = TRUE;
    }
    else if (node->graphics_set_aga && setup.prefer_aga_graphics &&
	     !strEqual(node->graphics_set, node->graphics_set_aga))
    {
      setString(&node->graphics_set, node->graphics_set_aga);
      settings_changed = TRUE;
    }

    if (node->node_group != NULL)
      settings_changed |= adjustTreeGraphicsForEMC(node->node_group);

    node = node->next;
  }

  return settings_changed;
}

void dumpTreeInfo(TreeInfo *node, int depth)
{
  int i;

  printf("Dumping TreeInfo:\n");

  while (node)
  {
    for (i = 0; i < (depth + 1) * 3; i++)
      printf(" ");

    printf("'%s' / '%s'\n", node->identifier, node->name);

    /*
    // use for dumping artwork info tree
    printf("subdir == '%s' ['%s', '%s'] [%d])\n",
	   node->subdir, node->fullpath, node->basepath, node->in_user_dir);
    */

    if (node->node_group != NULL)
      dumpTreeInfo(node->node_group, depth + 1);

    node = node->next;
  }
}

void sortTreeInfoBySortFunction(TreeInfo **node_first,
				int (*compare_function)(const void *,
							const void *))
{
  int num_nodes = numTreeInfo(*node_first);
  TreeInfo **sort_array;
  TreeInfo *node = *node_first;
  int i = 0;

  if (num_nodes == 0)
    return;

  /* allocate array for sorting structure pointers */
  sort_array = checked_calloc(num_nodes * sizeof(TreeInfo *));

  /* writing structure pointers to sorting array */
  while (i < num_nodes && node)		/* double boundary check... */
  {
    sort_array[i] = node;

    i++;
    node = node->next;
  }

  /* sorting the structure pointers in the sorting array */
  qsort(sort_array, num_nodes, sizeof(TreeInfo *),
	compare_function);

  /* update the linkage of list elements with the sorted node array */
  for (i = 0; i < num_nodes - 1; i++)
    sort_array[i]->next = sort_array[i + 1];
  sort_array[num_nodes - 1]->next = NULL;

  /* update the linkage of the main list anchor pointer */
  *node_first = sort_array[0];

  free(sort_array);

  /* now recursively sort the level group structures */
  node = *node_first;
  while (node)
  {
    if (node->node_group != NULL)
      sortTreeInfoBySortFunction(&node->node_group, compare_function);

    node = node->next;
  }
}

void sortTreeInfo(TreeInfo **node_first)
{
  sortTreeInfoBySortFunction(node_first, compareTreeInfoEntries);
}


/* ========================================================================= */
/* some stuff from "files.c"                                                 */
/* ========================================================================= */

#if defined(PLATFORM_WIN32)
#ifndef S_IRGRP
#define S_IRGRP S_IRUSR
#endif
#ifndef S_IROTH
#define S_IROTH S_IRUSR
#endif
#ifndef S_IWGRP
#define S_IWGRP S_IWUSR
#endif
#ifndef S_IWOTH
#define S_IWOTH S_IWUSR
#endif
#ifndef S_IXGRP
#define S_IXGRP S_IXUSR
#endif
#ifndef S_IXOTH
#define S_IXOTH S_IXUSR
#endif
#ifndef S_IRWXG
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#endif
#ifndef S_ISGID
#define S_ISGID 0
#endif
#endif	/* PLATFORM_WIN32 */

/* file permissions for newly written files */
#define MODE_R_ALL		(S_IRUSR | S_IRGRP | S_IROTH)
#define MODE_W_ALL		(S_IWUSR | S_IWGRP | S_IWOTH)
#define MODE_X_ALL		(S_IXUSR | S_IXGRP | S_IXOTH)

#define MODE_W_PRIVATE		(S_IWUSR)
#define MODE_W_PUBLIC_FILE	(S_IWUSR | S_IWGRP)
#define MODE_W_PUBLIC_DIR	(S_IWUSR | S_IWGRP | S_ISGID)

#define DIR_PERMS_PRIVATE	(MODE_R_ALL | MODE_X_ALL | MODE_W_PRIVATE)
#define DIR_PERMS_PUBLIC	(MODE_R_ALL | MODE_X_ALL | MODE_W_PUBLIC_DIR)
#define DIR_PERMS_PUBLIC_ALL	(MODE_R_ALL | MODE_X_ALL | MODE_W_ALL)

#define FILE_PERMS_PRIVATE	(MODE_R_ALL | MODE_W_PRIVATE)
#define FILE_PERMS_PUBLIC	(MODE_R_ALL | MODE_W_PUBLIC_FILE)
#define FILE_PERMS_PUBLIC_ALL	(MODE_R_ALL | MODE_W_ALL)


char *getHomeDir()
{
  static char *dir = NULL;

#if defined(PLATFORM_WIN32)
  if (dir == NULL)
  {
    dir = checked_malloc(MAX_PATH + 1);

    if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, dir)))
      strcpy(dir, ".");
  }
#elif defined(PLATFORM_UNIX)
  if (dir == NULL)
  {
    if ((dir = getenv("HOME")) == NULL)
    {
      struct passwd *pwd;

      if ((pwd = getpwuid(getuid())) != NULL)
	dir = getStringCopy(pwd->pw_dir);
      else
	dir = ".";
    }
  }
#else
  dir = ".";
#endif

  return dir;
}

char *getCommonDataDir(void)
{
  static char *common_data_dir = NULL;

#if defined(PLATFORM_WIN32)
  if (common_data_dir == NULL)
  {
    char *dir = checked_malloc(MAX_PATH + 1);

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, dir))
	&& !strEqual(dir, ""))		/* empty for Windows 95/98 */
      common_data_dir = getPath2(dir, program.userdata_subdir);
    else
      common_data_dir = options.rw_base_directory;
  }
#else
  if (common_data_dir == NULL)
    common_data_dir = options.rw_base_directory;
#endif

  return common_data_dir;
}

char *getPersonalDataDir(void)
{
  static char *personal_data_dir = NULL;

#if defined(PLATFORM_MACOSX)
  if (personal_data_dir == NULL)
    personal_data_dir = getPath2(getHomeDir(), "Documents");
#else
  if (personal_data_dir == NULL)
    personal_data_dir = getHomeDir();
#endif

  return personal_data_dir;
}

char *getUserGameDataDir(void)
{
  static char *user_game_data_dir = NULL;

#if defined(PLATFORM_ANDROID)
  if (user_game_data_dir == NULL)
    user_game_data_dir = (char *)(SDL_AndroidGetExternalStorageState() &
				  SDL_ANDROID_EXTERNAL_STORAGE_WRITE ?
				  SDL_AndroidGetExternalStoragePath() :
				  SDL_AndroidGetInternalStoragePath());
#else
  if (user_game_data_dir == NULL)
    user_game_data_dir = getPath2(getPersonalDataDir(),
				  program.userdata_subdir);
#endif

  return user_game_data_dir;
}

char *getSetupDir()
{
  return getUserGameDataDir();
}

static mode_t posix_umask(mode_t mask)
{
#if defined(PLATFORM_UNIX)
  return umask(mask);
#else
  return 0;
#endif
}

static int posix_mkdir(const char *pathname, mode_t mode)
{
#if defined(PLATFORM_WIN32)
  return mkdir(pathname);
#else
  return mkdir(pathname, mode);
#endif
}

static boolean posix_process_running_setgid()
{
#if defined(PLATFORM_UNIX)
  return (getgid() != getegid());
#else
  return FALSE;
#endif
}

void createDirectory(char *dir, char *text, int permission_class)
{
  if (directoryExists(dir))
    return;

  /* leave "other" permissions in umask untouched, but ensure group parts
     of USERDATA_DIR_MODE are not masked */
  mode_t dir_mode = (permission_class == PERMS_PRIVATE ?
		     DIR_PERMS_PRIVATE : DIR_PERMS_PUBLIC);
  mode_t last_umask = posix_umask(0);
  mode_t group_umask = ~(dir_mode & S_IRWXG);
  int running_setgid = posix_process_running_setgid();

  if (permission_class == PERMS_PUBLIC)
  {
    /* if we're setgid, protect files against "other" */
    /* else keep umask(0) to make the dir world-writable */

    if (running_setgid)
      posix_umask(last_umask & group_umask);
    else
      dir_mode = DIR_PERMS_PUBLIC_ALL;
  }

  if (posix_mkdir(dir, dir_mode) != 0)
    Error(ERR_WARN, "cannot create %s directory '%s': %s",
	  text, dir, strerror(errno));

  if (permission_class == PERMS_PUBLIC && !running_setgid)
    chmod(dir, dir_mode);

  posix_umask(last_umask);		/* restore previous umask */
}

void InitUserDataDirectory()
{
  createDirectory(getUserGameDataDir(), "user data", PERMS_PRIVATE);
}

void SetFilePermissions(char *filename, int permission_class)
{
  int running_setgid = posix_process_running_setgid();
  int perms = (permission_class == PERMS_PRIVATE ?
	       FILE_PERMS_PRIVATE : FILE_PERMS_PUBLIC);

  if (permission_class == PERMS_PUBLIC && !running_setgid)
    perms = FILE_PERMS_PUBLIC_ALL;

  chmod(filename, perms);
}

char *getCookie(char *file_type)
{
  static char cookie[MAX_COOKIE_LEN + 1];

  if (strlen(program.cookie_prefix) + 1 +
      strlen(file_type) + strlen("_FILE_VERSION_x.x") > MAX_COOKIE_LEN)
    return "[COOKIE ERROR]";	/* should never happen */

  sprintf(cookie, "%s_%s_FILE_VERSION_%d.%d",
	  program.cookie_prefix, file_type,
	  program.version_major, program.version_minor);

  return cookie;
}

void fprintFileHeader(FILE *file, char *basename)
{
  char *prefix = "# ";
  char *sep1 = "=";

  fprintf_line_with_prefix(file, prefix, sep1, 77);
  fprintf(file, "%s%s\n", prefix, basename);
  fprintf_line_with_prefix(file, prefix, sep1, 77);
  fprintf(file, "\n");
}

int getFileVersionFromCookieString(const char *cookie)
{
  const char *ptr_cookie1, *ptr_cookie2;
  const char *pattern1 = "_FILE_VERSION_";
  const char *pattern2 = "?.?";
  const int len_cookie = strlen(cookie);
  const int len_pattern1 = strlen(pattern1);
  const int len_pattern2 = strlen(pattern2);
  const int len_pattern = len_pattern1 + len_pattern2;
  int version_major, version_minor;

  if (len_cookie <= len_pattern)
    return -1;

  ptr_cookie1 = &cookie[len_cookie - len_pattern];
  ptr_cookie2 = &cookie[len_cookie - len_pattern2];

  if (strncmp(ptr_cookie1, pattern1, len_pattern1) != 0)
    return -1;

  if (ptr_cookie2[0] < '0' || ptr_cookie2[0] > '9' ||
      ptr_cookie2[1] != '.' ||
      ptr_cookie2[2] < '0' || ptr_cookie2[2] > '9')
    return -1;

  version_major = ptr_cookie2[0] - '0';
  version_minor = ptr_cookie2[2] - '0';

  return VERSION_IDENT(version_major, version_minor, 0, 0);
}

boolean checkCookieString(const char *cookie, const char *template)
{
  const char *pattern = "_FILE_VERSION_?.?";
  const int len_cookie = strlen(cookie);
  const int len_template = strlen(template);
  const int len_pattern = strlen(pattern);

  if (len_cookie != len_template)
    return FALSE;

  if (strncmp(cookie, template, len_cookie - len_pattern) != 0)
    return FALSE;

  return TRUE;
}


/* ------------------------------------------------------------------------- */
/* setup file list and hash handling functions                               */
/* ------------------------------------------------------------------------- */

char *getFormattedSetupEntry(char *token, char *value)
{
  int i;
  static char entry[MAX_LINE_LEN];

  /* if value is an empty string, just return token without value */
  if (*value == '\0')
    return token;

  /* start with the token and some spaces to format output line */
  sprintf(entry, "%s:", token);
  for (i = strlen(entry); i < token_value_position; i++)
    strcat(entry, " ");

  /* continue with the token's value */
  strcat(entry, value);

  return entry;
}

SetupFileList *newSetupFileList(char *token, char *value)
{
  SetupFileList *new = checked_malloc(sizeof(SetupFileList));

  new->token = getStringCopy(token);
  new->value = getStringCopy(value);

  new->next = NULL;

  return new;
}

void freeSetupFileList(SetupFileList *list)
{
  if (list == NULL)
    return;

  checked_free(list->token);
  checked_free(list->value);

  if (list->next)
    freeSetupFileList(list->next);

  free(list);
}

char *getListEntry(SetupFileList *list, char *token)
{
  if (list == NULL)
    return NULL;

  if (strEqual(list->token, token))
    return list->value;
  else
    return getListEntry(list->next, token);
}

SetupFileList *setListEntry(SetupFileList *list, char *token, char *value)
{
  if (list == NULL)
    return NULL;

  if (strEqual(list->token, token))
  {
    checked_free(list->value);

    list->value = getStringCopy(value);

    return list;
  }
  else if (list->next == NULL)
    return (list->next = newSetupFileList(token, value));
  else
    return setListEntry(list->next, token, value);
}

SetupFileList *addListEntry(SetupFileList *list, char *token, char *value)
{
  if (list == NULL)
    return NULL;

  if (list->next == NULL)
    return (list->next = newSetupFileList(token, value));
  else
    return addListEntry(list->next, token, value);
}

#if ENABLE_UNUSED_CODE
#ifdef DEBUG
static void printSetupFileList(SetupFileList *list)
{
  if (!list)
    return;

  printf("token: '%s'\n", list->token);
  printf("value: '%s'\n", list->value);

  printSetupFileList(list->next);
}
#endif
#endif

#ifdef DEBUG
DEFINE_HASHTABLE_INSERT(insert_hash_entry, char, char);
DEFINE_HASHTABLE_SEARCH(search_hash_entry, char, char);
DEFINE_HASHTABLE_CHANGE(change_hash_entry, char, char);
DEFINE_HASHTABLE_REMOVE(remove_hash_entry, char, char);
#else
#define insert_hash_entry hashtable_insert
#define search_hash_entry hashtable_search
#define change_hash_entry hashtable_change
#define remove_hash_entry hashtable_remove
#endif

unsigned int get_hash_from_key(void *key)
{
  /*
    djb2

    This algorithm (k=33) was first reported by Dan Bernstein many years ago in
    'comp.lang.c'. Another version of this algorithm (now favored by Bernstein)
    uses XOR: hash(i) = hash(i - 1) * 33 ^ str[i]; the magic of number 33 (why
    it works better than many other constants, prime or not) has never been
    adequately explained.

    If you just want to have a good hash function, and cannot wait, djb2
    is one of the best string hash functions i know. It has excellent
    distribution and speed on many different sets of keys and table sizes.
    You are not likely to do better with one of the "well known" functions
    such as PJW, K&R, etc.

    Ozan (oz) Yigit [http://www.cs.yorku.ca/~oz/hash.html]
  */

  char *str = (char *)key;
  unsigned int hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;	/* hash * 33 + c */

  return hash;
}

static int keys_are_equal(void *key1, void *key2)
{
  return (strEqual((char *)key1, (char *)key2));
}

SetupFileHash *newSetupFileHash()
{
  SetupFileHash *new_hash =
    create_hashtable(16, 0.75, get_hash_from_key, keys_are_equal);

  if (new_hash == NULL)
    Error(ERR_EXIT, "create_hashtable() failed -- out of memory");

  return new_hash;
}

void freeSetupFileHash(SetupFileHash *hash)
{
  if (hash == NULL)
    return;

  hashtable_destroy(hash, 1);	/* 1 == also free values stored in hash */
}

char *getHashEntry(SetupFileHash *hash, char *token)
{
  if (hash == NULL)
    return NULL;

  return search_hash_entry(hash, token);
}

void setHashEntry(SetupFileHash *hash, char *token, char *value)
{
  char *value_copy;

  if (hash == NULL)
    return;

  value_copy = getStringCopy(value);

  /* change value; if it does not exist, insert it as new */
  if (!change_hash_entry(hash, token, value_copy))
    if (!insert_hash_entry(hash, getStringCopy(token), value_copy))
      Error(ERR_EXIT, "cannot insert into hash -- aborting");
}

char *removeHashEntry(SetupFileHash *hash, char *token)
{
  if (hash == NULL)
    return NULL;

  return remove_hash_entry(hash, token);
}

#if ENABLE_UNUSED_CODE
#if DEBUG
static void printSetupFileHash(SetupFileHash *hash)
{
  BEGIN_HASH_ITERATION(hash, itr)
  {
    printf("token: '%s'\n", HASH_ITERATION_TOKEN(itr));
    printf("value: '%s'\n", HASH_ITERATION_VALUE(itr));
  }
  END_HASH_ITERATION(hash, itr)
}
#endif
#endif

#define ALLOW_TOKEN_VALUE_SEPARATOR_BEING_WHITESPACE		1
#define CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING		0
#define CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH		0

static boolean token_value_separator_found = FALSE;
#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
static boolean token_value_separator_warning = FALSE;
#endif
#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
static boolean token_already_exists_warning = FALSE;
#endif

static boolean getTokenValueFromSetupLineExt(char *line,
					     char **token_ptr, char **value_ptr,
					     char *filename, char *line_raw,
					     int line_nr,
					     boolean separator_required)
{
  static char line_copy[MAX_LINE_LEN + 1], line_raw_copy[MAX_LINE_LEN + 1];
  char *token, *value, *line_ptr;

  /* when externally invoked via ReadTokenValueFromLine(), copy line buffers */
  if (line_raw == NULL)
  {
    strncpy(line_copy, line, MAX_LINE_LEN);
    line_copy[MAX_LINE_LEN] = '\0';
    line = line_copy;

    strcpy(line_raw_copy, line_copy);
    line_raw = line_raw_copy;
  }

  /* cut trailing comment from input line */
  for (line_ptr = line; *line_ptr; line_ptr++)
  {
    if (*line_ptr == '#')
    {
      *line_ptr = '\0';
      break;
    }
  }

  /* cut trailing whitespaces from input line */
  for (line_ptr = &line[strlen(line)]; line_ptr >= line; line_ptr--)
    if ((*line_ptr == ' ' || *line_ptr == '\t') && *(line_ptr + 1) == '\0')
      *line_ptr = '\0';

  /* ignore empty lines */
  if (*line == '\0')
    return FALSE;

  /* cut leading whitespaces from token */
  for (token = line; *token; token++)
    if (*token != ' ' && *token != '\t')
      break;

  /* start with empty value as reliable default */
  value = "";

  token_value_separator_found = FALSE;

  /* find end of token to determine start of value */
  for (line_ptr = token; *line_ptr; line_ptr++)
  {
    /* first look for an explicit token/value separator, like ':' or '=' */
    if (*line_ptr == ':' || *line_ptr == '=')
    {
      *line_ptr = '\0';			/* terminate token string */
      value = line_ptr + 1;		/* set beginning of value */

      token_value_separator_found = TRUE;

      break;
    }
  }

#if ALLOW_TOKEN_VALUE_SEPARATOR_BEING_WHITESPACE
  /* fallback: if no token/value separator found, also allow whitespaces */
  if (!token_value_separator_found && !separator_required)
  {
    for (line_ptr = token; *line_ptr; line_ptr++)
    {
      if (*line_ptr == ' ' || *line_ptr == '\t')
      {
	*line_ptr = '\0';		/* terminate token string */
	value = line_ptr + 1;		/* set beginning of value */

	token_value_separator_found = TRUE;

	break;
      }
    }

#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
    if (token_value_separator_found)
    {
      if (!token_value_separator_warning)
      {
	Error(ERR_INFO_LINE, "-");

	if (filename != NULL)
	{
	  Error(ERR_WARN, "missing token/value separator(s) in config file:");
	  Error(ERR_INFO, "- config file: '%s'", filename);
	}
	else
	{
	  Error(ERR_WARN, "missing token/value separator(s):");
	}

	token_value_separator_warning = TRUE;
      }

      if (filename != NULL)
	Error(ERR_INFO, "- line %d: '%s'", line_nr, line_raw);
      else
	Error(ERR_INFO, "- line: '%s'", line_raw);
    }
#endif
  }
#endif

  /* cut trailing whitespaces from token */
  for (line_ptr = &token[strlen(token)]; line_ptr >= token; line_ptr--)
    if ((*line_ptr == ' ' || *line_ptr == '\t') && *(line_ptr + 1) == '\0')
      *line_ptr = '\0';

  /* cut leading whitespaces from value */
  for (; *value; value++)
    if (*value != ' ' && *value != '\t')
      break;

  *token_ptr = token;
  *value_ptr = value;

  return TRUE;
}

boolean getTokenValueFromSetupLine(char *line, char **token, char **value)
{
  /* while the internal (old) interface does not require a token/value
     separator (for downwards compatibility with existing files which
     don't use them), it is mandatory for the external (new) interface */

  return getTokenValueFromSetupLineExt(line, token, value, NULL, NULL, 0, TRUE);
}

static boolean loadSetupFileData(void *setup_file_data, char *filename,
				 boolean top_recursion_level, boolean is_hash)
{
  static SetupFileHash *include_filename_hash = NULL;
  char line[MAX_LINE_LEN], line_raw[MAX_LINE_LEN], previous_line[MAX_LINE_LEN];
  char *token, *value, *line_ptr;
  void *insert_ptr = NULL;
  boolean read_continued_line = FALSE;
  File *file;
  int line_nr = 0, token_count = 0, include_count = 0;

#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
  token_value_separator_warning = FALSE;
#endif

#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
  token_already_exists_warning = FALSE;
#endif

  if (!(file = openFile(filename, MODE_READ)))
  {
    Error(ERR_DEBUG, "cannot open configuration file '%s'", filename);

    return FALSE;
  }

  /* use "insert pointer" to store list end for constant insertion complexity */
  if (!is_hash)
    insert_ptr = setup_file_data;

  /* on top invocation, create hash to mark included files (to prevent loops) */
  if (top_recursion_level)
    include_filename_hash = newSetupFileHash();

  /* mark this file as already included (to prevent including it again) */
  setHashEntry(include_filename_hash, getBaseNamePtr(filename), "true");

  while (!checkEndOfFile(file))
  {
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

      strcpy(line, previous_line);	/* copy storage buffer to line */

      read_continued_line = FALSE;
    }

    /* if the last character is '\', continue at next line */
    if (strlen(line) > 0 && line[strlen(line) - 1] == '\\')
    {
      line[strlen(line) - 1] = '\0';	/* cut off trailing backslash */
      strcpy(previous_line, line);	/* copy line to storage buffer */

      read_continued_line = TRUE;

      continue;
    }

    if (!getTokenValueFromSetupLineExt(line, &token, &value, filename,
				       line_raw, line_nr, FALSE))
      continue;

    if (*token)
    {
      if (strEqual(token, "include"))
      {
	if (getHashEntry(include_filename_hash, value) == NULL)
	{
	  char *basepath = getBasePath(filename);
	  char *basename = getBaseName(value);
	  char *filename_include = getPath2(basepath, basename);

	  loadSetupFileData(setup_file_data, filename_include, FALSE, is_hash);

	  free(basepath);
	  free(basename);
	  free(filename_include);

	  include_count++;
	}
	else
	{
	  Error(ERR_WARN, "ignoring already processed file '%s'", value);
	}
      }
      else
      {
	if (is_hash)
	{
#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
	  char *old_value =
	    getHashEntry((SetupFileHash *)setup_file_data, token);

	  if (old_value != NULL)
	  {
	    if (!token_already_exists_warning)
	    {
	      Error(ERR_INFO_LINE, "-");
	      Error(ERR_WARN, "duplicate token(s) found in config file:");
	      Error(ERR_INFO, "- config file: '%s'", filename);

	      token_already_exists_warning = TRUE;
	    }

	    Error(ERR_INFO, "- token: '%s' (in line %d)", token, line_nr);
	    Error(ERR_INFO, "  old value: '%s'", old_value);
	    Error(ERR_INFO, "  new value: '%s'", value);
	  }
#endif

	  setHashEntry((SetupFileHash *)setup_file_data, token, value);
	}
	else
	{
	  insert_ptr = addListEntry((SetupFileList *)insert_ptr, token, value);
	}

	token_count++;
      }
    }
  }

  closeFile(file);

#if CHECK_TOKEN_VALUE_SEPARATOR__WARN_IF_MISSING
  if (token_value_separator_warning)
    Error(ERR_INFO_LINE, "-");
#endif

#if CHECK_TOKEN__WARN_IF_ALREADY_EXISTS_IN_HASH
  if (token_already_exists_warning)
    Error(ERR_INFO_LINE, "-");
#endif

  if (token_count == 0 && include_count == 0)
    Error(ERR_WARN, "configuration file '%s' is empty", filename);

  if (top_recursion_level)
    freeSetupFileHash(include_filename_hash);

  return TRUE;
}

void saveSetupFileHash(SetupFileHash *hash, char *filename)
{
  FILE *file;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write configuration file '%s'", filename);

    return;
  }

  BEGIN_HASH_ITERATION(hash, itr)
  {
    fprintf(file, "%s\n", getFormattedSetupEntry(HASH_ITERATION_TOKEN(itr),
						 HASH_ITERATION_VALUE(itr)));
  }
  END_HASH_ITERATION(hash, itr)

  fclose(file);
}

SetupFileList *loadSetupFileList(char *filename)
{
  SetupFileList *setup_file_list = newSetupFileList("", "");
  SetupFileList *first_valid_list_entry;

  if (!loadSetupFileData(setup_file_list, filename, TRUE, FALSE))
  {
    freeSetupFileList(setup_file_list);

    return NULL;
  }

  first_valid_list_entry = setup_file_list->next;

  /* free empty list header */
  setup_file_list->next = NULL;
  freeSetupFileList(setup_file_list);

  return first_valid_list_entry;
}

SetupFileHash *loadSetupFileHash(char *filename)
{
  SetupFileHash *setup_file_hash = newSetupFileHash();

  if (!loadSetupFileData(setup_file_hash, filename, TRUE, TRUE))
  {
    freeSetupFileHash(setup_file_hash);

    return NULL;
  }

  return setup_file_hash;
}


/* ========================================================================= */
/* setup file stuff                                                          */
/* ========================================================================= */

#define TOKEN_STR_LAST_LEVEL_SERIES		"last_level_series"
#define TOKEN_STR_LAST_PLAYED_LEVEL		"last_played_level"
#define TOKEN_STR_HANDICAP_LEVEL		"handicap_level"

/* level directory info */
#define LEVELINFO_TOKEN_IDENTIFIER		0
#define LEVELINFO_TOKEN_NAME			1
#define LEVELINFO_TOKEN_NAME_SORTING		2
#define LEVELINFO_TOKEN_AUTHOR			3
#define LEVELINFO_TOKEN_YEAR			4
#define LEVELINFO_TOKEN_PROGRAM_TITLE		5
#define LEVELINFO_TOKEN_PROGRAM_COPYRIGHT	6
#define LEVELINFO_TOKEN_PROGRAM_COMPANY		7
#define LEVELINFO_TOKEN_IMPORTED_FROM		8
#define LEVELINFO_TOKEN_IMPORTED_BY		9
#define LEVELINFO_TOKEN_TESTED_BY		10
#define LEVELINFO_TOKEN_LEVELS			11
#define LEVELINFO_TOKEN_FIRST_LEVEL		12
#define LEVELINFO_TOKEN_SORT_PRIORITY		13
#define LEVELINFO_TOKEN_LATEST_ENGINE		14
#define LEVELINFO_TOKEN_LEVEL_GROUP		15
#define LEVELINFO_TOKEN_READONLY		16
#define LEVELINFO_TOKEN_GRAPHICS_SET_ECS	17
#define LEVELINFO_TOKEN_GRAPHICS_SET_AGA	18
#define LEVELINFO_TOKEN_GRAPHICS_SET		19
#define LEVELINFO_TOKEN_SOUNDS_SET		20
#define LEVELINFO_TOKEN_MUSIC_SET		21
#define LEVELINFO_TOKEN_FILENAME		22
#define LEVELINFO_TOKEN_FILETYPE		23
#define LEVELINFO_TOKEN_SPECIAL_FLAGS		24
#define LEVELINFO_TOKEN_HANDICAP		25
#define LEVELINFO_TOKEN_SKIP_LEVELS		26

#define NUM_LEVELINFO_TOKENS			27

static LevelDirTree ldi;

static struct TokenInfo levelinfo_tokens[] =
{
  /* level directory info */
  { TYPE_STRING,	&ldi.identifier,	"identifier"		},
  { TYPE_STRING,	&ldi.name,		"name"			},
  { TYPE_STRING,	&ldi.name_sorting,	"name_sorting"		},
  { TYPE_STRING,	&ldi.author,		"author"		},
  { TYPE_STRING,	&ldi.year,		"year"			},
  { TYPE_STRING,	&ldi.program_title,	"program_title"		},
  { TYPE_STRING,	&ldi.program_copyright,	"program_copyright"	},
  { TYPE_STRING,	&ldi.program_company,	"program_company"	},
  { TYPE_STRING,	&ldi.imported_from,	"imported_from"		},
  { TYPE_STRING,	&ldi.imported_by,	"imported_by"		},
  { TYPE_STRING,	&ldi.tested_by,		"tested_by"		},
  { TYPE_INTEGER,	&ldi.levels,		"levels"		},
  { TYPE_INTEGER,	&ldi.first_level,	"first_level"		},
  { TYPE_INTEGER,	&ldi.sort_priority,	"sort_priority"		},
  { TYPE_BOOLEAN,	&ldi.latest_engine,	"latest_engine"		},
  { TYPE_BOOLEAN,	&ldi.level_group,	"level_group"		},
  { TYPE_BOOLEAN,	&ldi.readonly,		"readonly"		},
  { TYPE_STRING,	&ldi.graphics_set_ecs,	"graphics_set.ecs"	},
  { TYPE_STRING,	&ldi.graphics_set_aga,	"graphics_set.aga"	},
  { TYPE_STRING,	&ldi.graphics_set,	"graphics_set"		},
  { TYPE_STRING,	&ldi.sounds_set,	"sounds_set"		},
  { TYPE_STRING,	&ldi.music_set,		"music_set"		},
  { TYPE_STRING,	&ldi.level_filename,	"filename"		},
  { TYPE_STRING,	&ldi.level_filetype,	"filetype"		},
  { TYPE_STRING,	&ldi.special_flags,	"special_flags"		},
  { TYPE_BOOLEAN,	&ldi.handicap,		"handicap"		},
  { TYPE_BOOLEAN,	&ldi.skip_levels,	"skip_levels"		}
};

static struct TokenInfo artworkinfo_tokens[] =
{
  /* artwork directory info */
  { TYPE_STRING,	&ldi.identifier,	"identifier"		},
  { TYPE_STRING,	&ldi.subdir,		"subdir"		},
  { TYPE_STRING,	&ldi.name,		"name"			},
  { TYPE_STRING,	&ldi.name_sorting,	"name_sorting"		},
  { TYPE_STRING,	&ldi.author,		"author"		},
  { TYPE_STRING,	&ldi.program_title,	"program_title"		},
  { TYPE_STRING,	&ldi.program_copyright,	"program_copyright"	},
  { TYPE_STRING,	&ldi.program_company,	"program_company"	},
  { TYPE_INTEGER,	&ldi.sort_priority,	"sort_priority"		},
  { TYPE_STRING,	&ldi.basepath,		"basepath"		},
  { TYPE_STRING,	&ldi.fullpath,		"fullpath"		},
  { TYPE_BOOLEAN,	&ldi.in_user_dir,	"in_user_dir"		},
  { TYPE_INTEGER,	&ldi.color,		"color"			},
  { TYPE_STRING,	&ldi.class_desc,	"class_desc"		},

  { -1,			NULL,			NULL			},
};

static void setTreeInfoToDefaults(TreeInfo *ti, int type)
{
  ti->type = type;

  ti->node_top = (ti->type == TREE_TYPE_LEVEL_DIR    ? &leveldir_first :
		  ti->type == TREE_TYPE_GRAPHICS_DIR ? &artwork.gfx_first :
		  ti->type == TREE_TYPE_SOUNDS_DIR   ? &artwork.snd_first :
		  ti->type == TREE_TYPE_MUSIC_DIR    ? &artwork.mus_first :
		  NULL);

  ti->node_parent = NULL;
  ti->node_group = NULL;
  ti->next = NULL;

  ti->cl_first = -1;
  ti->cl_cursor = -1;

  ti->subdir = NULL;
  ti->fullpath = NULL;
  ti->basepath = NULL;
  ti->identifier = NULL;
  ti->name = getStringCopy(ANONYMOUS_NAME);
  ti->name_sorting = NULL;
  ti->author = getStringCopy(ANONYMOUS_NAME);
  ti->year = NULL;

  ti->program_title = NULL;
  ti->program_copyright = NULL;
  ti->program_company = NULL;

  ti->sort_priority = LEVELCLASS_UNDEFINED;	/* default: least priority */
  ti->latest_engine = FALSE;			/* default: get from level */
  ti->parent_link = FALSE;
  ti->in_user_dir = FALSE;
  ti->user_defined = FALSE;
  ti->color = 0;
  ti->class_desc = NULL;

  ti->infotext = getStringCopy(TREE_INFOTEXT(ti->type));

  if (ti->type == TREE_TYPE_LEVEL_DIR)
  {
    ti->imported_from = NULL;
    ti->imported_by = NULL;
    ti->tested_by = NULL;

    ti->graphics_set_ecs = NULL;
    ti->graphics_set_aga = NULL;
    ti->graphics_set = NULL;
    ti->sounds_set = NULL;
    ti->music_set = NULL;
    ti->graphics_path = getStringCopy(UNDEFINED_FILENAME);
    ti->sounds_path = getStringCopy(UNDEFINED_FILENAME);
    ti->music_path = getStringCopy(UNDEFINED_FILENAME);

    ti->level_filename = NULL;
    ti->level_filetype = NULL;

    ti->special_flags = NULL;

    ti->levels = 0;
    ti->first_level = 0;
    ti->last_level = 0;
    ti->level_group = FALSE;
    ti->handicap_level = 0;
    ti->readonly = TRUE;
    ti->handicap = TRUE;
    ti->skip_levels = FALSE;
  }
}

static void setTreeInfoToDefaultsFromParent(TreeInfo *ti, TreeInfo *parent)
{
  if (parent == NULL)
  {
    Error(ERR_WARN, "setTreeInfoToDefaultsFromParent(): parent == NULL");

    setTreeInfoToDefaults(ti, TREE_TYPE_UNDEFINED);

    return;
  }

  /* copy all values from the parent structure */

  ti->type = parent->type;

  ti->node_top = parent->node_top;
  ti->node_parent = parent;
  ti->node_group = NULL;
  ti->next = NULL;

  ti->cl_first = -1;
  ti->cl_cursor = -1;

  ti->subdir = NULL;
  ti->fullpath = NULL;
  ti->basepath = NULL;
  ti->identifier = NULL;
  ti->name = getStringCopy(ANONYMOUS_NAME);
  ti->name_sorting = NULL;
  ti->author = getStringCopy(parent->author);
  ti->year = getStringCopy(parent->year);

  ti->program_title = getStringCopy(parent->program_title);
  ti->program_copyright = getStringCopy(parent->program_copyright);
  ti->program_company = getStringCopy(parent->program_company);

  ti->sort_priority = parent->sort_priority;
  ti->latest_engine = parent->latest_engine;
  ti->parent_link = FALSE;
  ti->in_user_dir = parent->in_user_dir;
  ti->user_defined = parent->user_defined;
  ti->color = parent->color;
  ti->class_desc = getStringCopy(parent->class_desc);

  ti->infotext = getStringCopy(parent->infotext);

  if (ti->type == TREE_TYPE_LEVEL_DIR)
  {
    ti->imported_from = getStringCopy(parent->imported_from);
    ti->imported_by = getStringCopy(parent->imported_by);
    ti->tested_by = getStringCopy(parent->tested_by);

    ti->graphics_set_ecs = getStringCopy(parent->graphics_set_ecs);
    ti->graphics_set_aga = getStringCopy(parent->graphics_set_aga);
    ti->graphics_set = getStringCopy(parent->graphics_set);
    ti->sounds_set = getStringCopy(parent->sounds_set);
    ti->music_set = getStringCopy(parent->music_set);
    ti->graphics_path = getStringCopy(UNDEFINED_FILENAME);
    ti->sounds_path = getStringCopy(UNDEFINED_FILENAME);
    ti->music_path = getStringCopy(UNDEFINED_FILENAME);

    ti->level_filename = getStringCopy(parent->level_filename);
    ti->level_filetype = getStringCopy(parent->level_filetype);

    ti->special_flags = getStringCopy(parent->special_flags);

    ti->levels = parent->levels;
    ti->first_level = parent->first_level;
    ti->last_level = parent->last_level;
    ti->level_group = FALSE;
    ti->handicap_level = parent->handicap_level;
    ti->readonly = parent->readonly;
    ti->handicap = parent->handicap;
    ti->skip_levels = parent->skip_levels;
  }
}

static TreeInfo *getTreeInfoCopy(TreeInfo *ti)
{
  TreeInfo *ti_copy = newTreeInfo();

  /* copy all values from the original structure */

  ti_copy->type			= ti->type;

  ti_copy->node_top		= ti->node_top;
  ti_copy->node_parent		= ti->node_parent;
  ti_copy->node_group		= ti->node_group;
  ti_copy->next			= ti->next;

  ti_copy->cl_first		= ti->cl_first;
  ti_copy->cl_cursor		= ti->cl_cursor;

  ti_copy->subdir		= getStringCopy(ti->subdir);
  ti_copy->fullpath		= getStringCopy(ti->fullpath);
  ti_copy->basepath		= getStringCopy(ti->basepath);
  ti_copy->identifier		= getStringCopy(ti->identifier);
  ti_copy->name			= getStringCopy(ti->name);
  ti_copy->name_sorting		= getStringCopy(ti->name_sorting);
  ti_copy->author		= getStringCopy(ti->author);
  ti_copy->year			= getStringCopy(ti->year);

  ti_copy->program_title	= getStringCopy(ti->program_title);
  ti_copy->program_copyright	= getStringCopy(ti->program_copyright);
  ti_copy->program_company	= getStringCopy(ti->program_company);

  ti_copy->imported_from	= getStringCopy(ti->imported_from);
  ti_copy->imported_by		= getStringCopy(ti->imported_by);
  ti_copy->tested_by		= getStringCopy(ti->tested_by);

  ti_copy->graphics_set_ecs	= getStringCopy(ti->graphics_set_ecs);
  ti_copy->graphics_set_aga	= getStringCopy(ti->graphics_set_aga);
  ti_copy->graphics_set		= getStringCopy(ti->graphics_set);
  ti_copy->sounds_set		= getStringCopy(ti->sounds_set);
  ti_copy->music_set		= getStringCopy(ti->music_set);
  ti_copy->graphics_path	= getStringCopy(ti->graphics_path);
  ti_copy->sounds_path		= getStringCopy(ti->sounds_path);
  ti_copy->music_path		= getStringCopy(ti->music_path);

  ti_copy->level_filename	= getStringCopy(ti->level_filename);
  ti_copy->level_filetype	= getStringCopy(ti->level_filetype);

  ti_copy->special_flags	= getStringCopy(ti->special_flags);

  ti_copy->levels		= ti->levels;
  ti_copy->first_level		= ti->first_level;
  ti_copy->last_level		= ti->last_level;
  ti_copy->sort_priority	= ti->sort_priority;

  ti_copy->latest_engine	= ti->latest_engine;

  ti_copy->level_group		= ti->level_group;
  ti_copy->parent_link		= ti->parent_link;
  ti_copy->in_user_dir		= ti->in_user_dir;
  ti_copy->user_defined		= ti->user_defined;
  ti_copy->readonly		= ti->readonly;
  ti_copy->handicap		= ti->handicap;
  ti_copy->skip_levels		= ti->skip_levels;

  ti_copy->color		= ti->color;
  ti_copy->class_desc		= getStringCopy(ti->class_desc);
  ti_copy->handicap_level	= ti->handicap_level;

  ti_copy->infotext		= getStringCopy(ti->infotext);

  return ti_copy;
}

void freeTreeInfo(TreeInfo *ti)
{
  if (ti == NULL)
    return;

  checked_free(ti->subdir);
  checked_free(ti->fullpath);
  checked_free(ti->basepath);
  checked_free(ti->identifier);

  checked_free(ti->name);
  checked_free(ti->name_sorting);
  checked_free(ti->author);
  checked_free(ti->year);

  checked_free(ti->program_title);
  checked_free(ti->program_copyright);
  checked_free(ti->program_company);

  checked_free(ti->class_desc);

  checked_free(ti->infotext);

  if (ti->type == TREE_TYPE_LEVEL_DIR)
  {
    checked_free(ti->imported_from);
    checked_free(ti->imported_by);
    checked_free(ti->tested_by);

    checked_free(ti->graphics_set_ecs);
    checked_free(ti->graphics_set_aga);
    checked_free(ti->graphics_set);
    checked_free(ti->sounds_set);
    checked_free(ti->music_set);

    checked_free(ti->graphics_path);
    checked_free(ti->sounds_path);
    checked_free(ti->music_path);

    checked_free(ti->level_filename);
    checked_free(ti->level_filetype);

    checked_free(ti->special_flags);
  }

  // recursively free child node
  if (ti->node_group)
    freeTreeInfo(ti->node_group);

  // recursively free next node
  if (ti->next)
    freeTreeInfo(ti->next);

  checked_free(ti);
}

void setSetupInfo(struct TokenInfo *token_info,
		  int token_nr, char *token_value)
{
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;

  if (token_value == NULL)
    return;

  /* set setup field to corresponding token value */
  switch (token_type)
  {
    case TYPE_BOOLEAN:
    case TYPE_SWITCH:
      *(boolean *)setup_value = get_boolean_from_string(token_value);
      break;

    case TYPE_SWITCH3:
      *(int *)setup_value = get_switch3_from_string(token_value);
      break;

    case TYPE_KEY:
      *(Key *)setup_value = getKeyFromKeyName(token_value);
      break;

    case TYPE_KEY_X11:
      *(Key *)setup_value = getKeyFromX11KeyName(token_value);
      break;

    case TYPE_INTEGER:
      *(int *)setup_value = get_integer_from_string(token_value);
      break;

    case TYPE_STRING:
      checked_free(*(char **)setup_value);
      *(char **)setup_value = getStringCopy(token_value);
      break;

    default:
      break;
  }
}

static int compareTreeInfoEntries(const void *object1, const void *object2)
{
  const TreeInfo *entry1 = *((TreeInfo **)object1);
  const TreeInfo *entry2 = *((TreeInfo **)object2);
  int class_sorting1 = 0, class_sorting2 = 0;
  int compare_result;

  if (entry1->type == TREE_TYPE_LEVEL_DIR)
  {
    class_sorting1 = LEVELSORTING(entry1);
    class_sorting2 = LEVELSORTING(entry2);
  }
  else if (entry1->type == TREE_TYPE_GRAPHICS_DIR ||
	   entry1->type == TREE_TYPE_SOUNDS_DIR ||
	   entry1->type == TREE_TYPE_MUSIC_DIR)
  {
    class_sorting1 = ARTWORKSORTING(entry1);
    class_sorting2 = ARTWORKSORTING(entry2);
  }

  if (entry1->parent_link || entry2->parent_link)
    compare_result = (entry1->parent_link ? -1 : +1);
  else if (entry1->sort_priority == entry2->sort_priority)
  {
    char *name1 = getStringToLower(entry1->name_sorting);
    char *name2 = getStringToLower(entry2->name_sorting);

    compare_result = strcmp(name1, name2);

    free(name1);
    free(name2);
  }
  else if (class_sorting1 == class_sorting2)
    compare_result = entry1->sort_priority - entry2->sort_priority;
  else
    compare_result = class_sorting1 - class_sorting2;

  return compare_result;
}

static TreeInfo *createParentTreeInfoNode(TreeInfo *node_parent)
{
  TreeInfo *ti_new;

  if (node_parent == NULL)
    return NULL;

  ti_new = newTreeInfo();
  setTreeInfoToDefaults(ti_new, node_parent->type);

  ti_new->node_parent = node_parent;
  ti_new->parent_link = TRUE;

  setString(&ti_new->identifier, node_parent->identifier);
  setString(&ti_new->name, ".. (parent directory)");
  setString(&ti_new->name_sorting, ti_new->name);

  setString(&ti_new->subdir, STRING_PARENT_DIRECTORY);
  setString(&ti_new->fullpath, node_parent->fullpath);

  ti_new->sort_priority = node_parent->sort_priority;
  ti_new->latest_engine = node_parent->latest_engine;

  setString(&ti_new->class_desc, getLevelClassDescription(ti_new));

  pushTreeInfo(&node_parent->node_group, ti_new);

  return ti_new;
}

static TreeInfo *createTopTreeInfoNode(TreeInfo *node_first)
{
  TreeInfo *ti_new, *ti_new2;

  if (node_first == NULL)
    return NULL;

  ti_new = newTreeInfo();
  setTreeInfoToDefaults(ti_new, TREE_TYPE_LEVEL_DIR);

  ti_new->node_parent = NULL;
  ti_new->parent_link = FALSE;

  setString(&ti_new->identifier, node_first->identifier);
  setString(&ti_new->name, "level sets");
  setString(&ti_new->name_sorting, ti_new->name);

  setString(&ti_new->subdir, STRING_TOP_DIRECTORY);
  setString(&ti_new->fullpath, ".");

  ti_new->sort_priority = node_first->sort_priority;;
  ti_new->latest_engine = node_first->latest_engine;

  setString(&ti_new->class_desc, "level sets");

  ti_new->node_group = node_first;
  ti_new->level_group = TRUE;

  ti_new2 = createParentTreeInfoNode(ti_new);

  setString(&ti_new2->name, ".. (main menu)");
  setString(&ti_new2->name_sorting, ti_new2->name);

  return ti_new;
}


/* -------------------------------------------------------------------------- */
/* functions for handling level and custom artwork info cache                 */
/* -------------------------------------------------------------------------- */

static void LoadArtworkInfoCache()
{
  InitCacheDirectory();

  if (artworkinfo_cache_old == NULL)
  {
    char *filename = getPath2(getCacheDir(), ARTWORKINFO_CACHE_FILE);

    /* try to load artwork info hash from already existing cache file */
    artworkinfo_cache_old = loadSetupFileHash(filename);

    /* if no artwork info cache file was found, start with empty hash */
    if (artworkinfo_cache_old == NULL)
      artworkinfo_cache_old = newSetupFileHash();

    free(filename);
  }

  if (artworkinfo_cache_new == NULL)
    artworkinfo_cache_new = newSetupFileHash();
}

static void SaveArtworkInfoCache()
{
  char *filename = getPath2(getCacheDir(), ARTWORKINFO_CACHE_FILE);

  InitCacheDirectory();

  saveSetupFileHash(artworkinfo_cache_new, filename);

  free(filename);
}

static char *getCacheTokenPrefix(char *prefix1, char *prefix2)
{
  static char *prefix = NULL;

  checked_free(prefix);

  prefix = getStringCat2WithSeparator(prefix1, prefix2, ".");

  return prefix;
}

/* (identical to above function, but separate string buffer needed -- nasty) */
static char *getCacheToken(char *prefix, char *suffix)
{
  static char *token = NULL;

  checked_free(token);

  token = getStringCat2WithSeparator(prefix, suffix, ".");

  return token;
}

static char *getFileTimestampString(char *filename)
{
  return getStringCopy(i_to_a(getFileTimestampEpochSeconds(filename)));
}

static boolean modifiedFileTimestamp(char *filename, char *timestamp_string)
{
  struct stat file_status;

  if (timestamp_string == NULL)
    return TRUE;

  if (stat(filename, &file_status) != 0)	/* cannot stat file */
    return TRUE;

  return (file_status.st_mtime != atoi(timestamp_string));
}

static TreeInfo *getArtworkInfoCacheEntry(LevelDirTree *level_node, int type)
{
  char *identifier = level_node->subdir;
  char *type_string = ARTWORK_DIRECTORY(type);
  char *token_prefix = getCacheTokenPrefix(type_string, identifier);
  char *token_main = getCacheToken(token_prefix, "CACHED");
  char *cache_entry = getHashEntry(artworkinfo_cache_old, token_main);
  boolean cached = (cache_entry != NULL && strEqual(cache_entry, "true"));
  TreeInfo *artwork_info = NULL;

  if (!use_artworkinfo_cache)
    return NULL;

  if (cached)
  {
    int i;

    artwork_info = newTreeInfo();
    setTreeInfoToDefaults(artwork_info, type);

    /* set all structure fields according to the token/value pairs */
    ldi = *artwork_info;
    for (i = 0; artworkinfo_tokens[i].type != -1; i++)
    {
      char *token = getCacheToken(token_prefix, artworkinfo_tokens[i].text);
      char *value = getHashEntry(artworkinfo_cache_old, token);

      /* if defined, use value from cache, else keep default value */
      if (value != NULL)
	setSetupInfo(artworkinfo_tokens, i, value);
    }

    *artwork_info = ldi;

    char *filename_levelinfo = getPath2(getLevelDirFromTreeInfo(level_node),
					LEVELINFO_FILENAME);
    char *filename_artworkinfo = getPath2(getSetupArtworkDir(artwork_info),
					  ARTWORKINFO_FILENAME(type));

    /* check if corresponding "levelinfo.conf" file has changed */
    token_main = getCacheToken(token_prefix, "TIMESTAMP_LEVELINFO");
    cache_entry = getHashEntry(artworkinfo_cache_old, token_main);

    if (modifiedFileTimestamp(filename_levelinfo, cache_entry))
      cached = FALSE;

    /* check if corresponding "<artworkinfo>.conf" file has changed */
    token_main = getCacheToken(token_prefix, "TIMESTAMP_ARTWORKINFO");
    cache_entry = getHashEntry(artworkinfo_cache_old, token_main);

    if (modifiedFileTimestamp(filename_artworkinfo, cache_entry))
      cached = FALSE;

    checked_free(filename_levelinfo);
    checked_free(filename_artworkinfo);
  }

  if (!cached && artwork_info != NULL)
  {
    freeTreeInfo(artwork_info);

    return NULL;
  }

  return artwork_info;
}

static void setArtworkInfoCacheEntry(TreeInfo *artwork_info,
				     LevelDirTree *level_node, int type)
{
  char *identifier = level_node->subdir;
  char *type_string = ARTWORK_DIRECTORY(type);
  char *token_prefix = getCacheTokenPrefix(type_string, identifier);
  char *token_main = getCacheToken(token_prefix, "CACHED");
  boolean set_cache_timestamps = TRUE;
  int i;

  setHashEntry(artworkinfo_cache_new, token_main, "true");

  if (set_cache_timestamps)
  {
    char *filename_levelinfo = getPath2(getLevelDirFromTreeInfo(level_node),
					LEVELINFO_FILENAME);
    char *filename_artworkinfo = getPath2(getSetupArtworkDir(artwork_info),
					  ARTWORKINFO_FILENAME(type));
    char *timestamp_levelinfo = getFileTimestampString(filename_levelinfo);
    char *timestamp_artworkinfo = getFileTimestampString(filename_artworkinfo);

    token_main = getCacheToken(token_prefix, "TIMESTAMP_LEVELINFO");
    setHashEntry(artworkinfo_cache_new, token_main, timestamp_levelinfo);

    token_main = getCacheToken(token_prefix, "TIMESTAMP_ARTWORKINFO");
    setHashEntry(artworkinfo_cache_new, token_main, timestamp_artworkinfo);

    checked_free(filename_levelinfo);
    checked_free(filename_artworkinfo);
    checked_free(timestamp_levelinfo);
    checked_free(timestamp_artworkinfo);
  }

  ldi = *artwork_info;
  for (i = 0; artworkinfo_tokens[i].type != -1; i++)
  {
    char *token = getCacheToken(token_prefix, artworkinfo_tokens[i].text);
    char *value = getSetupValue(artworkinfo_tokens[i].type,
				artworkinfo_tokens[i].value);
    if (value != NULL)
      setHashEntry(artworkinfo_cache_new, token, value);
  }
}


/* -------------------------------------------------------------------------- */
/* functions for loading level info and custom artwork info                   */
/* -------------------------------------------------------------------------- */

/* forward declaration for recursive call by "LoadLevelInfoFromLevelDir()" */
static void LoadLevelInfoFromLevelDir(TreeInfo **, TreeInfo *, char *);

static boolean LoadLevelInfoFromLevelConf(TreeInfo **node_first,
					  TreeInfo *node_parent,
					  char *level_directory,
					  char *directory_name)
{
  char *directory_path = getPath2(level_directory, directory_name);
  char *filename = getPath2(directory_path, LEVELINFO_FILENAME);
  SetupFileHash *setup_file_hash;
  LevelDirTree *leveldir_new = NULL;
  int i;

  /* unless debugging, silently ignore directories without "levelinfo.conf" */
  if (!options.debug && !fileExists(filename))
  {
    free(directory_path);
    free(filename);

    return FALSE;
  }

  setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash == NULL)
  {
    Error(ERR_WARN, "ignoring level directory '%s'", directory_path);

    free(directory_path);
    free(filename);

    return FALSE;
  }

  leveldir_new = newTreeInfo();

  if (node_parent)
    setTreeInfoToDefaultsFromParent(leveldir_new, node_parent);
  else
    setTreeInfoToDefaults(leveldir_new, TREE_TYPE_LEVEL_DIR);

  leveldir_new->subdir = getStringCopy(directory_name);

  /* set all structure fields according to the token/value pairs */
  ldi = *leveldir_new;
  for (i = 0; i < NUM_LEVELINFO_TOKENS; i++)
    setSetupInfo(levelinfo_tokens, i,
		 getHashEntry(setup_file_hash, levelinfo_tokens[i].text));
  *leveldir_new = ldi;

  if (strEqual(leveldir_new->name, ANONYMOUS_NAME))
    setString(&leveldir_new->name, leveldir_new->subdir);

  if (leveldir_new->identifier == NULL)
    leveldir_new->identifier = getStringCopy(leveldir_new->subdir);

  if (leveldir_new->name_sorting == NULL)
    leveldir_new->name_sorting = getStringCopy(leveldir_new->name);

  if (node_parent == NULL)		/* top level group */
  {
    leveldir_new->basepath = getStringCopy(level_directory);
    leveldir_new->fullpath = getStringCopy(leveldir_new->subdir);
  }
  else					/* sub level group */
  {
    leveldir_new->basepath = getStringCopy(node_parent->basepath);
    leveldir_new->fullpath = getPath2(node_parent->fullpath, directory_name);
  }

  leveldir_new->last_level =
    leveldir_new->first_level + leveldir_new->levels - 1;

  leveldir_new->in_user_dir =
    (!strEqual(leveldir_new->basepath, options.level_directory));

  /* adjust some settings if user's private level directory was detected */
  if (leveldir_new->sort_priority == LEVELCLASS_UNDEFINED &&
      leveldir_new->in_user_dir &&
      (strEqual(leveldir_new->subdir, getLoginName()) ||
       strEqual(leveldir_new->name,   getLoginName()) ||
       strEqual(leveldir_new->author, getRealName())))
  {
    leveldir_new->sort_priority = LEVELCLASS_PRIVATE_START;
    leveldir_new->readonly = FALSE;
  }

  leveldir_new->user_defined =
    (leveldir_new->in_user_dir && IS_LEVELCLASS_PRIVATE(leveldir_new));

  leveldir_new->color = LEVELCOLOR(leveldir_new);

  setString(&leveldir_new->class_desc, getLevelClassDescription(leveldir_new));

  leveldir_new->handicap_level =	/* set handicap to default value */
    (leveldir_new->user_defined || !leveldir_new->handicap ?
     leveldir_new->last_level : leveldir_new->first_level);

  DrawInitText(leveldir_new->name, 150, FC_YELLOW);

  pushTreeInfo(node_first, leveldir_new);

  freeSetupFileHash(setup_file_hash);

  if (leveldir_new->level_group)
  {
    /* create node to link back to current level directory */
    createParentTreeInfoNode(leveldir_new);

    /* recursively step into sub-directory and look for more level series */
    LoadLevelInfoFromLevelDir(&leveldir_new->node_group,
			      leveldir_new, directory_path);
  }

  free(directory_path);
  free(filename);

  return TRUE;
}

static void LoadLevelInfoFromLevelDir(TreeInfo **node_first,
				      TreeInfo *node_parent,
				      char *level_directory)
{
  Directory *dir;
  DirectoryEntry *dir_entry;
  boolean valid_entry_found = FALSE;

  if ((dir = openDirectory(level_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read level directory '%s'", level_directory);

    return;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	/* loop all entries */
  {
    char *directory_name = dir_entry->basename;
    char *directory_path = getPath2(level_directory, directory_name);

    /* skip entries for current and parent directory */
    if (strEqual(directory_name, ".") ||
	strEqual(directory_name, ".."))
    {
      free(directory_path);

      continue;
    }

    /* find out if directory entry is itself a directory */
    if (!dir_entry->is_directory)			/* not a directory */
    {
      free(directory_path);

      continue;
    }

    free(directory_path);

    if (strEqual(directory_name, GRAPHICS_DIRECTORY) ||
	strEqual(directory_name, SOUNDS_DIRECTORY) ||
	strEqual(directory_name, MUSIC_DIRECTORY))
      continue;

    valid_entry_found |= LoadLevelInfoFromLevelConf(node_first, node_parent,
						    level_directory,
						    directory_name);
  }

  closeDirectory(dir);

  /* special case: top level directory may directly contain "levelinfo.conf" */
  if (node_parent == NULL && !valid_entry_found)
  {
    /* check if this directory directly contains a file "levelinfo.conf" */
    valid_entry_found |= LoadLevelInfoFromLevelConf(node_first, node_parent,
						    level_directory, ".");
  }

  if (!valid_entry_found)
    Error(ERR_WARN, "cannot find any valid level series in directory '%s'",
	  level_directory);
}

boolean AdjustGraphicsForEMC()
{
  boolean settings_changed = FALSE;

  settings_changed |= adjustTreeGraphicsForEMC(leveldir_first_all);
  settings_changed |= adjustTreeGraphicsForEMC(leveldir_first);

  return settings_changed;
}

void LoadLevelInfo()
{
  InitUserLevelDirectory(getLoginName());

  DrawInitText("Loading level series", 120, FC_GREEN);

  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, options.level_directory);
  LoadLevelInfoFromLevelDir(&leveldir_first, NULL, getUserLevelDir(NULL));

  leveldir_first = createTopTreeInfoNode(leveldir_first);

  /* after loading all level set information, clone the level directory tree
     and remove all level sets without levels (these may still contain artwork
     to be offered in the setup menu as "custom artwork", and are therefore
     checked for existing artwork in the function "LoadLevelArtworkInfo()") */
  leveldir_first_all = leveldir_first;
  cloneTree(&leveldir_first, leveldir_first_all, TRUE);

  AdjustGraphicsForEMC();

  /* before sorting, the first entries will be from the user directory */
  leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

  if (leveldir_first == NULL)
    Error(ERR_EXIT, "cannot find any valid level series in any directory");

  sortTreeInfo(&leveldir_first);

#if ENABLE_UNUSED_CODE
  dumpTreeInfo(leveldir_first, 0);
#endif
}

static boolean LoadArtworkInfoFromArtworkConf(TreeInfo **node_first,
					      TreeInfo *node_parent,
					      char *base_directory,
					      char *directory_name, int type)
{
  char *directory_path = getPath2(base_directory, directory_name);
  char *filename = getPath2(directory_path, ARTWORKINFO_FILENAME(type));
  SetupFileHash *setup_file_hash = NULL;
  TreeInfo *artwork_new = NULL;
  int i;

  if (fileExists(filename))
    setup_file_hash = loadSetupFileHash(filename);

  if (setup_file_hash == NULL)	/* no config file -- look for artwork files */
  {
    Directory *dir;
    DirectoryEntry *dir_entry;
    boolean valid_file_found = FALSE;

    if ((dir = openDirectory(directory_path)) != NULL)
    {
      while ((dir_entry = readDirectory(dir)) != NULL)
      {
	if (FileIsArtworkType(dir_entry->filename, type))
	{
	  valid_file_found = TRUE;

	  break;
	}
      }

      closeDirectory(dir);
    }

    if (!valid_file_found)
    {
      if (!strEqual(directory_name, "."))
	Error(ERR_WARN, "ignoring artwork directory '%s'", directory_path);

      free(directory_path);
      free(filename);

      return FALSE;
    }
  }

  artwork_new = newTreeInfo();

  if (node_parent)
    setTreeInfoToDefaultsFromParent(artwork_new, node_parent);
  else
    setTreeInfoToDefaults(artwork_new, type);

  artwork_new->subdir = getStringCopy(directory_name);

  if (setup_file_hash)	/* (before defining ".color" and ".class_desc") */
  {
    /* set all structure fields according to the token/value pairs */
    ldi = *artwork_new;
    for (i = 0; i < NUM_LEVELINFO_TOKENS; i++)
      setSetupInfo(levelinfo_tokens, i,
		   getHashEntry(setup_file_hash, levelinfo_tokens[i].text));
    *artwork_new = ldi;

    if (strEqual(artwork_new->name, ANONYMOUS_NAME))
      setString(&artwork_new->name, artwork_new->subdir);

    if (artwork_new->identifier == NULL)
      artwork_new->identifier = getStringCopy(artwork_new->subdir);

    if (artwork_new->name_sorting == NULL)
      artwork_new->name_sorting = getStringCopy(artwork_new->name);
  }

  if (node_parent == NULL)		/* top level group */
  {
    artwork_new->basepath = getStringCopy(base_directory);
    artwork_new->fullpath = getStringCopy(artwork_new->subdir);
  }
  else					/* sub level group */
  {
    artwork_new->basepath = getStringCopy(node_parent->basepath);
    artwork_new->fullpath = getPath2(node_parent->fullpath, directory_name);
  }

  artwork_new->in_user_dir =
    (!strEqual(artwork_new->basepath, OPTIONS_ARTWORK_DIRECTORY(type)));

  /* (may use ".sort_priority" from "setup_file_hash" above) */
  artwork_new->color = ARTWORKCOLOR(artwork_new);

  setString(&artwork_new->class_desc, getLevelClassDescription(artwork_new));

  if (setup_file_hash == NULL)	/* (after determining ".user_defined") */
  {
    if (strEqual(artwork_new->subdir, "."))
    {
      if (artwork_new->user_defined)
      {
	setString(&artwork_new->identifier, "private");
	artwork_new->sort_priority = ARTWORKCLASS_PRIVATE;
      }
      else
      {
	setString(&artwork_new->identifier, "classic");
	artwork_new->sort_priority = ARTWORKCLASS_CLASSICS;
      }

      /* set to new values after changing ".sort_priority" */
      artwork_new->color = ARTWORKCOLOR(artwork_new);

      setString(&artwork_new->class_desc,
		getLevelClassDescription(artwork_new));
    }
    else
    {
      setString(&artwork_new->identifier, artwork_new->subdir);
    }

    setString(&artwork_new->name, artwork_new->identifier);
    setString(&artwork_new->name_sorting, artwork_new->name);
  }

  pushTreeInfo(node_first, artwork_new);

  freeSetupFileHash(setup_file_hash);

  free(directory_path);
  free(filename);

  return TRUE;
}

static void LoadArtworkInfoFromArtworkDir(TreeInfo **node_first,
					  TreeInfo *node_parent,
					  char *base_directory, int type)
{
  Directory *dir;
  DirectoryEntry *dir_entry;
  boolean valid_entry_found = FALSE;

  if ((dir = openDirectory(base_directory)) == NULL)
  {
    /* display error if directory is main "options.graphics_directory" etc. */
    if (base_directory == OPTIONS_ARTWORK_DIRECTORY(type))
      Error(ERR_WARN, "cannot read directory '%s'", base_directory);

    return;
  }

  while ((dir_entry = readDirectory(dir)) != NULL)	/* loop all entries */
  {
    char *directory_name = dir_entry->basename;
    char *directory_path = getPath2(base_directory, directory_name);

    /* skip directory entries for current and parent directory */
    if (strEqual(directory_name, ".") ||
	strEqual(directory_name, ".."))
    {
      free(directory_path);

      continue;
    }

    /* skip directory entries which are not a directory */
    if (!dir_entry->is_directory)			/* not a directory */
    {
      free(directory_path);

      continue;
    }

    free(directory_path);

    /* check if this directory contains artwork with or without config file */
    valid_entry_found |= LoadArtworkInfoFromArtworkConf(node_first, node_parent,
							base_directory,
							directory_name, type);
  }

  closeDirectory(dir);

  /* check if this directory directly contains artwork itself */
  valid_entry_found |= LoadArtworkInfoFromArtworkConf(node_first, node_parent,
						      base_directory, ".",
						      type);
  if (!valid_entry_found)
    Error(ERR_WARN, "cannot find any valid artwork in directory '%s'",
	  base_directory);
}

static TreeInfo *getDummyArtworkInfo(int type)
{
  /* this is only needed when there is completely no artwork available */
  TreeInfo *artwork_new = newTreeInfo();

  setTreeInfoToDefaults(artwork_new, type);

  setString(&artwork_new->subdir,   UNDEFINED_FILENAME);
  setString(&artwork_new->fullpath, UNDEFINED_FILENAME);
  setString(&artwork_new->basepath, UNDEFINED_FILENAME);

  setString(&artwork_new->identifier,   UNDEFINED_FILENAME);
  setString(&artwork_new->name,         UNDEFINED_FILENAME);
  setString(&artwork_new->name_sorting, UNDEFINED_FILENAME);

  return artwork_new;
}

void LoadArtworkInfo()
{
  LoadArtworkInfoCache();

  DrawInitText("Looking for custom artwork", 120, FC_GREEN);

  LoadArtworkInfoFromArtworkDir(&artwork.gfx_first, NULL,
				options.graphics_directory,
				TREE_TYPE_GRAPHICS_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.gfx_first, NULL,
				getUserGraphicsDir(),
				TREE_TYPE_GRAPHICS_DIR);

  LoadArtworkInfoFromArtworkDir(&artwork.snd_first, NULL,
				options.sounds_directory,
				TREE_TYPE_SOUNDS_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.snd_first, NULL,
				getUserSoundsDir(),
				TREE_TYPE_SOUNDS_DIR);

  LoadArtworkInfoFromArtworkDir(&artwork.mus_first, NULL,
				options.music_directory,
				TREE_TYPE_MUSIC_DIR);
  LoadArtworkInfoFromArtworkDir(&artwork.mus_first, NULL,
				getUserMusicDir(),
				TREE_TYPE_MUSIC_DIR);

  if (artwork.gfx_first == NULL)
    artwork.gfx_first = getDummyArtworkInfo(TREE_TYPE_GRAPHICS_DIR);
  if (artwork.snd_first == NULL)
    artwork.snd_first = getDummyArtworkInfo(TREE_TYPE_SOUNDS_DIR);
  if (artwork.mus_first == NULL)
    artwork.mus_first = getDummyArtworkInfo(TREE_TYPE_MUSIC_DIR);

  /* before sorting, the first entries will be from the user directory */
  artwork.gfx_current =
    getTreeInfoFromIdentifier(artwork.gfx_first, setup.graphics_set);
  if (artwork.gfx_current == NULL)
    artwork.gfx_current =
      getTreeInfoFromIdentifier(artwork.gfx_first, GFX_DEFAULT_SUBDIR);
  if (artwork.gfx_current == NULL)
    artwork.gfx_current = getFirstValidTreeInfoEntry(artwork.gfx_first);

  artwork.snd_current =
    getTreeInfoFromIdentifier(artwork.snd_first, setup.sounds_set);
  if (artwork.snd_current == NULL)
    artwork.snd_current =
      getTreeInfoFromIdentifier(artwork.snd_first, SND_DEFAULT_SUBDIR);
  if (artwork.snd_current == NULL)
    artwork.snd_current = getFirstValidTreeInfoEntry(artwork.snd_first);

  artwork.mus_current =
    getTreeInfoFromIdentifier(artwork.mus_first, setup.music_set);
  if (artwork.mus_current == NULL)
    artwork.mus_current =
      getTreeInfoFromIdentifier(artwork.mus_first, MUS_DEFAULT_SUBDIR);
  if (artwork.mus_current == NULL)
    artwork.mus_current = getFirstValidTreeInfoEntry(artwork.mus_first);

  artwork.gfx_current_identifier = artwork.gfx_current->identifier;
  artwork.snd_current_identifier = artwork.snd_current->identifier;
  artwork.mus_current_identifier = artwork.mus_current->identifier;

#if ENABLE_UNUSED_CODE
  printf("graphics set == %s\n\n", artwork.gfx_current_identifier);
  printf("sounds set == %s\n\n", artwork.snd_current_identifier);
  printf("music set == %s\n\n", artwork.mus_current_identifier);
#endif

  sortTreeInfo(&artwork.gfx_first);
  sortTreeInfo(&artwork.snd_first);
  sortTreeInfo(&artwork.mus_first);

#if ENABLE_UNUSED_CODE
  dumpTreeInfo(artwork.gfx_first, 0);
  dumpTreeInfo(artwork.snd_first, 0);
  dumpTreeInfo(artwork.mus_first, 0);
#endif
}

void LoadArtworkInfoFromLevelInfo(ArtworkDirTree **artwork_node,
				  LevelDirTree *level_node)
{
  int type = (*artwork_node)->type;

  /* recursively check all level directories for artwork sub-directories */

  while (level_node)
  {
    /* check all tree entries for artwork, but skip parent link entries */
    if (!level_node->parent_link)
    {
      TreeInfo *artwork_new = getArtworkInfoCacheEntry(level_node, type);
      boolean cached = (artwork_new != NULL);

      if (cached)
      {
	pushTreeInfo(artwork_node, artwork_new);
      }
      else
      {
	TreeInfo *topnode_last = *artwork_node;
	char *path = getPath2(getLevelDirFromTreeInfo(level_node),
			      ARTWORK_DIRECTORY(type));

	LoadArtworkInfoFromArtworkDir(artwork_node, NULL, path, type);

	if (topnode_last != *artwork_node)	/* check for newly added node */
	{
	  artwork_new = *artwork_node;

	  setString(&artwork_new->identifier,   level_node->subdir);
	  setString(&artwork_new->name,         level_node->name);
	  setString(&artwork_new->name_sorting, level_node->name_sorting);

	  artwork_new->sort_priority = level_node->sort_priority;
	  artwork_new->color = LEVELCOLOR(artwork_new);
	}

	free(path);
      }

      /* insert artwork info (from old cache or filesystem) into new cache */
      if (artwork_new != NULL)
	setArtworkInfoCacheEntry(artwork_new, level_node, type);
    }

    DrawInitText(level_node->name, 150, FC_YELLOW);

    if (level_node->node_group != NULL)
      LoadArtworkInfoFromLevelInfo(artwork_node, level_node->node_group);

    level_node = level_node->next;
  }
}

void LoadLevelArtworkInfo()
{
  print_timestamp_init("LoadLevelArtworkInfo");

  DrawInitText("Looking for custom level artwork", 120, FC_GREEN);

  print_timestamp_time("DrawTimeText");

  LoadArtworkInfoFromLevelInfo(&artwork.gfx_first, leveldir_first_all);
  print_timestamp_time("LoadArtworkInfoFromLevelInfo (gfx)");
  LoadArtworkInfoFromLevelInfo(&artwork.snd_first, leveldir_first_all);
  print_timestamp_time("LoadArtworkInfoFromLevelInfo (snd)");
  LoadArtworkInfoFromLevelInfo(&artwork.mus_first, leveldir_first_all);
  print_timestamp_time("LoadArtworkInfoFromLevelInfo (mus)");

  SaveArtworkInfoCache();

  print_timestamp_time("SaveArtworkInfoCache");

  /* needed for reloading level artwork not known at ealier stage */

  if (!strEqual(artwork.gfx_current_identifier, setup.graphics_set))
  {
    artwork.gfx_current =
      getTreeInfoFromIdentifier(artwork.gfx_first, setup.graphics_set);
    if (artwork.gfx_current == NULL)
      artwork.gfx_current =
	getTreeInfoFromIdentifier(artwork.gfx_first, GFX_DEFAULT_SUBDIR);
    if (artwork.gfx_current == NULL)
      artwork.gfx_current = getFirstValidTreeInfoEntry(artwork.gfx_first);
  }

  if (!strEqual(artwork.snd_current_identifier, setup.sounds_set))
  {
    artwork.snd_current =
      getTreeInfoFromIdentifier(artwork.snd_first, setup.sounds_set);
    if (artwork.snd_current == NULL)
      artwork.snd_current =
	getTreeInfoFromIdentifier(artwork.snd_first, SND_DEFAULT_SUBDIR);
    if (artwork.snd_current == NULL)
      artwork.snd_current = getFirstValidTreeInfoEntry(artwork.snd_first);
  }

  if (!strEqual(artwork.mus_current_identifier, setup.music_set))
  {
    artwork.mus_current =
      getTreeInfoFromIdentifier(artwork.mus_first, setup.music_set);
    if (artwork.mus_current == NULL)
      artwork.mus_current =
	getTreeInfoFromIdentifier(artwork.mus_first, MUS_DEFAULT_SUBDIR);
    if (artwork.mus_current == NULL)
      artwork.mus_current = getFirstValidTreeInfoEntry(artwork.mus_first);
  }

  print_timestamp_time("getTreeInfoFromIdentifier");

  sortTreeInfo(&artwork.gfx_first);
  sortTreeInfo(&artwork.snd_first);
  sortTreeInfo(&artwork.mus_first);

  print_timestamp_time("sortTreeInfo");

#if ENABLE_UNUSED_CODE
  dumpTreeInfo(artwork.gfx_first, 0);
  dumpTreeInfo(artwork.snd_first, 0);
  dumpTreeInfo(artwork.mus_first, 0);
#endif

  print_timestamp_done("LoadLevelArtworkInfo");
}

static boolean AddUserLevelSetToLevelInfoExt(char *level_subdir_new)
{
  // get level info tree node of first (original) user level set
  char *level_subdir_old = getLoginName();
  LevelDirTree *leveldir_old = getTreeInfoFromIdentifier(leveldir_first,
							 level_subdir_old);
  if (leveldir_old == NULL)		// should not happen
    return FALSE;

  int draw_deactivation_mask = GetDrawDeactivationMask();

  // override draw deactivation mask (temporarily disable drawing)
  SetDrawDeactivationMask(REDRAW_ALL);

  // load new level set config and add it next to first user level set
  LoadLevelInfoFromLevelConf(&leveldir_old->next, NULL,
			     leveldir_old->basepath, level_subdir_new);

  // set draw deactivation mask to previous value
  SetDrawDeactivationMask(draw_deactivation_mask);

  // get level info tree node of newly added user level set
  LevelDirTree *leveldir_new = getTreeInfoFromIdentifier(leveldir_first,
							 level_subdir_new);
  if (leveldir_new == NULL)		// should not happen
    return FALSE;

  // correct top link and parent node link of newly created tree node
  leveldir_new->node_top    = leveldir_old->node_top;
  leveldir_new->node_parent = leveldir_old->node_parent;

  // sort level info tree to adjust position of newly added level set
  sortTreeInfo(&leveldir_first);

  return TRUE;
}

void AddUserLevelSetToLevelInfo(char *level_subdir_new)
{
  if (!AddUserLevelSetToLevelInfoExt(level_subdir_new))
    Error(ERR_EXIT, "internal level set structure corrupted -- aborting");
}

char *getArtworkIdentifierForUserLevelSet(int type)
{
  char *classic_artwork_set = getClassicArtworkSet(type);

  /* check for custom artwork configured in "levelinfo.conf" */
  char *leveldir_artwork_set =
    *LEVELDIR_ARTWORK_SET_PTR(leveldir_current, type);
  boolean has_leveldir_artwork_set =
    (leveldir_artwork_set != NULL && !strEqual(leveldir_artwork_set,
					       classic_artwork_set));

  /* check for custom artwork in sub-directory "graphics" etc. */
  TreeInfo *artwork_first_node = ARTWORK_FIRST_NODE(artwork, type);
  char *leveldir_identifier = leveldir_current->identifier;
  boolean has_artwork_subdir =
    (getTreeInfoFromIdentifier(artwork_first_node,
			       leveldir_identifier) != NULL);

  return (has_leveldir_artwork_set ? leveldir_artwork_set :
	  has_artwork_subdir       ? leveldir_identifier :
	  classic_artwork_set);
}

TreeInfo *getArtworkTreeInfoForUserLevelSet(int type)
{
  char *artwork_set = getArtworkIdentifierForUserLevelSet(type);
  TreeInfo *artwork_first_node = ARTWORK_FIRST_NODE(artwork, type);

  return getTreeInfoFromIdentifier(artwork_first_node, artwork_set);
}

boolean checkIfCustomArtworkExistsForCurrentLevelSet()
{
  char *graphics_set =
    getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_GRAPHICS);
  char *sounds_set =
    getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_SOUNDS);
  char *music_set =
    getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_MUSIC);

  return (!strEqual(graphics_set, GFX_CLASSIC_SUBDIR) ||
	  !strEqual(sounds_set,   SND_CLASSIC_SUBDIR) ||
	  !strEqual(music_set,    MUS_CLASSIC_SUBDIR));
}

boolean UpdateUserLevelSet(char *level_subdir, char *level_name,
			   char *level_author, int num_levels)
{
  char *filename = getPath2(getUserLevelDir(level_subdir), LEVELINFO_FILENAME);
  char *filename_tmp = getStringCat2(filename, ".tmp");
  FILE *file = NULL;
  FILE *file_tmp = NULL;
  char line[MAX_LINE_LEN];
  boolean success = FALSE;
  LevelDirTree *leveldir = getTreeInfoFromIdentifier(leveldir_first,
						     level_subdir);
  // update values in level directory tree

  if (level_name != NULL)
    setString(&leveldir->name, level_name);

  if (level_author != NULL)
    setString(&leveldir->author, level_author);

  if (num_levels != -1)
    leveldir->levels = num_levels;

  // update values that depend on other values

  setString(&leveldir->name_sorting, leveldir->name);

  leveldir->last_level = leveldir->first_level + leveldir->levels - 1;

  // sort order of level sets may have changed
  sortTreeInfo(&leveldir_first);

  if ((file     = fopen(filename,     MODE_READ)) &&
      (file_tmp = fopen(filename_tmp, MODE_WRITE)))
  {
    while (fgets(line, MAX_LINE_LEN, file))
    {
      if (strPrefix(line, "name:") && level_name != NULL)
	fprintf(file_tmp, "%-32s%s\n", "name:", level_name);
      else if (strPrefix(line, "author:") && level_author != NULL)
	fprintf(file_tmp, "%-32s%s\n", "author:", level_author);
      else if (strPrefix(line, "levels:") && num_levels != -1)
	fprintf(file_tmp, "%-32s%d\n", "levels:", num_levels);
      else
	fputs(line, file_tmp);
    }

    success = TRUE;
  }

  if (file)
    fclose(file);

  if (file_tmp)
    fclose(file_tmp);

  if (success)
    success = (rename(filename_tmp, filename) == 0);

  free(filename);
  free(filename_tmp);

  return success;
}

boolean CreateUserLevelSet(char *level_subdir, char *level_name,
			   char *level_author, int num_levels,
			   boolean use_artwork_set)
{
  LevelDirTree *level_info;
  char *filename;
  FILE *file;
  int i;

  // create user level sub-directory, if needed
  createDirectory(getUserLevelDir(level_subdir), "user level", PERMS_PRIVATE);

  filename = getPath2(getUserLevelDir(level_subdir), LEVELINFO_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write level info file '%s'", filename);
    free(filename);

    return FALSE;
  }

  level_info = newTreeInfo();

  /* always start with reliable default values */
  setTreeInfoToDefaults(level_info, TREE_TYPE_LEVEL_DIR);

  setString(&level_info->name, level_name);
  setString(&level_info->author, level_author);
  level_info->levels = num_levels;
  level_info->first_level = 1;
  level_info->sort_priority = LEVELCLASS_PRIVATE_START;
  level_info->readonly = FALSE;

  if (use_artwork_set)
  {
    level_info->graphics_set =
      getStringCopy(getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_GRAPHICS));
    level_info->sounds_set =
      getStringCopy(getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_SOUNDS));
    level_info->music_set =
      getStringCopy(getArtworkIdentifierForUserLevelSet(ARTWORK_TYPE_MUSIC));
  }

  token_value_position = TOKEN_VALUE_POSITION_SHORT;

  fprintFileHeader(file, LEVELINFO_FILENAME);

  ldi = *level_info;
  for (i = 0; i < NUM_LEVELINFO_TOKENS; i++)
  {
    if (i == LEVELINFO_TOKEN_NAME ||
	i == LEVELINFO_TOKEN_AUTHOR ||
	i == LEVELINFO_TOKEN_LEVELS ||
	i == LEVELINFO_TOKEN_FIRST_LEVEL ||
	i == LEVELINFO_TOKEN_SORT_PRIORITY ||
	i == LEVELINFO_TOKEN_READONLY ||
	(use_artwork_set && (i == LEVELINFO_TOKEN_GRAPHICS_SET ||
			     i == LEVELINFO_TOKEN_SOUNDS_SET ||
			     i == LEVELINFO_TOKEN_MUSIC_SET)))
      fprintf(file, "%s\n", getSetupLine(levelinfo_tokens, "", i));

    /* just to make things nicer :) */
    if (i == LEVELINFO_TOKEN_AUTHOR ||
	i == LEVELINFO_TOKEN_FIRST_LEVEL ||
	(use_artwork_set && i == LEVELINFO_TOKEN_READONLY))
      fprintf(file, "\n");	
  }

  token_value_position = TOKEN_VALUE_POSITION_DEFAULT;

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  freeTreeInfo(level_info);
  free(filename);

  return TRUE;
}

static void SaveUserLevelInfo()
{
  CreateUserLevelSet(getLoginName(), getLoginName(), getRealName(), 100, FALSE);
}

char *getSetupValue(int type, void *value)
{
  static char value_string[MAX_LINE_LEN];

  if (value == NULL)
    return NULL;

  switch (type)
  {
    case TYPE_BOOLEAN:
      strcpy(value_string, (*(boolean *)value ? "true" : "false"));
      break;

    case TYPE_SWITCH:
      strcpy(value_string, (*(boolean *)value ? "on" : "off"));
      break;

    case TYPE_SWITCH3:
      strcpy(value_string, (*(int *)value == AUTO  ? "auto" :
			    *(int *)value == FALSE ? "off" : "on"));
      break;

    case TYPE_YES_NO:
      strcpy(value_string, (*(boolean *)value ? "yes" : "no"));
      break;

    case TYPE_YES_NO_AUTO:
      strcpy(value_string, (*(int *)value == AUTO  ? "auto" :
			    *(int *)value == FALSE ? "no" : "yes"));
      break;

    case TYPE_ECS_AGA:
      strcpy(value_string, (*(boolean *)value ? "AGA" : "ECS"));
      break;

    case TYPE_KEY:
      strcpy(value_string, getKeyNameFromKey(*(Key *)value));
      break;

    case TYPE_KEY_X11:
      strcpy(value_string, getX11KeyNameFromKey(*(Key *)value));
      break;

    case TYPE_INTEGER:
      sprintf(value_string, "%d", *(int *)value);
      break;

    case TYPE_STRING:
      if (*(char **)value == NULL)
	return NULL;

      strcpy(value_string, *(char **)value);
      break;

    default:
      value_string[0] = '\0';
      break;
  }

  if (type & TYPE_GHOSTED)
    strcpy(value_string, "n/a");

  return value_string;
}

char *getSetupLine(struct TokenInfo *token_info, char *prefix, int token_nr)
{
  int i;
  char *line;
  static char token_string[MAX_LINE_LEN];
  int token_type = token_info[token_nr].type;
  void *setup_value = token_info[token_nr].value;
  char *token_text = token_info[token_nr].text;
  char *value_string = getSetupValue(token_type, setup_value);

  /* build complete token string */
  sprintf(token_string, "%s%s", prefix, token_text);

  /* build setup entry line */
  line = getFormattedSetupEntry(token_string, value_string);

  if (token_type == TYPE_KEY_X11)
  {
    Key key = *(Key *)setup_value;
    char *keyname = getKeyNameFromKey(key);

    /* add comment, if useful */
    if (!strEqual(keyname, "(undefined)") &&
	!strEqual(keyname, "(unknown)"))
    {
      /* add at least one whitespace */
      strcat(line, " ");
      for (i = strlen(line); i < token_comment_position; i++)
	strcat(line, " ");

      strcat(line, "# ");
      strcat(line, keyname);
    }
  }

  return line;
}

void LoadLevelSetup_LastSeries()
{
  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup.conf                                            */
  /* ----------------------------------------------------------------------- */

  char *filename = getPath2(getSetupDir(), LEVELSETUP_FILENAME);
  SetupFileHash *level_setup_hash = NULL;

  /* always start with reliable default values */
  leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

  if (!strEqual(DEFAULT_LEVELSET, UNDEFINED_LEVELSET))
  {
    leveldir_current = getTreeInfoFromIdentifier(leveldir_first,
						 DEFAULT_LEVELSET);
    if (leveldir_current == NULL)
      leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);
  }

  if ((level_setup_hash = loadSetupFileHash(filename)))
  {
    char *last_level_series =
      getHashEntry(level_setup_hash, TOKEN_STR_LAST_LEVEL_SERIES);

    leveldir_current = getTreeInfoFromIdentifier(leveldir_first,
						 last_level_series);
    if (leveldir_current == NULL)
      leveldir_current = getFirstValidTreeInfoEntry(leveldir_first);

    freeSetupFileHash(level_setup_hash);
  }
  else
  {
    Error(ERR_DEBUG, "using default setup values");
  }

  free(filename);
}

static void SaveLevelSetup_LastSeries_Ext(boolean deactivate_last_level_series)
{
  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup.conf                                            */
  /* ----------------------------------------------------------------------- */

  // check if the current level directory structure is available at this point
  if (leveldir_current == NULL)
    return;

  char *filename = getPath2(getSetupDir(), LEVELSETUP_FILENAME);
  char *level_subdir = leveldir_current->subdir;
  FILE *file;

  InitUserDataDirectory();

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);

    free(filename);

    return;
  }

  fprintFileHeader(file, LEVELSETUP_FILENAME);

  if (deactivate_last_level_series)
    fprintf(file, "# %s\n# ", "the following level set may have caused a problem and was deactivated");

  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_LAST_LEVEL_SERIES,
					       level_subdir));

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

void SaveLevelSetup_LastSeries()
{
  SaveLevelSetup_LastSeries_Ext(FALSE);
}

void SaveLevelSetup_LastSeries_Deactivate()
{
  SaveLevelSetup_LastSeries_Ext(TRUE);
}

static void checkSeriesInfo()
{
  static char *level_directory = NULL;
  Directory *dir;

  /* check for more levels besides the 'levels' field of 'levelinfo.conf' */

  level_directory = getPath2((leveldir_current->in_user_dir ?
			      getUserLevelDir(NULL) :
			      options.level_directory),
			     leveldir_current->fullpath);

  if ((dir = openDirectory(level_directory)) == NULL)
  {
    Error(ERR_WARN, "cannot read level directory '%s'", level_directory);

    return;
  }

  closeDirectory(dir);
}

void LoadLevelSetup_SeriesInfo()
{
  char *filename;
  SetupFileHash *level_setup_hash = NULL;
  char *level_subdir = leveldir_current->subdir;
  int i;

  /* always start with reliable default values */
  level_nr = leveldir_current->first_level;

  for (i = 0; i < MAX_LEVELS; i++)
  {
    LevelStats_setPlayed(i, 0);
    LevelStats_setSolved(i, 0);
  }

  checkSeriesInfo();

  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup/<level series>/levelsetup.conf                  */
  /* ----------------------------------------------------------------------- */

  level_subdir = leveldir_current->subdir;

  filename = getPath2(getLevelSetupDir(level_subdir), LEVELSETUP_FILENAME);

  if ((level_setup_hash = loadSetupFileHash(filename)))
  {
    char *token_value;

    /* get last played level in this level set */

    token_value = getHashEntry(level_setup_hash, TOKEN_STR_LAST_PLAYED_LEVEL);

    if (token_value)
    {
      level_nr = atoi(token_value);

      if (level_nr < leveldir_current->first_level)
	level_nr = leveldir_current->first_level;
      if (level_nr > leveldir_current->last_level)
	level_nr = leveldir_current->last_level;
    }

    /* get handicap level in this level set */

    token_value = getHashEntry(level_setup_hash, TOKEN_STR_HANDICAP_LEVEL);

    if (token_value)
    {
      int level_nr = atoi(token_value);

      if (level_nr < leveldir_current->first_level)
	level_nr = leveldir_current->first_level;
      if (level_nr > leveldir_current->last_level + 1)
	level_nr = leveldir_current->last_level;

      if (leveldir_current->user_defined || !leveldir_current->handicap)
	level_nr = leveldir_current->last_level;

      leveldir_current->handicap_level = level_nr;
    }

    /* get number of played and solved levels in this level set */

    BEGIN_HASH_ITERATION(level_setup_hash, itr)
    {
      char *token = HASH_ITERATION_TOKEN(itr);
      char *value = HASH_ITERATION_VALUE(itr);

      if (strlen(token) == 3 &&
	  token[0] >= '0' && token[0] <= '9' &&
	  token[1] >= '0' && token[1] <= '9' &&
	  token[2] >= '0' && token[2] <= '9')
      {
	int level_nr = atoi(token);

	if (value != NULL)
	  LevelStats_setPlayed(level_nr, atoi(value));	/* read 1st column */

	value = strchr(value, ' ');

	if (value != NULL)
	  LevelStats_setSolved(level_nr, atoi(value));	/* read 2nd column */
      }
    }
    END_HASH_ITERATION(hash, itr)

    freeSetupFileHash(level_setup_hash);
  }
  else
  {
    Error(ERR_DEBUG, "using default setup values");
  }

  free(filename);
}

void SaveLevelSetup_SeriesInfo()
{
  char *filename;
  char *level_subdir = leveldir_current->subdir;
  char *level_nr_str = int2str(level_nr, 0);
  char *handicap_level_str = int2str(leveldir_current->handicap_level, 0);
  FILE *file;
  int i;

  /* ----------------------------------------------------------------------- */
  /* ~/.<program>/levelsetup/<level series>/levelsetup.conf                  */
  /* ----------------------------------------------------------------------- */

  InitLevelSetupDirectory(level_subdir);

  filename = getPath2(getLevelSetupDir(level_subdir), LEVELSETUP_FILENAME);

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot write setup file '%s'", filename);
    free(filename);
    return;
  }

  fprintFileHeader(file, LEVELSETUP_FILENAME);

  fprintf(file, "%s\n", getFormattedSetupEntry(TOKEN_STR_LAST_PLAYED_LEVEL,
					       level_nr_str));
  fprintf(file, "%s\n\n", getFormattedSetupEntry(TOKEN_STR_HANDICAP_LEVEL,
						 handicap_level_str));

  for (i = leveldir_current->first_level; i <= leveldir_current->last_level;
       i++)
  {
    if (LevelStats_getPlayed(i) > 0 ||
	LevelStats_getSolved(i) > 0)
    {
      char token[16];
      char value[16];

      sprintf(token, "%03d", i);
      sprintf(value, "%d %d", LevelStats_getPlayed(i), LevelStats_getSolved(i));

      fprintf(file, "%s\n", getFormattedSetupEntry(token, value));
    }
  }

  fclose(file);

  SetFilePermissions(filename, PERMS_PRIVATE);

  free(filename);
}

int LevelStats_getPlayed(int nr)
{
  return (nr >= 0 && nr < MAX_LEVELS ? level_stats[nr].played : 0);
}

int LevelStats_getSolved(int nr)
{
  return (nr >= 0 && nr < MAX_LEVELS ? level_stats[nr].solved : 0);
}

void LevelStats_setPlayed(int nr, int value)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].played = value;
}

void LevelStats_setSolved(int nr, int value)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].solved = value;
}

void LevelStats_incPlayed(int nr)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].played++;
}

void LevelStats_incSolved(int nr)
{
  if (nr >= 0 && nr < MAX_LEVELS)
    level_stats[nr].solved++;
}
