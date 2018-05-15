// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// conf_hlp.c
// ============================================================================

#include "libgame/libgame.h"
#include "main.h"


/* List values that are not defined in the configuration file are set to
   reliable default values. If that value is GFX_ARG_UNDEFINED, it will
   be dynamically determined, using some of the other list values. */

struct ConfigInfo helpanim_config[] =
{
  { "player_1.moving.down",			"16"			},
  { "player_1.moving.up",			"16"			},
  { "player_1.moving.left",			"16"			},
  { "player_1.moving.right",			"16"			},
  { "player_1.pushing.left",			"16"			},
  { "player_1.pushing.right",			"16"			},
  { "end",					""			},

  { "sand",					"50"			},
  { "sand.digging.left",			"8"			},
  { "empty_space",				"10"			},
  { "sand",					"50"			},
  { "sand.digging.right",			"8"			},
  { "empty_space",				"10"			},
  { "sand",					"50"			},
  { "sand.digging.up",				"8"			},
  { "empty_space",				"10"			},
  { "sand",					"50"			},
  { "sand.digging.down",			"8"			},
  { "empty_space",				"10"			},
  { "end",					""			},

  { "empty_space",				"-1"			},
  { "end",					""			},

  { "quicksand_empty",				"-1"			},
  { "end",					""			},

  { "steelwall",				"-1"			},
  { "end",					""			},

  { "wall",					"-1"			},
  { "end",					""			},

  { "expandable_wall.growing.left",		"20"			},
  { "wall",					"50"			},
  { "expandable_wall.growing.right",		"20"			},
  { "wall",					"50"			},
  { "expandable_wall.growing.up",		"20"			},
  { "wall",					"50"			},
  { "expandable_wall.growing.down",		"20"			},
  { "wall",					"50"			},
  { "empty_space",				"20"			},
  { "end",					""			},

  { "invisible_wall",				"-1"			},
  { "end",					""			},

  { "wall_slippery",				"-1"			},
  { "end",					""			},

  { "char_space",				"10"			},
  { "char_exclam",				"10"			},
  { "char_quotedbl",				"10"			},
  { "char_numbersign",				"10"			},
  { "char_dollar",				"10"			},
  { "char_percent",				"10"			},
  { "char_ampersand",				"10"			},
  { "char_apostrophe",				"10"			},
  { "char_parenleft",				"10"			},
  { "char_parenright",				"10"			},
  { "char_asterisk",				"10"			},
  { "char_plus",				"10"			},
  { "char_comma",				"10"			},
  { "char_minus",				"10"			},
  { "char_period",				"10"			},
  { "char_slash",				"10"			},
  { "char_0",					"10"			},
  { "char_1",					"10"			},
  { "char_2",					"10"			},
  { "char_3",					"10"			},
  { "char_4",					"10"			},
  { "char_5",					"10"			},
  { "char_6",					"10"			},
  { "char_7",					"10"			},
  { "char_8",					"10"			},
  { "char_9",					"10"			},
  { "char_colon",				"10"			},
  { "char_semicolon",				"10"			},
  { "char_less",				"10"			},
  { "char_equal",				"10"			},
  { "char_greater",				"10"			},
  { "char_question",				"10"			},
  { "char_at",					"10"			},
  { "char_a",					"10"			},
  { "char_b",					"10"			},
  { "char_c",					"10"			},
  { "char_d",					"10"			},
  { "char_e",					"10"			},
  { "char_f",					"10"			},
  { "char_g",					"10"			},
  { "char_h",					"10"			},
  { "char_i",					"10"			},
  { "char_j",					"10"			},
  { "char_k",					"10"			},
  { "char_l",					"10"			},
  { "char_m",					"10"			},
  { "char_n",					"10"			},
  { "char_o",					"10"			},
  { "char_p",					"10"			},
  { "char_q",					"10"			},
  { "char_r",					"10"			},
  { "char_s",					"10"			},
  { "char_t",					"10"			},
  { "char_u",					"10"			},
  { "char_v",					"10"			},
  { "char_w",					"10"			},
  { "char_x",					"10"			},
  { "char_y",					"10"			},
  { "char_z",					"10"			},
  { "char_bracketleft",				"10"			},
  { "char_backslash",				"10"			},
  { "char_bracketright",			"10"			},
  { "char_asciicircum",				"10"			},
  { "char_underscore",				"10"			},
  { "char_copyright",				"10"			},
  { "char_aumlaut",				"10"			},
  { "char_oumlaut",				"10"			},
  { "char_uumlaut",				"10"			},
  { "char_degree",				"10"			},
  { "char_trademark",				"10"			},
  { "char_cursor",				"10"			},
  { "end",					""			},

