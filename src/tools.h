// ============================================================================
// Rocks'n'Diamonds - McDuffin Strikes Back!
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// tools.h
// ============================================================================

#ifndef TOOLS_H
#define TOOLS_H

#include "main.h"
#include "game.h"


/* for DrawElementShifted */
#define NO_CUTTING		0
#define CUT_ABOVE		(1 << 0)
#define CUT_BELOW		(1 << 1)
#define CUT_LEFT		(1 << 2)
#define CUT_RIGHT		(1 << 3)

/* for masking functions */
#define NO_MASKING		0
#define USE_MASKING		1
 
/* for MoveDoor */
#define DOOR_OPEN_1		(1 << 0)
#define DOOR_OPEN_2		(1 << 1)
#define DOOR_CLOSE_1		(1 << 2)
#define DOOR_CLOSE_2		(1 << 3)
#define DOOR_OPEN_ALL		(DOOR_OPEN_1 | DOOR_OPEN_2)
#define DOOR_CLOSE_ALL		(DOOR_CLOSE_1 | DOOR_CLOSE_2)
#define DOOR_ACTION_1		(DOOR_OPEN_1 | DOOR_CLOSE_1)
#define DOOR_ACTION_2		(DOOR_OPEN_2 | DOOR_CLOSE_2)
#define DOOR_ACTION		(DOOR_ACTION_1 | DOOR_ACTION_2)
#define DOOR_COPY_BACK		(1 << 4)
#define DOOR_NO_COPY_BACK	(1 << 5)
#define DOOR_NO_DELAY		(1 << 6)
#define DOOR_FORCE_ANIM		(1 << 7)
#define DOOR_FORCE_REDRAW	(1 << 8)
#define DOOR_GET_STATE		(1 << 9)
#define DOOR_SET_STATE		(1 << 10)

#define DOOR_1			(DOOR_ACTION_1)
#define DOOR_2			(DOOR_ACTION_2)
#define DOOR_OPEN		(DOOR_OPEN_ALL)
#define DOOR_CLOSE		(DOOR_CLOSE_ALL)

#define DOOR_INDEX_FROM_TOKEN(x)	((x) == DOOR_1 ? 0 : 1)
#define DOOR_TOKEN_FROM_INDEX(x)	((x) == 0 ? DOOR_1 ? : DOOR_2)
#define REDRAW_DOOR_FROM_TOKEN(x)	((x) == DOOR_1 ? REDRAW_DOOR_1 : \
					 REDRAW_DOOR_2)

/* for Request */
#define REQ_ASK			(1 << 0)
#define REQ_CONFIRM		(1 << 1)
#define REQ_PLAYER		(1 << 2)
#define REQ_STAY_OPEN		(1 << 3)
#define REQ_STAY_CLOSED		(1 << 4)
#define REQ_REOPEN		(1 << 5)

#define REQUEST_WAIT_FOR_INPUT	(REQ_ASK | REQ_CONFIRM | REQ_PLAYER)


int correctLevelPosX_EM(int);
int correctLevelPosY_EM(int);

int getLevelFromScreenX(int);
int getLevelFromScreenY(int);

void DumpTile(int, int);
void DumpTileFromScreen(int, int);

void DrawMaskedBorder_FIELD();
void DrawMaskedBorder_DOOR_1();
void DrawMaskedBorder_DOOR_2();
void DrawMaskedBorder_DOOR_3();
void DrawMaskedBorder_ALL();
void DrawMaskedBorder(int);
void DrawMaskedBorderToTarget(int);
void DrawTileCursor(int);

void SetDrawtoField(int);
void RedrawPlayfield();
void BlitScreenToBitmap_RND(Bitmap *);
void BlitScreenToBitmap(Bitmap *);
void BackToFront();
void BackToFront_WithFrameDelay(unsigned int);

