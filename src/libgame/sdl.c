// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// sdl.c
// ============================================================================

#include "system.h"
#include "sound.h"
#include "joystick.h"
#include "misc.h"
#include "setup.h"

#define ENABLE_UNUSED_CODE	0	/* currently unused functions */

#define DEBUG_JOYSTICKS		0


/* ========================================================================= */
/* video functions                                                           */
/* ========================================================================= */

/* SDL internal variables */
#if defined(TARGET_SDL2)
static SDL_Window *sdl_window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_Texture *sdl_texture_stream = NULL;
static SDL_Texture *sdl_texture_target = NULL;
static boolean fullscreen_enabled = FALSE;
#endif

static boolean limit_screen_updates = FALSE;


/* functions from SGE library */
void sge_Line(SDL_Surface *, Sint16, Sint16, Sint16, Sint16, Uint32);

#if defined(USE_TOUCH_INPUT_OVERLAY)
/* functions to draw overlay graphics for touch device input */
static void DrawTouchInputOverlay();
#endif

void SDLLimitScreenUpdates(boolean enable)
{
  limit_screen_updates = enable;
}

static void FinalizeScreen(int draw_target)
{
  // copy global animations to render target buffer, if defined (below border)
  if (gfx.draw_global_anim_function != NULL)
    gfx.draw_global_anim_function(draw_target, DRAW_GLOBAL_ANIM_STAGE_1);

  // copy global masked border to render target buffer, if defined
  if (gfx.draw_global_border_function != NULL)
    gfx.draw_global_border_function(draw_target);

  // copy global animations to render target buffer, if defined (above border)
  if (gfx.draw_global_anim_function != NULL)
    gfx.draw_global_anim_function(draw_target, DRAW_GLOBAL_ANIM_STAGE_2);

  // copy tile selection cursor to render target buffer, if defined (above all)
  if (gfx.draw_tile_cursor_function != NULL)
    gfx.draw_tile_cursor_function(draw_target);
}

static void UpdateScreenExt(SDL_Rect *rect, boolean with_frame_delay)
{
  static unsigned int update_screen_delay = 0;
  unsigned int update_screen_delay_value = 50;		/* (milliseconds) */
  SDL_Surface *screen = backbuffer->surface;

  if (limit_screen_updates &&
      !DelayReached(&update_screen_delay, update_screen_delay_value))
    return;

  LimitScreenUpdates(FALSE);

#if 0
  {
    static int LastFrameCounter = 0;
    boolean changed = (FrameCounter != LastFrameCounter);

    printf("::: FrameCounter == %d [%s]\n", FrameCounter,
	   (changed ? "-" : "SAME FRAME UPDATED"));

    LastFrameCounter = FrameCounter;

    /*
    if (FrameCounter % 2)
      return;
    */
  }
#endif

  if (video.screen_rendering_mode == SPECIAL_RENDERING_BITMAP &&
      gfx.final_screen_bitmap != NULL)	// may not be initialized yet
  {
    // draw global animations using bitmaps instead of using textures
    // to prevent texture scaling artefacts (this is potentially slower)

    BlitBitmap(backbuffer, gfx.final_screen_bitmap, 0, 0,
	       gfx.win_xsize, gfx.win_ysize, 0, 0);

    FinalizeScreen(DRAW_TO_SCREEN);

    screen = gfx.final_screen_bitmap->surface;

    // force full window redraw
    rect = NULL;
  }

#if defined(TARGET_SDL2)
  SDL_Texture *sdl_texture = sdl_texture_stream;

  // deactivate use of target texture if render targets are not supported
  if ((video.screen_rendering_mode == SPECIAL_RENDERING_TARGET ||
       video.screen_rendering_mode == SPECIAL_RENDERING_DOUBLE) &&
      sdl_texture_target == NULL)
    video.screen_rendering_mode = SPECIAL_RENDERING_OFF;

  if (video.screen_rendering_mode == SPECIAL_RENDERING_TARGET)
    sdl_texture = sdl_texture_target;

  if (rect)
  {
    int bytes_x = screen->pitch / video.width;
    int bytes_y = screen->pitch;

    SDL_UpdateTexture(sdl_texture, rect,
		      screen->pixels + rect->x * bytes_x + rect->y * bytes_y,
		      screen->pitch);
  }
  else
  {
    SDL_UpdateTexture(sdl_texture, NULL, screen->pixels, screen->pitch);
  }

  int xoff = video.screen_xoffset;
  int yoff = video.screen_yoffset;
  SDL_Rect dst_rect_screen = { xoff, yoff, video.width, video.height };
  SDL_Rect *src_rect1 = NULL, *dst_rect1 = NULL;
  SDL_Rect *src_rect2 = NULL, *dst_rect2 = NULL;

  if (video.screen_rendering_mode == SPECIAL_RENDERING_TARGET ||
      video.screen_rendering_mode == SPECIAL_RENDERING_DOUBLE)
    dst_rect2 = &dst_rect_screen;
  else
    dst_rect1 = &dst_rect_screen;

#if defined(HAS_SCREEN_KEYBOARD)
  if (video.shifted_up || video.shifted_up_delay)
  {
    int time_current = SDL_GetTicks();
    int pos = video.shifted_up_pos;
    int pos_last = video.shifted_up_pos_last;

    if (!DelayReachedExt(&video.shifted_up_delay, video.shifted_up_delay_value,
			 time_current))
    {
      int delay = time_current - video.shifted_up_delay;
      int delay_value = video.shifted_up_delay_value;

      pos = pos_last + (pos - pos_last) * delay / delay_value;
    }
    else
    {
      video.shifted_up_pos_last = pos;
      video.shifted_up_delay = 0;
    }

    SDL_Rect src_rect_up = { 0,    pos,  video.width, video.height - pos };
    SDL_Rect dst_rect_up = { xoff, yoff, video.width, video.height - pos };

    if (video.screen_rendering_mode == SPECIAL_RENDERING_TARGET ||
	video.screen_rendering_mode == SPECIAL_RENDERING_DOUBLE)
    {
      src_rect2 = &src_rect_up;
      dst_rect2 = &dst_rect_up;
    }
    else
    {
      src_rect1 = &src_rect_up;
      dst_rect1 = &dst_rect_up;
    }
  }
#endif

  // clear render target buffer
  SDL_RenderClear(sdl_renderer);

  // set renderer to use target texture for rendering
  if (video.screen_rendering_mode == SPECIAL_RENDERING_TARGET ||
      video.screen_rendering_mode == SPECIAL_RENDERING_DOUBLE)
    SDL_SetRenderTarget(sdl_renderer, sdl_texture_target);

  // copy backbuffer texture to render target buffer
  if (video.screen_rendering_mode != SPECIAL_RENDERING_TARGET)
    SDL_RenderCopy(sdl_renderer, sdl_texture_stream, src_rect1, dst_rect1);

  if (video.screen_rendering_mode != SPECIAL_RENDERING_BITMAP)
    FinalizeScreen(DRAW_TO_SCREEN);

  // when using target texture, copy it to screen buffer
  if (video.screen_rendering_mode == SPECIAL_RENDERING_TARGET ||
      video.screen_rendering_mode == SPECIAL_RENDERING_DOUBLE)
  {
    SDL_SetRenderTarget(sdl_renderer, NULL);
    SDL_RenderCopy(sdl_renderer, sdl_texture_target, src_rect2, dst_rect2);
  }

#if defined(USE_TOUCH_INPUT_OVERLAY)
  // draw overlay graphics for touch device input, if needed
  DrawTouchInputOverlay();
#endif

#endif

  // global synchronization point of the game to align video frame delay
  if (with_frame_delay)
    WaitUntilDelayReached(&video.frame_delay, video.frame_delay_value);

#if defined(TARGET_SDL2)
 // show render target buffer on screen
  SDL_RenderPresent(sdl_renderer);
#else	// TARGET_SDL
  if (rect)
    SDL_UpdateRects(screen, 1, rect);
  else
    SDL_UpdateRect(screen, 0, 0, 0, 0);
#endif
}

static void UpdateScreen_WithFrameDelay(SDL_Rect *rect)
{
  UpdateScreenExt(rect, TRUE);
}

static void UpdateScreen_WithoutFrameDelay(SDL_Rect *rect)
{
  UpdateScreenExt(rect, FALSE);
}

static void SDLSetWindowIcon(char *basename)
{
  /* (setting the window icon on Mac OS X would replace the high-quality
     dock icon with the currently smaller (and uglier) icon from file) */

#if !defined(PLATFORM_MACOSX)
  char *filename = getCustomImageFilename(basename);
  SDL_Surface *surface;

  if (filename == NULL)
  {
    Error(ERR_WARN, "SDLSetWindowIcon(): cannot find file '%s'", basename);

    return;
  }

  if ((surface = IMG_Load(filename)) == NULL)
  {
    Error(ERR_WARN, "IMG_Load() failed: %s", SDL_GetError());

    return;
  }

  /* set transparent color */
  SDL_SetColorKey(surface, SET_TRANSPARENT_PIXEL,
		  SDL_MapRGB(surface->format, 0x00, 0x00, 0x00));

#if defined(TARGET_SDL2)
  SDL_SetWindowIcon(sdl_window, surface);
#else
  SDL_WM_SetIcon(surface, NULL);
#endif
#endif
}

#if defined(TARGET_SDL2)

static boolean equalSDLPixelFormat(SDL_PixelFormat *format1,
				   SDL_PixelFormat *format2)
{
  return (format1->format	 == format2->format &&
	  format1->BitsPerPixel	 == format2->BitsPerPixel &&
	  format1->BytesPerPixel == format2->BytesPerPixel &&
	  format1->Rmask	 == format2->Rmask &&
	  format1->Gmask	 == format2->Gmask &&
	  format1->Bmask	 == format2->Bmask);
}

static Pixel SDLGetColorKey(SDL_Surface *surface)
{
  Pixel color_key;

  if (SDL_GetColorKey(surface, &color_key) != 0)
    return -1;

  return color_key;
}

static boolean SDLHasColorKey(SDL_Surface *surface)
{
  return (SDLGetColorKey(surface) != -1);
}

static boolean SDLHasAlpha(SDL_Surface *surface)
{
  SDL_BlendMode blend_mode;

  if (SDL_GetSurfaceBlendMode(surface, &blend_mode) != 0)
    return FALSE;

  return (blend_mode == SDL_BLENDMODE_BLEND);
}

static void SDLSetAlpha(SDL_Surface *surface, boolean set, int alpha)
{
  SDL_BlendMode blend_mode = (set ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);

  SDL_SetSurfaceBlendMode(surface, blend_mode);
  SDL_SetSurfaceAlphaMod(surface, alpha);
}

SDL_Surface *SDLGetNativeSurface(SDL_Surface *surface)
{
  SDL_PixelFormat format;
  SDL_Surface *new_surface;

  if (surface == NULL)
    return NULL;

  if (backbuffer && backbuffer->surface)
  {
    format = *backbuffer->surface->format;
    format.Amask = surface->format->Amask;	// keep alpha channel
  }
  else
  {
    format = *surface->format;
  }

  new_surface = SDL_ConvertSurface(surface, &format, 0);

  if (new_surface == NULL)
    Error(ERR_EXIT, "SDL_ConvertSurface() failed: %s", SDL_GetError());

  return new_surface;
}

