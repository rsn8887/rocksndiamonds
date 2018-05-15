/***********************************************************
* Mirror Magic -- McDuffin's Revenge                       *
*----------------------------------------------------------*
* (c) 1994-2001 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* tools.h                                                  *
***********************************************************/

#ifndef MM_TOOLS_H
#define MM_TOOLS_H

#include "main_mm.h"


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
#define DOOR_NO_DELAY		(1 << 5)
#define DOOR_GET_STATE		(1 << 6)

/* for Request */
#define REQ_ASK			(1 << 0)
#define REQ_OPEN		(1 << 1)
#define REQ_CLOSE		(1 << 2)
#define REQ_CONFIRM		(1 << 3)
#define REQ_STAY_CLOSED		(1 << 4)
#define REQ_STAY_OPEN		(1 << 5)

#define REQUEST_WAIT_FOR	(REQ_ASK | REQ_CONFIRM)

/* font types */
#define FS_SMALL		0
#define FS_BIG			1
#define FS_MEDIUM		2


void SetDrawtoField_MM(int);
void BackToFront();
void FadeToFront();
void ClearWindow();

void MarkTileDirty(int, int);

void DrawAllPlayers_MM(void);
void DrawPlayerField_MM(int, int);
void DrawPlayer_MM(struct PlayerInfo *);
void DrawGraphicAnimation_MM(int, int, int, int);

void DrawGraphic_MM(int, int, int);
void DrawGraphicExt_MM(DrawBuffer *, int, int, int);
void DrawGraphicThruMask_MM(int, int, int);
void DrawGraphicThruMaskExt_MM(DrawBuffer *, int, int, int);
void DrawMiniGraphic_MM(int, int, int);
void getMiniGraphicSource(int, Bitmap **, int *, int *);
void DrawMiniGraphicExt_MM(DrawBuffer *, int, int, int);
void DrawGraphicShifted_MM(int, int, int, int, int, int, int);
void DrawGraphicShiftedThruMask_MM(int, int, int, int, int, int);
void DrawScreenElementExt_MM(int, int, int, int, int, int, int);
void DrawLevelElementExt_MM(int, int, int, int, int, int, int);
void DrawScreenElementShifted_MM(int, int, int, int, int, int);
void DrawLevelElementShifted_MM(int, int, int, int, int, int);
void DrawScreenElementThruMask_MM(int, int, int);
void DrawLevelElementThruMask_MM(int, int, int);
void DrawLevelFieldThruMask_MM(int, int);
void ErdreichAnbroeckeln(int, int);
void DrawScreenElement_MM(int, int, int);
void DrawLevelElement_MM(int, int, int);
void DrawScreenField_MM(int, int);
void DrawLevelField_MM(int, int);
void DrawMiniElement_MM(int, int, int);
void DrawMiniElementOrWall_MM(int, int, int, int);

void DrawField_MM(int, int);
void DrawLevel_MM(void);
void DrawLaser_MM(void);
void DrawElement_MM(int, int, int);
void DrawWallsExt_MM(int, int, int, int);
void DrawWalls_MM(int, int, int);
void DrawWallsAnimation_MM(int, int, int, int, int);
void DrawMiniLevel_MM(int, int, int, int);
void DrawMicroLevel_MM(int, int, boolean);

boolean Request(char *, unsigned int);
unsigned int OpenDoor(unsigned int);
unsigned int CloseDoor(unsigned int);
unsigned int GetDoorState(void);
unsigned int MoveDoor(unsigned int);
void DrawSpecialEditorDoor_MM();
void UndrawSpecialEditorDoor();
Pixel ReadPixel(DrawBuffer *, int, int);
void SetRGB(unsigned int, unsigned short, unsigned short, unsigned short);

void CreateToolButtons();

int get_base_element(int);
int get_element_phase(int);
int get_num_elements(int);
int get_rotated_element(int, int);

int el2gfx(int);

#endif
