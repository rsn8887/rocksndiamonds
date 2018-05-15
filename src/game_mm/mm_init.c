// ============================================================================
// Mirror Magic -- McDuffin's Revenge
// ----------------------------------------------------------------------------
// (c) 1994-2017 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// mm_init.c
// ============================================================================

#include "main_mm.h"

#include "mm_main.h"


struct EngineSnapshotInfo_MM engine_snapshot_mm;


unsigned int InitEngineRandom_MM(int seed)
{
  return InitEngineRandom(seed);
}

void InitElementProperties_MM()
{
  int i,j;

  static int ep_grid[] =
  {
    EL_GRID_STEEL_00,
    EL_GRID_STEEL_01,
    EL_GRID_STEEL_02,
    EL_GRID_STEEL_03,
    EL_GRID_WOOD_00,
    EL_GRID_WOOD_01,
    EL_GRID_WOOD_02,
    EL_GRID_WOOD_03,
  };
  static int ep_grid_num = sizeof(ep_grid) / sizeof(int);

  static int ep_mcduffin[] =
  {
    EL_MCDUFFIN_RIGHT,
    EL_MCDUFFIN_UP,
    EL_MCDUFFIN_LEFT,
    EL_MCDUFFIN_DOWN,
  };
  static int ep_mcduffin_num = sizeof(ep_mcduffin) / sizeof(int);

  static int ep_rectangle[] =
  {
    EL_EXIT_CLOSED,
    EL_EXIT_OPENING_1,
    EL_EXIT_OPENING_2,
    EL_EXIT_OPEN,
    EL_BLOCK_STONE,
    EL_BLOCK_WOOD,
    EL_GATE_STONE,
    EL_GATE_WOOD
  };
  static int ep_rectangle_num = sizeof(ep_rectangle) / sizeof(int);

  static int ep_mirror[] =
  {
    EL_MIRROR_00,
    EL_MIRROR_01,
    EL_MIRROR_02,
    EL_MIRROR_03,
    EL_MIRROR_04,
    EL_MIRROR_05,
    EL_MIRROR_06,
    EL_MIRROR_07,
    EL_MIRROR_08,
    EL_MIRROR_09,
    EL_MIRROR_10,
    EL_MIRROR_11,
    EL_MIRROR_12,
    EL_MIRROR_13,
    EL_MIRROR_14,
    EL_MIRROR_15,
  };
  static int ep_mirror_num = sizeof(ep_mirror) / sizeof(int);

  static int ep_mirror_fixed[] =
  {
    EL_MIRROR_FIXED_00,
    EL_MIRROR_FIXED_01,
    EL_MIRROR_FIXED_02,
    EL_MIRROR_FIXED_03,
  };
  static int ep_mirror_fixed_num = sizeof(ep_mirror_fixed) / sizeof(int);

  static int ep_polar[] =
  {
    EL_POLAR_00,
    EL_POLAR_01,
    EL_POLAR_02,
    EL_POLAR_03,
    EL_POLAR_04,
    EL_POLAR_05,
    EL_POLAR_06,
    EL_POLAR_07,
    EL_POLAR_08,
    EL_POLAR_09,
    EL_POLAR_10,
    EL_POLAR_11,
    EL_POLAR_12,
    EL_POLAR_13,
    EL_POLAR_14,
    EL_POLAR_15,
  };
  static int ep_polar_num = sizeof(ep_polar) / sizeof(int);

  static int ep_polar_cross[] =
  {
    EL_POLAR_CROSS_00,
    EL_POLAR_CROSS_01,
    EL_POLAR_CROSS_02,
    EL_POLAR_CROSS_03,
  };
  static int ep_polar_cross_num = sizeof(ep_polar_cross) / sizeof(int);

  static int ep_beamer[] =
  {
    EL_BEAMER_00,
    EL_BEAMER_01,
    EL_BEAMER_02,
    EL_BEAMER_03,
    EL_BEAMER_04,
    EL_BEAMER_05,
    EL_BEAMER_06,
    EL_BEAMER_07,
    EL_BEAMER_08,
    EL_BEAMER_09,
    EL_BEAMER_10,
    EL_BEAMER_11,
    EL_BEAMER_12,
    EL_BEAMER_13,
    EL_BEAMER_14,
    EL_BEAMER_15,
  };
  static int ep_beamer_num = sizeof(ep_beamer) / sizeof(int);

  static int ep_reflecting[] =
  {
  };
  static int ep_reflecting_num = sizeof(ep_reflecting) / sizeof(int);

  static int ep_absorbing[] =
  {
  };
  static int ep_absorbing_num = sizeof(ep_absorbing) / sizeof(int);

  static int ep_inactive[] =
  {
  };
  static int ep_inactive_num = sizeof(ep_inactive) / sizeof(int);

  static int ep_wall[] =
  {
  };
  static int ep_wall_num = sizeof(ep_wall) / sizeof(int);

  static int ep_pacman[] =
  {
    EL_PACMAN_RIGHT,
    EL_PACMAN_UP,
    EL_PACMAN_LEFT,
    EL_PACMAN_DOWN,
  };
  static int ep_pacman_num = sizeof(ep_pacman) / sizeof(int);

  static long ep_bit[] =
  {
    EP_BIT_GRID,
    EP_BIT_MCDUFFIN,
    EP_BIT_RECTANGLE,
    EP_BIT_MIRROR,
    EP_BIT_MIRROR_FIXED,
    EP_BIT_POLAR,
    EP_BIT_POLAR_CROSS,
    EP_BIT_BEAMER,
    EP_BIT_REFLECTING,
    EP_BIT_ABSORBING,
    EP_BIT_INACTIVE,
    EP_BIT_WALL,
    EP_BIT_PACMAN,
  };
  static int *ep_array[] =
  {
    ep_grid,
    ep_mcduffin,
    ep_rectangle,
    ep_mirror,
    ep_mirror_fixed,
    ep_polar,
    ep_polar_cross,
    ep_beamer,
    ep_reflecting,
    ep_absorbing,
    ep_inactive,
    ep_wall,
    ep_pacman,
  };
  static int *ep_num[] =
  {
    &ep_grid_num,
    &ep_mcduffin_num,
    &ep_rectangle_num,
    &ep_mirror_num,
    &ep_mirror_fixed_num,
    &ep_polar_num,
    &ep_polar_cross_num,
    &ep_beamer_num,
    &ep_reflecting_num,
    &ep_absorbing_num,
    &ep_inactive_num,
    &ep_wall_num,
    &ep_pacman_num,
  };
  static int num_properties = sizeof(ep_num) / sizeof(int *);

  for(i = 0; i < MAX_ELEMENTS; i++)
    Elementeigenschaften[i] = 0;

  for(i = 0; i < num_properties; i++)
    for(j = 0; j < *(ep_num[i]); j++)
      Elementeigenschaften[(ep_array[i])[j]] |= ep_bit[i];

  for(i = EL_CHAR_START; i <= EL_CHAR_END; i++)
    Elementeigenschaften[i] |= (EP_BIT_CHAR | EP_BIT_INACTIVE);

  for(i = EL_WALL_START; i <= EL_WALL_END; i++)
    Elementeigenschaften[i] |= EP_BIT_WALL;
}

void mm_open_all()
{
  InitElementProperties_MM();
}

void mm_close_all()
{
}
