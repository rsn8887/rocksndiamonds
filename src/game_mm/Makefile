# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1989-2017 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# The native Mirror Magic game engine is based on:
# - Mirror Magic II by Holger Schemel (Linux/DOS/Windows version, 1995)
# - Mindbender by Holger Schemel (Amiga version, 1989)
# -----------------------------------------------------------------------------
# src/game_mm/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

SRCS =	mm_init.c		\
	mm_main.c		\
	mm_game.c		\
	mm_files.c		\
	mm_tools.c

OBJS =	mm_init.o		\
	mm_main.o		\
	mm_game.o		\
	mm_files.o		\
	mm_tools.o

GAME_MM = game_mm.a


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(GAME_MM)

$(GAME_MM): $(OBJS)
	$(AR) cru $(GAME_MM) $(OBJS)
	$(RANLIB) $(GAME_MM)

.c.o:
	$(CC) $(PROFILING) $(CFLAGS) -c $*.c

clean:
	$(RM) $(OBJS)
	$(RM) $(GAME_MM)


# -----------------------------------------------------------------------------
# development only
# -----------------------------------------------------------------------------

depend:
	for i in $(SRCS); do $(CPP) $(CFLAGS) -M $$i; done > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
