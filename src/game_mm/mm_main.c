// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_main.c
// ============================================================================

#include "main_mm.h"

#include "mm_main.h"


struct GameInfo_MM game_mm;
struct LevelInfo_MM native_mm_level;

short		Ur[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Hit[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Box[MAX_LEV_FIELDX][MAX_LEV_FIELDY];
short		Angle[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

short		Frame[MAX_LEV_FIELDX][MAX_LEV_FIELDY];

unsigned int	Elementeigenschaften[MAX_ELEMENTS];

struct LaserInfo	laser;

short		LX,LY, XS,YS, ELX,ELY;
short		CT,Ct;

int		dSX, dSY;
int		cSX, cSY;
int		cSX2, cSY2;
int		cFX, cFY;

Pixel		pen_fg, pen_bg, pen_ray, pen_magicolor[2];
int		color_status;

struct XY	Step[16] =
{
  {  1,  0 },
  {  2, -1 },
  {  1, -1 },
  {  1, -2 },
  {  0, -1 },
  { -1, -2 },
  { -1, -1 },
  { -2, -1 },
  { -1,  0 },
  { -2,  1 },
  { -1,  1 },
  { -1,  2 },
  {  0,  1 },
  {  1,  2 },
  {  1,  1 },
  {  2,  1 }
};

/* "Sign" has the following structure:

   each 4-bit-value represents the values d*8 + c*4 + b*2 + a*1
   at the pixel positions

   a b     1 2
   c d     4 8

   so the value "0xA" (=> (d=1)*8 + (c=0)*4 + (b=1)*2 + (a=0)*1)
   would result in the pixel pattern

   0 1        _ x
   0 1    or  _ x

   x x    x x    x x    x x    x x
   x _    x x    _ _    x x    _ x
      6                      2
   x x     5      4      3     x x
   x x  7                   1  x x

   x _                         _ x
   x _  8                   0  _ x

   x x  9                  15  x x
   x x     11    12     13     x x
      10                     14
   x _    x x    _ _    x x    _ x
   x x    x x    x x    x x    x x

 */

short Sign[16] =
{
  0xA, 0xF, 0xB, 0xF,
  0x3, 0xF, 0x7, 0xF,
  0x5, 0xF, 0xD, 0xF,
  0xC, 0xF, 0xE, 0xF
};

char *element_info_mm[] =
{
  "empty space",				/* 0 */
  "mirror (0\xb0)",
  "mirror (11.25\xb0)",
  "mirror (22.5\xb0)",
  "mirror (33.75\xb0)",
  "mirror (45\xb0)",
  "mirror (56.25\xb0)",
  "mirror (67.5\xb0)",
  "mirror (78.75\xb0)",
  "mirror (90\xb0)",
  "mirror (101.25\xb0)",			/* 10 */
  "mirror (112.5\xb0)",
  "mirror (123.75\xb0)",
  "mirror (135\xb0)",
  "mirror (146.25\xb0)",
  "mirror (157.5\xb0)",
  "mirror (168.75\xb0)",
  "fixed steel polarizer (0\xb0)",
  "fixed steel polarizer (90\xb0)",
  "fixed steel polarizer (45\xb0)",
  "fixed steel polarizer (135\xb0)",		/* 20 */
  "Gregor McDuffin (looking right)",
  "Gregor McDuffin (looking up)",
  "Gregor McDuffin (looking left)",
  "Gregor McDuffin (looking down)",
  "closed exit",
  "opening exit",
  "opening exit",
  "open exit",
  "magic kettle",
  "bomb",					/* 30 */
  "prism",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",					/* 40 */
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "steel wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",				/* 50 */
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "wooden wall",				/* 60 */
  "wooden wall",
  "wooden wall",
  "wooden wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",					/* 70 */
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "ice wall",
  "amoeba wall",				/* 80 */
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",				/* 90 */
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "amoeba wall",
  "wooden block",
  "gray ball",
  "teleporter (0\xb0)",
  "teleporter (22.5\xb0)",
  "teleporter (45\xb0)",			/* 100 */
  "teleporter (67.5\xb0)",
  "teleporter (90\xb0)",
  "teleporter (112.5\xb0)",
  "teleporter (135\xb0)",
  "teleporter (157.5\xb0)",
  "teleporter (180\xb0)",
  "teleporter (202.5\xb0)",
  "teleporter (225\xb0)",
  "teleporter (247.5\xb0)",
  "teleporter (270\xb0)",			/* 110 */
  "teleporter (292.5\xb0)",
  "teleporter (315\xb0)",
  "teleporter (337.5\xb0)",
  "fuse",
  "pac man (starts moving right)",
  "pac man (starts moving up)",
  "pac man (starts moving left)",
  "pac man (starts moving down)",
  "polarizer (0\xb0)",
  "polarizer (11.25\xb0)",			/* 120 */
  "polarizer (22.5\xb0)",
  "polarizer (33.75\xb0)",
  "polarizer (45\xb0)",
  "polarizer (56.25\xb0)",
  "polarizer (67.5\xb0)",
  "polarizer (78.75\xb0)",
  "polarizer (90\xb0)",
  "polarizer (101.25\xb0)",
  "polarizer (112.5\xb0)",
  "polarizer (123.75\xb0)",			/* 130 */
  "polarizer (135\xb0)",
  "polarizer (146.25\xb0)",
  "polarizer (157.5\xb0)",
  "polarizer (168.75\xb0)",
  "two-way polarizer (0\xb0)",
  "two-way polarizer (22.5\xb0)",
  "two-way polarizer (45\xb0)",
  "two-way polarizer (67.5\xb0)",
  "fixed mirror (0\xb0)",
  "fixed mirror (45\xb0)",			/* 140 */
  "fixed mirror (90\xb0)",
  "fixed mirror (135\xb0)",
  "reflecting stone lock",
  "key",
  "light bulb (dark)",
  "light bulb (glowing)",
  "bonus ball",
  "reflecting stone block",
  "wooden lock",
  "extra energy ball (full)",			/* 150 */
  "fixed wooden polarizer (0\xb0)",
  "fixed wooden polarizer (90\xb0)",
  "fixed wooden polarizer (45\xb0)",
  "fixed wooden polarizer (135\xb0)",
  "extra energy ball (empty)",
  "unused",
  "unused",
  "unused",
  "unused",
  "letter ' '",					/* 160 */
  "letter '!'",
  "letter '\"'",
  "letter '#'",
  "letter '$'",
  "letter '%'",
  "letter '&'",
  "letter '''",
  "letter '('",
  "letter ')'",
  "letter '*'",					/* 170 */
  "letter '+'",
  "letter ','",
  "letter '-'",
  "letter '.'",
  "letter '/'",
  "letter '0'",
  "letter '1'",
  "letter '2'",
  "letter '3'",
  "letter '4'",					/* 180 */
  "letter '5'",
  "letter '6'",
  "letter '7'",
  "letter '8'",
  "letter '9'",
  "letter ':'",
  "letter ';'",
  "letter '<'",
  "letter '='",
  "letter '>'",					/* 190 */
  "letter '?'",
  "letter '@'",
  "letter 'A'",
  "letter 'B'",
  "letter 'C'",
  "letter 'D'",
  "letter 'E'",
  "letter 'F'",
  "letter 'G'",
  "letter 'H'",					/* 200 */
  "letter 'I'",
  "letter 'J'",
  "letter 'K'",
  "letter 'L'",
  "letter 'M'",
  "letter 'N'",
  "letter 'O'",
  "letter 'P'",
  "letter 'Q'",
  "letter 'R'",					/* 210 */
  "letter 'S'",
  "letter 'T'",
  "letter 'U'",
  "letter 'V'",
  "letter 'W'",
  "letter 'X'",
  "letter 'Y'",
  "letter 'Z'",
  "letter '\xc4'",
  "letter '\xd6'",				/* 220 */
  "letter '\xdc'",
  "letter '^'",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",					/* 230 */
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "letter ''",
  "mirror (0\xb0)",				/* 240 */
  "mirror (11.25\xb0)",
  "mirror (22.5\xb0)",
  "mirror (33.75\xb0)",
  "mirror (45\xb0)",
  "mirror (56.25\xb0)",
  "mirror (67.5\xb0)",
  "mirror (78.75\xb0)",
  "mirror (90\xb0)",
  "mirror (101.25\xb0)",
  "mirror (112.5\xb0)",				/* 250 */
  "mirror (123.75\xb0)",
  "mirror (135\xb0)",
  "mirror (146.25\xb0)",
  "mirror (157.5\xb0)",
  "mirror (168.75\xb0)",
  "fixed wooden polarizer (0\xb0)",
  "fixed wooden polarizer (22.5\xb0)",
  "fixed wooden polarizer (45\xb0)",
  "fixed wooden polarizer (67.5\xb0)",
  "fixed wooden polarizer (90\xb0)",		/* 260 */
  "fixed wooden polarizer (112.5\xb0)",
  "fixed wooden polarizer (135\xb0)",
  "fixed wooden polarizer (157.5\xb0)",
  "fixed steel polarizer (0\xb0)",
  "fixed steel polarizer (22.5\xb0)",
  "fixed steel polarizer (45\xb0)",
  "fixed steel polarizer (67.5\xb0)",
  "fixed steel polarizer (90\xb0)",
  "fixed steel polarizer (112.5\xb0)",
  "fixed steel polarizer (135\xb0)",		/* 270 */
  "fixed steel polarizer (157.5\xb0)",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",		/* 280 */
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style wooden wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",			/* 290 */
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",			/* 300 */
  "deflektor style steel wall",
  "deflektor style steel wall",
  "deflektor style steel wall",
  "empty space",
  "cell",
  "mine",
  "refractor",
  "laser cannon (shooting right)",
  "laser cannon (shooting up)",
  "laser cannon (shooting left)",		/* 310 */
  "laser cannon (shooting down)",
  "laser receiver (directed right)",
  "laser receiver (directed up)",
  "laser receiver (directed left)",
  "laser receiver (directed down)",
  "fibre optic (1a)",
  "fibre optic (1b)",
  "fibre optic (2a)",
  "fibre optic (2b)",
  "fibre optic (3a)",				/* 320 */
  "fibre optic (3b)",
  "fibre optic (4a)",
  "fibre optic (4b)",
  "rotating mirror (0\xb0)",
  "rotating mirror (11.25\xb0)",
  "rotating mirror (22.5\xb0)",
  "rotating mirror (33.75\xb0)",
  "rotating mirror (45\xb0)",
  "rotating mirror (56.25\xb0)",
  "rotating mirror (67.5\xb0)",			/* 330 */
  "rotating mirror (78.75\xb0)",
  "rotating mirror (90\xb0)",
  "rotating mirror (101.25\xb0)",
  "rotating mirror (112.5\xb0)",
  "rotating mirror (123.75\xb0)",
  "rotating mirror (135\xb0)",
  "rotating mirror (146.25\xb0)",
  "rotating mirror (157.5\xb0)",
  "rotating mirror (168.75\xb0)",
  "rotating wooden polarizer (0\xb0)",		/* 340 */
  "rotating wooden polarizer (22.5\xb0)",
  "rotating wooden polarizer (45\xb0)",
  "rotating wooden polarizer (67.5\xb0)",
  "rotating wooden polarizer (90\xb0)",
  "rotating wooden polarizer (112.5\xb0)",
  "rotating wooden polarizer (135\xb0)",
  "rotating wooden polarizer (157.5\xb0)",
  "rotating steel polarizer (0\xb0)",
  "rotating steel polarizer (22.5\xb0)",
  "rotating steel polarizer (45\xb0)",		/* 350 */
  "rotating steel polarizer (67.5\xb0)",
  "rotating steel polarizer (90\xb0)",
  "rotating steel polarizer (112.5\xb0)",
  "rotating steel polarizer (135\xb0)",
  "rotating steel polarizer (157.5\xb0)",
  "red teleporter (0\xb0)",
  "red teleporter (22.5\xb0)",
  "red teleporter (45\xb0)",
  "red teleporter (67.5\xb0)",
  "red teleporter (90\xb0)",			/* 360 */
  "red teleporter (112.5\xb0)",
  "red teleporter (135\xb0)",
  "red teleporter (157.5\xb0)",
  "red teleporter (180\xb0)",
  "red teleporter (202.5\xb0)",
  "red teleporter (225\xb0)",
  "red teleporter (247.5\xb0)",
  "red teleporter (270\xb0)",
  "red teleporter (292.5\xb0)",
  "red teleporter (315\xb0)",			/* 370 */
  "red teleporter (337.5\xb0)",
  "yellow teleporter (0\xb0)",
  "yellow teleporter (22.5\xb0)",
  "yellow teleporter (45\xb0)",
  "yellow teleporter (67.5\xb0)",
  "yellow teleporter (90\xb0)",
  "yellow teleporter (112.5\xb0)",
  "yellow teleporter (135\xb0)",
  "yellow teleporter (157.5\xb0)",
  "yellow teleporter (180\xb0)",		/* 380 */
  "yellow teleporter (202.5\xb0)",
  "yellow teleporter (225\xb0)",
  "yellow teleporter (247.5\xb0)",
  "yellow teleporter (270\xb0)",
  "yellow teleporter (292.5\xb0)",
  "yellow teleporter (315\xb0)",
  "yellow teleporter (337.5\xb0)",
  "green teleporter (0\xb0)",
  "green teleporter (22.5\xb0)",
  "green teleporter (45\xb0)",			/* 390 */
  "green teleporter (67.5\xb0)",
  "green teleporter (90\xb0)",
  "green teleporter (112.5\xb0)",
  "green teleporter (135\xb0)",
  "green teleporter (157.5\xb0)",
  "green teleporter (180\xb0)",
  "green teleporter (202.5\xb0)",
  "green teleporter (225\xb0)",
  "green teleporter (247.5\xb0)",
  "green teleporter (270\xb0)",			/* 400 */
  "green teleporter (292.5\xb0)",
  "green teleporter (315\xb0)",
  "green teleporter (337.5\xb0)",
  "blue teleporter (0\xb0)",
  "blue teleporter (22.5\xb0)",
  "blue teleporter (45\xb0)",
  "blue teleporter (67.5\xb0)",
  "blue teleporter (90\xb0)",
  "blue teleporter (112.5\xb0)",
  "blue teleporter (135\xb0)",			/* 410 */
  "blue teleporter (157.5\xb0)",
  "blue teleporter (180\xb0)",
  "blue teleporter (202.5\xb0)",
  "blue teleporter (225\xb0)",
  "blue teleporter (247.5\xb0)",
  "blue teleporter (270\xb0)",
  "blue teleporter (292.5\xb0)",
  "blue teleporter (315\xb0)",
  "blue teleporter (337.5\xb0)",
  "unknown",					/* 420 */

  /*
  "-------------------------------",
  */
};
int num_element_info_mm = sizeof(element_info_mm) / sizeof(char *);
