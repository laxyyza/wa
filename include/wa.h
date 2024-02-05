#ifndef _WA_H_
#define _WA_H_

#include <stdbool.h>
#include "wa_event.h"

typedef struct wa_window wa_window_t;

typedef struct 
{
    void (*event)(wa_window_t* window, const wa_event_t* event, void* data);
    void (*update)(wa_window_t* window, void* data);
    void (*close)(wa_window_t* window, void* data);
} wa_callbacks_t;

typedef struct 
{
    int w;
    int h;
    bool fullscreen;
    const char* title;

    struct { /* Wayland specific */
        bool tear;
        const char* app_id;
    } wayland;
} wa_window_state_t;

typedef struct 
{
    wa_window_state_t   window;
    wa_callbacks_t      callbacks;
    void*               user_data;
} wa_state_t;

wa_window_t*    wa_window_create(const char* title, int w, int h, bool fullscrenn);
wa_window_t*    wa_window_create_from_state(wa_state_t* state);
void            wa_state_set_default(wa_state_t* state);
void            wa_window_set_callbacks(wa_window_t* window, const wa_callbacks_t* callbacks);
wa_state_t*     wa_window_get_state(wa_window_t* window);
int             wa_window_mainloop(wa_window_t* window);
void            wa_window_set_fullscreen(wa_window_t* window, bool fullscreen);
void            wa_window_stop(wa_window_t* window);
void            wa_window_delete(wa_window_t* window);

#endif // _WA_H_
