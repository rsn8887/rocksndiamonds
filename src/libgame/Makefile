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


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
