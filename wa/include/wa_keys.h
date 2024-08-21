#ifndef _WA_KEYS_H_
#define _WA_KEYS_H_

#ifdef _WA_WAYLAND
#include <xkbcommon/xkbcommon.h>

#define WA_KEY_F XKB_KEY_f
#define WA_KEY_R XKB_KEY_r
#endif // _WA_WAYLAND

#ifdef _WA_WIN32

#define WA_KEY_F 'F'
#define WA_KEY_R 'R'
#endif // _WA_WIN32

#endif
