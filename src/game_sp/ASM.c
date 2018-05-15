// ----------------------------------------------------------------------------
// ASM.c
// ----------------------------------------------------------------------------

#include "ASM.h"


void MovLowByte(int *p, int i)
{
  *p = (*p & 0xff00) | (i & 0xff);
}

void MovHighByte(int *p, int i)
{
  *p = (*p & 0xff) | ((i << 8) & 0xff00);
}

int LowByte(int i)
{
  return (i & 0xff);
}

int HighByte(int i)
{
  return ((i >> 8) & 0xff);
}

int SgnHighByte(int i)
{
  return (signed char)HighByte(i);
}

int ByteToInt(byte b)
{
  return (signed char)b;
}
