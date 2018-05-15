// ----------------------------------------------------------------------------
// ASM.h
// ----------------------------------------------------------------------------

#ifndef ASM_H
#define ASM_H

#include "global.h"


extern void MovLowByte(int *, int);
extern void MovHighByte(int *, int);
extern int LowByte(int);
extern int HighByte(int);
extern int SgnHighByte(int);
extern int ByteToInt(byte);

#endif /* ASM_H */
