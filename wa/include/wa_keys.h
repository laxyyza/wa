#ifndef _WA_KEYS_H_
#define _WA_KEYS_H_

#ifdef _WA_WAYLAND
#include <xkbcommon/xkbcommon.h>

#define WA_KEY_F XKB_KEY_f
#define WA_KEY_R XKB_KEY_r
#define WA_KEY_V XKB_KEY_v
#define WA_KEY_D XKB_KEY_d
#define WA_KEY_A XKB_KEY_a
#define WA_KEY_W XKB_KEY_w
#define WA_KEY_S XKB_KEY_s
#define WA_KEY_Q XKB_KEY_q
#endif // _WA_WAYLAND

#ifdef _WA_WIN32

#define WA_KEY_F 'F'
#define WA_KEY_R 'R'
#define WA_KEY_V 'V'
#define WA_KEY_D 'D'
#define WA_KEY_A 'A'
#define WA_KEY_W 'W'
#define WA_KEY_S 'S'
#define WA_KEY_Q 'Q'
#endif // _WA_WIN32

#endif
