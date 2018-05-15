// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// text.h
// ============================================================================

#ifndef TEXT_H
#define TEXT_H

#include "system.h"


/* default fonts */
#define FONT_INITIAL_1		0
#define FONT_INITIAL_2		1
#define FONT_INITIAL_3		2
#define FONT_INITIAL_4		3

/* font colors */
#define FC_RED			FONT_INITIAL_1
#define FC_BLUE			FONT_INITIAL_2
#define FC_GREEN		FONT_INITIAL_3
#define FC_YELLOW		FONT_INITIAL_4

/* text output definitions */
#define MAX_OUTPUT_LINESIZE	1024

/* special constants for old ISO-8859-1 character byte values */
#define CHAR_BYTE_UMLAUT_A	((char)0xc4)
#define CHAR_BYTE_UMLAUT_O	((char)0xd6)
#define CHAR_BYTE_UMLAUT_U	((char)0xdc)
#define CHAR_BYTE_UMLAUT_a	((char)0xe4)
#define CHAR_BYTE_UMLAUT_o	((char)0xf6)
#define CHAR_BYTE_UMLAUT_u	((char)0xfc)
#define CHAR_BYTE_SHARP_S	((char)0xdf)
#define CHAR_BYTE_COPYRIGHT	((char)0xa9)
#define CHAR_BYTE_REGISTERED	((char)0xae)
#define CHAR_BYTE_DEGREE	((char)0xb0)
#define CHAR_BYTE_CURSOR	((char)0xa0)

/* special character mapping for default fonts */
#define FONT_ASCII_CURSOR	((char)160)
#define FONT_ASCII_BUTTON	((char)128)
#define FONT_ASCII_UP		((char)129)
#define FONT_ASCII_DOWN		((char)130)
#define FONT_ASCII_LEFT		((char)'<')
#define FONT_ASCII_RIGHT	((char)'>')

#define MAP_FONT_ASCII(c)	((c) >= 'a' && (c) <= 'z' ? 'A' + (c) - 'a' : \
				 (c) == CHAR_BYTE_COPYRIGHT  ?  96 :	\
				 (c) == CHAR_BYTE_UMLAUT_a   ?  97 :	\
				 (c) == CHAR_BYTE_UMLAUT_A   ?  97 :	\
				 (c) == CHAR_BYTE_UMLAUT_o   ?  98 :	\
				 (c) == CHAR_BYTE_UMLAUT_O   ?  98 :	\
				 (c) == CHAR_BYTE_UMLAUT_u   ?  99 :	\
				 (c) == CHAR_BYTE_UMLAUT_U   ?  99 :	\
				 (c) == CHAR_BYTE_DEGREE     ? 100 :	\
				 (c) == CHAR_BYTE_REGISTERED ? 101 :	\
				 (c) == FONT_ASCII_CURSOR    ? 102 :	\
				 (c) == FONT_ASCII_BUTTON    ? 109 :	\
				 (c) == FONT_ASCII_UP	     ? 110 :	\
				 (c) == FONT_ASCII_DOWN	     ? 111 :	\
				 (c))

/* 64 regular ordered ASCII characters, 6 special characters, 1 cursor char. */
#define MIN_NUM_CHARS_PER_FONT			64
#define DEFAULT_NUM_CHARS_PER_FONT		(MIN_NUM_CHARS_PER_FONT + 6 +1)
#define DEFAULT_NUM_CHARS_PER_LINE		16


/* font structure definitions */

void InitFontInfo(struct FontBitmapInfo *, int,
		  int (*function1)(int), int (*function2)(char *));
void FreeFontInfo(struct FontBitmapInfo *);

struct FontBitmapInfo *getFontBitmapInfo(int);

int getFontWidth(int);
int getFontHeight(int);
int getFontDrawOffsetX(int);
int getFontDrawOffsetY(int);
int getTextWidth(char *, int);

void getFontCharSource(int, char, Bitmap **, int *, int *);

int maxWordLengthInString(char *);

void DrawInitText(char *, int, int);

void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawTextS(int, int, int, char *);
void DrawTextSCentered(int, int, char *);
void DrawTextSAligned(int, int, char *, int, int);
void DrawTextAligned(int, int, char *, int, int);
void DrawText(int, int, char *, int);
void DrawTextExt(DrawBuffer *, int, int, char *, int, int);

char *GetTextBufferFromFile(char *, int);
int DrawTextBuffer(int, int, char *, int, int, int, int, int, int,
		   boolean, boolean, boolean);
int DrawTextBufferVA(int, int, char *, va_list, int, int, int, int, int, int,
		     boolean, boolean, boolean);
int DrawTextFile(int, int, char *, int, int, int, int, int, int,
		 boolean, boolean, boolean);

#endif	/* TEXT_H */
