// ----------------------------------------------------------------------------
// Globals.h
// ----------------------------------------------------------------------------

#ifndef GLOBALS_H
#define GLOBALS_H

#include "global.h"


#ifndef False
#define False				0
#define True				(!False)
#endif

#define ScrollDelta			((int)1)

#define ZoomFactor			(2)

#define StretchWidth			(ZoomFactor * 16)
#define TwoPixels			(ZoomFactor * 2)


// ----------------------------------------------------------------------------
// elements (stored in file and playfield)
// ----------------------------------------------------------------------------

#define fiSpace				(0)
#define fiZonk				(1)
#define fiBase				(2)
#define fiMurphy			(3)
#define fiInfotron			(4)
#define fiRAM				(5)
#define fiHardWare			(6)
#define fiExit				(7)
#define fiOrangeDisk			(8)
#define fiPortRight			(9)
#define fiPortDown			(10)
#define fiPortLeft			(11)
#define fiPortUp			(12)
#define fiSpPortRight			(13)
#define fiSpPortDown			(14)
#define fiSpPortLeft			(15)
#define fiSpPortUp			(16)
#define fiSnikSnak			(17)
#define fiYellowDisk			(18)
#define fiTerminal			(19)
#define fiRedDisk			(20)
#define fiPortUpAndDown			(21)
#define fiPortLeftAndRight		(22)
#define fiPortAllDirections		(23)
#define fiElectron			(24)
#define fiBug				(25)
#define fiRAMLeft			(26)
#define fiRAMRight			(27)
#define fiHWFirst			(28)
#define fiHW0				(fiHWFirst + 0)		// fiHWFirst
#define fiHW1				(fiHWFirst + 1)
#define fiHW2				(fiHWFirst + 2)
#define fiHW3				(fiHWFirst + 3)
#define fiHW4				(fiHWFirst + 4)
#define fiHW5				(fiHWFirst + 5)
#define fiHW6				(fiHWFirst + 6)
#define fiHW7				(fiHWFirst + 7)
#define fiHW8				(fiHWFirst + 8)
#define fiHW9				(fiHWFirst + 9)		// fiHWLast
#define fiHWLast			(37)
#define fiRAMTop			(38)
#define fiRAMBottom			(39)
#define fiWallSpace			(40)

#define fiExplosion			(0x1F)

#define fiFirst				(0)
#define fiLast				(40)


// ----------------------------------------------------------------------------
// graphics and animations (used at runtime to display the elements)
// ----------------------------------------------------------------------------

// graphics and animations directly related to file elements

#define aniSpace			IMG_EMPTY_SPACE
#define aniZonk				IMG_SP_ZONK
#define aniBase				IMG_SP_BASE
#define aniMurphy			IMG_SP_MURPHY
#define aniInfotron			IMG_SP_INFOTRON
#define aniRAM				IMG_SP_CHIP_SINGLE
#define aniHardWare			IMG_SP_HARDWARE_GRAY
#define aniExit				IMG_SP_EXIT_CLOSED
#define aniOrangeDisk			IMG_SP_DISK_ORANGE
#define aniPortRight			IMG_SP_PORT_RIGHT
#define aniPortDown			IMG_SP_PORT_DOWN
#define aniPortLeft			IMG_SP_PORT_LEFT
#define aniPortUp			IMG_SP_PORT_UP
#define aniSpPortRight			IMG_SP_GRAVITY_PORT_RIGHT
#define aniSpPortDown			IMG_SP_GRAVITY_PORT_DOWN
#define aniSpPortLeft			IMG_SP_GRAVITY_PORT_LEFT
#define aniSpPortUp			IMG_SP_GRAVITY_PORT_UP
#define aniSnikSnak			IMG_SP_SNIKSNAK
#define aniYellowDisk			IMG_SP_DISK_YELLOW
#define aniTerminal			IMG_SP_TERMINAL
#define aniRedDisk			IMG_SP_DISK_RED
#define aniPortUpAndDown		IMG_SP_PORT_VERTICAL
#define aniPortLeftAndRight		IMG_SP_PORT_HORIZONTAL
#define aniPortAllDirections		IMG_SP_PORT_ANY
#define aniElectron			IMG_SP_ELECTRON
#define aniBug				IMG_SP_BUGGY_BASE
#define aniRAMLeft			IMG_SP_CHIP_LEFT
#define aniRAMRight			IMG_SP_CHIP_RIGHT
#define aniHWFirst			IMG_SP_HARDWARE_BASE_1
#define aniHW0				IMG_SP_HARDWARE_BASE_1
#define aniHW1				IMG_SP_HARDWARE_GREEN
#define aniHW2				IMG_SP_HARDWARE_BLUE
#define aniHW3				IMG_SP_HARDWARE_RED
#define aniHW4				IMG_SP_HARDWARE_YELLOW
#define aniHW5				IMG_SP_HARDWARE_BASE_2
#define aniHW6				IMG_SP_HARDWARE_BASE_3
#define aniHW7				IMG_SP_HARDWARE_BASE_4
#define aniHW8				IMG_SP_HARDWARE_BASE_5
#define aniHW9				IMG_SP_HARDWARE_BASE_6
#define aniHWLast			IMG_SP_HARDWARE_BASE_6
#define aniRAMTop			IMG_SP_CHIP_TOP
#define aniRAMBottom			IMG_SP_CHIP_BOTTOM
#define aniWallSpace			IMG_INVISIBLE_WALL
#define aniHWTrash1			
#define aniHWTrash2			
#define aniHWMurphy			