boolean SDLSetNativeSurface(SDL_Surface **surface)
{
  SDL_Surface *new_surface;

  if (surface == NULL ||
      *surface == NULL ||
      backbuffer == NULL ||
      backbuffer->surface == NULL)
    return FALSE;

  // if pixel format already optimized for destination surface, do nothing
  if (equalSDLPixelFormat((*surface)->format, backbuffer->surface->format))
    return FALSE;

  new_surface = SDLGetNativeSurface(*surface);

  SDL_FreeSurface(*surface);

  *surface = new_surface;

  return TRUE;
}

#else

static Pixel SDLGetColorKey(SDL_Surface *surface)
{
  if ((surface->flags & SDL_SRCCOLORKEY) == 0)
    return -1;

  return surface->format->colorkey;
}

static boolean SDLHasColorKey(SDL_Surface *surface)
{
  return (SDLGetColorKey(surface) != -1);
}

static boolean SDLHasAlpha(SDL_Surface *surface)
{
  return ((surface->flags & SDL_SRCALPHA) != 0);
}

static void SDLSetAlpha(SDL_Surface *surface, boolean set, int alpha)
{
  SDL_SetAlpha(surface, (set ? SDL_SRCALPHA : 0), alpha);
}

SDL_Surface *SDLGetNativeSurface(SDL_Surface *surface)
{
  SDL_Surface *new_surface;

  if (surface == NULL)
    return NULL;

  if (!video.initialized)
    new_surface = SDL_ConvertSurface(surface, surface->format, SURFACE_FLAGS);
  else if (SDLHasAlpha(surface))
    new_surface = SDL_DisplayFormatAlpha(surface);
  else
    new_surface = SDL_DisplayFormat(surface);

  if (new_surface == NULL)
    Error(ERR_EXIT, "%s() failed: %s",
	  (video.initialized ? "SDL_DisplayFormat" : "SDL_ConvertSurface"),
	  SDL_GetError());

  return new_surface;
}

boolean SDLSetNativeSurface(SDL_Surface **surface)
{
  SDL_Surface *new_surface;

  if (surface == NULL ||
      *surface == NULL ||
      !video.initialized)
    return FALSE;

  new_surface = SDLGetNativeSurface(*surface);

  SDL_FreeSurface(*surface);

  *surface = new_surface;

  return TRUE;
}

#endif

#if defined(TARGET_SDL2)
static SDL_Texture *SDLCreateTextureFromSurface(SDL_Surface *surface)
{
  if (program.headless)
    return NULL;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);

  if (texture == NULL)
    Error(ERR_EXIT, "SDL_CreateTextureFromSurface() failed: %s",
	  SDL_GetError());

  return texture;
}
#endif

void SDLCreateBitmapTextures(Bitmap *bitmap)
{
#if defined(TARGET_SDL2)
  if (bitmap == NULL)
    return;

  if (bitmap->texture)
    SDL_DestroyTexture(bitmap->texture);
  if (bitmap->texture_masked)
    SDL_DestroyTexture(bitmap->texture_masked);

  bitmap->texture        = SDLCreateTextureFromSurface(bitmap->surface);
  bitmap->texture_masked = SDLCreateTextureFromSurface(bitmap->surface_masked);
#endif
}

void SDLFreeBitmapTextures(Bitmap *bitmap)
{
#if defined(TARGET_SDL2)
  if (bitmap == NULL)
    return;

  if (bitmap->texture)
    SDL_DestroyTexture(bitmap->texture);
  if (bitmap->texture_masked)
    SDL_DestroyTexture(bitmap->texture_masked);

  bitmap->texture = NULL;
  bitmap->texture_masked = NULL;
#endif
}

void SDLInitVideoDisplay(void)
{
#if !defined(TARGET_SDL2)
  if (!strEqual(setup.system.sdl_videodriver, ARG_DEFAULT))
    SDL_putenv(getStringCat2("SDL_VIDEODRIVER=", setup.system.sdl_videodriver));

  SDL_putenv("SDL_VIDEO_CENTERED=1");
#endif

  /* initialize SDL video */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
    Error(ERR_EXIT, "SDL_InitSubSystem() failed: %s", SDL_GetError());

  /* set default SDL depth */
#if !defined(TARGET_SDL2)
  video.default_depth = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
#else
  video.default_depth = 32;	// (how to determine video depth in SDL2?)
#endif
}

inline static void SDLInitVideoBuffer_VideoBuffer(boolean fullscreen)
{
  if (program.headless)
    return;

  video.window_scaling_percent = setup.window_scaling_percent;
  video.window_scaling_quality = setup.window_scaling_quality;

  SDLSetScreenRenderingMode(setup.screen_rendering_mode);

#if defined(TARGET_SDL2)
  // SDL 2.0: support for (desktop) fullscreen mode available
  video.fullscreen_available = TRUE;
#else
  // SDL 1.2: no support for fullscreen mode in R'n'D anymore
  video.fullscreen_available = FALSE;
#endif

  /* open SDL video output device (window or fullscreen mode) */
  if (!SDLSetVideoMode(fullscreen))
    Error(ERR_EXIT, "setting video mode failed");

  /* !!! SDL2 can only set the window icon if the window already exists !!! */
  /* set window icon */
  SDLSetWindowIcon(program.icon_filename);

  /* set window and icon title */
  SDLSetWindowTitle();
}

inline static void SDLInitVideoBuffer_DrawBuffer()
{
  /* SDL cannot directly draw to the visible video framebuffer like X11,
     but always uses a backbuffer, which is then blitted to the visible
     video framebuffer with 'SDL_UpdateRect' (or replaced with the current
     visible video framebuffer with 'SDL_Flip', if the hardware supports
     this). Therefore do not use an additional backbuffer for drawing, but
     use a symbolic buffer (distinguishable from the SDL backbuffer) called
     'window', which indicates that the SDL backbuffer should be updated to
     the visible video framebuffer when attempting to blit to it.

     For convenience, it seems to be a good idea to create this symbolic
     buffer 'window' at the same size as the SDL backbuffer. Although it
     should never be drawn to directly, it would do no harm nevertheless. */

  /* create additional (symbolic) buffer for double-buffering */
  ReCreateBitmap(&window, video.width, video.height);

  /* create dummy drawing buffer for headless mode, if needed */
  if (program.headless)
    ReCreateBitmap(&backbuffer, video.width, video.height);
}

void SDLInitVideoBuffer(boolean fullscreen)
{
  SDLInitVideoBuffer_VideoBuffer(fullscreen);
  SDLInitVideoBuffer_DrawBuffer();
}

static boolean SDLCreateScreen(boolean fullscreen)
{
  SDL_Surface *new_surface = NULL;

#if defined(TARGET_SDL2)
  int surface_flags_window     = SURFACE_FLAGS | SDL_WINDOW_RESIZABLE;
  int surface_flags_fullscreen = SURFACE_FLAGS | SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
  int surface_flags_window     = SURFACE_FLAGS;
  int surface_flags_fullscreen = SURFACE_FLAGS;	// (no fullscreen in SDL 1.2)
#endif

#if defined(TARGET_SDL2)
#if 1
  int renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
#else
  /* If SDL_CreateRenderer() is called from within a VirtualBox Windows VM
     _without_ enabling 2D/3D acceleration and/or guest additions installed,
     it will crash if flags are *not* set to SDL_RENDERER_SOFTWARE (because
     it will try to use accelerated graphics and apparently fails miserably) */
  int renderer_flags = SDL_RENDERER_SOFTWARE;
#endif

  SDLSetScreenSizeAndOffsets(video.width, video.height);
#endif

  int width  = video.width;
  int height = video.height;
  int screen_width  = video.screen_width;
  int screen_height = video.screen_height;
  int surface_flags = (fullscreen ? surface_flags_fullscreen :
		       surface_flags_window);

  // default window size is unscaled
  video.window_width  = screen_width;
  video.window_height = screen_height;

#if defined(TARGET_SDL2)

  // store if initial screen mode is fullscreen mode when changing screen size
  video.fullscreen_initial = fullscreen;

  float window_scaling_factor = (float)setup.window_scaling_percent / 100;

  video.window_width  = window_scaling_factor * screen_width;
  video.window_height = window_scaling_factor * screen_height;

  if (sdl_texture_stream)
  {
    SDL_DestroyTexture(sdl_texture_stream);
    sdl_texture_stream = NULL;
  }

  if (sdl_texture_target)
  {
    SDL_DestroyTexture(sdl_texture_target);
    sdl_texture_target = NULL;
  }

  if (!(fullscreen && fullscreen_enabled))
  {
    if (sdl_renderer)
    {
      SDL_DestroyRenderer(sdl_renderer);
      sdl_renderer = NULL;
    }

    if (sdl_window)
    {
      SDL_DestroyWindow(sdl_window);
      sdl_window = NULL;
    }
  }

  if (sdl_window == NULL)
    sdl_window = SDL_CreateWindow(program.window_title,
				  SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  video.window_width,
				  video.window_height,
				  surface_flags);

  if (sdl_window != NULL)
  {
    if (sdl_renderer == NULL)
      sdl_renderer = SDL_CreateRenderer(sdl_window, -1, renderer_flags);

    if (sdl_renderer != NULL)
    {
      SDL_RenderSetLogicalSize(sdl_renderer, screen_width, screen_height);
      // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
      SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, setup.window_scaling_quality);

      sdl_texture_stream = SDL_CreateTexture(sdl_renderer,
					     SDL_PIXELFORMAT_ARGB8888,
					     SDL_TEXTUREACCESS_STREAMING,
					     width, height);

      if (SDL_RenderTargetSupported(sdl_renderer))
	sdl_texture_target = SDL_CreateTexture(sdl_renderer,
					       SDL_PIXELFORMAT_ARGB8888,
					       SDL_TEXTUREACCESS_TARGET,
					       width, height);

      if (sdl_texture_stream != NULL)
      {
	// use SDL default values for RGB masks and no alpha channel
	new_surface = SDL_CreateRGBSurface(0, width, height, 32, 0,0,0, 0);

	if (new_surface == NULL)
	  Error(ERR_WARN, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());
      }
      else
      {
	Error(ERR_WARN, "SDL_CreateTexture() failed: %s", SDL_GetError());
      }
    }
    else
    {
      Error(ERR_WARN, "SDL_CreateRenderer() failed: %s", SDL_GetError());
    }
  }
  else
  {
    Error(ERR_WARN, "SDL_CreateWindow() failed: %s", SDL_GetError());
  }

#else	// TARGET_SDL

  if (gfx.final_screen_bitmap == NULL)
    gfx.final_screen_bitmap = CreateBitmapStruct();

  gfx.final_screen_bitmap->width = width;
  gfx.final_screen_bitmap->height = height;

  gfx.final_screen_bitmap->surface =
    SDL_SetVideoMode(width, height, video.depth, surface_flags);

  if (gfx.final_screen_bitmap->surface != NULL)
  {
    new_surface =
      SDL_CreateRGBSurface(surface_flags, width, height, video.depth, 0,0,0, 0);

    if (new_surface == NULL)
      Error(ERR_WARN, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

#if 0
    new_surface = gfx.final_screen_bitmap->surface;
    gfx.final_screen_bitmap = NULL;
#endif

  }
  else
  {
    Error(ERR_WARN, "SDL_SetVideoMode() failed: %s", SDL_GetError());
  }
#endif

#if defined(TARGET_SDL2)
  // store fullscreen state ("video.fullscreen_enabled" may not reflect this!)
  if (new_surface != NULL)
    fullscreen_enabled = fullscreen;
#endif

  if (backbuffer == NULL)
    backbuffer = CreateBitmapStruct();

  backbuffer->width  = video.width;
  backbuffer->height = video.height;

  if (backbuffer->surface)
    SDL_FreeSurface(backbuffer->surface);

  backbuffer->surface = new_surface;

  return (new_surface != NULL);
}

