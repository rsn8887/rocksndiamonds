#include <string.h>
#include <stdbool.h>

#include <switch.h>
#include "switch_kbd.h"

void kbdswitch_get(char *title, const char *initial_text, int maxLen, int multiline, char *buf) {
	
	Result rc=0;
	
	SwkbdConfig kbd;
	
	rc = swkbdCreate(&kbd, 0);
	
	if (R_SUCCEEDED(rc)) {
		// Select a Preset to use, if any.
		swkbdConfigMakePresetDefault(&kbd);
		//swkbdConfigMakePresetPassword(&kbd);
		//swkbdConfigMakePresetUserName(&kbd);
		//swkbdConfigMakePresetDownloadCode(&kbd);
		
		// Optional, set any text if you want (see swkbd.h).
		//swkbdConfigSetOkButtonText(&kbd, "Submit");
		//swkbdConfigSetLeftOptionalSymbolKey(&kbd, "a");
		//swkbdConfigSetRightOptionalSymbolKey(&kbd, "b");
		//swkbdConfigSetHeaderText(&kbd, "Header");
		//swkbdConfigSetSubText(&kbd, "Sub");
		//swkbdConfigSetGuideText(&kbd, "Guide");
		
		//swkbdConfigSetTextCheckCallback(&kbd, validate_text);//Optional, enable to use TextCheck.
		
		// Set the initial string if you want.
		swkbdConfigSetInitialText(&kbd, initial_text);
		
		// You can set arg fields directly if you want.
		
		rc = swkbdShow(&kbd, buf, 500);
		
		swkbdClose(&kbd);
	}
}
