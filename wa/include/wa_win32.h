#ifndef _WA_WIN32_H_
#define _WA_WIN32_H_

#include "wa.h"
#include "wa_log.h"
#include <GL/gl.h>
#include <GL/wglext.h>
#include <windows.h>

#define KEY_COUNT 256

typedef struct wa_window
{
    HWND hwnd;
    HINSTANCE instance;
    RECT rect;
    HDC hdc;
    HGLRC hglrc;

    const char* class_name;

    wa_state_t state;
    uint8_t key_states[KEY_COUNT];

    bool running;
} wa_window_t;

#endif // _WA_WIN32_H_