  { "emerald",					"50"			},
  { "emerald.collecting",			"8"			},
  { "empty_space",				"10"			},
  { "end",					""			},

  { "diamond",					"50"			},
  { "diamond.collecting",			"8"			},
  { "empty_space",				"10"			},
  { "end",					""			},

  { "bd_diamond",				"50"			},
  { "bd_diamond.collecting",			"8"			},
  { "empty_space",				"10"			},
  { "end",					""			},

  { "emerald_yellow",				"50"			},
  { "emerald_yellow.collecting",		"8"			},
  { "empty_space",				"10"			},
  { "emerald_red",				"50"			},
  { "emerald_red.collecting",			"8"			},
  { "empty_space",				"10"			},
  { "emerald_purple",				"50"			},
  { "emerald_purple.collecting",		"8"			},
  { "empty_space",				"10"			},
  { "end",					""			},

  { "bd_rock",					"-1"			},
  { "end",					""			},

  { "bomb",					"100"			},
  { "bomb.exploding",				"16"			},
  { "empty_space",				"10"			},
  { "end",					""			},

  { "nut",					"100"			},
  { "nut.breaking",				"6"			},
  { "emerald",					"20"			},
  { "end",					""			},

  { "wall_emerald",				"100"			},
  { "wall_emerald.exploding",			"16"			},
  { "emerald",					"20"			},
  { "end",					""			},

  { "wall_diamond",				"100"			},
  { "wall_diamond.exploding",			"16"			},
  { "diamond",					"20"			},
  { "end",					""			},

  { "wall_bd_diamond",				"100"			},
  { "wall_bd_diamond.exploding",		"16"			},
  { "bd_diamond",				"20"			},
  { "end",					""			},

  { "wall_emerald_yellow",			"100"			},
  { "wall_emerald_yellow.exploding",		"16"			},
  { "emerald_yellow",				"20"			},
  { "wall_emerald_red",				"100"			},
  { "wall_emerald_red.exploding",		"16"			},
  { "emerald_red",				"20"			},
  { "wall_emerald_purple",			"100"			},
  { "wall_emerald_purple.exploding",		"16"			},
  { "emerald_purple",				"20"			},
  { "end",					""			},

  { "acid",					"-1"			},
  { "end",					""			},

  { "key_1",					"50"			},
  { "key_2",					"50"			},
  { "key_3",					"50"			},
  { "key_4",					"50"			},
  { "end",					""			},

  { "gate_1",					"50"			},
  { "gate_2",					"50"			},
  { "gate_3",					"50"			},
  { "gate_4",					"50"			},
  { "end",					""			},

  { "gate_1_gray",				"50"			},
  { "gate_2_gray",				"50"			},
  { "gate_3_gray",				"50"			},
  { "gate_4_gray",				"50"			},
  { "end",					""			},

  { "dynamite",					"-1"			},
  { "end",					""			},

  { "dynamite.active",				"96"			},
  { "dynamite.exploding",			"16"			},
  { "empty_space",				"20"			},
  { "end",					""			},

  { "dynabomb.active",				"100"			},
  { "dynabomb.exploding",			"16"			},
  { "empty_space",				"20"			},
  { "end",					""			},

  { "dynabomb_increase_number",			"-1"			},
  { "end",					""			},

  { "dynabomb_increase_size",			"-1"			},
  { "end",					""			},

  { "dynabomb_increase_power",			"-1"			},
  { "end",					""			},

  { "spaceship.turning_from_right.up",		"8"			},
  { "spaceship.turning_from_up.left",		"8"			},
  { "spaceship.turning_from_left.down",		"8"			},
  { "spaceship.turning_from_down.right",	"8"			},
  { "end",					""			},

  { "bug.turning_from_right.up",		"8"			},
  { "bug.turning_from_up.left",			"8"			},
  { "bug.turning_from_left.down",		"8"			},
  { "bug.turning_from_down.right",		"8"			},
  { "end",					""			},

  { "bd_butterfly",				"-1"			},
  { "end",					""			},

  { "bd_firefly",				"-1"			},
  { "end",					""			},

  { "pacman.right",				"16"			},
  { "pacman.up",				"16"			},
  { "pacman.left",				"16"			},
  { "pacman.down",				"16"			},
  { "end",					""			},

  { "yamyam",					"-1"			},
  { "end",					""			},

  { "dark_yamyam",				"-1"			},
  { "end",					""			},

  { "robot",					"-1"			},
  { "end",					""			},

