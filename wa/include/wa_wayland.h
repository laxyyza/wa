#ifndef _WA_WAYLAND_H_
#define _WA_WAYLAND_H_

#include "wa.h"
#include "xdg-shell.h"
#include "xdg-decoration.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <EGL/egl.h>
#include <xkbcommon/xkbcommon.h>

typedef struct wa_window
{
    struct wl_display*      wl_display;
    struct wl_registry*     wl_registry;
    struct wl_compositor*   wl_compositor;
    struct wl_surface*      wl_surface;
    struct wl_buffer*       wl_buffer;
    struct wl_seat*         wl_seat;
    struct wl_keyboard*     wl_keyboard;
    struct wl_pointer*      wl_pointer;
    struct wl_callback*     frame_done_callback;
    struct wl_egl_window*   wl_egl_window;
    struct wl_output**      wl_outputs;
    size_t                  n_outputs;

    struct xdg_wm_base*     xdg_shell;
    struct xdg_toplevel*    xdg_toplevel;
    struct xdg_surface*     xdg_surface;
    struct zxdg_decoration_manager_v1* xdg_decoration_manager;
    struct zxdg_toplevel_decoration_v1* xdg_toplevel_decoration;

    struct wl_registry_listener     wl_registry_listener;
    struct wl_callback_listener     wl_frame_done_listener;
    struct wl_keyboard_listener     wl_keyboard_listener;
    struct wl_pointer_listener      wl_pointer_listener;
    struct wl_output_listener       wl_output_listener;
    struct wl_seat_listener         wl_seat_listener;
    struct xdg_wm_base_listener     xdg_shell_listener;
    struct xdg_toplevel_listener    xdg_toplevel_listener;
    struct xdg_surface_listener     xdg_surface_listener;

    struct xkb_context*     xkb_context;
    struct xkb_keymap*      xkb_keymap;
    struct xkb_state*       xkb_state;

    EGLDisplay  egl_display;
    EGLSurface  egl_surface;
    EGLContext  egl_context;
    EGLConfig   egl_config;
    EGLint      egl_major;
    EGLint      egl_minor;

    wa_state_t          state;

    bool running;
} wa_window_t;


/* _input */
void wa_kb_map(void* data, struct wl_keyboard* keyboard, uint32_t frmt, int fd, uint32_t size);
void wa_kb_enter(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* array);
void wa_kb_leave(void* data, struct wl_keyboard* keyboard, uint32_t serial, struct wl_surface* surface);
void wa_kb_key(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t t, uint32_t key, uint32_t state);
void wa_kb_mod(void* data, struct wl_keyboard* keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void wa_kb_rep(void* data, struct wl_keyboard* keyboard, int32_t rate, int32_t del);
void wa_point_enter(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
void wa_point_leave(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface);
void wa_point_move(void* data, struct wl_pointer* pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
void wa_point_button(void* data, struct wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
void wa_point_axis(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis_type, wl_fixed_t value);
void wa_point_frame(void* data, struct wl_pointer* pointer);
void wa_point_axis_src(void* data, struct wl_pointer* pointer, uint32_t axis_src);
void wa_point_axis_stop(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis);
void wa_point_axis_discrete(void* data, struct wl_pointer* pointer, uint32_t axis, int32_t discrete);
void wa_point_axis120(void* data, struct wl_pointer* pointer, uint32_t axis_type, int value);
void wa_point_axis_dir(void* data, struct wl_pointer* pointer, uint32_t axis_type, uint32_t dir);
void wa_seat_cap(void* data, struct wl_seat* seat, uint32_t cap);
void wa_seat_name(void* data, struct wl_seat* seat, const char* name);

#endif // _WA_WAYLAND_H_
