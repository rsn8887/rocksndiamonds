#ifndef __KEYBOARD_SWITCH_H__
#define __KEYBOARD_SWITCH_H__

#ifdef __cplusplus
extern "C" {
#endif

void kbdswitch_get(char *title, const char *initial_text, int maxLen, int multiline, char *buf);

#ifdef __cplusplus
}
#endif

#endif
