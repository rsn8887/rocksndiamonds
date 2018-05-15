// ----------------------------------------------------------------------------
// vb_lib.c
// ----------------------------------------------------------------------------

#include "main_sp.h"

#include "vb_lib.h"


/* helper functions for constructs not supported by C */

int MyGetTickCount()
{
  return random_linux_libc(RANDOM_SIMPLE);
}
