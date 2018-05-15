
#include "main_sp.h"
#include "global.h"


/* ------------------------------------------------------------------------- */
/* functions for loading Supaplex level                                      */
/* ------------------------------------------------------------------------- */

void setTapeInfoToDefaults_SP()
{
  native_sp_level.demo.is_available = FALSE;
  native_sp_level.demo.length = 0;
}

void setLevelInfoToDefaults_SP()
{
  LevelInfoType *header = &native_sp_level.header;
  char *empty_title = "-------- EMPTY --------";
  int i, x, y;

  native_sp_level.game_sp = &game_sp;

  native_sp_level.width  = SP_STD_PLAYFIELD_WIDTH;
  native_sp_level.height = SP_STD_PLAYFIELD_HEIGHT;

  for (x = 0; x < native_sp_level.width; x++)
    for (y = 0; y < native_sp_level.height; y++)
      native_sp_level.playfield[x][y] = fiSpace;

  /* copy string (without terminating '\0' character!) */
  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    header->LevelTitle[i] = empty_title[i];

  header->InitialGravity = 0;
  header->Version = 0;
  header->InitialFreezeZonks = 0;
  header->InfotronsNeeded = 0;
  header->SpecialPortCount = 0;
  header->SpeedByte = 0;
  header->CheckSumByte = 0;
  header->DemoRandomSeed = 0;

  for (i = 0; i < SP_MAX_SPECIAL_PORTS; i++)
  {
    SpecialPortType *port = &header->SpecialPort[i];

    port->PortLocation = 0;
    port->Gravity = 0;
    port->FreezeZonks = 0;
    port->FreezeEnemies = 0;
  }

  /* set raw header bytes (used for subsequent buffer zone) to "hardware" */
  for (i = 0; i < SP_HEADER_SIZE; i++)
    native_sp_level.header_raw_bytes[i] = 0x20;

  setTapeInfoToDefaults_SP();
}

void copyInternalEngineVars_SP()
{
  int count;
  int i, x, y;

  LInfo = native_sp_level.header;

  FieldWidth  = native_sp_level.width;
  FieldHeight = native_sp_level.height;
  HeaderSize = 96;

  FieldMax = (FieldWidth * FieldHeight) + HeaderSize - 1;
  LevelMax = (FieldWidth * FieldHeight) - 1;

  /* initialize preceding playfield buffer */
  for (i = -game_sp.preceding_buffer_size; i < 0; i++)
    PlayField16[i] = 0;

  /* initialize preceding playfield buffer */
  for (i = -SP_MAX_PLAYFIELD_WIDTH; i < 0; i++)
    PlayField8[i] = 0;

  count = 0;
  for (i = 0; game_sp.preceding_buffer[i] != NULL; i++)
  {
    char *s = game_sp.preceding_buffer[i];
    boolean hi_byte = FALSE;	/* little endian data => start with low byte */

    while (s[0] != '\0' && s[1] != '\0')
    {
      int hi_nibble = s[0] - (s[0] > '9' ? 'a' - 10 : '0');
      int lo_nibble = s[1] - (s[1] > '9' ? 'a' - 10 : '0');
      int byte = (hi_nibble << 4) | lo_nibble;

      if (hi_byte)
	byte <<= 8;

      PlayField16[-game_sp.preceding_buffer_size + count] |= byte;

      if (hi_byte)
	count++;

      hi_byte = !hi_byte;

      s += 2;

      while (*s == ' ')
	s++;
    }
  }

  count = 0;
  for (y = 0; y < native_sp_level.height; y++)
    for (x = 0; x < native_sp_level.width; x++)
      PlayField8[count++] = native_sp_level.playfield[x][y];

  /* add raw header bytes to subsequent playfield buffer zone */
  for (i = 0; i < SP_HEADER_SIZE; i++)
    PlayField8[count++] = native_sp_level.header_raw_bytes[i];

  for (i = 0; i < count; i++)
  {
    PlayField16[i] = PlayField8[i];
    DisPlayField[i] = PlayField8[i];
    PlayField8[i] = 0;
  }

  if (native_sp_level.demo.is_available)
    DemoAvailable = True;

  GravityFlag = LInfo.InitialGravity;
  FreezeZonks = LInfo.InitialFreezeZonks;

  LevelLoaded = True;

  /* random seed set by main game tape code to native random generator seed */
}