  { "mole.moving.right",			"16"			},
  { "mole.moving.up",				"16"			},
  { "mole.moving.left",				"16"			},
  { "mole.moving.down",				"16"			},
  { "end",					""			},

  { "penguin.moving.right",			"16"			},
  { "penguin.moving.up",			"16"			},
  { "penguin.moving.left",			"16"			},
  { "penguin.moving.down",			"16"			},
  { "end",					""			},

  { "pig.moving.right",				"16"			},
  { "pig.moving.up",				"16"			},
  { "pig.moving.left",				"16"			},
  { "pig.moving.down",				"16"			},
  { "end",					""			},

  { "dragon.moving.right",			"16"			},
  { "dragon.moving.up",				"16"			},
  { "dragon.moving.left",			"16"			},
  { "dragon.moving.down",			"16"			},
  { "end",					""			},

  { "satellite",				"-1"			},
  { "end",					""			},

  { "robot_wheel",				"50"			},
  { "robot_wheel.active",			"100"			},
  { "end",					""			},

  { "lamp",					"50"			},
  { "lamp.active",				"50"			},
  { "end",					""			},

  { "time_orb_full",				"50"			},
  { "time_orb_empty",				"50"			},
  { "end",					""			},

  { "amoeba_drop",				"50"			},
  { "amoeba.growing",				"6"			},
  { "amoeba_wet",				"20"			},
  { "end",					""			},

  { "amoeba_dead",				"-1"			},
  { "end",					""			},

  { "amoeba_wet",				"-1"			},
  { "end",					""			},

  { "amoeba_wet",				"100"			},
  { "amoeba.growing",				"6"			},
  { "end",					""			},

  { "amoeba_full",				"50"			},
  { "amoeba_dead",				"50"			},
  { "amoeba.exploding",				"16"			},
  { "diamond",					"20"			},
  { "end",					""			},

  { "game_of_life",				"-1"			},
  { "end",					""			},

  { "biomaze",					"-1"			},
  { "end",					""			},

  { "magic_wall.active",			"-1"			},
  { "end",					""			},

  { "bd_magic_wall.active",			"-1"			},
  { "end",					""			},

  { "exit_closed",				"200"			},
  { "exit.opening",				"30"			},
  { "exit_open",				"100"			},
  { "exit.closing",				"30"			},
  { "end",					""			},

  { "exit_open",				"-1"			},
  { "end",					""			},

  { "sokoban_object",				"-1"			},
  { "end",					""			},

  { "sokoban_field_empty",			"-1"			},
  { "end",					""			},

  { "sokoban_field_full",			"-1"			},
  { "end",					""			},

  { "speed_pill",				"-1"			},
  { "end",					""			},

  { NULL,					NULL			}
};

