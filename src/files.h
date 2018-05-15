// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// files.h
// ============================================================================

#ifndef FILES_H
#define FILES_H

#include "main.h"


#define LEVEL_PACKED_START		100
#define PACKED_LEVELS(x)		(LEVEL_PACKED_START + x)

#define LEVEL_FILE_TYPE_RND_PACKED	PACKED_LEVELS(LEVEL_FILE_TYPE_RND)
#define LEVEL_FILE_TYPE_EM_PACKED	PACKED_LEVELS(LEVEL_FILE_TYPE_EM)

#define IS_SINGLE_LEVEL_FILE(x)		(x < LEVEL_PACKED_START)
#define IS_PACKED_LEVEL_FILE(x)		(x > LEVEL_PACKED_START)


void setElementChangePages(struct ElementInfo *, int);
void setElementChangeInfoToDefaults(struct ElementChangeInfo *);
void copyElementInfo(struct ElementInfo *, struct ElementInfo *);

char *getDefaultLevelFilename(int);
char *getLocalLevelTemplateFilename();
char *getGlobalLevelTemplateFilename();

void LoadLevelFromFilename(struct LevelInfo *, char *);
void LoadLevel(int);
void LoadLevelTemplate(int);
void LoadLevelInfoOnly(int);
void SaveLevel(int);
void SaveLevelTemplate();
void SaveNativeLevel(struct LevelInfo *);
void DumpLevel(struct LevelInfo *);
boolean SaveLevelChecked(int);

void CopyNativeLevel_RND_to_Native(struct LevelInfo *);
void CopyNativeLevel_Native_to_RND(struct LevelInfo *);

void LoadTapeFromFilename(char *);
void LoadTape(int);
void LoadSolutionTape(int);
void SaveTape(int);
void DumpTape(struct TapeInfo *);
boolean SaveTapeChecked(int);
boolean SaveTapeChecked_LevelSolved(int);

void LoadScore(int);
void SaveScore(int);

void LoadSetupFromFilename(char *);
void LoadSetup();
void SaveSetup();

void LoadSetup_AutoSetup();
void SaveSetup_AutoSetup();

void LoadSetup_EditorCascade();
void SaveSetup_EditorCascade();

void SaveSetup_AddGameControllerMapping(char *);

boolean hideSetupEntry(void *);

void LoadCustomElementDescriptions();
void InitMenuDesignSettings_Static();
void LoadMenuDesignSettings();
void LoadMenuDesignSettings_AfterGraphics();
void LoadUserDefinedEditorElementList(int **, int *);
void LoadMusicInfo();
void LoadHelpAnimInfo();
void LoadHelpTextInfo();

void ConvertLevels();
void CreateLevelSketchImages();
void CreateCustomElementImages(char *);

#endif	/* FILES_H */
