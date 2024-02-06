#ifndef _WA_WAYLAND_H_
#define _WA_WAYLAND_H_

#include "wa.h"
#include "xdg-shell.h"
#include "xdg-decoration.h"

#include <stdlib.h>
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

#endif // _WA_WAYLAND_H_