struct ConfigInfo helptext_config[] =
{
  {
    "player_1",
    "THE HERO: (Is _this_ guy good old Rockford?)"
  },
  {
    "sand",
    "Normal sand: You can dig through it"
  },
  {
    "empty_space",
    "Empty field: You can walk through it"
  },
  {
    "quicksand_empty",
    "Quicksand: You cannot pass it, but rocks can fall through it"
  },
  {
    "steelwall",
    "Massive Wall: Nothing can go through it"
  },
  {
    "wall",
    "Normal Wall: You can't go through it, but you can bomb it away"
  },
  {
    "expandable_wall",
    "Growing Wall: Grows in several directions if there is an empty field"
  },
  {
    "invisible_wall",
    "Invisible Wall: Behaves like normal wall, but is invisible"
  },
  {
    "wall_slippery",
    "Old Wall: Like normal wall, but some things can fall down from it"
  },
  {
    "char_space",
    "Letter Wall: Looks like a letter, behaves like a normal wall"
  },
  {
    "emerald",
    "Emerald: You must collect enough of them to finish a level"
  },
  {
    "diamond",
    "Diamond: Counts as 3 emeralds, but can be destroyed by rocks"
  },
  {
    "bd_diamond",
    "Diamond (BD style): Counts like one emerald and behaves a bit different"
  },
  {
    "emerald_yellow",
    "Colorful Gems: Seem to behave like Emeralds"
  },
  {
    "bd_rock",
    "Rock: Smashes several things; Can be moved by the player"
  },
  {
    "bomb",
    "Bomb: You can move it, but be careful when dropping it"
  },
  {
    "nut",
    "Nut: Throw a rock on it to open it; Each nut contains an emerald"
  },
  {
    "wall_emerald",
    "Wall with an emerald inside: Bomb the wall away to get it"
  },
  {
    "wall_diamond",
    "Wall with a diamond inside: Bomb the wall away to get it"
  },
  {
    "wall_bd_diamond",
    "Wall with BD style diamond inside: Bomb the wall away to get it"
  },
  {
    "wall_emerald_yellow",
    "Wall with colorful gem inside: Bomb the wall away to get it"
  },
  {
    "acid",
    "Acid: Things that fall in are gone forever (including our hero)"
  },
  {
    "key_1",
    "Key: Opens the door that has the same color (red/yellow/green/blue)"
  },
  {
    "gate_1",
    "Door: Can be opened by the key with the same color"
  },
  {
    "gate_1_gray",
    "Door: You have to find out the right color of the key for it"
  },
  {
    "dynamite",
    "Dynamite: Collect it and use it to destroy walls or kill enemies"
  },
  {
    "dynamite.active",
    "Dynamite: This one explodes after a few seconds"
  },
  {
    "dynabomb",
    "Dyna Bomb: Explodes in 4 directions with variable explosion size"
  },
  {
    "dynabomb_increase_number",
    "Dyna Bomb: Increases the number of dyna bombs available at a time"
  },
  {
    "dynabomb_increase_size",
    "Dyna Bomb: Increases the size of explosion of dyna bombs"
  },
  {
    "dynabomb_increase_power",
    "Dyna Bomb: Increases the power of explosion of dyna bombs"
  },
  {
    "spaceship",
    "Spaceship: Moves at the left side of walls; don't touch it!"
  },
  {
    "bug",
    "Bug: Moves at the right side of walls; don't touch it!"
  },
  {
    "bd_butterfly",
    "Butterfly: Moves at the right side of walls; don't touch it!"
  },
  {
    "bd_firefly",
    "Firefly: Moves at the left side of walls; don't touch it!"
  },
  {
    "pacman",
    "Pacman: Eats the amoeba and you, if you're not careful"
  },
  {
    "yamyam",
    "Yam Yam: Eats diamonds and you, if you're not careful"
  },
  {
    "dark_yamyam",
    "Dark Yam Yam: Eats almost everything"
  },
  {
    "robot",
    "Robot: Tries to kill the player"
  },
  {
    "mole",
    "The mole: Eats the amoeba and turns empty space into normal sand"
  },
  {
    "penguin",
    "The penguin: Guide him to the exit, but keep him away from monsters!"
  },
  {
    "pig",
    "The Pig: Harmless, but eats all gems it can get"
  },
  {
    "dragon",
    "The Dragon: Breathes fire, especially to some monsters"
  },
  {
    "satellite",
    "Sonde: Follows you everywhere; harmless, but may block your way"
  },
  {
    "robot_wheel",
    "Magic Wheel: Touch it to get rid of the robots for some seconds"
  },
  {
    "lamp",
    "Light Bulb: All of them must be switched on to finish a level"
  },
  {
    "time_orb_full",
    "Extra Time Orb: Adds some seconds to the time available for the level"
  },
  {
    "amoeba_drop",
    "Amoeba Drop: Grows to an amoeba on the ground - don't touch it"
  },
  {
    "amoeba_dead",
    "Dead Amoeba: Does not grow, but can still kill bugs and spaceships"
  },
  {
    "amoeba_wet",
    "Normal Amoeba: Grows through empty fields, sand and quicksand"
  },
  {
    "amoeba_wet",
    "Dropping Amoeba: This one makes drops that grow to a new amoeba"
  },
  {
    "amoeba_full",
    "Living Amoeba (BD style): Contains other element, when surrounded"
  },
  {
    "game_of_life",
    "Game Of Life: Behaves like the well known 'Game Of Life' (2333 style)"
  },
  {
    "biomaze",
    "Biomaze: A bit like the 'Game Of Life', but builds crazy mazes"
  },
  {
    "magic_wall",
    "Magic Wall: Changes rocks, emeralds and diamonds when they pass it"
  },
  {
    "bd_magic_wall",
    "Magic Wall (BD style): Changes rocks and BD style diamonds"
  },
  {
    "exit_closed",
    "Exit door: Opens if you have enough emeralds to finish the level"
  },
  {
    "exit_open",
    "Open exit door: Enter here to leave the level and exit the actual game"
  },
  {
    "sokoban_object",
    "Sokoban element: Object which must be pushed to an empty field"
  },
  {
    "sokoban_field_empty",
    "Sokoban element: Empty field where a Sokoban object can be placed on"
  },
  {
    "sokoban_field_full",
    "Sokoban element: Field with object which can be pushed away"
  },
  {
    "speed_pill",
    "Speed pill: Lets the player run twice as fast as normally"
  },

  {
    NULL,
    NULL
  }
};
