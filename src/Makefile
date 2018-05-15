# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2015 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# src/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

ifndef PLATFORM				# unknown platform -- default to Unix
PLATFORM = unix
endif

AR = ar
RANLIB = ranlib
ETAGS = etags
RM = rm -f

CONVERT = convert
WINDRES = windres

CONVERT_ICON_ARGS = -transparent black -background transparent

PROGBASE = rocksndiamonds
PROGNAME = ../$(PROGBASE)

EDITION ?= default


# -----------------------------------------------------------------------------
# configuring platform
# -----------------------------------------------------------------------------

ifeq ($(PLATFORM),macosx)		# compiling on Mac OS X
EXTRA_LDFLAGS = -lstdc++
override PLATFORM = unix
endif

ifeq ($(PLATFORM),unix)			# compiling on Unix/Linux (generic)
PROFILING_FLAGS = -pg
endif

ifeq ($(PLATFORM),cross-win32)		# cross-compiling to Windows
PROGNAME = ../$(PROGBASE).exe
EXTRA_LDFLAGS = -lshfolder -lwsock32
endif


# -----------------------------------------------------------------------------
# configuring target
# -----------------------------------------------------------------------------

ifndef TARGET				# auto-detect compiling for SDL or SDL2
  SDL_VERSION := $(shell sdl2-config --version 2> /dev/null)
  ifdef SDL_VERSION
    TARGET = sdl2
  else
    SDL_VERSION := $(shell sdl-config --version 2> /dev/null)
    ifdef SDL_VERSION
      TARGET = sdl
    else
      $(error SDL/SDL2 library not found)
    endif
  endif
endif

# $(info Using SDL version $(SDL_VERSION) [TARGET == $(TARGET)])

ifeq ($(TARGET),sdl)			# compiling for SDL target
SYS_CFLAGS = -DTARGET_SDL $(shell sdl-config --cflags)
SDL_LIBS = -lSDL_image -lSDL_mixer -lSDL_net
SYS_LDFLAGS = $(SDL_LIBS) $(shell sdl-config --libs)
endif

ifeq ($(TARGET),sdl2)			# compiling for SDL2 target
SYS_CFLAGS = -DTARGET_SDL2 $(shell sdl2-config --cflags)
SDL_LIBS = -lSDL2_image -lSDL2_mixer -lSDL2_net
SYS_LDFLAGS = $(SDL_LIBS) $(shell sdl2-config --libs)
endif


# -----------------------------------------------------------------------------
# configuring compile-time definitions
# -----------------------------------------------------------------------------

ifdef RO_GAME_DIR			# path to read-only game data specified
CONFIG_RO_GAME_DIR = -DRO_GAME_DIR="\"$(RO_GAME_DIR)\""
endif

ifdef RW_GAME_DIR			# path to writable game data specified
CONFIG_RW_GAME_DIR = -DRW_GAME_DIR="\"$(RW_GAME_DIR)\""
endif

CONFIG = $(CONFIG_RO_GAME_DIR) $(CONFIG_RW_GAME_DIR) $(JOYSTICK)

DEBUG = -DDEBUG -g

# PROFILING = $(PROFILING_FLAGS)

# OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -O3 -Wall			# only for debugging purposes
OPTIONS = $(DEBUG) -Wall			# only for debugging purposes
# OPTIONS = $(DEBUG) -Wall -ansi -pedantic	# only for debugging purposes
# OPTIONS = -O3 -Wall -ansi -pedantic
# OPTIONS = -O3 -Wall
# OPTIONS = -O3

ifdef BUILD_DIST			# distribution build
SYS_LDFLAGS := $(shell echo $(SYS_LDFLAGS) |	\
		       sed -e "s%-rpath,[^ ]*%-rpath,'\$$ORIGIN/lib'%")
OPTIONS = -O3 -Wall
endif

CFLAGS = $(OPTIONS) $(SYS_CFLAGS)  $(EXTRA_CFLAGS) $(CONFIG)
LDFLAGS =           $(SYS_LDFLAGS) $(EXTRA_LDFLAGS) -lm


SRCS =	main.c		\
	conf_gfx.c	\
	conf_snd.c	\
	conf_mus.c	\
	conf_hlp.c	\
	init.c		\
	config.c	\
	events.c	\
	tools.c		\
	screens.c	\
	game.c		\
	editor.c	\
	files.c		\
	tape.c		\
	anim.c		\
	network.c	\
	netserv.c

OBJS =	main.o		\
	conf_gfx.o	\
	conf_snd.o	\
	conf_mus.o	\
	conf_hlp.o	\
	init.o		\
	config.o	\
	events.o	\
	tools.o		\
	screens.o	\
	game.o		\
	editor.o	\
	files.o		\
	tape.o		\
	anim.o		\
	network.o	\
	netserv.o

CNFS =	conf_gfx.h	\
	conf_snd.h	\
	conf_mus.h	\
	conf_chr.c	\
	conf_chr.h	\
	conf_cus.c	\
	conf_cus.h	\
	conf_grp.c	\
	conf_grp.h	\
	conf_e2g.c	\
	conf_esg.c	\
	conf_e2s.c	\
	conf_fnt.c	\
	conf_g2s.c	\
	conf_g2m.c	\
	conf_var.c	\
	conf_act.c

CNFS_CMD = ../build-scripts/create_element_defs.pl

TIMESTAMP_FILE = conftime.h
TIMESTAMP_FORMAT = %Y-%m-%d %H:%M

# use SOURCE_DATE_EPOCH, or else last Git commit date, or else current date
SOURCE_DATE_EPOCH ?= $(shell test -d ../.git && test `git ls-files -m | wc -l` -eq 0 && git show -s --format=%ct || date +%s)

