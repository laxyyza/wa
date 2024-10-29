#ifndef _WA_EVENT_H_
#define _WA_EVENT_H_

#include <stdint.h>
#include <stdbool.h>
#include "wa_keys.h"

#define WA_KEY_NAME_LEN 32

enum wa_event_type
{
    WA_EVENT_KEYBOARD,
    WA_EVENT_POINTER,
    WA_EVENT_RESIZE,
    WA_EVENT_MOUSE_BUTTON,
    WA_EVENT_MOUSE_WHEEL,
};

typedef struct 
{
	wa_key_t key;
	bool pressed;
} wa_event_key_t;

typedef struct 
{
	int x;
	int y;
} wa_event_pointer_t;

typedef struct 
{
	int w;
	int h;
	int old_w;
	int old_h;
} wa_event_resize_t;

typedef struct 
{
	wa_mouse_butt_t button;
	bool pressed;
} wa_event_mouse_t;

typedef struct 
{
	int32_t value;
	int32_t axis;
} wa_event_wheel_t;

typedef struct  
{
    enum wa_event_type type;
    union {
		wa_event_key_t		keyboard;
		wa_event_pointer_t	pointer;
		wa_event_resize_t	resize;
		wa_event_mouse_t	mouse;
		wa_event_wheel_t	wheel;
    };
} wa_event_t;

#endif // _WA_EVENT_H_