// graphics and animations related to in-game animations for element actions

#define aniMurphyDropping		IMG_SP_MURPHY_DROPPING

#define aniMurphySleepLeft		IMG_SP_MURPHY_SLEEPING_LEFT
#define aniMurphySleepRight		IMG_SP_MURPHY_SLEEPING_RIGHT
#define aniMurphyTouchLeft		IMG_SP_MURPHY_SNAPPING_LEFT
#define aniMurphyTouchRight		IMG_SP_MURPHY_SNAPPING_RIGHT
#define aniMurphyTouchUp		IMG_SP_MURPHY_SNAPPING_UP
#define aniMurphyTouchDown		IMG_SP_MURPHY_SNAPPING_DOWN
#define aniMurphyYawn			IMG_SP_MURPHY_BORING_1
#define aniPushLeft			IMG_SP_MURPHY_PUSHING_LEFT
#define aniPushRight			IMG_SP_MURPHY_PUSHING_RIGHT

#define aniBugActivating		IMG_SP_BUGGY_BASE_ACTIVATING
#define aniBugDeactivating		IMG_SP_BUGGY_BASE_ACTIVATING
#define aniBugActive			IMG_SP_BUGGY_BASE_ACTIVE
#define aniZonkRollLeft			IMG_SP_ZONK_MOVING_LEFT
#define aniZonkRollRight		IMG_SP_ZONK_MOVING_RIGHT
#define aniEatInfotronLeft		IMG_SP_MURPHY_COLLECTING_LEFT
#define aniEatInfotronRight		IMG_SP_MURPHY_COLLECTING_RIGHT
#define aniInfotronRollLeft		IMG_SP_INFOTRON_MOVING_LEFT
#define aniInfotronRollRight		IMG_SP_INFOTRON_MOVING_RIGHT
#define aniMurphyMoveLeft		IMG_SP_MURPHY_MOVING_LEFT
#define aniMurphyMoveRight		IMG_SP_MURPHY_MOVING_RIGHT
#define aniMurphyMoveUpLeft		IMG_SP_MURPHY_MOVING_LEFT
#define aniMurphyMoveUpRight		IMG_SP_MURPHY_MOVING_RIGHT
#define aniMurphyDigLeft		IMG_SP_MURPHY_DIGGING_LEFT
#define aniMurphyDigRight		IMG_SP_MURPHY_DIGGING_RIGHT
#define aniMurphyDigUpLeft		IMG_SP_MURPHY_DIGGING_LEFT
#define aniMurphyDigUpRight		IMG_SP_MURPHY_DIGGING_RIGHT
#define aniMurphyEatLeft		IMG_SP_MURPHY_COLLECTING_LEFT
#define aniMurphyEatRight		IMG_SP_MURPHY_COLLECTING_RIGHT
#define aniMurphyEatUpLeft		IMG_SP_MURPHY_COLLECTING_LEFT
#define aniMurphyEatUpRight		IMG_SP_MURPHY_COLLECTING_RIGHT
#define aniSplitUpDown			IMG_SP_MURPHY
#define aniMurphyExit			IMG_SP_MURPHY_SHRINKING
#define aniElectron			IMG_SP_ELECTRON
#define aniExplosion			IMG_SP_DEFAULT_EXPLODING
#define aniTouchBase			IMG_SP_BASE_SNAPPING
#define aniTouchInfotron		IMG_SP_INFOTRON_COLLECTING
#define aniTouchRedDisk			IMG_SP_DISK_RED_COLLECTING
#define aniYellowDisk			IMG_SP_DISK_YELLOW
#define aniOrangeDisk			IMG_SP_DISK_ORANGE
#define aniRedDisk			IMG_SP_DISK_RED

#define aniSnikSnakDown			IMG_SP_SNIKSNAK_DOWN
#define aniSnikSnakLeft			IMG_SP_SNIKSNAK_LEFT
#define aniSnikSnakRight		IMG_SP_SNIKSNAK_RIGHT
#define aniSnikSnakUp			IMG_SP_SNIKSNAK_UP

#define aniSnikSnakTurnLeftToUp		IMG_SP_SNIKSNAK_TURNING_FROM_LEFT_UP
#define aniSnikSnakTurnLeftToDown	IMG_SP_SNIKSNAK_TURNING_FROM_LEFT_DOWN
#define aniSnikSnakTurnRightToUp	IMG_SP_SNIKSNAK_TURNING_FROM_RIGHT_UP
#define aniSnikSnakTurnRightToDown	IMG_SP_SNIKSNAK_TURNING_FROM_RIGHT_DOWN
#define aniSnikSnakTurnUpToLeft		IMG_SP_SNIKSNAK_TURNING_FROM_UP_LEFT
#define aniSnikSnakTurnUpToRight	IMG_SP_SNIKSNAK_TURNING_FROM_UP_RIGHT
#define aniSnikSnakTurnDownToLeft	IMG_SP_SNIKSNAK_TURNING_FROM_DOWN_LEFT
#define aniSnikSnakTurnDownToRight	IMG_SP_SNIKSNAK_TURNING_FROM_DOWN_RIGHT

