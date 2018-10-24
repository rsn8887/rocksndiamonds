# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2014 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# src/libgame/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	system.c	\
	gadgets.c	\
	text.c		\
	sound.c		\
	joystick.c	\
	snapshot.c	\
	image.c		\
	random.c	\
	hash.c		\
	setup.c		\
	misc.c		\
	sdl.c

OBJS =	system.o	\
	gadgets.o	\
	text.o		\
	sound.o		\
	snapshot.o	\
	joystick.o	\
	image.o		\
	random.o	\
	hash.o		\
	setup.o		\
	misc.o		\
	sdl.o

SRCS_VITA = psp2-dirent.c \
	psp2_kbdvita.c \
	psp2_input.c \
	psp2_touch.c

OBJS_VITA = psp2-dirent.o \
	psp2_kbdvita.o \
	psp2_input.o \
	psp2_touch.o

SRCS_SWITCH = \
	psp2_input.c \
	psp2_touch.c

OBJS_SWITCH = \
	psp2_input.o \
	psp2_touch.o

ifeq ($(TARGET),vita)
SRCS += $(SRCS_VITA)
OBJS += $(OBJS_VITA)
endif

ifeq ($(TARGET),switch)
SRCS += $(SRCS_SWITCH)
OBJS += $(OBJS_SWITCH)
endif

LIBGAME = libgame.a


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(LIBGAME)

$(LIBGAME): $(OBJS)
	$(AR) cru $(LIBGAME) $(OBJS)
	$(RANLIB) $(LIBGAME)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(LIBGAME)
	$(RM) $(OBJS_VITA)

# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
