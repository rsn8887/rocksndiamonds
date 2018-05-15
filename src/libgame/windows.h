// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// windows.h
// ============================================================================

#ifndef WINDOWS_H
#define WINDOWS_H

/* prevent including lots of conflicting macros */
#define WIN32_LEAN_AND_MEAN

#include <shlobj.h>


/* some symbols are already defined on Windows */
typedef int boolean_internal;
#define boolean boolean_internal

#define CreateBitmap CreateBitmap_internal
#define GetPixel GetPixel_internal
#define CloseWindow CloseWindow_internal
#define FloodFill FloodFill_internal

#ifdef LoadImage
#undef LoadImage
#define LoadImage LoadImage_internal
#endif

#ifdef PlaySound
#undef PlaySound
#define PlaySound PlaySound_internal
#endif

#ifdef DrawText
#undef DrawText
#define DrawText DrawText_internal
#endif

#endif /* WINDOWS_H */