static void LoadNativeLevelFromFileStream_SP(File *file, int width, int height,
					     boolean demo_available)
{
  LevelInfoType *header = &native_sp_level.header;
  int i, x, y;

  /* for details of the Supaplex level format, see Herman Perk's Supaplex
     documentation file "SPFIX63.DOC" from his Supaplex "SpeedFix" package */

  native_sp_level.width  = MIN(width,  SP_MAX_PLAYFIELD_WIDTH);
  native_sp_level.height = MIN(height, SP_MAX_PLAYFIELD_HEIGHT);

  /* read level playfield (width * height == 60 * 24 tiles == 1440 bytes) */
  /* (MPX levels may have non-standard playfield size -- check max. size) */
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      byte element = getFile8Bit(file);

      if (x < SP_MAX_PLAYFIELD_WIDTH &&
	  y < SP_MAX_PLAYFIELD_HEIGHT)
	native_sp_level.playfield[x][y] = element;
    }
  }

  /* read level header (96 bytes) */

  ReadUnusedBytesFromFile(file, 4);	/* (not used by Supaplex engine) */

  /* initial gravity: 1 == "on", anything else (0) == "off" */
  header->InitialGravity = getFile8Bit(file);

  /* SpeedFixVersion XOR 0x20 */
  header->Version = getFile8Bit(file);

  /* level title in uppercase letters, padded with dashes ("-") (23 bytes) */
  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    header->LevelTitle[i] = getFile8Bit(file);

  /* initial "freeze zonks": 2 == "on", anything else (0, 1) == "off" */
  header->InitialFreezeZonks = getFile8Bit(file);

  /* number of infotrons needed; 0 means that Supaplex will count the total
     amount of infotrons in the level and use the low byte of that number
     (a multiple of 256 infotrons will result in "0 infotrons needed"!) */
  header->InfotronsNeeded = getFile8Bit(file);

  /* number of special ("gravity") port entries below (maximum 10 allowed) */
  header->SpecialPortCount = getFile8Bit(file);

  /* database of properties of up to 10 special ports (6 bytes per port) */
  for (i = 0; i < SP_MAX_SPECIAL_PORTS; i++)
  {
    SpecialPortType *port = &header->SpecialPort[i];

    /* high and low byte of the location of a special port; if (x, y) are the
       coordinates of a port in the field and (0, 0) is the top-left corner,
       the 16 bit value here calculates as 2 * (x + (y * 60)) (this is twice
       of what may be expected: Supaplex works with a game field in memory
       which is 2 bytes per tile) */
    port->PortLocation = getFile16BitBE(file);		/* yes, big endian */

    /* change gravity: 1 == "turn on", anything else (0) == "turn off" */
    port->Gravity = getFile8Bit(file);

    /* "freeze zonks": 2 == "turn on", anything else (0, 1) == "turn off" */
    port->FreezeZonks = getFile8Bit(file);

    /* "freeze enemies": 1 == "turn on", anything else (0) == "turn off" */
    port->FreezeEnemies = getFile8Bit(file);

    ReadUnusedBytesFromFile(file, 1);	/* (not used by Supaplex engine) */
  }

  /* SpeedByte XOR Highbyte(RandomSeed) */
  header->SpeedByte = getFile8Bit(file);

  /* CheckSum XOR SpeedByte */
  header->CheckSumByte = getFile8Bit(file);

  /* random seed used for recorded demos */
  header->DemoRandomSeed = getFile16BitLE(file);	/* yes, little endian */

  /* auto-determine number of infotrons if it was stored as "0" -- see above */
  if (header->InfotronsNeeded == 0)
  {
    for (x = 0; x < native_sp_level.width; x++)
      for (y = 0; y < native_sp_level.height; y++)
	if (native_sp_level.playfield[x][y] == fiInfotron)
	  header->InfotronsNeeded++;

    header->InfotronsNeeded &= 0xff;	/* only use low byte -- see above */
  }

  /* read raw level header bytes (96 bytes) */

  seekFile(file, -(SP_HEADER_SIZE), SEEK_CUR);	/* rewind file */
  for (i = 0; i < SP_HEADER_SIZE; i++)
    native_sp_level.header_raw_bytes[i] = getByteFromFile(file);

  /* also load demo tape, if available (only in single level files) */

  if (demo_available)
  {
    int level_nr = getFile8Bit(file);

    level_nr &= 0x7f;			/* clear highest bit */
    level_nr = (level_nr < 1   ? 1   :
		level_nr > 111 ? 111 : level_nr);

    native_sp_level.demo.level_nr = level_nr;

    for (i = 0; i < SP_MAX_TAPE_LEN && !checkEndOfFile(file); i++)
    {
      native_sp_level.demo.data[i] = getFile8Bit(file);

      if (native_sp_level.demo.data[i] == 0xff)	/* "end of demo" byte */
	break;
    }

    if (i >= SP_MAX_TAPE_LEN)
      Error(ERR_WARN, "SP demo truncated: size exceeds maximum SP demo size %d",
	    SP_MAX_TAPE_LEN);

    native_sp_level.demo.length = i;
    native_sp_level.demo.is_available = (native_sp_level.demo.length > 0);
  }
}