boolean SDLSetVideoMode(boolean fullscreen)
{
  boolean success = FALSE;

  SetWindowTitle();

  if (fullscreen && !video.fullscreen_enabled && video.fullscreen_available)
  {
    /* switch display to fullscreen mode, if available */
    success = SDLCreateScreen(TRUE);

    if (!success)
    {
      /* switching display to fullscreen mode failed -- do not try it again */
      video.fullscreen_available = FALSE;
    }
    else
    {
      video.fullscreen_enabled = TRUE;
    }
  }

  if ((!fullscreen && video.fullscreen_enabled) || !success)
  {
    /* switch display to window mode */
    success = SDLCreateScreen(FALSE);

    if (!success)
    {
      /* switching display to window mode failed -- should not happen */
    }
    else
    {
      video.fullscreen_enabled = FALSE;
      video.window_scaling_percent = setup.window_scaling_percent;
      video.window_scaling_quality = setup.window_scaling_quality;

      SDLSetScreenRenderingMode(setup.screen_rendering_mode);
    }
  }

#if defined(TARGET_SDL2)
  SDLRedrawWindow();			// map window
#endif

#ifdef DEBUG
#if defined(PLATFORM_WIN32)
  // experimental drag and drop code

  SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

  {
    SDL_SysWMinfo wminfo;
    HWND hwnd;
    boolean wminfo_success = FALSE;

    SDL_VERSION(&wminfo.version);
#if defined(TARGET_SDL2)
    if (sdl_window)
      wminfo_success = SDL_GetWindowWMInfo(sdl_window, &wminfo);
#else
    wminfo_success = (SDL_GetWMInfo(&wminfo) == 1);
#endif

    if (wminfo_success)
    {
#if defined(TARGET_SDL2)
      hwnd = wminfo.info.win.window;
#else
      hwnd = wminfo.window;
#endif

      DragAcceptFiles(hwnd, TRUE);
    }
  }
#endif
#endif

  return success;
}

void SDLSetWindowTitle()
{
#if defined(TARGET_SDL2)
  if (sdl_window == NULL)
    return;

  SDL_SetWindowTitle(sdl_window, program.window_title);
#else
  SDL_WM_SetCaption(program.window_title, program.window_title);
#endif
}

#if defined(TARGET_SDL2)
void SDLSetWindowScaling(int window_scaling_percent)
{
  if (sdl_window == NULL)
    return;

  float window_scaling_factor = (float)window_scaling_percent / 100;
  int new_window_width  = (int)(window_scaling_factor * video.screen_width);
  int new_window_height = (int)(window_scaling_factor * video.screen_height);

  SDL_SetWindowSize(sdl_window, new_window_width, new_window_height);

  video.window_scaling_percent = window_scaling_percent;
  video.window_width  = new_window_width;
  video.window_height = new_window_height;

  SetWindowTitle();
}

void SDLSetWindowScalingQuality(char *window_scaling_quality)
{
  SDL_Texture *new_texture;

  if (sdl_texture_stream == NULL)
    return;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, window_scaling_quality);

  new_texture = SDL_CreateTexture(sdl_renderer,
				  SDL_PIXELFORMAT_ARGB8888,
				  SDL_TEXTUREACCESS_STREAMING,
				  video.width, video.height);

  if (new_texture != NULL)
  {
    SDL_DestroyTexture(sdl_texture_stream);

    sdl_texture_stream = new_texture;
  }

  if (SDL_RenderTargetSupported(sdl_renderer))
    new_texture = SDL_CreateTexture(sdl_renderer,
				    SDL_PIXELFORMAT_ARGB8888,
				    SDL_TEXTUREACCESS_TARGET,
				    video.width, video.height);
  else
    new_texture = NULL;

  if (new_texture != NULL)
  {
    SDL_DestroyTexture(sdl_texture_target);

    sdl_texture_target = new_texture;
  }

  SDLRedrawWindow();

  video.window_scaling_quality = window_scaling_quality;
}

