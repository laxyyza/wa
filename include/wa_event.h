#ifndef _WA_EVENT_H_
#define _WA_EVENT_H_

#include <stdint.h>

enum wa_event_type
{
    WA_EVENT_KEYBOARD,
    WA_EVENT_POINTER,
    WA_EVENT_RESIZE
};

enum wa_keyboard_state
{
    WA_KEYBOARD_PRESSED,
    WA_KEYBOARD_RELEASED,
};

typedef struct  
{
    enum wa_event_type type;
    union {
        struct {
            uint32_t key;
            uint32_t unicode;
            enum wa_keyboard_state state;
        } keyboard;
        struct {
            int x;
            int h;
        } pointer;
        struct {
            int w;
            int h;
            int old_w;
            int old_h;
        } resize;
    };
} wa_event_t;

#endif // _WA_EVENT_H_