#define aniTerminalActive		IMG_SP_TERMINAL_ACTIVE

#define aniDefaultExplosion		IMG_SP_DEFAULT_EXPLODING
#define aniElectronExplosion		IMG_SP_ELECTRON_EXPLODING

#define imgFrameCorner			IMG_SP_FRAME_CORNER
#define imgFrameHorizontal		IMG_SP_FRAME_HORIZONTAL
#define imgFrameVertical		IMG_SP_FRAME_VERTICAL


// ----------------------------------------------------------------------------
// input keys
// ----------------------------------------------------------------------------

#define keyNone				(0)
#define keyUp				(1)
#define keyLeft				(2)
#define keyDown				(3)
#define keyRight			(4)
#define keySpaceUp			(5)
#define keySpaceLeft			(6)
#define keySpaceDown			(7)
#define keySpaceRight			(8)
#define keySpace			(9)


// ----------------------------------------------------------------------------
// data structures
// ----------------------------------------------------------------------------

#ifndef HAS_LevelDescriptor
typedef struct
{
  int Width;
  int Height;
  int OffSet;
  int Size;
} LevelDescriptor;
#define HAS_LevelDescriptor
#endif

#ifndef HAS_SpecialPortType
typedef struct
{
  short PortLocation; // = 2*(x+(y*60))
  byte Gravity; // 1 = turn on, anything else (0) = turn off
  byte FreezeZonks; // 2 = turn on, anything else (0) = turn off  (1=off!)
  byte FreezeEnemies; // 1 = turn on, anything else (0) = turn off
  byte UnUsed;
} SpecialPortType;
#define HAS_SpecialPortType
#endif

#ifndef HAS_LevelInfoType
typedef struct
{
  byte UnUsed[4];
  byte InitialGravity; // 1=on, anything else (0) = off
  byte Version; // SpeedFixVersion XOR &H20
  char LevelTitle[23];
  byte InitialFreezeZonks; // 2=on, anything else (0) = off.  (1=off too!)
  byte InfotronsNeeded;

  // Number of Infotrons needed. 0 means that Supaplex will count the total
  // amount of Infotrons in the level, and use the low byte of that number.
  // (A multiple of 256 Infotrons will then result in 0-to-eat, etc.!)
  byte SpecialPortCount; // Maximum 10 allowed!
  SpecialPortType SpecialPort[10];
  byte SpeedByte; // = Speed XOR Highbyte(RandomSeed)
  byte CheckSumByte; // = CheckSum XOR SpeedByte
  short DemoRandomSeed;
} LevelInfoType;
#define HAS_LevelInfoType
#endif


extern int GetSI(int X, int Y);
extern int GetStretchX(int si);
extern int GetStretchY(int si);
extern int GetX(int si);
extern int GetY(int si);
extern void InitGlobals();

extern void PrepareLevel();

extern int getSequenceLength(int sequence);
extern boolean isSnappingSequence(int sequence);

extern int Min(int A, int B);
extern int Max(int A, int B);

extern int fiGraphic[];
extern int aniSnikSnakTurningLeft[];
extern int aniSnikSnakTurningRight[];

extern boolean LevelLoaded;

extern boolean DemoAvailable;
extern boolean menBorder;

extern int *PlayField16;
extern byte *PlayField8;
extern byte DisPlayField[SP_MAX_PLAYFIELD_SIZE + SP_HEADER_SIZE];

extern int FieldHeight;
extern int FieldMax, LevelMax;
extern int FieldWidth;
extern int FreezeZonks;
extern int HeaderSize;
extern int TimerVar;
extern short RandomSeed;

extern LevelInfoType LInfo;
extern int ScrollMinX, ScrollMaxX, ScrollMinY, ScrollMaxY;
extern int ScrollX, ScrollY;

extern int AllowRedDiskCheat, AllowEatRightRedDiskBug;
extern int Data_SubRest, Data_SubRstFlg;
extern int DemoKeyCode;
extern int GameBusyFlag;
extern int InfotronsNeeded, TotalInfotronsNeeded;
extern int KillMurphyFlag, MurphyMoveCounter;
extern int MurphyExplodePos, SplitMoveFlag, RedDiskReleaseMurphyPos;
extern int MurphyPosIndex, MurphyXPos, MurphyYPos;
extern int MurphyScreenXPos, MurphyScreenYPos;
extern int MurphyVarFaceLeft;
extern int RedDiskCount;
extern int RedDiskReleaseFlag, MovingPictureSequencePhase;
extern int RedDiskReleasePhase;
extern int ScratchGravity, GravityFlag;
extern int SnikSnaksElectronsFrozen;
extern int YellowDisksExploded;
extern int YawnSleepCounter;

#endif /* GLOBALS_H */