void FadeIn(int);
void FadeOut(int);
void FadeSetEnterMenu();
void FadeSetLeaveMenu();
void FadeSetEnterScreen();
void FadeSetNextScreen();
void FadeSetLeaveScreen();
void FadeSetFromType(int);
void FadeSetDisabled();
void FadeSkipNextFadeIn();
void FadeSkipNextFadeOut();

Bitmap *getGlobalBorderBitmapFromStatus(int);

void ClearField();
void SetWindowBackgroundImageIfDefined(int);
void SetMainBackgroundImageIfDefined(int);
void SetDoorBackgroundImageIfDefined(int);
void SetWindowBackgroundImage(int);
void SetMainBackgroundImage(int);
void SetDoorBackgroundImage(int);
void SetPanelBackground();
void DrawBackground(int, int, int, int);
void DrawBackgroundForFont(int, int, int, int, int);
void DrawBackgroundForGraphic(int, int, int, int, int);
boolean CheckIfGlobalBorderHasChanged();
void RedrawGlobalBorder();

void MarkTileDirty(int, int);
void SetBorderElement();
void FloodFillLevel(int, int, int, short[][MAX_LEV_FIELDY], int, int);
void FloodFillLevelExt(int, int, int, int, int y, short field[][y], int, int);

void SetRandomAnimationValue(int, int);
int getGraphicAnimationFrame(int, int);

void DrawFixedGraphicAnimation(int, int, int);
void DrawFixedGraphicAnimationExt(DrawBuffer *, int, int, int, int, int);

void DrawLevelGraphicAnimation(int, int, int);
void DrawLevelElementAnimation(int, int, int);
void DrawLevelGraphicAnimationIfNeeded(int, int, int);
void DrawLevelElementAnimationIfNeeded(int, int, int);

void DrawAllPlayers(void);
void DrawPlayerField(int, int);
void DrawPlayer(struct PlayerInfo *);

void getGraphicSourceBitmap(int, int, Bitmap **);
void getGraphicSourceXY(int, int, int *, int *, boolean);
void getSizedGraphicSourceExt(int, int, int, Bitmap **, int *, int *, boolean);
void getSizedGraphicSource(int, int, int, Bitmap **, int *, int *);
void getFixedGraphicSource(int, int, Bitmap **, int *, int *);
void getMiniGraphicSource(int, Bitmap **, int *, int *);
void getGraphicSource(int, int, Bitmap **, int *, int *);

void DrawGraphic(int, int, int, int);
void DrawGraphicExt(DrawBuffer *, int, int, int, int);
void DrawGraphicThruMask(int, int, int, int);
void DrawGraphicThruMaskExt(DrawBuffer *, int, int, int, int);

void DrawFixedGraphic(int, int, int, int);
void DrawFixedGraphicExt(DrawBuffer *, int, int, int, int);
void DrawFixedGraphicThruMask(int, int, int, int);
void DrawFixedGraphicThruMaskExt(DrawBuffer *, int, int, int, int);

void DrawSizedGraphic(int, int, int, int, int);
void DrawSizedGraphicExt(DrawBuffer *, int, int, int, int, int);
void DrawSizedGraphicThruMask(int, int, int, int, int);
void DrawSizedGraphicThruMaskExt(DrawBuffer *, int, int, int, int, int);

void DrawMiniGraphic(int, int, int);
void DrawMiniGraphicExt(DrawBuffer *, int, int, int);

void DrawScreenElementExt(int, int, int, int, int, int, int);
void DrawLevelElementExt(int, int, int, int, int, int, int);
void DrawScreenElementShifted(int, int, int, int, int, int);
void DrawLevelElementShifted(int, int, int, int, int, int);
void DrawScreenElementThruMask(int, int, int);
void DrawLevelElementThruMask(int, int, int);
void DrawLevelFieldThruMask(int, int);
void DrawLevelFieldCrumbled(int, int);
void DrawLevelFieldCrumbledDigging(int, int, int, int);
void DrawLevelFieldCrumbledNeighbours(int, int);
void DrawScreenElement(int, int, int);
void DrawLevelElement(int, int, int);
void DrawScreenField(int, int);
void DrawLevelField(int, int);