boolean LoadNativeLevel_SP(char *filename, int level_pos,
			   boolean level_info_only)
{
  File *file;
  int i, l, x, y;
  char name_first, name_last;
  struct LevelInfo_SP multipart_level;
  int multipart_xpos, multipart_ypos;
  boolean is_multipart_level;
  boolean is_first_part;
  boolean reading_multipart_level = FALSE;
  boolean use_empty_level = FALSE;
  LevelInfoType *header = &native_sp_level.header;
  boolean is_single_level_file = (strSuffixLower(filename, ".sp") ||
				  strSuffixLower(filename, ".mpx"));
  boolean demo_available = is_single_level_file;
  boolean is_mpx_file = strSuffixLower(filename, ".mpx");
  int file_seek_pos = level_pos * SP_STD_LEVEL_SIZE;
  int level_width  = SP_STD_PLAYFIELD_WIDTH;
  int level_height = SP_STD_PLAYFIELD_HEIGHT;

  /* always start with reliable default values */
  setLevelInfoToDefaults_SP();
  copyInternalEngineVars_SP();

  if (!(file = openFile(filename, MODE_READ)))
  {
    if (!level_info_only)
      Error(ERR_WARN, "cannot open file '%s' -- using empty level", filename);

    return FALSE;
  }

  if (is_mpx_file)
  {
    char mpx_chunk_name[4 + 1];
    int mpx_version;
    int mpx_level_count;
    LevelDescriptor *mpx_level_desc;

    getFileChunkBE(file, mpx_chunk_name, NULL);

    if (!strEqual(mpx_chunk_name, "MPX "))
    {
      Error(ERR_WARN, "cannot find MPX ID in file '%s' -- using empty level",
	    filename);

      return FALSE;
    }

    mpx_version = getFile16BitLE(file);

    if (mpx_version != 1)
    {
      Error(ERR_WARN, "unknown MPX version in file '%s' -- using empty level",
	    filename);

      return FALSE;
    }

    mpx_level_count = getFile16BitLE(file);

    if (mpx_level_count < 1)
    {
      Error(ERR_WARN, "no MPX levels found in file '%s' -- using empty level",
	    filename);

      return FALSE;
    }

    if (level_pos >= mpx_level_count)
    {
      Error(ERR_WARN, "MPX level not found in file '%s' -- using empty level",
	    filename);

      return FALSE;
    }

    mpx_level_desc = checked_calloc(mpx_level_count * sizeof(LevelDescriptor));

    for (i = 0; i < mpx_level_count; i++)
    {
      LevelDescriptor *ldesc = &mpx_level_desc[i];

      ldesc->Width  = getFile16BitLE(file);
      ldesc->Height = getFile16BitLE(file);
      ldesc->OffSet = getFile32BitLE(file);	/* starts with 1, not with 0 */
      ldesc->Size   = getFile32BitLE(file);
    }

    level_width  = mpx_level_desc[level_pos].Width;
    level_height = mpx_level_desc[level_pos].Height;

    file_seek_pos = mpx_level_desc[level_pos].OffSet - 1;
  }

  /* position file stream to the requested level (in case of level package) */
  if (seekFile(file, file_seek_pos, SEEK_SET) != 0)
  {
    Error(ERR_WARN, "cannot fseek in file '%s' -- using empty level", filename);

    return FALSE;
  }

  /* there exist Supaplex level package files with multi-part levels which
     can be detected as follows: instead of leading and trailing dashes ('-')
     to pad the level name, they have leading and trailing numbers which are
     the x and y coordinations of the current part of the multi-part level;
     if there are '?' characters instead of numbers on the left or right side
     of the level name, the multi-part level consists of only horizontal or
     vertical parts */

  for (l = level_pos; l < SP_NUM_LEVELS_PER_PACKAGE; l++)
  {
    LoadNativeLevelFromFileStream_SP(file, level_width, level_height,
				     demo_available);

    /* check if this level is a part of a bigger multi-part level */

    if (is_single_level_file)
      break;

    name_first = header->LevelTitle[0];
    name_last  = header->LevelTitle[SP_LEVEL_NAME_LEN - 1];

    is_multipart_level =
      ((name_first == '?' || (name_first >= '0' && name_first <= '9')) &&
       (name_last  == '?' || (name_last  >= '0' && name_last  <= '9')));

    is_first_part =
      ((name_first == '?' || name_first == '1') &&
       (name_last  == '?' || name_last  == '1'));

    if (is_multipart_level)
    {
      /* correct leading multipart level meta information in level name */
      for (i = 0;
	   i < SP_LEVEL_NAME_LEN && header->LevelTitle[i] == name_first;
	   i++)
	header->LevelTitle[i] = '-';

      /* correct trailing multipart level meta information in level name */
      for (i = SP_LEVEL_NAME_LEN - 1;
	   i >= 0 && header->LevelTitle[i] == name_last;
	   i--)
	header->LevelTitle[i] = '-';
    }

    /* ---------- check for normal single level ---------- */

    if (!reading_multipart_level && !is_multipart_level)
    {
      /* the current level is simply a normal single-part level, and we are
	 not reading a multi-part level yet, so return the level as it is */

      break;
    }

    /* ---------- check for empty level (unused multi-part) ---------- */

    if (!reading_multipart_level && is_multipart_level && !is_first_part)
    {
      /* this is a part of a multi-part level, but not the first part
	 (and we are not already reading parts of a multi-part level);
	 in this case, use an empty level instead of the single part */

      use_empty_level = TRUE;

      break;
    }

    /* ---------- check for finished multi-part level ---------- */

    if (reading_multipart_level &&
	(!is_multipart_level ||
	 !strEqualN(header->LevelTitle, multipart_level.header.LevelTitle,
		    SP_LEVEL_NAME_LEN)))
    {
      /* we are already reading parts of a multi-part level, but this level is
	 either not a multi-part level, or a part of a different multi-part
	 level; in both cases, the multi-part level seems to be complete */

      break;
    }

    /* ---------- here we have one part of a multi-part level ---------- */

    reading_multipart_level = TRUE;

    if (is_first_part)	/* start with first part of new multi-part level */
    {
      /* copy level info structure from first part */
      multipart_level = native_sp_level;

      /* clear playfield of new multi-part level */
      for (x = 0; x < SP_MAX_PLAYFIELD_WIDTH; x++)
	for (y = 0; y < SP_MAX_PLAYFIELD_HEIGHT; y++)
	  multipart_level.playfield[x][y] = fiSpace;
    }

    if (name_first == '?')
      name_first = '1';
    if (name_last == '?')
      name_last = '1';

    multipart_xpos = (int)(name_first - '0');
    multipart_ypos = (int)(name_last  - '0');

    if (multipart_xpos * SP_STD_PLAYFIELD_WIDTH  > SP_MAX_PLAYFIELD_WIDTH ||
	multipart_ypos * SP_STD_PLAYFIELD_HEIGHT > SP_MAX_PLAYFIELD_HEIGHT)
    {
      Error(ERR_WARN, "multi-part level is too big -- ignoring part of it");

      break;
    }

    multipart_level.width  = MAX(multipart_level.width,
				 multipart_xpos * SP_STD_PLAYFIELD_WIDTH);
    multipart_level.height = MAX(multipart_level.height,
				 multipart_ypos * SP_STD_PLAYFIELD_HEIGHT);

    /* copy level part at the right position of multi-part level */
    for (x = 0; x < SP_STD_PLAYFIELD_WIDTH; x++)
    {
      for (y = 0; y < SP_STD_PLAYFIELD_HEIGHT; y++)
      {
	int start_x = (multipart_xpos - 1) * SP_STD_PLAYFIELD_WIDTH;
	int start_y = (multipart_ypos - 1) * SP_STD_PLAYFIELD_HEIGHT;

	multipart_level.playfield[start_x + x][start_y + y] =
	  native_sp_level.playfield[x][y];
      }
    }
  }

  closeFile(file);

  if (use_empty_level)
  {
    setLevelInfoToDefaults_SP();

    Error(ERR_WARN, "single part of multi-part level -- using empty level");
  }

  if (reading_multipart_level)
    native_sp_level = multipart_level;

  copyInternalEngineVars_SP();

  return TRUE;
}

