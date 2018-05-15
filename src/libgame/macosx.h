// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// macosx.h
// ============================================================================

#ifndef MACOSX_H
#define MACOSX_H


/* define some Mac OS X specific paths */

#define MAC_APP_BINARY_SUBDIR	"Contents/MacOS"
#define MAC_APP_FILES_SUBDIR	"Contents/Resources"


/* some symbols are already defined on Mac OS X */
#define Delay Delay_internal
#define DrawLine DrawLine_internal
#define DrawText DrawText_internal
#define GetPixel GetPixel_internal

#endif /* MACOSX_H */
