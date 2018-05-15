/* 2000-08-20T09:41:18Z
 *
 * identify all emerald mine caves and turn them into v6 format.
 * fixes illegal tiles, acid, wheel, limits times, cleans flags.
 *
 * these tables weed out bad tiles for older caves (eg. wheel on -> wheel off)
 * and clean up v6 caves (acid, number limits) which should(!) be
 * inconsequential, but no doubt it will break some caves.
 */

#include "main_em.h"


#define ALLOW_ROLLING_SPRING

static unsigned char remap_v6[256] =
{
  /* filter crap for v6 */

  0,0,2,2,         4,4,118,118,     8,9,10,11,       12,13,14,15,
  16,16,18,18,     20,21,22,23,     24,25,26,27,     28,28,118,28,
  0,16,2,18,       36,37,37,37,     40,41,42,43,     44,45,128,128,
  128,148,148,     148,45,45,45,    148,0,57,58,     59,60,61,62,63,

#ifdef ALLOW_ROLLING_SPRING
  64,65,66,67,     68,69,69,71,     72,73,74,75,     118,75,75,75,
#else
  64,65,66,67,     68,69,69,69,     69,73,74,75,     118,75,75,75,
#endif
  75,75,75,75,     75,153,153,153,  153,153,153,153, 153,153,153,153,
  153,153,153,99,  100,68,68,68,    68,68,68,68,     68,118,118,118,
  118,118,114,115, 131,118,118,119, 120,121,122,118, 118,118,118,118,

  128,129,130,131, 132,133,134,135, 136,137,138,139, 140,141,142,143,
  144,145,146,147, 148,149,150,151, 152,153,154,155, 156,157,158,159,
  160,161,162,163, 164,165,165,118, 168,169,170,171, 172,173,174,175,
  176,177,178,179, 180,181,182,183, 184,185,186,187, 188,189,68,191,

  192,193,194,195, 196,197,198,199, 200,201,202,203, 204,205,206,207,
  208,209,210,211, 212,213,214,215, 216,217,218,219, 220,221,222,223,
  224,225,226,227, 228,229,230,231, 232,233,234,235, 236,237,238,239,
  240,241,242,243, 244,245,153,153, 153,153,153,153, 153,153,153,153
};

static unsigned char remap_v5[256] =
{
  /* filter crap for v5 */

  0,0,2,2,         4,4,118,118,     8,9,10,11,       12,13,14,15,
  16,16,18,18,     20,21,22,23,     24,25,26,27,     28,28,118,28,
  0,16,2,18,       36,37,37,37,     147,41,42,43,    44,45,128,128,
  128,148,148,148, 45,45,45,148,    0,57,58,59,      60,61,62,63,

  64,65,66,67,     68,153,153,153,  153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,68,68,68,68, 68,68,68,68,118, 118,118,
  118,118,114,115, 131,118,118,119, 120,121,122,118, 118,118,118,118,

  128,129,130,131, 132,133,134,135, 136,137,138,139, 140,141,142,143,
  144,145,146,147, 148,149,150,151, 152,153,154,155, 156,157,158,159,
  160,153,153,153, 153,153,153,118, 168,169,170,171, 172,173,174,175,
  176,177,178,179, 180,181,182,183, 184,185,186,187, 188,189,68,153,

  153,153,153,153, 153,153,153,153, 200,201,202,203, 204,205,206,207,
  208,209,210,211, 212,213,214,215, 216,217,218,219, 220,221,222,223,
  224,225,226,227, 228,229,230,231, 232,233,234,235, 236,237,238,239,
  240,241,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153
};

static unsigned char remap_v4[256] =
{
  /* filter crap for v4 */

  0,0,2,2,         4,4,118,118,     8,9,10,11,       12,13,14,15,
  16,16,18,18,     20,21,22,23,     24,25,26,27,     28,28,118,28,
  0,16,2,18,       36,37,37,37,     147,41,42,43,    44,45,128,128,
  128,148,148,148, 45,45,45,148,    0,153,153,59,    60,61,62,63,

  64,65,66,153,    153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153,
  153,118,114,115, 131,118,118,119, 120,121,122,118, 118,118,118,118,

  128,129,130,131, 132,133,134,135, 136,137,138,139, 140,141,142,143,
  144,145,146,147, 148,149,150,151, 152,68,154,155,  156,157,158,160,
  160,160,160,160, 160,160,160,160, 160,160,160,160, 160,160,160,175,
  153,153,153,153, 153,153,153,153, 153,153,153,153, 153,153,68,153,

  153,153,153,153, 153,153,153,153, 200,201,202,203, 204,205,206,207,
  208,209,210,211, 212,213,214,215, 216,217,218,219, 220,221,222,223,
  224,225,226,227, 228,229,230,231, 232,233,234,235, 236,237,238,239,
  240,241,153,153, 153,153,153,153, 153,153,153,153, 153,153,153,153
};

static unsigned char remap_v4eater[28] =
{
  /* filter crap for v4 */

  128,18,2,0,4,8,16,20,28,37,
  41,45,130,129,131,132,133,134,135,136,
  146,147,175,65,66,64,2,18
};