void SaveNativeLevel_SP(char *filename)
{
  LevelInfoType *header = &native_sp_level.header;
  FILE *file;
  int i, x, y;

  if (!(file = fopen(filename, MODE_WRITE)))
  {
    Error(ERR_WARN, "cannot save native level file '%s'", filename);

    return;
  }

  /* write level playfield (width * height == 60 * 24 tiles == 1440 bytes) */
  for (y = 0; y < native_sp_level.height; y++)
    for (x = 0; x < native_sp_level.width; x++)
      putFile8Bit(file, native_sp_level.playfield[x][y]);

  /* write level header (96 bytes) */

  WriteUnusedBytesToFile(file, 4);

  putFile8Bit(file, header->InitialGravity);
  putFile8Bit(file, header->Version);

  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    putFile8Bit(file, header->LevelTitle[i]);

  putFile8Bit(file, header->InitialFreezeZonks);
  putFile8Bit(file, header->InfotronsNeeded);
  putFile8Bit(file, header->SpecialPortCount);

  for (i = 0; i < SP_MAX_SPECIAL_PORTS; i++)
  {
    SpecialPortType *port = &header->SpecialPort[i];

    putFile16BitBE(file, port->PortLocation);
    putFile8Bit(file, port->Gravity);
    putFile8Bit(file, port->FreezeZonks);
    putFile8Bit(file, port->FreezeEnemies);

    WriteUnusedBytesToFile(file, 1);
  }

  putFile8Bit(file, header->SpeedByte);
  putFile8Bit(file, header->CheckSumByte);
  putFile16BitLE(file, header->DemoRandomSeed);

  /* also save demo tape, if available */

  if (native_sp_level.demo.is_available)
  {
    putFile8Bit(file, native_sp_level.demo.level_nr);

    for (i = 0; i < native_sp_level.demo.length; i++)
      putFile8Bit(file, native_sp_level.demo.data[i]);

    putFile8Bit(file, 0xff);	/* "end of demo" byte */
  }

  fclose(file);
}