void SDLSetWindowFullscreen(boolean fullscreen)
{
  if (sdl_window == NULL)
    return;

  int flags = (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

  if (SDL_SetWindowFullscreen(sdl_window, flags) == 0)
    video.fullscreen_enabled = fullscreen_enabled = fullscreen;

  // if screen size was changed in fullscreen mode, correct desktop window size
  if (!fullscreen && video.fullscreen_initial)
  {
    SDLSetWindowScaling(setup.window_scaling_percent);
    SDL_SetWindowPosition(sdl_window,
			  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    video.fullscreen_initial = FALSE;
  }
}

void SDLSetDisplaySize()
{
  SDL_Rect display_bounds;

  SDL_GetDisplayBounds(0, &display_bounds);

  video.display_width  = display_bounds.w;
  video.display_height = display_bounds.h;

#if 0
  Error(ERR_DEBUG, "SDL real screen size: %d x %d",
	video.display_width, video.display_height);
#endif
}

void SDLSetScreenSizeAndOffsets(int width, int height)
{
  // set default video screen size and offsets
  video.screen_width = width;
  video.screen_height = height;
  video.screen_xoffset = 0;
  video.screen_yoffset = 0;

#if defined(USE_COMPLETE_DISPLAY)
  float ratio_video   = (float) width / height;
  float ratio_display = (float) video.display_width / video.display_height;

  if (ratio_video != ratio_display)
  {
    // adjust drawable screen size to cover the whole device display

    if (ratio_video < ratio_display)
      video.screen_width  *= ratio_display / ratio_video;
    else
      video.screen_height *= ratio_video / ratio_display;

    video.screen_xoffset = (video.screen_width  - width)  / 2;
    video.screen_yoffset = (video.screen_height - height) / 2;

#if 0
    Error(ERR_DEBUG, "Changing screen from %dx%d to %dx%d (%.2f to %.2f)",
	  width, height,
	  video.screen_width, video.screen_height,
	  ratio_video, ratio_display);
#endif
  }
#endif
}

void SDLSetScreenSizeForRenderer(int width, int height)
{
  SDL_RenderSetLogicalSize(sdl_renderer, width, height);
}

void SDLSetScreenProperties()
{
  SDLSetScreenSizeAndOffsets(video.width, video.height);
  SDLSetScreenSizeForRenderer(video.screen_width, video.screen_height);
}

#endif

void SDLSetScreenRenderingMode(char *screen_rendering_mode)
{
#if defined(TARGET_SDL2)
  video.screen_rendering_mode =
    (strEqual(screen_rendering_mode, STR_SPECIAL_RENDERING_BITMAP) ?
     SPECIAL_RENDERING_BITMAP :
     strEqual(screen_rendering_mode, STR_SPECIAL_RENDERING_TARGET) ?
     SPECIAL_RENDERING_TARGET:
     strEqual(screen_rendering_mode, STR_SPECIAL_RENDERING_DOUBLE) ?
     SPECIAL_RENDERING_DOUBLE : SPECIAL_RENDERING_OFF);
#else
  video.screen_rendering_mode = SPECIAL_RENDERING_BITMAP;
#endif
}

void SDLRedrawWindow()
{
  UpdateScreen_WithoutFrameDelay(NULL);
}

void SDLCreateBitmapContent(Bitmap *bitmap, int width, int height,
			    int depth)
{
  if (program.headless)
    return;

  SDL_Surface *surface =
    SDL_CreateRGBSurface(SURFACE_FLAGS, width, height, depth, 0,0,0, 0);

  if (surface == NULL)
    Error(ERR_EXIT, "SDL_CreateRGBSurface() failed: %s", SDL_GetError());

  SDLSetNativeSurface(&surface);

  bitmap->surface = surface;
}

void SDLFreeBitmapPointers(Bitmap *bitmap)
{
  if (bitmap->surface)
    SDL_FreeSurface(bitmap->surface);
  if (bitmap->surface_masked)
    SDL_FreeSurface(bitmap->surface_masked);

  bitmap->surface = NULL;
  bitmap->surface_masked = NULL;

#if defined(TARGET_SDL2)
  if (bitmap->texture)
    SDL_DestroyTexture(bitmap->texture);
  if (bitmap->texture_masked)
    SDL_DestroyTexture(bitmap->texture_masked);

  bitmap->texture = NULL;
  bitmap->texture_masked = NULL;
#endif
}

void SDLCopyArea(Bitmap *src_bitmap, Bitmap *dst_bitmap,
		 int src_x, int src_y, int width, int height,
		 int dst_x, int dst_y, int mask_mode)
{
  Bitmap *real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect src_rect, dst_rect;

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.w = width;
  dst_rect.h = height;

  // if (src_bitmap != backbuffer || dst_bitmap != window)
  if (!(src_bitmap == backbuffer && dst_bitmap == window))
    SDL_BlitSurface((mask_mode == BLIT_MASKED ?
		     src_bitmap->surface_masked : src_bitmap->surface),
		    &src_rect, real_dst_bitmap->surface, &dst_rect);

  if (dst_bitmap == window)
    UpdateScreen_WithFrameDelay(&dst_rect);
}

void SDLBlitTexture(Bitmap *bitmap,
		    int src_x, int src_y, int width, int height,
		    int dst_x, int dst_y, int mask_mode)
{
#if defined(TARGET_SDL2)
  SDL_Texture *texture;
  SDL_Rect src_rect;
  SDL_Rect dst_rect;

  texture =
    (mask_mode == BLIT_MASKED ? bitmap->texture_masked : bitmap->texture);

  if (texture == NULL)
    return;

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.w = width;
  dst_rect.h = height;

  SDL_RenderCopy(sdl_renderer, texture, &src_rect, &dst_rect);
#endif
}

void SDLFillRectangle(Bitmap *dst_bitmap, int x, int y, int width, int height,
		      Uint32 color)
{
  Bitmap *real_dst_bitmap = (dst_bitmap == window ? backbuffer : dst_bitmap);
  SDL_Rect rect;

  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;

  SDL_FillRect(real_dst_bitmap->surface, &rect, color);

  if (dst_bitmap == window)
    UpdateScreen_WithFrameDelay(&rect);
}

void PrepareFadeBitmap(int draw_target)
{
  Bitmap *fade_bitmap =
    (draw_target == DRAW_TO_FADE_SOURCE ? gfx.fade_bitmap_source :
     draw_target == DRAW_TO_FADE_TARGET ? gfx.fade_bitmap_target : NULL);

  if (fade_bitmap == NULL)
    return;

  // copy backbuffer to fading buffer
  BlitBitmap(backbuffer, fade_bitmap, 0, 0, gfx.win_xsize, gfx.win_ysize, 0, 0);

  // add border and animations to fading buffer
  FinalizeScreen(draw_target);
}

void SDLFadeRectangle(int x, int y, int width, int height,
		      int fade_mode, int fade_delay, int post_delay,
		      void (*draw_border_function)(void))
{
  SDL_Surface *surface_backup = gfx.fade_bitmap_backup->surface;
  SDL_Surface *surface_source = gfx.fade_bitmap_source->surface;
  SDL_Surface *surface_target = gfx.fade_bitmap_target->surface;
  SDL_Surface *surface_black  = gfx.fade_bitmap_black->surface;
  SDL_Surface *surface_screen = backbuffer->surface;
  SDL_Rect src_rect, dst_rect;
  SDL_Rect dst_rect2;
  int src_x = x, src_y = y;
  int dst_x = x, dst_y = y;
  unsigned int time_last, time_current;

  // store function for drawing global masked border
  void (*draw_global_border_function)(int) = gfx.draw_global_border_function;

  // deactivate drawing of global border while fading, if needed
  if (draw_border_function == NULL)
    gfx.draw_global_border_function = NULL;

  src_rect.x = src_x;
  src_rect.y = src_y;
  src_rect.w = width;
  src_rect.h = height;

  dst_rect.x = dst_x;
  dst_rect.y = dst_y;
  dst_rect.w = width;		/* (ignored) */
  dst_rect.h = height;		/* (ignored) */

  dst_rect2 = dst_rect;

  // before fading in, store backbuffer (without animation graphics)
  if (fade_mode & (FADE_TYPE_FADE_IN | FADE_TYPE_TRANSFORM))
    SDL_BlitSurface(surface_screen, &dst_rect, surface_backup, &src_rect);

  /* copy source and target surfaces to temporary surfaces for fading */
  if (fade_mode & FADE_TYPE_TRANSFORM)
  {
    // (source and target fading buffer already prepared)
  }
  else if (fade_mode & FADE_TYPE_FADE_IN)
  {
    // (target fading buffer already prepared)
    SDL_BlitSurface(surface_black,  &src_rect, surface_source, &src_rect);
  }
  else		/* FADE_TYPE_FADE_OUT */
  {
    // (source fading buffer already prepared)
    SDL_BlitSurface(surface_black,  &src_rect, surface_target, &src_rect);
  }

  time_current = SDL_GetTicks();

  if (fade_mode == FADE_MODE_MELT)
  {
    boolean done = FALSE;
    int melt_pixels = 2;
    int melt_columns = width / melt_pixels;
    int ypos[melt_columns];
    int max_steps = height / 8 + 32;
    int steps_done = 0;
    float steps = 0;
    int i;

    SDL_BlitSurface(surface_source, &src_rect, surface_screen, &dst_rect);

    SDLSetAlpha(surface_target, FALSE, 0);	/* disable alpha blending */

    ypos[0] = -GetSimpleRandom(16);

    for (i = 1 ; i < melt_columns; i++)
    {
      int r = GetSimpleRandom(3) - 1;	/* randomly choose from { -1, 0, -1 } */

      ypos[i] = ypos[i - 1] + r;

      if (ypos[i] > 0)
        ypos[i] = 0;
      else
        if (ypos[i] == -16)
          ypos[i] = -15;
    }

    while (!done)
    {
      int steps_final;

      time_last = time_current;
      time_current = SDL_GetTicks();
      steps += max_steps * ((float)(time_current - time_last) / fade_delay);
      steps_final = MIN(MAX(0, steps), max_steps);

      steps_done++;

      done = (steps_done >= steps_final);

      for (i = 0 ; i < melt_columns; i++)
      {
	if (ypos[i] < 0)
        {
          ypos[i]++;

          done = FALSE;
        }
	else if (ypos[i] < height)
	{
	  int y1 = 16;
	  int y2 = 8;
	  int y3 = 8;
	  int dy = (ypos[i] < y1) ? ypos[i] + 1 : y2 + GetSimpleRandom(y3);

	  if (ypos[i] + dy >= height)
	    dy = height - ypos[i];

	  /* copy part of (appearing) target surface to upper area */
	  src_rect.x = src_x + i * melt_pixels;
	  // src_rect.y = src_y + ypos[i];
	  src_rect.y = src_y;
	  src_rect.w = melt_pixels;
	  // src_rect.h = dy;
	  src_rect.h = ypos[i] + dy;

	  dst_rect.x = dst_x + i * melt_pixels;
	  // dst_rect.y = dst_y + ypos[i];
	  dst_rect.y = dst_y;

	  if (steps_done >= steps_final)
	    SDL_BlitSurface(surface_target, &src_rect,
			    surface_screen, &dst_rect);

	  ypos[i] += dy;

	  /* copy part of (disappearing) source surface to lower area */
	  src_rect.x = src_x + i * melt_pixels;
	  src_rect.y = src_y;
	  src_rect.w = melt_pixels;
	  src_rect.h = height - ypos[i];

	  dst_rect.x = dst_x + i * melt_pixels;
	  dst_rect.y = dst_y + ypos[i];

	  if (steps_done >= steps_final)
	    SDL_BlitSurface(surface_source, &src_rect,
			    surface_screen, &dst_rect);

	  done = FALSE;
	}
	else
	{
	  src_rect.x = src_x + i * melt_pixels;
	  src_rect.y = src_y;
	  src_rect.w = melt_pixels;
	  src_rect.h = height;

	  dst_rect.x = dst_x + i * melt_pixels;
	  dst_rect.y = dst_y;

	  if (steps_done >= steps_final)
	    SDL_BlitSurface(surface_target, &src_rect,
			    surface_screen, &dst_rect);
	}
      }

      if (steps_done >= steps_final)
      {
	if (draw_border_function != NULL)
	  draw_border_function();

	UpdateScreen_WithFrameDelay(&dst_rect2);
      }
    }
  }
  else if (fade_mode == FADE_MODE_CURTAIN)
  {
    float xx;
    int xx_final;
    int xx_size = width / 2;

    SDL_BlitSurface(surface_target, &src_rect, surface_screen, &dst_rect);

    SDLSetAlpha(surface_source, FALSE, 0);	/* disable alpha blending */

    for (xx = 0; xx < xx_size;)
    {
      time_last = time_current;
      time_current = SDL_GetTicks();
      xx += xx_size * ((float)(time_current - time_last) / fade_delay);
      xx_final = MIN(MAX(0, xx), xx_size);

      src_rect.x = src_x;
      src_rect.y = src_y;
      src_rect.w = width;
      src_rect.h = height;

      dst_rect.x = dst_x;
      dst_rect.y = dst_y;

      /* draw new (target) image to screen buffer */
      SDL_BlitSurface(surface_target, &src_rect, surface_screen, &dst_rect);

      if (xx_final < xx_size)
      {
	src_rect.w = xx_size - xx_final;
	src_rect.h = height;

	/* draw old (source) image to screen buffer (left side) */

	src_rect.x = src_x + xx_final;
	dst_rect.x = dst_x;

	SDL_BlitSurface(surface_source, &src_rect, surface_screen, &dst_rect);

	/* draw old (source) image to screen buffer (right side) */

	src_rect.x = src_x + xx_size;
	dst_rect.x = dst_x + xx_size + xx_final;

	SDL_BlitSurface(surface_source, &src_rect, surface_screen, &dst_rect);
      }

      if (draw_border_function != NULL)
	draw_border_function();

      /* only update the region of the screen that is affected from fading */
      UpdateScreen_WithFrameDelay(&dst_rect2);
    }
  }
  else		/* fading in, fading out or cross-fading */
  {
    float alpha;
    int alpha_final;

    for (alpha = 0.0; alpha < 255.0;)
    {
      time_last = time_current;
      time_current = SDL_GetTicks();
      alpha += 255 * ((float)(time_current - time_last) / fade_delay);
      alpha_final = MIN(MAX(0, alpha), 255);

      /* draw existing (source) image to screen buffer */
      SDL_BlitSurface(surface_source, &src_rect, surface_screen, &dst_rect);

      /* draw new (target) image to screen buffer using alpha blending */
      SDLSetAlpha(surface_target, TRUE, alpha_final);
      SDL_BlitSurface(surface_target, &src_rect, surface_screen, &dst_rect);

      if (draw_border_function != NULL)
	draw_border_function();

      /* only update the region of the screen that is affected from fading */
      UpdateScreen_WithFrameDelay(&dst_rect);
    }
  }

  if (post_delay > 0)
  {
    unsigned int time_post_delay;

    time_current = SDL_GetTicks();
    time_post_delay = time_current + post_delay;

    while (time_current < time_post_delay)
    {
      // updating the screen contains waiting for frame delay (non-busy)
      UpdateScreen_WithFrameDelay(NULL);

      time_current = SDL_GetTicks();
    }
  }

  // restore function for drawing global masked border
  gfx.draw_global_border_function = draw_global_border_function;

  // after fading in, restore backbuffer (without animation graphics)
  if (fade_mode & (FADE_TYPE_FADE_IN | FADE_TYPE_TRANSFORM))
    SDL_BlitSurface(surface_backup, &dst_rect, surface_screen, &src_rect);
}

void SDLDrawSimpleLine(Bitmap *dst_bitmap, int from_x, int from_y,
		       int to_x, int to_y, Uint32 color)
{
  SDL_Surface *surface = dst_bitmap->surface;
  SDL_Rect rect;

  if (from_x > to_x)
    swap_numbers(&from_x, &to_x);

  if (from_y > to_y)
    swap_numbers(&from_y, &to_y);

  rect.x = from_x;
  rect.y = from_y;
  rect.w = (to_x - from_x + 1);
  rect.h = (to_y - from_y + 1);

  SDL_FillRect(surface, &rect, color);
}

void SDLDrawLine(Bitmap *dst_bitmap, int from_x, int from_y,
		 int to_x, int to_y, Uint32 color)
{
  sge_Line(dst_bitmap->surface, from_x, from_y, to_x, to_y, color);
}

#if ENABLE_UNUSED_CODE
void SDLDrawLines(SDL_Surface *surface, struct XY *points,
		  int num_points, Uint32 color)
{
  int i, x, y;
  int line_width = 4;

  for (i = 0; i < num_points - 1; i++)
  {
    for (x = 0; x < line_width; x++)
    {
      for (y = 0; y < line_width; y++)
      {
	int dx = x - line_width / 2;
	int dy = y - line_width / 2;

	if ((x == 0 && y == 0) ||
	    (x == 0 && y == line_width - 1) ||
	    (x == line_width - 1 && y == 0) ||
	    (x == line_width - 1 && y == line_width - 1))
	  continue;

	sge_Line(surface, points[i].x + dx, points[i].y + dy,
		 points[i+1].x + dx, points[i+1].y + dy, color);
      }
    }
  }
}
#endif

Pixel SDLGetPixel(Bitmap *src_bitmap, int x, int y)
{
  SDL_Surface *surface = src_bitmap->surface;

  switch (surface->format->BytesPerPixel)
  {
    case 1:		/* assuming 8-bpp */
    {
      return *((Uint8 *)surface->pixels + y * surface->pitch + x);
    }
    break;

    case 2:		/* probably 15-bpp or 16-bpp */
    {
      return *((Uint16 *)surface->pixels + y * surface->pitch / 2 + x);
    }
    break;

  case 3:		/* slow 24-bpp mode; usually not used */
    {
      /* does this work? */
      Uint8 *pix = (Uint8 *)surface->pixels + y * surface->pitch + x * 3;
      Uint32 color = 0;
      int shift;

      shift = surface->format->Rshift;
      color |= *(pix + shift / 8) >> shift;
      shift = surface->format->Gshift;
      color |= *(pix + shift / 8) >> shift;
      shift = surface->format->Bshift;
      color |= *(pix + shift / 8) >> shift;

      return color;
    }
    break;

  case 4:		/* probably 32-bpp */
    {
      return *((Uint32 *)surface->pixels + y * surface->pitch / 4 + x);
    }
    break;
  }

  return 0;
}


/* ========================================================================= */
/* The following functions were taken from the SGE library                   */
/* (SDL Graphics Extension Library) by Anders Lindström                      */
/* http://www.etek.chalmers.se/~e8cal1/sge/index.html                        */
/* ========================================================================= */

void _PutPixel(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  if (x >= 0 && x <= surface->w - 1 && y >= 0 && y <= surface->h - 1)
  {
    switch (surface->format->BytesPerPixel)
    {
      case 1:
      {
	/* Assuming 8-bpp */
	*((Uint8 *)surface->pixels + y*surface->pitch + x) = color;
      }
      break;

      case 2:
      {
	/* Probably 15-bpp or 16-bpp */
	*((Uint16 *)surface->pixels + y*surface->pitch/2 + x) = color;
      }
      break;

      case 3:
      {
	/* Slow 24-bpp mode, usually not used */
	Uint8 *pix;
	int shift;

	/* Gack - slow, but endian correct */
	pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3;
	shift = surface->format->Rshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Gshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Bshift;
	*(pix+shift/8) = color>>shift;
      }
      break;

      case 4:
      {
	/* Probably 32-bpp */
	*((Uint32 *)surface->pixels + y*surface->pitch/4 + x) = color;
      }
      break;
    }
  }
}

void _PutPixelRGB(SDL_Surface *surface, Sint16 x, Sint16 y,
		  Uint8 R, Uint8 G, Uint8 B)
{
  _PutPixel(surface, x, y, SDL_MapRGB(surface->format, R, G, B));
}

void _PutPixel8(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  *((Uint8 *)surface->pixels + y*surface->pitch + x) = color;
}

void _PutPixel16(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  *((Uint16 *)surface->pixels + y*surface->pitch/2 + x) = color;
}

void _PutPixel24(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  Uint8 *pix;
  int shift;

  /* Gack - slow, but endian correct */
  pix = (Uint8 *)surface->pixels + y * surface->pitch + x*3;
  shift = surface->format->Rshift;
  *(pix+shift/8) = color>>shift;
  shift = surface->format->Gshift;
  *(pix+shift/8) = color>>shift;
  shift = surface->format->Bshift;
  *(pix+shift/8) = color>>shift;
}

void _PutPixel32(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  *((Uint32 *)surface->pixels + y*surface->pitch/4 + x) = color;
}

void _PutPixelX(SDL_Surface *dest,Sint16 x,Sint16 y,Uint32 color)
{
  switch (dest->format->BytesPerPixel)
  {
    case 1:
      *((Uint8 *)dest->pixels + y*dest->pitch + x) = color;
      break;

    case 2:
      *((Uint16 *)dest->pixels + y*dest->pitch/2 + x) = color;
      break;

    case 3:
      _PutPixel24(dest,x,y,color);
      break;

    case 4:
      *((Uint32 *)dest->pixels + y*dest->pitch/4 + x) = color;
      break;
  }
}

void sge_PutPixel(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color)
{
  if (SDL_MUSTLOCK(surface))
  {
    if (SDL_LockSurface(surface) < 0)
    {
      return;
    }
  }

  _PutPixel(surface, x, y, color);

  if (SDL_MUSTLOCK(surface))
  {
    SDL_UnlockSurface(surface);
  }
}

void sge_PutPixelRGB(SDL_Surface *surface, Sint16 x, Sint16 y,
		  Uint8 r, Uint8 g, Uint8 b)
{
  sge_PutPixel(surface, x, y, SDL_MapRGB(surface->format, r, g, b));
}

Sint32 sge_CalcYPitch(SDL_Surface *dest, Sint16 y)
{
  if (y >= 0 && y <= dest->h - 1)
  {
    switch (dest->format->BytesPerPixel)
    {
      case 1:
	return y*dest->pitch;
	break;

      case 2:
	return y*dest->pitch/2;
	break;

      case 3:
	return y*dest->pitch;
	break;

      case 4:
	return y*dest->pitch/4;
	break;
    }
  }

  return -1;
}

void sge_pPutPixel(SDL_Surface *surface, Sint16 x, Sint32 ypitch, Uint32 color)
{
  if (x >= 0 && x <= surface->w - 1 && ypitch >= 0)
  {
    switch (surface->format->BytesPerPixel)
    {
      case 1:
      {
	/* Assuming 8-bpp */
	*((Uint8 *)surface->pixels + ypitch + x) = color;
      }
      break;

      case 2:
      {
	/* Probably 15-bpp or 16-bpp */
	*((Uint16 *)surface->pixels + ypitch + x) = color;
      }
      break;

      case 3:
      {
	/* Slow 24-bpp mode, usually not used */
	Uint8 *pix;
	int shift;

	/* Gack - slow, but endian correct */
	pix = (Uint8 *)surface->pixels + ypitch + x*3;
	shift = surface->format->Rshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Gshift;
	*(pix+shift/8) = color>>shift;
	shift = surface->format->Bshift;
	*(pix+shift/8) = color>>shift;
      }
      break;

      case 4:
      {
	/* Probably 32-bpp */
	*((Uint32 *)surface->pixels + ypitch + x) = color;
      }
      break;
    }
  }
}

void sge_HLine(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y,
	       Uint32 Color)
{
  SDL_Rect l;

  if (SDL_MUSTLOCK(Surface))
  {
    if (SDL_LockSurface(Surface) < 0)
    {
      return;
    }
  }

  if (x1 > x2)
  {
    Sint16 tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  /* Do the clipping */
  if (y < 0 || y > Surface->h - 1 || x1 > Surface->w - 1 || x2 < 0)
    return;
  if (x1 < 0)
    x1 = 0;
  if (x2 > Surface->w - 1)
    x2 = Surface->w - 1;

  l.x = x1;
  l.y = y;
  l.w = x2 - x1 + 1;
  l.h = 1;

  SDL_FillRect(Surface, &l, Color);

  if (SDL_MUSTLOCK(Surface))
  {
    SDL_UnlockSurface(Surface);
  }
}

void sge_HLineRGB(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y,
		  Uint8 R, Uint8 G, Uint8 B)
{
  sge_HLine(Surface, x1, x2, y, SDL_MapRGB(Surface->format, R, G, B));
}

void _HLine(SDL_Surface *Surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 Color)
{
  SDL_Rect l;

  if (x1 > x2)
  {
    Sint16 tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  /* Do the clipping */
  if (y < 0 || y > Surface->h - 1 || x1 > Surface->w - 1 || x2 < 0)
    return;
  if (x1 < 0)
    x1 = 0;
  if (x2 > Surface->w - 1)
    x2 = Surface->w - 1;

  l.x = x1;
  l.y = y;
  l.w = x2 - x1 + 1;
  l.h = 1;

  SDL_FillRect(Surface, &l, Color);
}

void sge_VLine(SDL_Surface *Surface, Sint16 x, Sint16 y1, Sint16 y2,
	       Uint32 Color)
{
  SDL_Rect l;

  if (SDL_MUSTLOCK(Surface))
  {
    if (SDL_LockSurface(Surface) < 0)
    {
      return;
    }
  }

  if (y1 > y2)
  {
    Sint16 tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  /* Do the clipping */
  if (x < 0 || x > Surface->w - 1 || y1 > Surface->h - 1 || y2 < 0)
    return;
  if (y1 < 0)
    y1 = 0;
  if (y2 > Surface->h - 1)
    y2 = Surface->h - 1;

  l.x = x;
  l.y = y1;
  l.w = 1;
  l.h = y2 - y1 + 1;

  SDL_FillRect(Surface, &l, Color);

  if (SDL_MUSTLOCK(Surface))
  {
    SDL_UnlockSurface(Surface);
  }
}

void sge_VLineRGB(SDL_Surface *Surface, Sint16 x, Sint16 y1, Sint16 y2,
		  Uint8 R, Uint8 G, Uint8 B)
{
  sge_VLine(Surface, x, y1, y2, SDL_MapRGB(Surface->format, R, G, B));
}

void _VLine(SDL_Surface *Surface, Sint16 x, Sint16 y1, Sint16 y2, Uint32 Color)
{
  SDL_Rect l;

  if (y1 > y2)
  {
    Sint16 tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  /* Do the clipping */
  if (x < 0 || x > Surface->w - 1 || y1 > Surface->h - 1 || y2 < 0)
    return;
  if (y1 < 0)
    y1 = 0;
  if (y2 > Surface->h - 1)
    y2 = Surface->h - 1;

  l.x = x;
  l.y = y1;
  l.w = 1;
  l.h = y2 - y1 + 1;

  SDL_FillRect(Surface, &l, Color);
}

void sge_DoLine(SDL_Surface *Surface, Sint16 x1, Sint16 y1,
		Sint16 x2, Sint16 y2, Uint32 Color,
		void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y,
			      Uint32 Color))
{
  Sint16 dx, dy, sdx, sdy, x, y, px, py;

  dx = x2 - x1;
  dy = y2 - y1;

  sdx = (dx < 0) ? -1 : 1;
  sdy = (dy < 0) ? -1 : 1;

  dx = sdx * dx + 1;
  dy = sdy * dy + 1;

  x = y = 0;

  px = x1;
  py = y1;

  if (dx >= dy)
  {
    for (x = 0; x < dx; x++)
    {
      Callback(Surface, px, py, Color);

      y += dy;
      if (y >= dx)
      {
	y -= dx;
	py += sdy;
      }

      px += sdx;
    }
  }
  else
  {
    for (y = 0; y < dy; y++)
    {
      Callback(Surface, px, py, Color);

      x += dx;
      if (x >= dy)
      {
	x -= dy;
	px += sdx;
      }

      py += sdy;
    }
  }
}

void sge_DoLineRGB(SDL_Surface *Surface, Sint16 X1, Sint16 Y1,
		   Sint16 X2, Sint16 Y2, Uint8 R, Uint8 G, Uint8 B,
		   void Callback(SDL_Surface *Surf, Sint16 X, Sint16 Y,
				 Uint32 Color))
{
  sge_DoLine(Surface, X1, Y1, X2, Y2,
	     SDL_MapRGB(Surface->format, R, G, B), Callback);
}

void sge_Line(SDL_Surface *Surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2,
	      Uint32 Color)
{
  if (SDL_MUSTLOCK(Surface))
  {
    if (SDL_LockSurface(Surface) < 0)
      return;
   }

   /* Draw the line */
   sge_DoLine(Surface, x1, y1, x2, y2, Color, _PutPixel);

   /* unlock the display */
   if (SDL_MUSTLOCK(Surface))
   {
      SDL_UnlockSurface(Surface);
   }
}

void sge_LineRGB(SDL_Surface *Surface, Sint16 x1, Sint16 y1, Sint16 x2,
		 Sint16 y2, Uint8 R, Uint8 G, Uint8 B)
{
  sge_Line(Surface, x1, y1, x2, y2, SDL_MapRGB(Surface->format, R, G, B));
}

void SDLPutPixel(Bitmap *dst_bitmap, int x, int y, Pixel pixel)
{
  sge_PutPixel(dst_bitmap->surface, x, y, pixel);
}


/*
  -----------------------------------------------------------------------------
  quick (no, it's slow) and dirty hack to "invert" rectangle inside SDL surface
  -----------------------------------------------------------------------------
*/

void SDLInvertArea(Bitmap *bitmap, int src_x, int src_y,
		   int width, int height, Uint32 color)
{
  int x, y;

  for (y = src_y; y < src_y + height; y++)
  {
    for (x = src_x; x < src_x + width; x++)
    {
      Uint32 pixel = SDLGetPixel(bitmap, x, y);

      SDLPutPixel(bitmap, x, y, pixel == BLACK_PIXEL ? color : BLACK_PIXEL);
    }
  }
}

void SDLCopyInverseMasked(Bitmap *src_bitmap, Bitmap *dst_bitmap,
			  int src_x, int src_y, int width, int height,
			  int dst_x, int dst_y)
{
  int x, y;

  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      Uint32 pixel = SDLGetPixel(src_bitmap, src_x + x, src_y + y);

      if (pixel != BLACK_PIXEL)
	SDLPutPixel(dst_bitmap, dst_x + x, dst_y + y, BLACK_PIXEL);
    }
  }
}


/* ========================================================================= */
/* The following functions were taken from the SDL_gfx library version 2.0.3 */
/* (Rotozoomer) by Andreas Schiffler                                         */
/* http://www.ferzkopp.net/Software/SDL_gfx-2.0/index.html                   */
/* ========================================================================= */

/*
  -----------------------------------------------------------------------------
  32 bit zoomer

  zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.
  -----------------------------------------------------------------------------
*/

typedef struct
{
  Uint8 r;
  Uint8 g;
  Uint8 b;
  Uint8 a;
} tColorRGBA;

int zoomSurfaceRGBA_scaleDownBy2(SDL_Surface *src, SDL_Surface *dst)
{
  int x, y;
  tColorRGBA *sp, *csp, *dp;
  int dgap;

  /* pointer setup */
  sp = csp = (tColorRGBA *) src->pixels;
  dp = (tColorRGBA *) dst->pixels;
  dgap = dst->pitch - dst->w * 4;

  for (y = 0; y < dst->h; y++)
  {
    sp = csp;

    for (x = 0; x < dst->w; x++)
    {
      tColorRGBA *sp0 = sp;
      tColorRGBA *sp1 = (tColorRGBA *) ((Uint8 *) sp + src->pitch);
      tColorRGBA *sp00 = &sp0[0];
      tColorRGBA *sp01 = &sp0[1];
      tColorRGBA *sp10 = &sp1[0];
      tColorRGBA *sp11 = &sp1[1];
      tColorRGBA new;

      /* create new color pixel from all four source color pixels */
      new.r = (sp00->r + sp01->r + sp10->r + sp11->r) / 4;
      new.g = (sp00->g + sp01->g + sp10->g + sp11->g) / 4;
      new.b = (sp00->b + sp01->b + sp10->b + sp11->b) / 4;
      new.a = (sp00->a + sp01->a + sp10->a + sp11->a) / 4;

      /* draw */
      *dp = new;

      /* advance source pointers */
      sp += 2;

      /* advance destination pointer */
      dp++;
    }

    /* advance source pointer */
    csp = (tColorRGBA *) ((Uint8 *) csp + 2 * src->pitch);

    /* advance destination pointers */
    dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
  }

  return 0;
}

int zoomSurfaceRGBA(SDL_Surface *src, SDL_Surface *dst)
{
  int x, y, *sax, *say, *csax, *csay;
  float sx, sy;
  tColorRGBA *sp, *csp, *csp0, *dp;
  int dgap;

  /* use specialized zoom function when scaling down to exactly half size */
  if (src->w == 2 * dst->w &&
      src->h == 2 * dst->h)
    return zoomSurfaceRGBA_scaleDownBy2(src, dst);

  /* variable setup */
  sx = (float) src->w / (float) dst->w;
  sy = (float) src->h / (float) dst->h;

  /* allocate memory for row increments */
  csax = sax = (int *)checked_malloc((dst->w + 1) * sizeof(Uint32));
  csay = say = (int *)checked_malloc((dst->h + 1) * sizeof(Uint32));

  /* precalculate row increments */
  for (x = 0; x <= dst->w; x++)
    *csax++ = (int)(sx * x);

  for (y = 0; y <= dst->h; y++)
    *csay++ = (int)(sy * y);

  /* pointer setup */
  sp = csp = csp0 = (tColorRGBA *) src->pixels;
  dp = (tColorRGBA *) dst->pixels;
  dgap = dst->pitch - dst->w * 4;

  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    sp = csp;
    csax = sax;

    for (x = 0; x < dst->w; x++)
    {
      /* draw */
      *dp = *sp;

      /* advance source pointers */
      csax++;
      sp = csp + *csax;

      /* advance destination pointer */
      dp++;
    }

    /* advance source pointer */
    csay++;
    csp = (tColorRGBA *) ((Uint8 *) csp0 + *csay * src->pitch);

    /* advance destination pointers */
    dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
  }

  free(sax);
  free(say);

  return 0;
}

/*
  -----------------------------------------------------------------------------
  8 bit zoomer

  zoomes 8 bit palette/Y 'src' surface to 'dst' surface
  -----------------------------------------------------------------------------
*/

int zoomSurfaceY(SDL_Surface * src, SDL_Surface * dst)
{
  Uint32 x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy;
  Uint8 *sp, *dp, *csp;
  int dgap;

  /* variable setup */
  sx = (Uint32) (65536.0 * (float) src->w / (float) dst->w);
  sy = (Uint32) (65536.0 * (float) src->h / (float) dst->h);

  /* allocate memory for row increments */
  sax = (Uint32 *)checked_malloc(dst->w * sizeof(Uint32));
  say = (Uint32 *)checked_malloc(dst->h * sizeof(Uint32));

  /* precalculate row increments */
  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++)
  {
    csx += sx;
    *csax = (csx >> 16);
    csx &= 0xffff;
    csax++;
  }

  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    csy += sy;
    *csay = (csy >> 16);
    csy &= 0xffff;
    csay++;
  }

  csx = 0;
  csax = sax;
  for (x = 0; x < dst->w; x++)
  {
    csx += (*csax);
    csax++;
  }

  csy = 0;
  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    csy += (*csay);
    csay++;
  }

  /* pointer setup */
  sp = csp = (Uint8 *) src->pixels;
  dp = (Uint8 *) dst->pixels;
  dgap = dst->pitch - dst->w;

  /* draw */
  csay = say;
  for (y = 0; y < dst->h; y++)
  {
    csax = sax;
    sp = csp;
    for (x = 0; x < dst->w; x++)
    {
      /* draw */
      *dp = *sp;

      /* advance source pointers */
      sp += (*csax);
      csax++;

      /* advance destination pointer */
      dp++;
    }

    /* advance source pointer (for row) */
    csp += ((*csay) * src->pitch);
    csay++;

    /* advance destination pointers */
    dp += dgap;
  }

  free(sax);
  free(say);

  return 0;
}

