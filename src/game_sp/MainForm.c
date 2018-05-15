// ----------------------------------------------------------------------------
// MainForm.c
// ----------------------------------------------------------------------------

#include "MainForm.h"


static void DrawFrame(int Delta);
static void ReStretch();

void DrawField(int X, int Y);
void DrawFieldAnimated(int X, int Y);
void DrawFieldNoAnimated(int X, int Y);

void DrawFrameIfNeeded()
{
  DrawFrame(0);

  /* !!! CHECK THIS !!! */
  if (! menBorder)
    DrawFrame(1);
}

void DisplayLevel()
{
  int X, Y;

  if (! LevelLoaded)
    return;

  ClearRectangle(backbuffer, REAL_SX, REAL_SY, FULL_SXSIZE, FULL_SYSIZE);
  ClearRectangle(bitmap_db_field_sp, 0, 0, FXSIZE, FYSIZE);

  SetDisplayRegion();

  DrawFrameIfNeeded();

  if (bPlaying)
  {
    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
        DrawFieldNoAnimated(X, Y);

    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
        DrawFieldAnimated(X, Y);
  }
  else
  {
    for (Y = DisplayMinY; Y <= DisplayMaxY; Y++)
      for (X = DisplayMinX; X <= DisplayMaxX; X++)
        DrawField(X, Y);
  }
}

void Form_Load()
{
  InitGlobals();

  ReStretch();
}

static void DrawFrame(int Delta)
{
  int i, LX, tY, RX, BY;

  LX = -1 + Delta;
  tY = -1 + Delta;
  RX = FieldWidth - Delta;
  BY = FieldHeight - Delta;

  DrawImage(LX, tY, (Delta > 0 ? imgFrameCorner : aniSpace));
  DrawImage(LX, BY, (Delta > 0 ? imgFrameCorner : aniSpace));
  DrawImage(RX, tY, (Delta > 0 ? imgFrameCorner : aniSpace));
  DrawImage(RX, BY, (Delta > 0 ? imgFrameCorner : aniSpace));

  for (i = LX + 1; i <= RX - 1; i++)
  {
    DrawImage(i, tY, (Delta > 0 ? imgFrameHorizontal : aniSpace));
    DrawImage(i, BY, (Delta > 0 ? imgFrameHorizontal : aniSpace));
  }

  for (i = tY + 1; i <= BY - 1; i++)
  {
    DrawImage(LX, i, (Delta > 0 ? imgFrameVertical : aniSpace));
    DrawImage(RX, i, (Delta > 0 ? imgFrameVertical : aniSpace));
  }

  if (Delta > 0)
  {
    // ...
    // ClearRectangle(bitmap_db_field_sp, 
  }
}

static void RestoreFrame()
{
  int i, LX, tY, RX, BY;

  LX = 0;
  tY = 0;
  RX = FieldWidth - 1;
  BY = FieldHeight - 1;

  for (i = LX; i <= RX; i++)
  {
    DrawField(i, tY);
    DrawField(i, BY);
  }

  for (i = tY + 1; i <= BY - 1; i++)
  {
    DrawField(LX, i);
    DrawField(RX, i);
  }
}

void SetDisplayRegion()
{
  if (! menBorder)
  {
    DisplayMinX = 1;
    DisplayMinY = 1;
    DisplayMaxX = FieldWidth - 2;
    DisplayMaxY = FieldHeight - 2;

    if (LevelLoaded)
      DrawFrame(1);
  }
  else
  {
    DisplayMinX = 0;
    DisplayMinY = 0;
    DisplayMaxX = FieldWidth - 1;
    DisplayMaxY = FieldHeight - 1;

    if (LevelLoaded)
      RestoreFrame();
  }
}

void menPlay_Click()
{
  bPlaying = True;

  subFetchAndInitLevelB();

  ReStretch();

  subMainGameLoop_Init();
}

static void ReStretch()
{
  if (LevelLoaded)
  {
    SetDisplayRegion();

    SetScrollEdges();

    ScrollTo(ScrollX, ScrollY);

    DisplayLevel();
  }

  subCalculateScreenScrollPos();

  ScrollTo(ScreenScrollXPos, ScreenScrollYPos);
}

void SetScrollEdges()
{
  int border1_offset = (menBorder ? 1 : 2);
  int border2_offset = (menBorder ? 0 : TILESIZE / 2);

  /* scroll correction for border frame (1 tile) or border element (2 tiles) */
  ScrollMinX = 0;
  ScrollMinY = 0;
  ScrollMaxX = (DisplayMaxX + border1_offset - SCR_FIELDX) * TILEX;
  ScrollMaxY = (DisplayMaxY + border1_offset - SCR_FIELDY) * TILEY;

  /* scroll correction for border element (half tile on left and right side) */
  ScrollMinX += border2_offset;
  ScrollMinY += border2_offset;
  ScrollMaxX -= border2_offset;
  ScrollMaxY -= border2_offset;

  /* scroll correction for even number of visible tiles (half tile shifted) */
  ScrollMinX -= game_sp.scroll_xoffset;
  ScrollMaxX -= game_sp.scroll_xoffset;
  ScrollMinY -= game_sp.scroll_yoffset;
  ScrollMaxY -= game_sp.scroll_yoffset;
}

void DrawField(int X, int Y)
{
  int tsi = GetSI(X, Y);
  int Tmp = LowByte(PlayField16[tsi]);

  if (Tmp < fiFirst || Tmp > fiLast)
    Tmp = fiSpace;

  if (Tmp == fiRAM ||
      Tmp == fiHardWare ||
      Tmp == fiBug ||
      Tmp == fiWallSpace)
    Tmp = DisPlayField[tsi];

  subCopyImageToScreen(tsi, fiGraphic[Tmp]);

  if (Tmp != fiSpace &&
      Tmp != fiSnikSnak &&
      Tmp != fiElectron)
    GfxGraphic[X][Y] = fiGraphic[Tmp];
}

void DrawFieldAnimated(int X, int Y)
{
  int tsi = GetSI(X, Y);
  int Tmp = LowByte(PlayField16[tsi]);

  switch (Tmp)
  {
    case fiSnikSnak:
      subDrawAnimatedSnikSnaks(tsi);
      break;

    case fiElectron:
      subDrawAnimatedElectrons(tsi);
      break;

    default:
      break;
  }
}

void DrawFieldNoAnimated(int X, int Y)
{
  int tsi = GetSI(X, Y);
  int Tmp = LowByte(PlayField16[tsi]);

  switch (Tmp)
  {
    case fiSnikSnak:
      subCopyImageToScreen(tsi, aniSpace);
      break;

    case fiElectron:
      subCopyImageToScreen(tsi, aniSpace);
      break;

    default:
      DrawField(X, Y);
      break;
  }
}

void DrawImage(int X, int Y, int graphic)
{
  DDSpriteBuffer_BltImg(StretchWidth * X, StretchWidth * Y, graphic, 0);
}
