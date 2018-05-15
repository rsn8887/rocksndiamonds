// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// gadgets.h
// ============================================================================

#ifndef GADGETS_H
#define GADGETS_H

#include "system.h"


#define GADGET_FRAME_DELAY_FIRST	250	/* delay after first click */
#define GADGET_FRAME_DELAY		100	/* delay for pressed butten */

/* gadget types */
#define GD_TYPE_NORMAL_BUTTON		(1 << 0)
#define GD_TYPE_TEXT_BUTTON		(1 << 1)
#define GD_TYPE_CHECK_BUTTON		(1 << 2)
#define GD_TYPE_RADIO_BUTTON		(1 << 3)
#define GD_TYPE_DRAWING_AREA		(1 << 4)
#define GD_TYPE_TEXT_INPUT_ALPHANUMERIC	(1 << 5)
#define GD_TYPE_TEXT_INPUT_NUMERIC	(1 << 6)
#define GD_TYPE_TEXT_AREA		(1 << 7)
#define GD_TYPE_SELECTBOX		(1 << 8)
#define GD_TYPE_SCROLLBAR_VERTICAL	(1 << 9)
#define GD_TYPE_SCROLLBAR_HORIZONTAL	(1 << 10)

#define GD_TYPE_BUTTON			(GD_TYPE_NORMAL_BUTTON | \
					 GD_TYPE_TEXT_BUTTON | \
					 GD_TYPE_CHECK_BUTTON | \
					 GD_TYPE_RADIO_BUTTON)
#define GD_TYPE_SCROLLBAR		(GD_TYPE_SCROLLBAR_VERTICAL | \
					 GD_TYPE_SCROLLBAR_HORIZONTAL)
#define GD_TYPE_TEXT_INPUT		(GD_TYPE_TEXT_INPUT_ALPHANUMERIC | \
					 GD_TYPE_TEXT_INPUT_NUMERIC)

/* gadget events */
#define GD_EVENT_PRESSED		(1 << 0)
#define GD_EVENT_RELEASED		(1 << 1)
#define GD_EVENT_MOVING			(1 << 2)
#define GD_EVENT_REPEATED		(1 << 3)
#define GD_EVENT_OFF_BORDERS		(1 << 4)
#define GD_EVENT_TEXT_RETURN		(1 << 5)
#define GD_EVENT_TEXT_LEAVING		(1 << 6)
#define GD_EVENT_INFO_ENTERING		(1 << 7)
#define GD_EVENT_INFO_LEAVING		(1 << 8)
#define GD_EVENT_PIXEL_PRECISE		(1 << 9)

/* gadget button states */
#define GD_BUTTON_UNPRESSED		0
#define GD_BUTTON_PRESSED		1

/* gadget structure constants */
#define MAX_GADGET_TEXTSIZE		1024
#define MAX_INFO_TEXTSIZE		1024

