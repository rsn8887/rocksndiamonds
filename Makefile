# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2015 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

# command name of your favorite ANSI C compiler
# (this must be set to "cc" for some systems)
CC = gcc

# command name of GNU make on your system
# (this must be set to "gmake" for some systems)
MAKE = make

# directory for read-only game data (like graphics, sounds, levels)
# (this directory is usually the game's installation directory)
# default is '.' to be able to run program without installation
# RO_GAME_DIR = .
# use the following setting for Debian / Ubuntu installations:
# RO_GAME_DIR = /usr/share/games/rocksndiamonds

# directory for writable game data (like highscore files)
# (if no "scores" directory exists, scores are saved in user data directory)
# default is '.' to be able to run program without installation
# RW_GAME_DIR = .
# use the following setting for Debian / Ubuntu installations:
# RW_GAME_DIR = /var/games/rocksndiamonds

# uncomment if system has no joystick include file
# JOYSTICK = -DNO_JOYSTICK

# path for cross-compiling (only needed for non-native Windows build)
CROSS_PATH_WIN32 = /usr/local/cross-tools/i386-mingw32msvc


# -----------------------------------------------------------------------------
# there should be no need to change anything below
# -----------------------------------------------------------------------------

.EXPORT_ALL_VARIABLES:

SRC_DIR = src
ANDROID_DIR ?= android

MAKE_CMD = $(MAKE) -C $(SRC_DIR)
MAKE_CMD_ANDROID = $(MAKE) -C $(ANDROID_DIR)


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all:
	@$(MAKE_CMD)

sdl:
	@$(MAKE_CMD) TARGET=sdl

sdl2:
	@$(MAKE_CMD) TARGET=sdl2

mac:
	@$(MAKE_CMD) PLATFORM=macosx

cross-win32:
	@PATH=$(CROSS_PATH_WIN32)/bin:${PATH} $(MAKE_CMD) PLATFORM=cross-win32

android-compile:
	@$(MAKE_CMD_ANDROID) compile

android-package:
	@$(MAKE_CMD_ANDROID) package

android-install:
	@$(MAKE_CMD_ANDROID) install

android-assets-toc:
	@$(MAKE_CMD_ANDROID) assets-toc

android-clean:
	@$(MAKE_CMD_ANDROID) clean

android: android-package

clean:
	@$(MAKE_CMD) clean

clean-git:
	@$(MAKE_CMD) clean-git

clean-android: android-clean


# -----------------------------------------------------------------------------
# development targets
# -----------------------------------------------------------------------------

MAKE_ENGINETEST = ./Scripts/make_enginetest.sh
MAKE_LEVELSKETCH = ./Scripts/make_levelsketch_images.sh

auto-conf:
	@$(MAKE_CMD) auto-conf

run: all
	@$(MAKE_CMD) run

gdb: all
	@$(MAKE_CMD) gdb

valgrind: all
	@$(MAKE_CMD) valgrind

tags:
	$(MAKE_CMD) tags

depend dep:
	$(MAKE_CMD) depend

enginetest: all
	$(MAKE_ENGINETEST)

enginetestcustom: all
	$(MAKE_ENGINETEST) custom

enginetestfast: all
	$(MAKE_ENGINETEST) fast

enginetestnew: all
	$(MAKE_ENGINETEST) new

leveltest: all
	$(MAKE_ENGINETEST) leveltest

levelsketch_images: all
	$(MAKE_LEVELSKETCH)


# -----------------------------------------------------------------------------
# distribution targets
# -----------------------------------------------------------------------------

MAKE_DIST = ./Scripts/make_dist.sh

dist-clean:
	@$(MAKE_CMD) dist-clean

dist-clean-android:
	@$(MAKE_CMD_ANDROID) dist-clean

dist-build-unix:
	@BUILD_DIST=TRUE $(MAKE)

dist-build-win32:
	@BUILD_DIST=TRUE $(MAKE) cross-win32

dist-build-macosx:
	@BUILD_DIST=TRUE $(MAKE)

dist-build-android:
	@BUILD_DIST=TRUE $(MAKE) android

dist-package-unix:
	$(MAKE_DIST) package unix

dist-package-win32:
	$(MAKE_DIST) package win

dist-package-macosx:
	$(MAKE_DIST) package mac

dist-package-android:
	$(MAKE_DIST) package android

dist-copy-package-unix:
	$(MAKE_DIST) copy-package unix

dist-copy-package-win32:
	$(MAKE_DIST) copy-package win

dist-copy-package-macosx:
	$(MAKE_DIST) copy-package mac

dist-copy-package-android:
	$(MAKE_DIST) copy-package android

dist-upload-unix:
	$(MAKE_DIST) upload unix

dist-upload-win32:
	$(MAKE_DIST) upload win

dist-upload-macosx:
	$(MAKE_DIST) upload mac

dist-upload-android:
	$(MAKE_DIST) upload android

dist-package-all:
	$(MAKE) dist-package-unix
	$(MAKE) dist-package-win32
	$(MAKE) dist-package-macosx
	$(MAKE) dist-package-android

dist-copy-package-all:
	$(MAKE) dist-copy-package-unix
	$(MAKE) dist-copy-package-win32
	$(MAKE) dist-copy-package-macosx
	$(MAKE) dist-copy-package-android

dist-upload-all:
	$(MAKE) dist-upload-unix
	$(MAKE) dist-upload-win32
	$(MAKE) dist-upload-macosx
	$(MAKE) dist-upload-android

dist-release-all: dist-package-all dist-copy-package-all dist-upload-all

package-all: dist-package-all

copy-package-all: dist-copy-package_all

upload-all: dist-upload-all

release-all: dist-release-all