/*
  -----------------------------------------------------------------------------
  zoomSurface()

  Zooms a 32bit or 8bit 'src' surface to newly created 'dst' surface.
  'zoomx' and 'zoomy' are scaling factors for width and height.
  If the surface is not 8bit or 32bit RGBA/ABGR it will be converted
  into a 32bit RGBA format on the fly.
  -----------------------------------------------------------------------------
*/

SDL_Surface *zoomSurface(SDL_Surface *src, int dst_width, int dst_height)
{
  SDL_Surface *zoom_src = NULL;
  SDL_Surface *zoom_dst = NULL;
  boolean is_converted = FALSE;
  boolean is_32bit;
  int i;

  if (src == NULL)
    return NULL;

  /* determine if source surface is 32 bit or 8 bit */
  is_32bit = (src->format->BitsPerPixel == 32);

  if (is_32bit || src->format->BitsPerPixel == 8)
  {
    /* use source surface 'as is' */
    zoom_src = src;
  }
  else
  {
    /* new source surface is 32 bit with a defined RGB ordering */
    zoom_src = SDL_CreateRGBSurface(SURFACE_FLAGS, src->w, src->h, 32,
				    0x000000ff, 0x0000ff00, 0x00ff0000,
				    (src->format->Amask ? 0xff000000 : 0));
    SDL_BlitSurface(src, NULL, zoom_src, NULL);
    is_32bit = TRUE;
    is_converted = TRUE;
  }

  /* allocate surface to completely contain the zoomed surface */
  if (is_32bit)
  {
    /* target surface is 32 bit with source RGBA/ABGR ordering */
    zoom_dst = SDL_CreateRGBSurface(SURFACE_FLAGS, dst_width, dst_height, 32,
				    zoom_src->format->Rmask,
				    zoom_src->format->Gmask,
				    zoom_src->format->Bmask,
				    zoom_src->format->Amask);
  }
  else
  {
    /* target surface is 8 bit */
    zoom_dst = SDL_CreateRGBSurface(SURFACE_FLAGS, dst_width, dst_height, 8,
				    0, 0, 0, 0);
  }

  /* lock source surface */
  SDL_LockSurface(zoom_src);

  /* check which kind of surface we have */
  if (is_32bit)
  {
    /* call the 32 bit transformation routine to do the zooming */
    zoomSurfaceRGBA(zoom_src, zoom_dst);
  }
  else
  {
    /* copy palette */
    for (i = 0; i < zoom_src->format->palette->ncolors; i++)
      zoom_dst->format->palette->colors[i] =
	zoom_src->format->palette->colors[i];
    zoom_dst->format->palette->ncolors = zoom_src->format->palette->ncolors;

    /* call the 8 bit transformation routine to do the zooming */
    zoomSurfaceY(zoom_src, zoom_dst);
  }

  /* unlock source surface */
  SDL_UnlockSurface(zoom_src);

  /* free temporary surface */
  if (is_converted)
    SDL_FreeSurface(zoom_src);

  /* return destination surface */
  return zoom_dst;
}