/* gadget creation tags */
#define GDI_END				0
#define GDI_CUSTOM_ID			1
#define GDI_CUSTOM_TYPE_ID		2
#define GDI_X				3
#define GDI_Y				4
#define GDI_WIDTH			5
#define GDI_HEIGHT			6
#define GDI_TYPE			7
#define GDI_STATE			8
#define GDI_CHECKED			9
#define GDI_RADIO_NR			10
#define GDI_NUMBER_VALUE		11
#define GDI_NUMBER_MIN			12
#define GDI_NUMBER_MAX			13
#define GDI_TEXT_VALUE			14
#define GDI_TEXT_SIZE			15
#define GDI_TEXT_FONT			16
#define GDI_TEXT_FONT_ACTIVE		17
#define GDI_TEXT_FONT_UNSELECTABLE	18
#define GDI_SELECTBOX_OPTIONS		19
#define GDI_SELECTBOX_INDEX		20
#define GDI_SELECTBOX_CHAR_UNSELECTABLE	21
#define GDI_DESIGN_UNPRESSED		22
#define GDI_DESIGN_PRESSED		23
#define GDI_ALT_DESIGN_UNPRESSED	24
#define GDI_ALT_DESIGN_PRESSED		25
#define GDI_BORDER_SIZE			26
#define GDI_BORDER_SIZE_SELECTBUTTON	27
#define GDI_DESIGN_WIDTH		28
#define GDI_DECORATION_DESIGN		29
#define GDI_DECORATION_POSITION		30
#define GDI_DECORATION_SIZE		31
#define GDI_DECORATION_SHIFTING		32
#define GDI_DECORATION_MASKED		33
#define GDI_EVENT_MASK			34
#define GDI_EVENT			35
#define GDI_CALLBACK_INFO		36
#define GDI_CALLBACK_ACTION		37
#define GDI_AREA_SIZE			38
#define GDI_ITEM_SIZE			39
#define GDI_SCROLLBAR_ITEMS_MAX		40
#define GDI_SCROLLBAR_ITEMS_VISIBLE	41
#define GDI_SCROLLBAR_ITEM_POSITION	42
#define GDI_WHEEL_AREA_X		43
#define GDI_WHEEL_AREA_Y		44
#define GDI_WHEEL_AREA_WIDTH		45
#define GDI_WHEEL_AREA_HEIGHT		46
#define GDI_INFO_TEXT			47
#define GDI_ACTIVE			48
#define GDI_DIRECT_DRAW			49

/* gadget deactivation hack */
#define GDI_ACTIVE_POS(a)		((a) < 0 ? POS_OFFSCREEN : (a))


typedef void (*gadget_function)(void *);

struct GadgetBorder
{
  int xsize, ysize;			/* size of gadget border */
  int xsize_selectbutton;		/* for selectbox gadgets */
  int width;				/* for selectbox/text input gadgets */
};

struct GadgetDesign
{
  Bitmap *bitmap;			/* Bitmap with gadget surface */
  int x, y;				/* position of rectangle in Bitmap */
};

struct GadgetDecoration
{
  struct GadgetDesign design;		/* decoration design structure */
  int x, y;				/* position of deco on the gadget */
  int width, height;			/* width and height of decoration */
  int xshift, yshift;			/* deco shifting when gadget pressed */
  boolean masked;			/* draw decoration masked over button */
};

struct GadgetEvent
{
  unsigned int type;			/* event type */
  int button;				/* button number for button events */
  int mx, my;				/* raw gadget position at event time */
  int x, y;				/* gadget position at event time */
  boolean off_borders;			/* mouse pointer outside gadget? */
  int item_x, item_y, item_position;	/* new item position */
};

struct GadgetDrawingArea
{
  int area_xsize, area_ysize;		/* size of drawing area (in items) */
  int item_xsize, item_ysize;		/* size of each item in drawing area */
};

struct GadgetTextButton
{
  char value[MAX_GADGET_TEXTSIZE + 1];	/* text written on the button */
  int size;				/* maximal size of button text */
};

struct GadgetTextInput
{
  char value[MAX_GADGET_TEXTSIZE + 1];	/* text string in input field */
  char last_value[MAX_GADGET_TEXTSIZE + 1];/* last text string in input field */
  int cursor_position;			/* actual text cursor position */
  int number_value;			/* integer value, if numeric */
  int number_min;			/* minimal allowed numeric value */
  int number_max;			/* maximal allowed numeric value */
  int size;				/* maximal size of input text */
};

struct GadgetTextArea
{
  char value[MAX_GADGET_TEXTSIZE + 1];	/* text string in input field */
  char last_value[MAX_GADGET_TEXTSIZE + 1];/* last text string in input field */
  int cursor_position;			/* actual text cursor position */
  int cursor_x;				/* actual x cursor position */
  int cursor_y;				/* actual y cursor position */
  int cursor_x_preferred;		/* "preferred" x cursor position */
  int size;				/* maximal size of input text */
  int xsize, ysize;			/* size of text area (in chars) */
};

struct GadgetSelectbox
{
  struct ValueTextInfo *options;	/* pointer to text/value array */
  int index;				/* index of actual text string */
  int size;				/* maximal size of text strings */
  char char_unselectable;		/* first char of unselectable options */