void DrawSizedWallParts_MM(int, int, int, int, boolean, int);
void DrawSizedElement(int, int, int, int);
void DrawSizedElementThruMask(int, int, int, int);
void DrawSizedElementOrWall(int, int, int, int, int);

void DrawMiniElement(int, int, int);
void DrawMiniElementOrWall(int, int, int, int);

void ShowEnvelope(int);
void ShowEnvelopeDoor(char *text, int);

void DrawLevel(int);
void DrawSizedLevel(int, int, int, int, int);
void DrawMiniLevel(int, int, int, int);
void DrawPreviewLevelInitial(void);
void DrawPreviewLevelAnimation(void);

void WaitForEventToContinue(void);
boolean Request(char *, unsigned int);
void InitGraphicCompatibilityInfo_Doors(void);
void InitDoors(void);
unsigned int OpenDoor(unsigned int);
unsigned int CloseDoor(unsigned int);
unsigned int GetDoorState(void);
unsigned int SetDoorState(unsigned int);
unsigned int MoveDoor(unsigned int);

void DrawSpecialEditorDoor();
void UndrawSpecialEditorDoor();

void CreateToolButtons();
void FreeToolButtons();

int map_element_RND_to_EM(int);
int map_element_EM_to_RND(int);
int map_direction_RND_to_EM(int);
int map_direction_EM_to_RND(int);

void map_android_clone_elements_RND_to_EM(struct LevelInfo *);
void map_android_clone_elements_EM_to_RND(struct LevelInfo *);

int map_element_RND_to_SP(int);
int map_element_SP_to_RND(int);
int map_action_SP_to_RND(int);

int map_element_RND_to_MM(int);
int map_element_MM_to_RND(int);
int map_action_MM_to_RND(int);
int map_sound_MM_to_RND(int);
int map_mm_wall_element(int);
int map_mm_wall_element_editor(int);

int get_next_element(int);
int el_act_dir2img(int, int, int);
int el_act2img(int, int);
int el_dir2img(int, int);
int el2baseimg(int);
int el2img(int);
int el2edimg(int);
int el2preimg(int);
int el2panelimg(int);
int font2baseimg(int);

int getBeltNrFromBeltElement(int);
int getBeltNrFromBeltActiveElement(int);
int getBeltNrFromBeltSwitchElement(int);
int getBeltDirNrFromBeltElement(int);
int getBeltDirNrFromBeltSwitchElement(int);
int getBeltDirFromBeltElement(int);
int getBeltDirFromBeltSwitchElement(int);
int getBeltElementFromBeltNrAndBeltDirNr(int, int);
int getBeltElementFromBeltNrAndBeltDir(int, int);
int getBeltSwitchElementFromBeltNrAndBeltDirNr(int, int);
int getBeltSwitchElementFromBeltNrAndBeltDir(int, int);

unsigned int InitRND(int);
void InitGraphicInfo_EM(void);

void PlayMenuSoundExt(int);
void PlayMenuSound();
void PlayMenuSoundStereo(int, int);
void PlayMenuSoundIfLoopExt(int);
void PlayMenuSoundIfLoop();
void PlayMenuMusicExt(int);
void PlayMenuMusic();
void PlayMenuSoundsAndMusic();
void FadeMenuSoundsAndMusic();
void PlaySoundActivating();
void PlaySoundSelecting();

void SetAnimStatus(int);
void SetGameStatus(int);
void SetFontStatus(int);
void ResetFontStatus();

void ToggleFullscreenOrChangeWindowScalingIfNeeded();
void ChangeViewportPropertiesIfNeeded();

boolean CheckIfPlayfieldViewportHasChanged();
boolean CheckIfGlobalBorderOrPlayfieldViewportHasChanged();

#endif	/* TOOLS_H */