static SDL_Surface *SDLGetOpaqueSurface(SDL_Surface *surface)
{
  SDL_Surface *new_surface;

  if (surface == NULL)
    return NULL;

  if ((new_surface = SDLGetNativeSurface(surface)) == NULL)
    Error(ERR_EXIT, "SDLGetNativeSurface() failed");

  /* remove alpha channel from native non-transparent surface, if defined */
  SDLSetAlpha(new_surface, FALSE, 0);

  /* remove transparent color from native non-transparent surface, if defined */
  SDL_SetColorKey(new_surface, UNSET_TRANSPARENT_PIXEL, 0);

  return new_surface;
}

Bitmap *SDLZoomBitmap(Bitmap *src_bitmap, int dst_width, int dst_height)
{
  Bitmap *dst_bitmap = CreateBitmapStruct();
  SDL_Surface *src_surface = src_bitmap->surface_masked;
  SDL_Surface *dst_surface;

  dst_width  = MAX(1, dst_width);	/* prevent zero bitmap width */
  dst_height = MAX(1, dst_height);	/* prevent zero bitmap height */

  dst_bitmap->width  = dst_width;
  dst_bitmap->height = dst_height;

  /* create zoomed temporary surface from source surface */
  dst_surface = zoomSurface(src_surface, dst_width, dst_height);

  /* create native format destination surface from zoomed temporary surface */
  SDLSetNativeSurface(&dst_surface);

  /* set color key for zoomed surface from source surface, if defined */
  if (SDLHasColorKey(src_surface))
    SDL_SetColorKey(dst_surface, SET_TRANSPARENT_PIXEL,
		    SDLGetColorKey(src_surface));

  /* create native non-transparent surface for opaque blitting */
  dst_bitmap->surface = SDLGetOpaqueSurface(dst_surface);

  /* set native transparent surface for masked blitting */
  dst_bitmap->surface_masked = dst_surface;

  return dst_bitmap;
}