static boolean filename_has_v1_format(char *filename)
{
  char *basename = getBaseNamePtr(filename);

  return (strlen(basename) == 3 &&
	  basename[0] == 'a' &&
	  basename[1] >= 'a' && basename[1] <= 'k' &&
	  basename[2] >= '0' && basename[2] <= '9');
}

int cleanup_em_level(unsigned char *src, int length, char *filename)
{
  int file_version = FILE_VERSION_EM_UNKNOWN;
  int i;

  if (length >= 2172 &&
      src[2106] == 255 &&		/* version id: */
      src[2107] == 54 &&		/* '6' */
      src[2108] == 48 &&		/* '0' */
      src[2109] == 48)			/* '0' */
  {
    /* ---------- this cave has V6 file format ---------- */
    file_version = FILE_VERSION_EM_V6;

    /* remap elements to internal EMC level format */
    for (i = 0; i < 2048; i++)
      src[i] = remap_v6[src[i]];
    for (i = 2048; i < 2084; i++)
      src[i] = remap_v6[src[i]];
    for (i = 2112; i < 2148; i++)
      src[i] = remap_v6[src[i]];
  }
  else if (length >= 2110 &&
	   src[2106] == 255 &&		/* version id: */
	   src[2107] == 53 &&		/* '5' */
	   src[2108] == 48 &&		/* '0' */
	   src[2109] == 48)		/* '0' */
  {
    /* ---------- this cave has V5 file format ---------- */
    file_version = FILE_VERSION_EM_V5;

    /* remap elements to internal EMC level format */
    for (i = 0; i < 2048; i++)
      src[i] = remap_v5[src[i]];
    for (i = 2048; i < 2084; i++)
      src[i] = remap_v5[src[i]];
    for (i = 2112; i < 2148; i++)
      src[i] = src[i - 64];
  }
  else if (length >= 2106 &&
	   (src[1983] == 27 ||		/* encrypted (only EM I/II/III) */
	    src[1983] == 116 ||		/* unencrypted (usual case) */
	    src[1983] == 131))		/* unencrypted (rare case) */
  {
    /* ---------- this cave has V1, V2 or V3 file format ---------- */

    boolean fix_copyright = FALSE;

    /*
      byte at position 1983 (0x07bf) is used as "magic byte":
      - 27  (0x1b)	=> encrypted level (V3 only / Kingsoft original games)
      - 116 (0x74)	=> unencrypted level (byte is corrected to 131 (0x83))
      - 131 (0x83)	=> unencrypted level (happens only in very rare cases)
    */

    if (src[1983] == 27)	/* (0x1b) -- after decryption: 116 (0x74) */
    {
      /* this is original (encrypted) Emerald Mine I, II or III level file */

      int first_byte = src[0];
      unsigned char code0 = 0x65;
      unsigned char code1 = 0x11;

      /* decode encrypted level data */
      for (i = 0; i < 2106; i++)
      {
	src[i] ^= code0;
	src[i] -= code1;

	code0 = (code0 + 7) & 0xff;
      }

      src[1] = 131;		/* needed for all Emerald Mine levels */

      /* first byte is either 0xf1 (EM I and III) or 0xf5 (EM II) */
      if (first_byte == 0xf5)
      {
	src[0] = 131;		/* only needed for Emerald Mine II levels */

	fix_copyright = TRUE;
      }

      /* ---------- this cave has V3 file format ---------- */
      file_version = FILE_VERSION_EM_V3;
    }
    else if (filename_has_v1_format(filename))
    {
      /* ---------- this cave has V1 file format ---------- */
      file_version = FILE_VERSION_EM_V1;
    }
    else
    {
      /* ---------- this cave has V2 file format ---------- */
      file_version = FILE_VERSION_EM_V2;
    }

    /* remap elements to internal EMC level format */
    for (i = 0; i < 2048; i++)
      src[i] = remap_v4[src[i]];
    for (i = 2048; i < 2084; i++)
      src[i] = remap_v4eater[src[i] >= 28 ? 0 : src[i]];
    for (i = 2112; i < 2148; i++)
      src[i] = src[i - 64];

    if (fix_copyright)		/* fix "(c)" sign in Emerald Mine II levels */
    {
      for (i = 0; i < 2048; i++)
	if (src[i] == 241)
	  src[i] = 254;		/* replace 'Xdecor_1' with 'Xalpha_copyr' */
    }
  }
  else
  {
    /* ---------- this cave has unknown file format ---------- */

    /* if file has length of old-style level file, print (wrong) magic byte */
    if (length < 2110)
      Error(ERR_WARN, "unknown magic byte 0x%02x at position 0x%04x",
	    src[1983], 1983);

    return FILE_VERSION_EM_UNKNOWN;
  }

  if (file_version < FILE_VERSION_EM_V6)
  {
    /* id */
    src[2106] = 255;		/* version id: */
    src[2107] = 54;		/* '6' */
    src[2108] = 48;		/* '0' */
    src[2109] = 48;		/* '0' */

    /* time */
    i = src[2094] * 10;
    /* stored level time of levels for the V2 player was changed to 50% of the
       time for the V1 player (original V3 levels already considered this) */
    if (file_version != FILE_VERSION_EM_V1 &&
	file_version != FILE_VERSION_EM_V3)
      i /= 2;
    src[2110] = i >> 8;
    src[2111] = i;

    for (i = 2148; i < 2172; i++)
      src[i] = 0;

    /* ball data */
    src[2159] = 128;
  }

  /* ---------- at this stage, the cave data always has V6 format ---------- */

  /* fix wheel */
  for (i = 0; i < 2048; i++)
    if (src[i] == 40)
      break;
  for (i++; i < 2048; i++)
    if (src[i] == 40)
      src[i] = 147;

#if 0

  /* fix acid */
  for (i = 64; i < 2048; i++)
    if (src[i] == 63)		/* replace element above 'Xacid_s' ... */
      src[i - 64] = 101;	/* ... with 'Xacid_1' */

#else

#if 1
  /* fix acid */
  for (i = 64; i < 2048; i++)
    if (src[i] == 63)		/* replace element above 'Xacid_s' ... */
      src[i - 64] = 101;	/* ... with 'Xacid_1' */

  /* fix acid with no base beneath it (see below for details (*)) */
  for (i = 64; i < 2048 - 1; i++)
  {
    if (file_version <= FILE_VERSION_EM_V2 &&
	src[i - 64] == 101 && src[i] != 63)	/* acid without base */
    {
      if (src[i - 1] == 101 ||			/* remove acid over acid row */
	  src[i + 1] == 101)
	src[i - 64] = 6;	/* replace element above with 'Xblank' */
      else
	src[i - 64] = 255;	/* replace element above with 'Xfake_acid_1' */
    }
  }

#else

  /* fix acid */
  for (i = 64; i < 2048; i++)
  {
    if (src[i] == 63)		/* 'Xacid_s' (acid pool, bottom middle) */
    {
      if (file_version <= FILE_VERSION_EM_V2 &&
	  i < 2048 - 64 && src[i + 64] == 63)
      {
	int obj_left  = remap_emerald[src[i - 1]];
	int obj_right = remap_emerald[src[i + 1]];

	if (obj_left == Xblank || obj_right == Xblank ||
	    obj_left == Xplant || obj_right == Xplant)
	  src[i - 64] = 6;	/* replace element above with 'Xblank' */
	else
	  src[i - 64] = 255;	/* replace element above with 'Xfake_acid_1' */
      }
      else
      {
	src[i - 64] = 101;	/* replace element above with 'Xacid_1' */
      }
    }
  }
#endif
#endif

  /* fix acid in eater 1 */
  for (i = 2051; i < 2057; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 2 */
  for (i = 2060; i < 2066; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 3 */
  for (i = 2069; i < 2075; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 4 */
  for (i = 2078; i < 2084; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 5 */
  for (i = 2115; i < 2121; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 6 */
  for (i = 2124; i < 2130; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 7 */
  for (i = 2133; i < 2139; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* fix acid in eater 8 */
  for (i = 2142; i < 2148; i++)
    if (src[i] == 63)
      src[i - 3] = 101;

  /* old style time */
  src[2094] = 0;

  /* player 1 pos */
  src[2096] &= 7;
  src[src[2096] << 8 | src[2097]] = 128;

  /* player 2 pos */
  src[2098] &= 7;
  src[src[2098] << 8 | src[2099]] = 128;

  /* amoeba speed */
  if ((src[2100] << 8 | src[2101]) > 9999)
  {
    src[2100] = 39;
    src[2101] = 15;
  }

  /* time wonderwall */
  if ((src[2102] << 8 | src[2103]) > 9999)
  {
    src[2102] = 39;
    src[2103] = 15;
  }

  /* time */
  if ((src[2110] << 8 | src[2111]) > 9999)
  {
    src[2110] = 39;
    src[2111] = 15;
  }

  /* wind direction */
  i = src[2149];
  i &= 15;
  i &= -i;
  src[2149] = i;

  /* time lenses */
  if ((src[2154] << 8 | src[2155]) > 9999)
  {
    src[2154] = 39;
    src[2155] = 15;
  }

  /* time magnify */
  if ((src[2156] << 8 | src[2157]) > 9999)
  {
    src[2156] = 39;
    src[2157] = 15;
  }

  /* ball object */
  src[2158] = 0;
  src[2159] = remap_v6[src[2159]];

  /* ball pause */
  if ((src[2160] << 8 | src[2161]) > 9999)
  {
    src[2160] = 39;
    src[2161] = 15;
  }

  /* ball data */
  src[2162] &= 129;
  if (src[2162] & 1)
    src[2163] = 0;

  /* android move pause */
  if ((src[2164] << 8 | src[2165]) > 9999)
  {
    src[2164] = 39;
    src[2165] = 15;
  }

  /* android clone pause */
  if ((src[2166] << 8 | src[2167]) > 9999)
  {
    src[2166] = 39;
    src[2167] = 15;
  }

  /* android data */
  src[2168] &= 31;

  /* size of v6 cave */
  length = 2172;

  if (options.debug)
    printf("::: EM level file version: %d\n", file_version);

  return file_version;
}

/* 2000-07-30T00:26:00Z
 *
 * Read emerald mine caves version 6
 *
 * v4 and v5 emerald mine caves are converted to v6 (which completely supports
 * older versions)
 * 
 * converting to the internal format loses /significant/ information which can
 * break lots of caves.
 * 
 * major incompatibilities:
 * - borderless caves behave completely differently, the player no longer
 *   "warps" to the other side.
 * - a compile time option for spring can make it behave differently when it
 *   rolls.
 * - a compile time option for rolling objects (stone, nut, spring, bomb) only
 *   in eater.
 * - acid is always deadly even with no base beneath it (this breaks cave 0 in
 *   downunder mine 16)
 *   (*) fixed (see above):
 *       - downunder mine 16, level 0, works again
 *       - downunder mine 11, level 71, corrected (only cosmetically)
 *
 * so far all below have not broken any caves:
 *
 * - active wheel inside an eater will not function, eater explosions will not
 *   change settings.
 * - initial collect objects (emerald, diamond, dynamite) don't exist.
 * - initial rolling objects will be moved manually and made into sitting
 *   objects.
 * - drips always appear from dots.
 * - more than one thing can fall into acid at the same time.
 * - acid explodes when the player walks into it, rather than splashing.
 * - simultaneous explosions may be in a slightly different order.
 * - quicksand states have been reduced.
 * - acid base is effectively an indestructable wall now which can affect eater
 *   explosions.
 * - android can clone forever with a clone pause of 0 (emeralds, diamonds,
 *   nuts, stones, bombs, springs).
 *
 * 2001-03-12T02:46:55Z
 * - rolling stuff is now allowed in the cave, i didn't like making this
 *   decision.
 * - if BAD_ROLL is not defined, initial rolling objects are moved by hand.
 * - initial collect objects break some cave in elvis mine 5.
 * - different timing for wonderwall break some cave in exception mine 2.
 * - i think i'm pretty locked into always using the bad roll. *sigh*
 * - rolling spring is now turned into regular spring. it appears the emc
 *   editor only uses the force code for initially moving spring. i will
 *   follow this in my editor.
 *
 * 2006-04-02
 * - introduced ALLOW_ROLLING_SPRING; if defined, do NOT turn rolling spring
 *   into regular spring, because this breaks at least E.M.C. Mine 3, level 79
 *   (see comment directly above)
 */

static unsigned short remap_emerald[256] =
{
  Xstone,		Xstone,		Xdiamond,	Xdiamond,
  Xalien,		Xalien,		Xblank,		Xblank,
  Xtank_n,		Xtank_e,	Xtank_s,	Xtank_w,
  Xtank_gon,		Xtank_goe,	Xtank_gos,	Xtank_gow,

  Xbomb,		Xbomb,		Xemerald,	Xemerald,
  Xbug_n,		Xbug_e,		Xbug_s,		Xbug_w,
  Xbug_gon,		Xbug_goe,	Xbug_gos,	Xbug_gow,
  Xdrip_eat,		Xdrip_eat,	Xdrip_eat,	Xdrip_eat,

  Xstone,		Xbomb,		Xdiamond,	Xemerald,
  Xwonderwall,		Xnut,		Xnut,		Xnut,
  Xwheel,		Xeater_n,	Xeater_s,	Xeater_w,
  Xeater_e,		Xsand_stone,	Xblank,		Xblank,

  Xblank,		Xsand,		Xsand,		Xsand,
  Xsand_stone,		Xsand_stone,	Xsand_stone,	Xsand,
  Xstone,		Xgrow_ew,	Xgrow_ns,	Xdynamite_1,
  Xdynamite_2,		Xdynamite_3,	Xdynamite_4,	Xacid_s,

#ifdef ALLOW_ROLLING_SPRING
  Xexit_1,		Xexit_2,	Xexit_3,	Xballoon,
  Xplant,		Xspring,	Xspring_fall,	Xspring_w,
  Xspring_e,		Xball_1,	Xball_2,	Xandroid,
  Xblank,		Xandroid,	Xandroid,	Xandroid,
#else
  Xexit_1,		Xexit_2,	Xexit_3,	Xballoon,
  Xplant,		Xspring,	Xspring,	Xspring,
  Xspring,		Xball_1,	Xball_2,	Xandroid,
  Xblank,		Xandroid,	Xandroid,	Xandroid,
#endif

  Xandroid,		Xandroid,	Xandroid,	Xandroid,
  Xandroid,		Xblank,		Xblank,		Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,

#ifdef BAD_ROLL

  Xblank,		Xblank,		Xblank,		Xspring_force_w,
  Xspring_force_e,	Xacid_1,	Xacid_2,	Xacid_3,
  Xacid_4,		Xacid_5,	Xacid_6,	Xacid_7,
  Xacid_8,		Xblank,		Xblank,		Xblank,

  Xblank,		Xblank,		Xnut_force_w,	Xnut_force_e,
  Xsteel_1,		Xblank,		Xblank,		Xbomb_force_w,
  Xbomb_force_e,	Xstone_force_w,	Xstone_force_e,	Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,

#else

  Xblank,		Xblank,		Xblank,		Xspring,
  Xspring,		Xacid_1,	Xacid_2,	Xacid_3,
  Xacid_4,		Xacid_5,	Xacid_6,	Xacid_7,
  Xacid_8,		Xblank,		Xblank,		Xblank,

  Xblank,		Xblank,		Xnut,		Xnut,
  Xsteel_1,		Xblank,		Xblank,		Xbomb,
  Xbomb,		Xstone,		Xstone,		Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,

#endif

  Xblank,		Xround_wall_1,	Xgrass,		Xsteel_1,
  Xwall_1,		Xkey_1,		Xkey_2,		Xkey_3,
  Xkey_4,		Xdoor_1,	Xdoor_2,	Xdoor_3,
  Xdoor_4,		Xdripper,	Xfake_door_1,	Xfake_door_2,

  Xfake_door_3,		Xfake_door_4,	Xwonderwall,	Xwheel,
  Xsand,		Xacid_nw,	Xacid_ne,	Xacid_sw,
  Xacid_se,		Xfake_blank,	Xamoeba_1,	Xamoeba_2,
  Xamoeba_3,		Xamoeba_4,	Xexit,		Xalpha_arrow_w,

  Xfake_grass,		Xlenses,	Xmagnify,	Xfake_blank,
  Xfake_grass,		Xswitch,	Xswitch,	Xblank,
  Xdecor_8,		Xdecor_9,	Xdecor_10,	Xdecor_5,
  Xalpha_comma,		Xalpha_quote,	Xalpha_minus,	Xdynamite,

  Xsteel_3,		Xdecor_6,	Xdecor_7,	Xsteel_2,
  Xround_wall_2,	Xdecor_2,	Xdecor_4,	Xdecor_3,
  Xwind_nesw,		Xwind_e,	Xwind_s,	Xwind_w,
  Xwind_n,		Xdirt,		Xplant,		Xkey_5,

  Xkey_6,		Xkey_7,		Xkey_8,		Xdoor_5,
  Xdoor_6,		Xdoor_7,	Xdoor_8,	Xbumper,
  Xalpha_a,		Xalpha_b,	Xalpha_c,	Xalpha_d,
  Xalpha_e,		Xalpha_f,	Xalpha_g,	Xalpha_h,

  Xalpha_i,		Xalpha_j,	Xalpha_k,	Xalpha_l,
  Xalpha_m,		Xalpha_n,	Xalpha_o,	Xalpha_p,
  Xalpha_q,		Xalpha_r,	Xalpha_s,	Xalpha_t,
  Xalpha_u,		Xalpha_v,	Xalpha_w,	Xalpha_x,

  Xalpha_y,		Xalpha_z,	Xalpha_0,	Xalpha_1,
  Xalpha_2,		Xalpha_3,	Xalpha_4,	Xalpha_5,
  Xalpha_6,		Xalpha_7,	Xalpha_8,	Xalpha_9,
  Xalpha_perio,		Xalpha_excla,	Xalpha_colon,	Xalpha_quest,

  Xalpha_arrow_e,	Xdecor_1,	Xfake_door_5,	Xfake_door_6,
  Xfake_door_7,		Xfake_door_8,	Xblank,		Xblank,
  Xblank,		Xblank,		Xblank,		Xblank,
#if 0
  Xblank,		Xblank,		Xblank,		Xblank,
#else
  /* special elements added to solve compatibility problems */
  Xblank,		Xblank,		Xalpha_copyr,	Xfake_acid_1
#endif
};

static int get_em_element(unsigned short em_element_raw, int file_version)
{
  int em_element = remap_emerald[em_element_raw];

  if (file_version < FILE_VERSION_EM_V5)
  {
    /* versions below V5 had no grass, but only sand/dirt */
    if (em_element == Xgrass)
      em_element = Xdirt;
  }

  return em_element;
}

void convert_em_level(unsigned char *src, int file_version)
{
  static int eater_offset[8] =
  {
    0x800, 0x809, 0x812, 0x81B, 0x840, 0x849, 0x852, 0x85B
  };
  int i, x, y, temp;

#if 1
  lev.time_seconds = src[0x83E] << 8 | src[0x83F];
  if (lev.time_seconds > 9999)
    lev.time_seconds = 9999;
#else
  temp = ((src[0x83E] << 8 | src[0x83F]) * 25 + 3) / 4;
  if (temp == 0 || temp > 9999)
    temp = 9999;
  lev.time_initial = temp;
#endif

  lev.required_initial = src[0x82F];

  for (i = 0; i < 2; i++)
  {
    temp = src[0x830 + i * 2] << 8 | src[0x831 + i * 2];
    ply[i].x_initial = (temp & 63) + 1;
    ply[i].y_initial = (temp >> 6 & 31) + 1;
  }

  temp = (src[0x834] << 8 | src[0x835]) * 28;
  if (temp > 9999)
    temp = 9999;
  lev.amoeba_time = temp;

  lev.android_move_time = src[0x874] << 8 | src[0x875];
  lev.android_clone_time = src[0x876] << 8 | src[0x877];

  lev.ball_random = src[0x872] & 1 ? 1 : 0;
  lev.ball_state_initial = src[0x872] & 128 ? 1 : 0;
  lev.ball_time = src[0x870] << 8 | src[0x871];

  lev.emerald_score = src[0x824];
  lev.diamond_score = src[0x825];
  lev.alien_score = src[0x826];
  lev.tank_score = src[0x827];
  lev.bug_score = src[0x828];
  lev.eater_score = src[0x829];
  lev.nut_score = src[0x82A];
  lev.dynamite_score = src[0x82B];
  lev.key_score = src[0x82C];
  lev.exit_score = src[0x82D] * 8 / 5;
  lev.lenses_score = src[0x867];
  lev.magnify_score = src[0x868];
  lev.slurp_score = src[0x869];

  lev.lenses_time = src[0x86A] << 8 | src[0x86B];
  lev.magnify_time = src[0x86C] << 8 | src[0x86D];
  lev.wheel_time = src[0x838] << 8 | src[0x839];

  lev.wind_cnt_initial = src[0x865] & 15 ? lev.wind_time : 0;
  temp = src[0x865];
  lev.wind_direction_initial = (temp & 8 ? 0 :
				temp & 1 ? 1 :
				temp & 2 ? 2 :
				temp & 4 ? 3 : 0);

  lev.wonderwall_time_initial = src[0x836] << 8 | src[0x837];

  for (i = 0; i < 8; i++)
    for (x = 0; x < 9; x++)
      lev.eater_array[i][x] =
	get_em_element(src[eater_offset[i] + x], file_version);

  temp = get_em_element(src[0x86F], file_version);
  for (y = 0; y < 8; y++)
  {
    if (src[0x872] & 1)
    {
      for (x = 0; x < 8; x++)
	lev.ball_array[y][x] = temp;
    }
    else
    {
      lev.ball_array[y][1] = (src[0x873] & 1)  ? temp : Xblank; /* north */
      lev.ball_array[y][6] = (src[0x873] & 2)  ? temp : Xblank; /* south */
      lev.ball_array[y][3] = (src[0x873] & 4)  ? temp : Xblank; /* west */
      lev.ball_array[y][4] = (src[0x873] & 8)  ? temp : Xblank; /* east */
      lev.ball_array[y][7] = (src[0x873] & 16) ? temp : Xblank; /* southeast */
      lev.ball_array[y][5] = (src[0x873] & 32) ? temp : Xblank; /* southwest */
      lev.ball_array[y][2] = (src[0x873] & 64) ? temp : Xblank; /* northeast */
      lev.ball_array[y][0] = (src[0x873] & 128)? temp : Xblank; /* northwest */
    }
  }

  temp = src[0x878] << 8 | src[0x879];

  if (temp & 1)
  {
    lev.android_array[Xemerald]		= Xemerald;
    lev.android_array[Xemerald_pause]	= Xemerald;
    lev.android_array[Xemerald_fall]	= Xemerald;
    lev.android_array[Yemerald_sB]	= Xemerald;
    lev.android_array[Yemerald_eB]	= Xemerald;
    lev.android_array[Yemerald_wB]	= Xemerald;
  }

  if (temp & 2)
  {
    lev.android_array[Xdiamond]		= Xdiamond;
    lev.android_array[Xdiamond_pause]	= Xdiamond;
    lev.android_array[Xdiamond_fall]	= Xdiamond;
    lev.android_array[Ydiamond_sB]	= Xdiamond;
    lev.android_array[Ydiamond_eB]	= Xdiamond;
    lev.android_array[Ydiamond_wB]	= Xdiamond;
  }

  if (temp & 4)
  {
    lev.android_array[Xstone]		= Xstone;
    lev.android_array[Xstone_pause]	= Xstone;
    lev.android_array[Xstone_fall]	= Xstone;
    lev.android_array[Ystone_sB]	= Xstone;
    lev.android_array[Ystone_eB]	= Xstone;
    lev.android_array[Ystone_wB]	= Xstone;
  }

  if (temp & 8)
  {
    lev.android_array[Xbomb]		= Xbomb;
    lev.android_array[Xbomb_pause]	= Xbomb;
    lev.android_array[Xbomb_fall]	= Xbomb;
    lev.android_array[Ybomb_sB]		= Xbomb;
    lev.android_array[Ybomb_eB]		= Xbomb;
    lev.android_array[Ybomb_wB]		= Xbomb;
  }

  if (temp & 16)
  {
    lev.android_array[Xnut]		= Xnut;
    lev.android_array[Xnut_pause]	= Xnut;
    lev.android_array[Xnut_fall]	= Xnut;
    lev.android_array[Ynut_sB]		= Xnut;
    lev.android_array[Ynut_eB]		= Xnut;
    lev.android_array[Ynut_wB]		= Xnut;
  }

  if (temp & 32)
  {
    lev.android_array[Xtank_n]		= Xtank_n;
    lev.android_array[Xtank_gon]	= Xtank_n;
    lev.android_array[Ytank_nB]		= Xtank_n;
    lev.android_array[Ytank_n_e]	= Xtank_n;
    lev.android_array[Ytank_n_w]	= Xtank_n;

    lev.android_array[Xtank_e]		= Xtank_e;
    lev.android_array[Xtank_goe]	= Xtank_e;
    lev.android_array[Ytank_eB]		= Xtank_e;
    lev.android_array[Ytank_e_s]	= Xtank_e;
    lev.android_array[Ytank_e_n]	= Xtank_e;

    lev.android_array[Xtank_s]		= Xtank_s;
    lev.android_array[Xtank_gos]	= Xtank_s;
    lev.android_array[Ytank_sB]		= Xtank_s;
    lev.android_array[Ytank_s_w]	= Xtank_s;
    lev.android_array[Ytank_s_e]	= Xtank_s;

    lev.android_array[Xtank_w]		= Xtank_w;
    lev.android_array[Xtank_gow]	= Xtank_w;
    lev.android_array[Ytank_wB]		= Xtank_w;
    lev.android_array[Ytank_w_n]	= Xtank_w;
    lev.android_array[Ytank_w_s]	= Xtank_w;
  }

  if (temp & 64)
  {
    lev.android_array[Xeater_n]		= Xeater_n;
    lev.android_array[Yeater_nB]	= Xeater_n;

    lev.android_array[Xeater_e]		= Xeater_e;
    lev.android_array[Yeater_eB]	= Xeater_e;

    lev.android_array[Xeater_s]		= Xeater_s;
    lev.android_array[Yeater_sB]	= Xeater_s;

    lev.android_array[Xeater_w]		= Xeater_w;
    lev.android_array[Yeater_wB]	= Xeater_w;
  }

  if (temp & 128)
  {
    lev.android_array[Xbug_n]		= Xbug_gon;
    lev.android_array[Xbug_gon]		= Xbug_gon;
    lev.android_array[Ybug_nB]		= Xbug_gon;
    lev.android_array[Ybug_n_e]		= Xbug_gon;
    lev.android_array[Ybug_n_w]		= Xbug_gon;

    lev.android_array[Xbug_e]		= Xbug_goe;
    lev.android_array[Xbug_goe]		= Xbug_goe;
    lev.android_array[Ybug_eB]		= Xbug_goe;
    lev.android_array[Ybug_e_s]		= Xbug_goe;
    lev.android_array[Ybug_e_n]		= Xbug_goe;

    lev.android_array[Xbug_s]		= Xbug_gos;
    lev.android_array[Xbug_gos]		= Xbug_gos;
    lev.android_array[Ybug_sB]		= Xbug_gos;
    lev.android_array[Ybug_s_w]		= Xbug_gos;
    lev.android_array[Ybug_s_e]		= Xbug_gos;

    lev.android_array[Xbug_w]		= Xbug_gow;
    lev.android_array[Xbug_gow]		= Xbug_gow;
    lev.android_array[Ybug_wB]		= Xbug_gow;
    lev.android_array[Ybug_w_n]		= Xbug_gow;
    lev.android_array[Ybug_w_s]		= Xbug_gow;
  }

  if (temp & 256)
  {
    lev.android_array[Xalien]		= Xalien;
    lev.android_array[Xalien_pause]	= Xalien;
    lev.android_array[Yalien_nB]	= Xalien;
    lev.android_array[Yalien_eB]	= Xalien;
    lev.android_array[Yalien_sB]	= Xalien;
    lev.android_array[Yalien_wB]	= Xalien;
  }

  if (temp & 512)
  {
    lev.android_array[Xspring]		= Xspring;
    lev.android_array[Xspring_pause]	= Xspring;
    lev.android_array[Xspring_e]	= Xspring;
    lev.android_array[Yspring_eB]	= Xspring;
    lev.android_array[Yspring_kill_eB]	= Xspring;
    lev.android_array[Xspring_w]	= Xspring;
    lev.android_array[Yspring_wB]	= Xspring;
    lev.android_array[Yspring_kill_wB]	= Xspring;
    lev.android_array[Xspring_fall]	= Xspring;
    lev.android_array[Yspring_sB]	= Xspring;
  }

  if (temp & 1024)
  {
    lev.android_array[Yballoon_nB]	= Xballoon;
    lev.android_array[Yballoon_eB]	= Xballoon;
    lev.android_array[Yballoon_sB]	= Xballoon;
    lev.android_array[Yballoon_wB]	= Xballoon;
    lev.android_array[Xballoon]		= Xballoon;
  }

  if (temp & 2048)
  {
    lev.android_array[Xdripper]		= Xdrip_eat;
    lev.android_array[XdripperB]	= Xdrip_eat;
    lev.android_array[Xamoeba_1]	= Xdrip_eat;
    lev.android_array[Xamoeba_2]	= Xdrip_eat;
    lev.android_array[Xamoeba_3]	= Xdrip_eat;
    lev.android_array[Xamoeba_4]	= Xdrip_eat;
    lev.android_array[Xamoeba_5]	= Xdrip_eat;
    lev.android_array[Xamoeba_6]	= Xdrip_eat;
    lev.android_array[Xamoeba_7]	= Xdrip_eat;
    lev.android_array[Xamoeba_8]	= Xdrip_eat;
  }

  if (temp & 4096)
  {
    lev.android_array[Xdynamite]	= Xdynamite;
  }

  for (temp = 1; temp < 2047; temp++)
  {
    switch (src[temp])
    {
      case 0x24:				/* wonderwall */
	lev.wonderwall_state_initial = 1;
	lev.wonderwall_time_initial = 9999;
	break;

      case 0x28:				/* wheel */
	lev.wheel_x_initial = temp & 63;
	lev.wheel_y_initial = temp >> 6;
	lev.wheel_cnt_initial = lev.wheel_time;
	break;

#ifndef BAD_ROLL
      case 0x63:				/* spring roll left */
	src[temp - 1] = 0x45;
	src[temp] = 0x80;
	break;

      case 0x64:				/* spring roll right */
	src[temp + 1] = 0x45;
	src[temp] = 0x80;
	break;

      case 0x72:				/* nut roll left */
	src[temp - 1] = 0x25;
	src[temp] = 0x80;
	break;

      case 0x73:				/* nut roll right */
	src[temp + 1] = 0x25;
	src[temp] = 0x80;
	break;

      case 0x77:				/* bomb roll left */
	src[temp - 1] = 0x10;
	src[temp] = 0x80;
	break;

      case 0x78:				/* bomb roll right */
	src[temp + 1] = 0x10;
	src[temp] = 0x80;
	break;

      case 0x79:				/* stone roll left */
	src[temp - 1] = 0x00;
	src[temp] = 0x80;
	break;

      case 0x7A:				/* stone roll right */
	src[temp + 1] = 0x00;
	src[temp] = 0x80;
	break;
#endif

      case 0xA3:				/* fake blank */
	lev.lenses_cnt_initial = 9999;
	break;

      case 0xA4:				/* fake grass */
	lev.magnify_cnt_initial = 9999;
	break;
    }
  }

  /* first fill the complete playfield with the default border element */
  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      native_em_level.cave[x][y] = ZBORDER;

  /* then copy the real level contents from level file into the playfield */
  temp = 0;
  for (y = 0; y < lev.height; y++)
    for (x = 0; x < lev.width; x++)
      native_em_level.cave[x + 1][y + 1] =
	get_em_element(src[temp++], file_version);

  /* at last, set the two players at their positions in the playfield */
  /* (native EM[C] levels always have exactly two players in a level) */
  for (i = 0; i < 2; i++)
    native_em_level.cave[ply[i].x_initial][ply[i].y_initial] = Zplayer;

  native_em_level.file_version = file_version;
}

void prepare_em_level(void)
{
  int i, x, y;
  int players_left;
  boolean team_mode;

  /* reset all runtime variables to their initial values */

  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Cave[y][x] = native_em_level.cave[x][y];

  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Next[y][x] = Cave[y][x];

  for (y = 0; y < HEIGHT; y++)
    for (x = 0; x < WIDTH; x++)
      Draw[y][x] = Cave[y][x];

  lev.time_initial = lev.time_seconds;
  lev.time = lev.time_initial;

  lev.required = lev.required_initial;
  lev.score = 0;

  lev.android_move_cnt  = lev.android_move_time;
  lev.android_clone_cnt = lev.android_clone_time;

  lev.ball_pos = 0;
  lev.ball_state = lev.ball_state_initial;
  lev.ball_cnt = lev.ball_time;

  lev.eater_pos = 0;
  lev.shine_cnt = 0;

  lev.lenses_cnt = lev.lenses_cnt_initial;
  lev.magnify_cnt = lev.magnify_cnt_initial;

  lev.wheel_cnt = lev.wheel_cnt_initial;
  lev.wheel_x   = lev.wheel_x_initial;
  lev.wheel_y   = lev.wheel_y_initial;

  lev.wind_direction = lev.wind_direction_initial;
  lev.wind_cnt       = lev.wind_cnt_initial;

  lev.wonderwall_state = lev.wonderwall_state_initial;
  lev.wonderwall_time  = lev.wonderwall_time_initial;

  lev.killed_out_of_time = FALSE;

  /* determine number of players in this level */
  lev.home_initial = 0;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].exists = 0;
    ply[i].alive_initial = FALSE;

    if (ply[i].x_initial > 0 && ply[i].y_initial > 0)
    {
      ply[i].exists = 1;

      lev.home_initial++;
    }
  }

  team_mode = getTeamMode_EM();

  if (!team_mode)
    lev.home_initial = 1;

  lev.home = lev.home_initial;
  players_left = lev.home_initial;

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    if (ply[i].exists)
    {
      if (players_left)
      {
	ply[i].alive_initial = TRUE;
	players_left--;
      }
      else
      {
	int x = ply[i].x_initial;
	int y = ply[i].y_initial;

	native_em_level.cave[x][y] = Xblank;

	Cave[y][x] = Next[y][x] = Draw[y][x] = Xblank;
      }
    }
  }

  for (i = 0; i < MAX_PLAYERS; i++)
  {
    ply[i].num = i;
    ply[i].alive = ply[i].alive_initial;
    ply[i].dynamite = 0;
    ply[i].dynamite_cnt = 0;
    ply[i].keys = 0;
    ply[i].anim = 0;
    ply[i].oldx = ply[i].x = ply[i].x_initial;
    ply[i].oldy = ply[i].y = ply[i].y_initial;
    ply[i].last_move_dir = MV_NONE;
    ply[i].joy_n = ply[i].joy_e = ply[i].joy_s = ply[i].joy_w = 0;
    ply[i].joy_snap  = ply[i].joy_drop = 0;
    ply[i].joy_stick = ply[i].joy_spin = 0;
  }

  // the following engine variables are initialized to version-specific values
  // in function InitGameEngine() (src/game.c):
  //
  // - game_em.use_single_button (default: TRUE)
  // - game_em.use_snap_key_bug (default: FALSE)

  game_em.any_player_moving = FALSE;
  game_em.any_player_snapping = FALSE;

  game_em.last_moving_player = 0;	/* default: first player */

  for (i = 0; i < MAX_PLAYERS; i++)
    game_em.last_player_direction[i] = MV_NONE;

  lev.exit_x = lev.exit_y = -1;	/* kludge for playing player exit sound */
}
