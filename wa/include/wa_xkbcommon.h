#ifndef _WA_XKBCOMMON_H_
#define _WA_XKBCOMMON_H_

#include "wa_keys.h"
#include <xkbcommon/xkbcommon.h>

wa_key_t wa_xkb_to_wa_key(xkb_keysym_t keysym);

#endif // _WA_XKBCOMMON_H_