/* ========================================================================= */
/* load image to bitmap                                                      */
/* ========================================================================= */

Bitmap *SDLLoadImage(char *filename)
{
  Bitmap *new_bitmap = CreateBitmapStruct();
  SDL_Surface *sdl_image_tmp;

  if (program.headless)
  {
    /* prevent sanity check warnings at later stage */
    new_bitmap->width = new_bitmap->height = 1;

    return new_bitmap;
  }

  print_timestamp_init("SDLLoadImage");

  print_timestamp_time(getBaseNamePtr(filename));

  /* load image to temporary surface */
  if ((sdl_image_tmp = IMG_Load(filename)) == NULL)
    Error(ERR_EXIT, "IMG_Load() failed: %s", SDL_GetError());

  print_timestamp_time("IMG_Load");

  UPDATE_BUSY_STATE();

  /* create native non-transparent surface for current image */
  if ((new_bitmap->surface = SDLGetOpaqueSurface(sdl_image_tmp)) == NULL)
    Error(ERR_EXIT, "SDLGetOpaqueSurface() failed");

  print_timestamp_time("SDLGetNativeSurface (opaque)");

  UPDATE_BUSY_STATE();

  /* set black pixel to transparent if no alpha channel / transparent color */
  if (!SDLHasAlpha(sdl_image_tmp) &&
      !SDLHasColorKey(sdl_image_tmp))
    SDL_SetColorKey(sdl_image_tmp, SET_TRANSPARENT_PIXEL,
		    SDL_MapRGB(sdl_image_tmp->format, 0x00, 0x00, 0x00));

  /* create native transparent surface for current image */
  if ((new_bitmap->surface_masked = SDLGetNativeSurface(sdl_image_tmp)) == NULL)
    Error(ERR_EXIT, "SDLGetNativeSurface() failed");

  print_timestamp_time("SDLGetNativeSurface (masked)");

  UPDATE_BUSY_STATE();

  /* free temporary surface */
  SDL_FreeSurface(sdl_image_tmp);

  new_bitmap->width = new_bitmap->surface->w;
  new_bitmap->height = new_bitmap->surface->h;

  print_timestamp_done("SDLLoadImage");

  return new_bitmap;
}


/* ------------------------------------------------------------------------- */
/* custom cursor fuctions                                                    */
/* ------------------------------------------------------------------------- */

static SDL_Cursor *create_cursor(struct MouseCursorInfo *cursor_info)
{
  return SDL_CreateCursor(cursor_info->data, cursor_info->mask,
			  cursor_info->width, cursor_info->height,
			  cursor_info->hot_x, cursor_info->hot_y);
}

void SDLSetMouseCursor(struct MouseCursorInfo *cursor_info)
{
  static struct MouseCursorInfo *last_cursor_info = NULL;
  static struct MouseCursorInfo *last_cursor_info2 = NULL;
  static SDL_Cursor *cursor_default = NULL;
  static SDL_Cursor *cursor_current = NULL;

  /* if invoked for the first time, store the SDL default cursor */
  if (cursor_default == NULL)
    cursor_default = SDL_GetCursor();

  /* only create new cursor if cursor info (custom only) has changed */
  if (cursor_info != NULL && cursor_info != last_cursor_info)
  {
    cursor_current = create_cursor(cursor_info);
    last_cursor_info = cursor_info;
  }

  /* only set new cursor if cursor info (custom or NULL) has changed */
  if (cursor_info != last_cursor_info2)
    SDL_SetCursor(cursor_info ? cursor_current : cursor_default);

  last_cursor_info2 = cursor_info;
}


/* ========================================================================= */
/* audio functions                                                           */
/* ========================================================================= */

void SDLOpenAudio(void)
{
  if (program.headless)
    return;

#if !defined(TARGET_SDL2)
  if (!strEqual(setup.system.sdl_audiodriver, ARG_DEFAULT))
    SDL_putenv(getStringCat2("SDL_AUDIODRIVER=", setup.system.sdl_audiodriver));
#endif

  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
  {
    Error(ERR_WARN, "SDL_InitSubSystem() failed: %s", SDL_GetError());
    return;
  }

  if (Mix_OpenAudio(DEFAULT_AUDIO_SAMPLE_RATE, MIX_DEFAULT_FORMAT,
		    AUDIO_NUM_CHANNELS_STEREO,
		    setup.system.audio_fragment_size) < 0)
  {
    Error(ERR_WARN, "Mix_OpenAudio() failed: %s", SDL_GetError());
    return;
  }

  audio.sound_available = TRUE;
  audio.music_available = TRUE;
  audio.loops_available = TRUE;
  audio.sound_enabled = TRUE;

  /* set number of available mixer channels */
  audio.num_channels = Mix_AllocateChannels(NUM_MIXER_CHANNELS);
  audio.music_channel = MUSIC_CHANNEL;
  audio.first_sound_channel = FIRST_SOUND_CHANNEL;

  Mixer_InitChannels();
}

void SDLCloseAudio(void)
{
  Mix_HaltMusic();
  Mix_HaltChannel(-1);

  Mix_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}


/* ========================================================================= */
/* event functions                                                           */
/* ========================================================================= */

void SDLWaitEvent(Event *event)
{
  SDL_WaitEvent(event);
}

void SDLHandleWindowManagerEvent(Event *event)
{
#ifdef DEBUG
#if defined(PLATFORM_WIN32)
  // experimental drag and drop code

  SDL_SysWMEvent *syswmevent = (SDL_SysWMEvent *)event;
  SDL_SysWMmsg *syswmmsg = (SDL_SysWMmsg *)(syswmevent->msg);

#if defined(TARGET_SDL2)
  if (syswmmsg->msg.win.msg == WM_DROPFILES)
#else
  if (syswmmsg->msg == WM_DROPFILES)
#endif
  {
#if defined(TARGET_SDL2)
    HDROP hdrop = (HDROP)syswmmsg->msg.win.wParam;
#else
    HDROP hdrop = (HDROP)syswmmsg->wParam;
#endif
    int i, num_files;

    printf("::: SDL_SYSWMEVENT:\n");

    num_files = DragQueryFile(hdrop, 0xffffffff, NULL, 0);

    for (i = 0; i < num_files; i++)
    {
      int buffer_len = DragQueryFile(hdrop, i, NULL, 0);
      char buffer[buffer_len + 1];

      DragQueryFile(hdrop, i, buffer, buffer_len + 1);

      printf("::: - '%s'\n", buffer);
    }

#if defined(TARGET_SDL2)
    DragFinish((HDROP)syswmmsg->msg.win.wParam);
#else
    DragFinish((HDROP)syswmmsg->wParam);
#endif
  }
#endif
#endif
}


/* ========================================================================= */
/* joystick functions                                                        */
/* ========================================================================= */

#if defined(TARGET_SDL2)
static void *sdl_joystick[MAX_PLAYERS];		// game controller or joystick
#else
static SDL_Joystick *sdl_joystick[MAX_PLAYERS];	// only joysticks supported
#endif
static int sdl_js_axis_raw[MAX_PLAYERS][2];
static int sdl_js_axis[MAX_PLAYERS][2];
static int sdl_js_button[MAX_PLAYERS][2];
static boolean sdl_is_controller[MAX_PLAYERS];

void SDLClearJoystickState()
{
  int i, j;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    for (j = 0; j < 2; j++)
    {
      sdl_js_axis_raw[i][j] = -1;
      sdl_js_axis[i][j] = 0;
      sdl_js_button[i][j] = 0;
    }
  }
}

boolean SDLOpenJoystick(int nr)
{
  if (nr < 0 || nr >= MAX_PLAYERS)
    return FALSE;

#if defined(TARGET_SDL2)
  sdl_is_controller[nr] = SDL_IsGameController(nr);
#else
  sdl_is_controller[nr] = FALSE;
#endif

#if DEBUG_JOYSTICKS
  Error(ERR_DEBUG, "opening joystick %d (%s)",
	nr, (sdl_is_controller[nr] ? "game controller" : "joystick"));
#endif

#if defined(TARGET_SDL2)
  if (sdl_is_controller[nr])
    sdl_joystick[nr] = SDL_GameControllerOpen(nr);
  else
    sdl_joystick[nr] = SDL_JoystickOpen(nr);
#else
  sdl_joystick[nr] = SDL_JoystickOpen(nr);
#endif

  return (sdl_joystick[nr] != NULL);
}

void SDLCloseJoystick(int nr)
{
  if (nr < 0 || nr >= MAX_PLAYERS)
    return;

#if DEBUG_JOYSTICKS
  Error(ERR_DEBUG, "closing joystick %d", nr);
#endif

#if defined(TARGET_SDL2)
  if (sdl_is_controller[nr])
    SDL_GameControllerClose(sdl_joystick[nr]);
  else
    SDL_JoystickClose(sdl_joystick[nr]);
#else
  SDL_JoystickClose(sdl_joystick[nr]);
#endif

  sdl_joystick[nr] = NULL;
}

boolean SDLCheckJoystickOpened(int nr)
{
  if (nr < 0 || nr >= MAX_PLAYERS)
    return FALSE;

#if defined(TARGET_SDL2)
  return (sdl_joystick[nr] != NULL ? TRUE : FALSE);
#else
  return (SDL_JoystickOpened(nr) ? TRUE : FALSE);
#endif
}

static void setJoystickAxis(int nr, int axis_id_raw, int axis_value)
{
#if defined(TARGET_SDL2)
  int axis_id = (axis_id_raw == SDL_CONTROLLER_AXIS_LEFTX ||
		 axis_id_raw == SDL_CONTROLLER_AXIS_RIGHTX ? 0 :
		 axis_id_raw == SDL_CONTROLLER_AXIS_LEFTY ||
		 axis_id_raw == SDL_CONTROLLER_AXIS_RIGHTY ? 1 : -1);
#else
  int axis_id = axis_id_raw % 2;
#endif

  if (nr < 0 || nr >= MAX_PLAYERS)
    return;

  if (axis_id == -1)
    return;

  // prevent (slightly jittering, but centered) axis A from resetting axis B
  if (ABS(axis_value) < JOYSTICK_PERCENT * JOYSTICK_MAX_AXIS_POS / 100 &&
      axis_id_raw != sdl_js_axis_raw[nr][axis_id])
    return;

  sdl_js_axis[nr][axis_id] = axis_value;
  sdl_js_axis_raw[nr][axis_id] = axis_id_raw;
}

