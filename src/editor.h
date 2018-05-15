// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// editor.h
// ============================================================================

#ifndef EDITOR_H
#define EDITOR_H

#include "main.h"

void CreateLevelEditorGadgets();
void FreeLevelEditorGadgets();
void UnmapLevelEditorGadgets();
void DrawLevelEd(void);
void HandleLevelEditorKeyInput(Key);
void HandleLevelEditorIdle();
void HandleEditorGadgetInfoText(void *ptr);
void RequestExitLevelEditor(boolean, boolean);
void PrintEditorElementList();

void DumpBrush();
void DumpBrush_Small();

#endif
