// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// config.c
// ============================================================================

#include "libgame/libgame.h"

#include "config.h"
#include "conftime.h"


char *getSourceDateString()
{
  return SOURCE_DATE_STRING;
}

char *getProgramTitleString()
{
  return program.program_title;
}

char *getProgramRealVersionString()
{
  static char program_version_string[32];

  sprintf(program_version_string, "%d.%d.%d.%d%s",
	  PROGRAM_VERSION_MAJOR, PROGRAM_VERSION_MINOR, PROGRAM_VERSION_PATCH,
	  PROGRAM_VERSION_BUILD, PROGRAM_VERSION_EXTRA);

  return program_version_string;
}

char *getProgramVersionString()
{
  return program.version_string;
}

char *getProgramInitString()
{
  static char *program_init_string = NULL;

  if (program_init_string == NULL)
  {
    program_init_string = checked_malloc(strlen(getProgramTitleString()) + 1 +
					 strlen(getProgramVersionString()) + 1);

    sprintf(program_init_string, "%s %s",
	    getProgramTitleString(), getProgramVersionString());
  }

  return program_init_string;
}

char *getConfigProgramTitleString()
{
  TreeInfo *graphics_current =
    getArtworkTreeInfoForUserLevelSet(ARTWORK_TYPE_GRAPHICS);

  return (leveldir_current->program_title ?
	  leveldir_current->program_title :
	  graphics_current->program_title ?
	  graphics_current->program_title :
	  setup.internal.program_title);
}

char *getConfigProgramCopyrightString()
{
  TreeInfo *graphics_current =
    getArtworkTreeInfoForUserLevelSet(ARTWORK_TYPE_GRAPHICS);

  return (leveldir_current->program_copyright ?
	  leveldir_current->program_copyright :
	  graphics_current->program_copyright ?
	  graphics_current->program_copyright :
	  setup.internal.program_copyright);
}

char *getConfigProgramCompanyString()
{
  TreeInfo *graphics_current =
    getArtworkTreeInfoForUserLevelSet(ARTWORK_TYPE_GRAPHICS);

  return (leveldir_current->program_company ?
	  leveldir_current->program_company :
	  graphics_current->program_company ?
	  graphics_current->program_company :
	  setup.internal.program_company);
}

char *getWindowTitleString()
{
  static char *window_title_string = NULL;

  checked_free(window_title_string);

#if defined(TARGET_SDL2)

#ifdef DEBUG
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 20 +
				       strlen(getSourceDateString()) + 2 + 1);

  if (setup.internal.show_scaling_in_title)
    sprintf(window_title_string, "%s (%d %%) [%s]",
	    getProgramInitString(), video.window_scaling_percent,
	    getSourceDateString());
  else
    sprintf(window_title_string, "%s [%s]",
	    getProgramInitString(),
	    getSourceDateString());
#else
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 20);

  if (setup.internal.show_scaling_in_title)
    sprintf(window_title_string, "%s (%d %%)",
	    getProgramInitString(), video.window_scaling_percent);
  else
    sprintf(window_title_string, "%s",
	    getProgramInitString());
#endif

#else

#ifdef DEBUG
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 1 +
				       strlen(getSourceDateString()) + 2 + 1);

  sprintf(window_title_string, "%s [%s]",
	  getProgramInitString(), getSourceDateString());
#else
  window_title_string = checked_malloc(strlen(getProgramInitString()) + 1);

  sprintf(window_title_string, "%s",
	  getProgramInitString());
#endif

#endif

  return window_title_string;
}