static void setJoystickButton(int nr, int button_id_raw, int button_state)
{
#if defined(TARGET_SDL2)
  int button_id = (button_id_raw == SDL_CONTROLLER_BUTTON_A ||
		   button_id_raw == SDL_CONTROLLER_BUTTON_X ||
		   button_id_raw == SDL_CONTROLLER_BUTTON_LEFTSHOULDER ||
		   button_id_raw == SDL_CONTROLLER_BUTTON_LEFTSTICK ||
		   button_id_raw == SDL_CONTROLLER_BUTTON_RIGHTSTICK ? 0 :
		   button_id_raw == SDL_CONTROLLER_BUTTON_B ||
		   button_id_raw == SDL_CONTROLLER_BUTTON_Y ||
		   button_id_raw == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER ? 1 :
		   -1);

  if (button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
    sdl_js_axis[nr][0] = button_state * JOYSTICK_XLEFT;
  else if (button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
    sdl_js_axis[nr][0] = button_state * JOYSTICK_XRIGHT;
  else if (button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_UP)
    sdl_js_axis[nr][1] = button_state * JOYSTICK_YUPPER;
  else if (button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
    sdl_js_axis[nr][1] = button_state * JOYSTICK_YLOWER;

  if (button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_LEFT ||
      button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_RIGHT ||
      button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_UP ||
      button_id_raw == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
    sdl_js_axis_raw[nr][0] = sdl_js_axis_raw[nr][1] = -1;
#else
  int button_id = button_id_raw % 2;
#endif

  if (nr < 0 || nr >= MAX_PLAYERS)
    return;

  if (button_id == -1)
    return;

  sdl_js_button[nr][button_id] = button_state;
}

void HandleJoystickEvent(Event *event)
{
  switch(event->type)
  {
#if defined(TARGET_SDL2)
    case SDL_CONTROLLERDEVICEADDED:
#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_CONTROLLERDEVICEADDED: device %d added",
	    event->cdevice.which);
#endif
      InitJoysticks();
      break;

    case SDL_CONTROLLERDEVICEREMOVED:
#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_CONTROLLERDEVICEREMOVED: device %d removed",
	    event->cdevice.which);
#endif
      InitJoysticks();
      break;

    case SDL_CONTROLLERAXISMOTION:
#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_CONTROLLERAXISMOTION: device %d, axis %d: %d",
	    event->caxis.which, event->caxis.axis, event->caxis.value);
#endif
      setJoystickAxis(event->caxis.which,
		      event->caxis.axis,
		      event->caxis.value);
      break;

    case SDL_CONTROLLERBUTTONDOWN:
#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_CONTROLLERBUTTONDOWN: device %d, button %d",
	    event->cbutton.which, event->cbutton.button);
#endif
      setJoystickButton(event->cbutton.which,
			event->cbutton.button,
			TRUE);
      break;

    case SDL_CONTROLLERBUTTONUP:
#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_CONTROLLERBUTTONUP: device %d, button %d",
	    event->cbutton.which, event->cbutton.button);
#endif
      setJoystickButton(event->cbutton.which,
			event->cbutton.button,
			FALSE);
      break;
#endif

    case SDL_JOYAXISMOTION:
      if (sdl_is_controller[event->jaxis.which])
	break;

#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_JOYAXISMOTION: device %d, axis %d: %d",
	    event->jaxis.which, event->jaxis.axis, event->jaxis.value);
#endif
      if (event->jaxis.axis < 4)
	setJoystickAxis(event->jaxis.which,
			event->jaxis.axis,
			event->jaxis.value);
      break;

    case SDL_JOYBUTTONDOWN:
      if (sdl_is_controller[event->jaxis.which])
	break;

#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_JOYBUTTONDOWN: device %d, button %d",
	    event->jbutton.which, event->jbutton.button);
#endif
      if (event->jbutton.button < 4)
	setJoystickButton(event->jbutton.which,
			  event->jbutton.button,
			  TRUE);
      break;

    case SDL_JOYBUTTONUP:
      if (sdl_is_controller[event->jaxis.which])
	break;

#if DEBUG_JOYSTICKS
      Error(ERR_DEBUG, "SDL_JOYBUTTONUP: device %d, button %d",
	    event->jbutton.which, event->jbutton.button);
#endif
      if (event->jbutton.button < 4)
	setJoystickButton(event->jbutton.which,
			  event->jbutton.button,
			  FALSE);
      break;

    default:
      break;
  }
}

void SDLInitJoysticks()
{
  static boolean sdl_joystick_subsystem_initialized = FALSE;
  boolean print_warning = !sdl_joystick_subsystem_initialized;
#if defined(TARGET_SDL2)
  char *mappings_file_base = getPath2(options.conf_directory,
				      GAMECONTROLLER_BASENAME);
  char *mappings_file_user = getPath2(getUserGameDataDir(),
				      GAMECONTROLLER_BASENAME);
  int num_mappings;
#endif
  int i;

  if (!sdl_joystick_subsystem_initialized)
  {
    sdl_joystick_subsystem_initialized = TRUE;

#if defined(TARGET_SDL2)
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
#else
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0)
#endif
    {
      Error(ERR_EXIT, "SDL_Init() failed: %s", SDL_GetError());
      return;
    }

#if defined(TARGET_SDL2)
    num_mappings = SDL_GameControllerAddMappingsFromFile(mappings_file_base);

    /* the included game controller base mappings should always be found */
    if (num_mappings == -1)
      Error(ERR_WARN, "no game controller base mappings found");
#if DEBUG_JOYSTICKS
    else
      Error(ERR_INFO, "%d game controller base mapping(s) added", num_mappings);
#endif

    num_mappings = SDL_GameControllerAddMappingsFromFile(mappings_file_user);

#if DEBUG_JOYSTICKS
    /* the personal game controller user mappings may or may not be found */
    if (num_mappings == -1)
      Error(ERR_WARN, "no game controller user mappings found");
    else
      Error(ERR_INFO, "%d game controller user mapping(s) added", num_mappings);

    Error(ERR_INFO, "%d joystick(s) found:", SDL_NumJoysticks());
#endif

    checked_free(mappings_file_base);
    checked_free(mappings_file_user);

#if DEBUG_JOYSTICKS
    for (i = 0; i < SDL_NumJoysticks(); i++)
    {
      const char *name, *type;

      if (SDL_IsGameController(i))
      {
	name = SDL_GameControllerNameForIndex(i);
	type = "game controller";
      }
      else
      {
	name = SDL_JoystickNameForIndex(i);
	type = "joystick";
      }

      Error(ERR_INFO, "- joystick %d (%s): '%s'",
	    i, type, (name ? name : "(Unknown)"));
    }
#endif
#endif
  }

  /* assign joysticks from configured to connected joystick for all players */
  for (i = 0; i < MAX_PLAYERS; i++)
  {
    /* get configured joystick for this player */
    char *device_name = setup.input[i].joy.device_name;
    int joystick_nr = getJoystickNrFromDeviceName(device_name);

    if (joystick_nr >= SDL_NumJoysticks())
    {
      if (setup.input[i].use_joystick && print_warning)
	Error(ERR_WARN, "cannot find joystick %d", joystick_nr);

      joystick_nr = -1;
    }

    /* store configured joystick number for each player */
    joystick.nr[i] = joystick_nr;
  }

  /* now open all connected joysticks (regardless if configured or not) */
  for (i = 0; i < SDL_NumJoysticks(); i++)
  {
    /* this allows subsequent calls to 'InitJoysticks' for re-initialization */
    if (SDLCheckJoystickOpened(i))
      SDLCloseJoystick(i);

    if (SDLOpenJoystick(i))
      joystick.status = JOYSTICK_ACTIVATED;
    else if (print_warning)
      Error(ERR_WARN, "cannot open joystick %d", i);
  }

  SDLClearJoystickState();
}

boolean SDLReadJoystick(int nr, int *x, int *y, boolean *b1, boolean *b2)
{
  if (nr < 0 || nr >= MAX_PLAYERS)
    return FALSE;

  if (x != NULL)
    *x = sdl_js_axis[nr][0];
  if (y != NULL)
    *y = sdl_js_axis[nr][1];

  if (b1 != NULL)
    *b1 = sdl_js_button[nr][0];
  if (b2 != NULL)
    *b2 = sdl_js_button[nr][1];

  return TRUE;
}


/* ========================================================================= */
/* touch input overlay functions                                             */
/* ========================================================================= */

#if defined(USE_TOUCH_INPUT_OVERLAY)
static void DrawTouchInputOverlay()
{
  static SDL_Texture *texture = NULL;
  static boolean initialized = FALSE;
  static boolean deactivated = TRUE;
  static int width = 0, height = 0;
  static int alpha_max = SDL_ALPHA_OPAQUE / 2;
  static int alpha_step = 5;
  static int alpha_last = 0;
  static int alpha = 0;
  boolean active = (overlay.enabled && overlay.active);

  if (!active && deactivated)
    return;

  if (active)
  {
    if (alpha < alpha_max)
      alpha = MIN(alpha + alpha_step, alpha_max);

    deactivated = FALSE;
  }
  else
  {
    alpha = MAX(0, alpha - alpha_step);

    if (alpha == 0)
      deactivated = TRUE;
  }

  if (!initialized)
  {
    char *basename = "overlay/VirtualButtons.png";
    char *filename = getCustomImageFilename(basename);

    if (filename == NULL)
      Error(ERR_EXIT, "LoadCustomImage(): cannot find file '%s'", basename);

    SDL_Surface *surface;

    if ((surface = IMG_Load(filename)) == NULL)
      Error(ERR_EXIT, "IMG_Load() failed: %s", SDL_GetError());

    width  = surface->w;
    height = surface->h;

    /* set black pixel to transparent if no alpha channel / transparent color */
    if (!SDLHasAlpha(surface) &&
	!SDLHasColorKey(surface))
      SDL_SetColorKey(surface, SET_TRANSPARENT_PIXEL,
		      SDL_MapRGB(surface->format, 0x00, 0x00, 0x00));

    if ((texture = SDLCreateTextureFromSurface(surface)) == NULL)
      Error(ERR_EXIT, "SDLCreateTextureFromSurface() failed");

    SDL_FreeSurface(surface);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, alpha_max);

    initialized = TRUE;
  }

  if (alpha != alpha_last)
    SDL_SetTextureAlphaMod(texture, alpha);

  alpha_last = alpha;

  float ratio_overlay = (float) width / height;
  float ratio_screen = (float) video.screen_width / video.screen_height;
  int width_scaled, height_scaled;
  int xpos, ypos;

  if (ratio_overlay > ratio_screen)
  {
    width_scaled = video.screen_width;
    height_scaled = video.screen_height * ratio_screen / ratio_overlay;
    xpos = 0;
    ypos = video.screen_height - height_scaled;
  }
  else
  {
    width_scaled = video.screen_width * ratio_overlay / ratio_screen;
    height_scaled = video.screen_height;
    xpos = (video.screen_width - width_scaled) / 2;
    ypos = 0;
  }

  SDL_Rect src_rect = { 0, 0, width, height };
  SDL_Rect dst_rect = { xpos, ypos, width_scaled, height_scaled };

  SDL_RenderCopy(sdl_renderer, texture, &src_rect, &dst_rect);
}
#endif