# get source date string from either GNU / Linux or BSD / Mac OS X style "date"
SOURCE_DATE_STRING := $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" "+$(TIMESTAMP_FORMAT)"  2>/dev/null || date -u -r "$(SOURCE_DATE_EPOCH)" "+$(TIMESTAMP_FORMAT)" 2>/dev/null || date -u "+$(TIMESTAMP_FORMAT)")

LIBGAME_DIR = libgame
LIBGAME = $(LIBGAME_DIR)/libgame.a

GAME_EM_DIR = game_em
GAME_EM = $(GAME_EM_DIR)/game_em.a

GAME_SP_DIR = game_sp
GAME_SP = $(GAME_SP_DIR)/game_sp.a

GAME_MM_DIR = game_mm
GAME_MM = $(GAME_MM_DIR)/game_mm.a

RNDLIBS = $(LIBGAME) $(GAME_EM) $(GAME_SP) $(GAME_MM)
AUTOCONF = conf_gfx.h conf_snd.h conf_mus.h

ICONBASE = windows_icon
ICON_BASEPATH = ../Special/Icons/windows_icons

ifeq ($(PLATFORM),cross-win32)
ICON_PATH = $(ICON_BASEPATH)/$(EDITION)
ICON = $(ICONBASE).o
endif

GRAPHICS_DIR = ../graphics


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(AUTOCONF) libgame_dir game_em_dir game_sp_dir game_mm_dir $(PROGNAME) graphics_dir

$(PROGNAME): $(RNDLIBS) $(TIMESTAMP_FILE) $(OBJS) $(ICON)
	$(CC) $(PROFILING) $(OBJS) $(ICON) $(RNDLIBS) $(LDFLAGS) -o $(PROGNAME)
ifdef BUILD_DIST
	strip $(PROGNAME)
endif

libgame_dir:
	@$(MAKE) -C $(LIBGAME_DIR)
$(LIBGAME):
	@$(MAKE) -C $(LIBGAME_DIR)

game_em_dir:
	@$(MAKE) -C $(GAME_EM_DIR)
$(GAME_EM):
	@$(MAKE) -C $(GAME_EM_DIR)

game_sp_dir:
	@$(MAKE) -C $(GAME_SP_DIR)
$(GAME_SP):
	@$(MAKE) -C $(GAME_SP_DIR)

game_mm_dir:
	@$(MAKE) -C $(GAME_MM_DIR)
$(GAME_MM):
	@$(MAKE) -C $(GAME_MM_DIR)

auto-conf:
	@for i in $(CNFS); do			\
		echo "$(CNFS_CMD) $$i > $$i";	\
		$(CNFS_CMD) $$i > $$i;		\
	done

auto-conf-clean:
	@for i in $(CNFS); do			\
		echo "$(RM) $$i";		\
		$(RM) $$i;			\
	done

conf_gfx.h: conf_gfx.c $(CNFS_CMD)
	@$(MAKE) auto-conf

conf_snd.h: conf_snd.c $(CNFS_CMD)
	@$(MAKE) auto-conf

conf_mus.h: conf_mus.c $(CNFS_CMD)
	@$(MAKE) auto-conf

$(TIMESTAMP_FILE): $(SRCS) $(RNDLIBS)
	@echo '#define SOURCE_DATE_STRING "$(SOURCE_DATE_STRING)"' \
	> $(TIMESTAMP_FILE)

$(ICON):
#	$(CONVERT) $(ICON32X32) $(CONVERT_ICON_ARGS) $(ICONBASE).ico
	$(CONVERT) $(ICON_PATH)/*.png $(CONVERT_ICON_ARGS) $(ICONBASE).ico
	echo "$(ICONBASE) ICON $(ICONBASE).ico" | $(WINDRES) -o $(ICON)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

graphics_dir:
	@test -f $(GRAPHICS_DIR)/Makefile && $(MAKE) -C $(GRAPHICS_DIR) || true

clean-obj:
	$(MAKE) -C $(LIBGAME_DIR) clean
	$(MAKE) -C $(GAME_EM_DIR) clean
	$(MAKE) -C $(GAME_SP_DIR) clean
	$(MAKE) -C $(GAME_MM_DIR) clean
	$(RM) $(OBJS)
	$(RM) $(RNDLIBS)

clean-ico:
	$(RM) $(ICONBASE).ico
	$(RM) $(ICONBASE).o

clean-bin:
	$(RM) $(PROGNAME)
	$(RM) ../*.exe

clean: clean-obj clean-ico clean-bin

clean-git: clean auto-conf-clean
	@$(MAKE) -C $(GRAPHICS_DIR) clean

dist-clean: clean-obj clean-ico


# -----------------------------------------------------------------------------
# run and test targets
# -----------------------------------------------------------------------------

run:
	cd .. && ./$(PROGBASE) --verbose

gdb:
	cd .. && gdb -batch -x GDB_COMMANDS ./$(PROGBASE)

valgrind:
	cd .. && valgrind -v --leak-check=yes ./$(PROGBASE) 2> valgrind.out


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

tags:
	$(ETAGS) *.[ch] $(LIBGAME_DIR)/*.[ch] $(GAME_EM_DIR)/*.[ch] $(GAME_SP_DIR)/*.[ch] $(GAME_MM_DIR)/*.[ch]

depend:
	$(MAKE) -C $(LIBGAME_DIR) depend
	$(MAKE) -C $(GAME_EM_DIR) depend
	$(MAKE) -C $(GAME_SP_DIR) depend
	$(MAKE) -C $(GAME_MM_DIR) depend
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
