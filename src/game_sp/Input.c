// ----------------------------------------------------------------------------
// Input.c
// ----------------------------------------------------------------------------

#include "Input.h"


int map_key_RND_to_SP(int key)
{
  if (key & KEY_BUTTON)
  {
    return (key & MV_UP		? keySpaceUp	:
	    key & MV_LEFT	? keySpaceLeft	:
	    key & MV_DOWN	? keySpaceDown	:
	    key & MV_RIGHT	? keySpaceRight	: keySpace);
  }
  else
  {
    return (key & MV_UP		? keyUp		:
	    key & MV_LEFT	? keyLeft	:
	    key & MV_DOWN	? keyDown	:
	    key & MV_RIGHT	? keyRight	: keyNone);
  }
}

int map_key_SP_to_RND(int key)
{
  return (key == keyUp		? MV_UP			:
	  key == keyLeft	? MV_LEFT		:
	  key == keyDown	? MV_DOWN		:
	  key == keyRight	? MV_RIGHT		:
	  key == keySpaceUp	? KEY_BUTTON | MV_UP	:
	  key == keySpaceLeft	? KEY_BUTTON | MV_LEFT	:
	  key == keySpaceDown	? KEY_BUTTON | MV_DOWN	:
	  key == keySpaceRight	? KEY_BUTTON | MV_RIGHT	:
	  key == keySpace	? KEY_BUTTON		: MV_NONE);
}

void subProcessKeyboardInput(byte action)
{
  DemoKeyCode = map_key_RND_to_SP(action);
}
