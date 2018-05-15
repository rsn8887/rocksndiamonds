// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// types.h
// ============================================================================

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>


#if !defined(PLATFORM_WIN32)
typedef int boolean;
typedef unsigned char byte;
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifdef AUTO
#undef AUTO
#endif

#define TRUE		1
#define FALSE		0
#define AUTO		-1

#ifndef MIN
#define MIN(a,b) 	((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) 	((a) > (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)		((a) < 0 ? -(a) : (a))
#endif

#ifndef SIGN
#define SIGN(a)		((a) < 0 ? -1 : ((a) > 0 ? 1 : 0))
#endif

#ifndef ODD
#define ODD(a)		(((a) & 1) == 1)
#endif

#ifndef EVEN
#define EVEN(a)		(((a) & 1) == 0)
#endif

#define SIZEOF_ARRAY(array, type)	(sizeof(array) / sizeof(type))
#define SIZEOF_ARRAY_INT(array)		SIZEOF_ARRAY(array, int)


struct ListNode
{
  char *key;
  void *content;
  struct ListNode *prev;
  struct ListNode *next;
};
typedef struct ListNode ListNode;

#endif /* TYPES_H */
