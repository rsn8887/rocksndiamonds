// ----------------------------------------------------------------------------
// MainForm.h
// ----------------------------------------------------------------------------

#ifndef MAINFORM_H
#define MAINFORM_H

#include "global.h"


extern void DrawFrameIfNeeded();

extern void DisplayLevel();
extern void DrawField(int X, int Y);
extern void DrawFieldAnimated(int X, int Y);
extern void DrawFieldNoAnimated(int X, int Y);
extern void DrawSprite(int X, int Y, int SpritePos);
extern void DrawImage(int X, int Y, int graphic);

extern void SetDisplayRegion();
extern void SetScrollEdges();

extern void menPlay_Click();

extern void Form_Load();

#endif /* MAINFORM_H */