  /* automatically determined values */
  int x, y;				/* open selectbox position */
  int width, height;			/* open selectbox size */
  int num_values;			/* number of text strings */
  Pixel inverse_color;			/* color for highlighting */

  /* runtime values */
  boolean open;				/* opening state of selectbox */
  boolean stay_open;			/* open after button release */
  int current_index;			/* index of text while selecting */
};

struct GadgetScrollbar
{
  int items_max;			/* number of items to access */
  int items_visible;			/* number of visible items */
  int item_position;			/* actual item position */
  int size_min;				/* minimal scrollbar size */
  int size_max;				/* this is either width or height */
  int size_max_cmp;			/* needed for minimal scrollbar size */
  int size;				/* scrollbar size on screen */
  int position;				/* scrollbar position on screen */
  int position_max;			/* bottom/right scrollbar position */
  int drag_position;			/* drag position on scrollbar */
  int correction;			/* scrollbar position correction */
};

struct GadgetWheelArea
{
  int x, y;				/* active area for wheel (start) */
  int width, height;			/* active area for wheel (size) */ 
};

struct GadgetInfo
{
  boolean deactivated;			/* flag to deactivate gadget */

  int id;				/* internal gadget identifier */
  int custom_id;			/* custom gadget identifier */
  int custom_type_id;			/* custom gadget type identifier */
  char info_text[MAX_INFO_TEXTSIZE + 1];/* short popup info text */
  int x, y;				/* gadget position */
  int width, height;			/* gadget size */
  unsigned int type;			/* type (button, text input, ...) */
  unsigned int state;			/* state (pressed, released, ...) */
  boolean checked;			/* check/radio button state */
  int radio_nr;				/* number of radio button series */
  boolean mapped;			/* gadget is mapped on the screen */
  boolean active;			/* gadget is active */
  boolean direct_draw;			/* directly draw to frontbuffer */
  int font;				/* font to use when inactive */
  int font_active;			/* font to use when active */
  int font_unselectable;		/* font to use when unselectable */
  struct GadgetBorder border;		/* gadget border design */
  struct GadgetDesign design[2];	/* 0: normal; 1: pressed */
  struct GadgetDesign alt_design[2];	/* alternative design */
  struct GadgetDecoration deco;		/* decoration on top of gadget */
  unsigned int event_mask;		/* possible events for this gadget */
  struct GadgetEvent event;		/* actual gadget event */
  gadget_function callback_info;	/* function for pop-up info text */
  gadget_function callback_action;	/* function for gadget action */
  struct GadgetDrawingArea drawing;	/* fields for drawing area gadget */
  struct GadgetTextButton textbutton;	/* fields for text button gadget */
  struct GadgetTextInput textinput;	/* fields for text input gadget */
  struct GadgetTextArea textarea;	/* fields for text area gadget */
  struct GadgetSelectbox selectbox;	/* fields for selectbox gadget */
  struct GadgetScrollbar scrollbar;	/* fields for scrollbar gadget */
  struct GadgetWheelArea wheelarea;	/* fields for scroll wheel area */
  struct GadgetInfo *next;		/* next list entry */
};


void InitGadgetsSoundCallback(void (*activating_function)(void),
			      void (*selecting_function)(void));

struct GadgetInfo *CreateGadget(int, ...);
void FreeGadget(struct GadgetInfo *);

void ModifyGadget(struct GadgetInfo *, int, ...);
void RedrawGadget(struct GadgetInfo *);

void MapGadget(struct GadgetInfo *);
void UnmapGadget(struct GadgetInfo *);
void UnmapAllGadgets();
void RemapAllGadgets();

boolean anyTextInputGadgetActive();
boolean anyTextAreaGadgetActive();
boolean anySelectboxGadgetActive();
boolean anyScrollbarGadgetActive();
boolean anyTextGadgetActive();

void ClickOnGadget(struct GadgetInfo *, int);

boolean HandleGadgets(int, int, int);
boolean HandleGadgetsKeyInput(Key);

#endif	/* GADGETS_H */
