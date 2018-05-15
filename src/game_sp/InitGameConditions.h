// ----------------------------------------------------------------------------
// InitGameConditions.h
// ----------------------------------------------------------------------------

#ifndef INITGAMECONDITIONS_H
#define INITGAMECONDITIONS_H

#include "global.h"


extern int subConvertToEasySymbols();

extern void InitMurphyPos();
extern void InitMurphyPosB(int);
extern void ResetInfotronsNeeded(int);
extern void subFetchAndInitLevel();
extern void subFetchAndInitLevelA();
extern void subFetchAndInitLevelB();
extern void subInitGameConditions();

#endif /* INITGAMECONDITIONS_H */
