# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2014 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# src/Android.mk
# =============================================================================

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH       := ../Special/Android/jni/SDL2
SDL_IMAGE_PATH := ../Special/Android/jni/SDL2_image
SDL_MIXER_PATH := ../Special/Android/jni/SDL2_mixer
SDL_NET_PATH   := ../Special/Android/jni/SDL2_net

LOCAL_C_INCLUDES :=				\
	$(LOCAL_PATH)/$(SDL_PATH)/include	\
	$(LOCAL_PATH)/$(SDL_IMAGE_PATH)		\
	$(LOCAL_PATH)/$(SDL_MIXER_PATH)		\
	$(LOCAL_PATH)/$(SDL_NET_PATH)

DEBUG = -DDEBUG -g
OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = -O3 -Wall				# distribution build

# LOCAL_CFLAGS := -DTARGET_SDL2
LOCAL_CFLAGS := -DTARGET_SDL2 $(OPTIONS)

# application source code files
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	libgame/system.c		\
	libgame/gadgets.c		\
	libgame/text.c			\
	libgame/sound.c			\
	libgame/joystick.c		\
	libgame/snapshot.c		\
	libgame/image.c			\
	libgame/random.c		\
	libgame/hash.c			\
	libgame/setup.c			\
	libgame/misc.c			\
	libgame/sdl.c			\
	game_em/cave.c			\
	game_em/convert.c		\
	game_em/graphics.c		\
	game_em/init.c			\
	game_em/input.c			\
	game_em/main.c			\
	game_em/sound.c			\
	game_em/synchro_1.c		\
	game_em/synchro_2.c		\
	game_em/synchro_3.c		\
	game_em/tab_generate.c		\
	game_em/ulaw_generate.c		\
	game_sp/init.c			\
	game_sp/file.c			\
	game_sp/main.c			\
	game_sp/vb_lib.c		\
	game_sp/ASM.c			\
	game_sp/BugsTerminals.c		\
	game_sp/DDScrollBuffer.c	\
	game_sp/DDSpriteBuffer.c	\
	game_sp/Display.c		\
	game_sp/DoGameStuff.c		\
	game_sp/Electrons.c		\
	game_sp/Explosions.c		\
	game_sp/Globals.c		\
	game_sp/Infotrons.c		\
	game_sp/InitGameConditions.c	\
	game_sp/Input.c			\
	game_sp/MainForm.c		\
	game_sp/MainGameLoop.c		\
	game_sp/Murphy.c		\
	game_sp/OrangeDisk.c		\
	game_sp/SnikSnaks.c		\
	game_sp/Sound.c			\
	game_sp/Zonk.c			\
	game_mm/mm_init.c		\
	game_mm/mm_main.c		\
	game_mm/mm_game.c		\
	game_mm/mm_files.c		\
	game_mm/mm_tools.c		\
	main.c				\
	conf_gfx.c			\
	conf_snd.c			\
	conf_mus.c			\
	conf_hlp.c			\
	init.c				\
	config.c			\
	events.c			\
	tools.c				\
	screens.c			\
	game.c				\
	editor.c			\
	files.c				\
	tape.c				\
	anim.c				\
	network.c			\
	netserv.c

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image SDL2_mixer SDL2_net smpeg2

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
