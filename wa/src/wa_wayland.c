#include "wa_wayland.h"

#include <string.h>
#include "wa_log.h"

#define WA_DEFAULT_APP_ID "wa"
#define WA_DEFAULT_TITLE "WA - Window Abstraction"

#define WA_EGL_ERROR eglGetErrorString(eglGetError())
#define WA_CMP(x) !strcmp(interface, x)

static void 
wa_default_event(_WA_UNUSED wa_window_t* window, _WA_UNUSED const wa_event_t* event, _WA_UNUSED void* data)
{

}

static void 
wa_default_update(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{

}

static void 
wa_default_draw(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void 
wa_default_close(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{
}

static void
wa_default_focus(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{
}

static void
wa_default_unfocus(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{
}

static void 
wa_window_resize(wa_window_t* window)
{
    const int w = window->state.window.w;
    const int h = window->state.window.h;

    if (window->wl_egl_window == NULL)
        return;

    wl_egl_window_resize(window->wl_egl_window, w, h, 0, 0);
    glViewport(0, 0, w, h);
    wa_log(WA_VBOSE, "Window resized: %dx%d\n", w, h);
}

static void 
wa_xdg_shell_ping(_WA_UNUSED void* data, struct xdg_wm_base* xdg_shell, uint32_t serial)
{
    xdg_wm_base_pong(xdg_shell, serial);
    wa_log(WA_VBOSE, "XDG Shell pong!\n");
}

static void 
wa_reg_glob(void* data, struct wl_registry* reg, uint32_t name, const char* interface, uint32_t version)
{
    wa_window_t* window = data;

    if (WA_CMP(wl_compositor_interface.name))
    {
        window->wl_compositor = wl_registry_bind(reg, name, &wl_compositor_interface, version);
    }
    else if (WA_CMP(xdg_wm_base_interface.name))
    {
        window->xdg_shell = wl_registry_bind(reg, name, &xdg_wm_base_interface, version);
        window->xdg_shell_listener.ping = wa_xdg_shell_ping;
        xdg_wm_base_add_listener(window->xdg_shell, &window->xdg_shell_listener, window);
    }
    else if (WA_CMP(wl_seat_interface.name))
    {
        window->wl_seat = wl_registry_bind(reg, name, &wl_seat_interface, version);
        window->wl_seat_listener.capabilities = wa_seat_cap;
        window->wl_seat_listener.name = wa_seat_name;
        wl_seat_add_listener(window->wl_seat, &window->wl_seat_listener, window);
    }
    else if (WA_CMP(wl_output_interface.name))
    {
        if (window->wl_outputs == NULL)
        {
            window->wl_outputs = calloc(1, sizeof(void*));
            window->n_outputs = 1;
        }
        else 
        {
            window->n_outputs++;
            window->wl_outputs = realloc(window->wl_outputs, sizeof(void*) * window->n_outputs);
        }

        window->wl_outputs[window->n_outputs - 1] = wl_registry_bind(reg, name, &wl_output_interface, version);
    }
    else if (WA_CMP(zxdg_decoration_manager_v1_interface.name))
    {
        window->xdg_decoration_manager = wl_registry_bind(reg, name, &zxdg_decoration_manager_v1_interface, version);
    }
    else
    {
        wa_log(WA_VBOSE, "Interface: %u '%s' v%u\n", name, interface, version);
        return;
    }

    wa_log(WA_VBOSE, "Using Interface: %u '%s' v%u\n", name, interface, version);
}

static void 
wa_reg_glob_rm(_WA_UNUSED void* data, _WA_UNUSED struct wl_registry* reg, uint32_t name)
{
    wa_log(WA_WARN, "Interface: %u removed.\n", name);
}

static void 
wa_draw(wa_window_t* window)
{
    window->state.callbacks.draw(window, window->state.user_data);

    eglSwapBuffers(window->egl_display, window->egl_surface);
}

static void 
wa_frame_done(void* data, struct wl_callback* callback, _WA_UNUSED uint32_t callback_data)
{
    wa_window_t* window = data;
    wl_callback_destroy(callback);
    callback = wl_surface_frame(window->wl_surface);
    wl_callback_add_listener(callback, &window->wl_frame_done_listener, data);

    wa_draw(window);
}

static void 
wa_xdg_surface_conf(_WA_UNUSED void* data, struct xdg_surface* xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
    wa_log(WA_VBOSE, "XDG Surface configire\n");
}

static void 
wa_init_xdg_decoration(wa_window_t* window)
{
    if (window->xdg_decoration_manager)
    {
        window->xdg_toplevel_decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
                window->xdg_decoration_manager, 
                window->xdg_toplevel
        );
        zxdg_toplevel_decoration_v1_set_mode(
                window->xdg_toplevel_decoration, 
                ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE
        );
    }
}

static void 
wa_toplevel_conf(void* data, _WA_UNUSED struct xdg_toplevel* toplevel, int w, int h, struct wl_array* states)
{
    wa_window_t* window = data;

    if (w == 0 || h == 0)
        return;

    if (w != window->state.window.w || h != window->state.window.h)
    {
        window->state.window.w = w;
        window->state.window.h = h;

        wa_window_resize(window);

        wa_event_t event = {
            .type = WA_EVENT_RESIZE,
            .resize.w = w,
            .resize.h = h
        };

        window->state.callbacks.event(window, &event, window->state.user_data);
    }

    uint16_t window_state = 0;

    uint32_t* state;
    wl_array_for_each(state, states)
    {
        switch (*state)
        {
            case XDG_TOPLEVEL_STATE_FULLSCREEN:
                window_state |= WA_STATE_FULLSCREEN;
                break;
            case XDG_TOPLEVEL_STATE_MAXIMIZED:
                window_state |= WA_STATE_MAXIMIZED;
                break;
            case XDG_TOPLEVEL_STATE_ACTIVATED:
                window_state |= WA_STATE_ACTIVE;
                break;
        }
    }
    window->state.window.state = window_state;
    wa_log(WA_VBOSE, "XDG Toplevel configure\n");
}

static void 
wa_toplevel_close(void* data, _WA_UNUSED struct xdg_toplevel* toplevel)
{
    wa_window_t* window = data;

    window->state.callbacks.close(window, window->state.user_data);

    window->running = false;

    wa_log(WA_VBOSE, "XDG Toplevel close\n");
}

static void 
wa_toplevel_conf_bounds(_WA_UNUSED void* data, _WA_UNUSED struct xdg_toplevel* toplevel, _WA_UNUSED int w, _WA_UNUSED int h)
{
    // wa_log(WA_DEBUG, "toplevel bounds: %dx%d\n", w, h);
}

static void 
wa_toplevel_wm_caps(_WA_UNUSED void* data, _WA_UNUSED struct xdg_toplevel* toplevel, _WA_UNUSED struct wl_array* caps)
{
    uint32_t* cap;
    wl_array_for_each(cap, caps)
    {
        switch (*cap)
        {
            case XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE:
                wa_log(WA_VBOSE, "CAP: MAXIMIZE!\n");
                break;
            case XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE:
                wa_log(WA_VBOSE, "CAP: MINIMIZE!\n");
                break;
            case XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN:
                wa_log(WA_VBOSE, "CAP: FULLSCREEN!\n");
                break;
            case XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU:
                wa_log(WA_VBOSE, "CAP: WINDOW MENU!\n");
                break;
            default:
                wa_log(WA_VBOSE, "CAP: Unknown %u\n", *cap);
                break;
        }
    }
}

static void 
wa_output_geo(_WA_UNUSED void* data, _WA_UNUSED struct wl_output* output, 
              int x, int y, int phy_w, int phy_h, int subpixel, 
              const char* make, const char* model, int transform)
{
    wa_log(WA_VBOSE, "Output\n\tx/y:\t%dx%d\n\tphy w/h:\t%dx%d\n\tsubpixel:\t%d\n\tmake:\t'%s'\n\tmodel:\t'%s'\n\ttransform:\t%d\n",
            x, y, phy_w, phy_h, subpixel, make, model, transform);
}

static void 
wa_output_mode(_WA_UNUSED void* data, _WA_UNUSED struct wl_output* output, uint32_t flags, int w, int h, int refresh_rate)
{
    wa_log(WA_VBOSE, "Mode: %dx%d @ %d (flags: %u)\n", w, h, refresh_rate, flags);
}

static void 
wa_output_done(_WA_UNUSED void* data, _WA_UNUSED struct wl_output* output)
{
    wa_log(WA_VBOSE, "%s()\n", __func__);
}

static void 
wa_output_scale(_WA_UNUSED void* data, _WA_UNUSED struct wl_output* output, int factor)
{
    wa_log(WA_VBOSE, "Scale:\t%d\n", factor);
}

static void 
wa_output_name(_WA_UNUSED void* data, _WA_UNUSED struct wl_output* output, const char* name)
{
    wa_log(WA_VBOSE, "Name:\t'%s'\n", name);
}

static void 
wa_output_desc(_WA_UNUSED void* data, _WA_UNUSED struct wl_output* output, const char* desc)
{
    wa_log(WA_VBOSE, "Desc:\t'%s'\n", desc);
}

static const char* 
eglGetErrorString(EGLint error)
{
    switch(error)
    {
        case EGL_SUCCESS: return "No error";
        case EGL_NOT_INITIALIZED: return "EGL not initialized or failed to initialize";
        case EGL_BAD_ACCESS: return "Resource inaccessible";
        case EGL_BAD_ALLOC: return "Cannot allocate resources";
        case EGL_BAD_ATTRIBUTE: return "Unrecognized attribute or attribute value";
        case EGL_BAD_CONTEXT: return "Invalid EGL context";
        case EGL_BAD_CONFIG: return "Invalid EGL frame buffer configuration";
        case EGL_BAD_CURRENT_SURFACE: return "Current surface is no longer valid";
        case EGL_BAD_DISPLAY: return "Invalid EGL display";
        case EGL_BAD_SURFACE: return "Invalid surface";
        case EGL_BAD_MATCH: return "Inconsistent arguments";
        case EGL_BAD_PARAMETER: return "Invalid argument";
        case EGL_BAD_NATIVE_PIXMAP: return "Invalid native pixmap";
        case EGL_BAD_NATIVE_WINDOW: return "Invalid native window";
        case EGL_CONTEXT_LOST: return "Context lost";
        default: return "Unknown";
    }
}

static bool 
wa_window_init_wayland(wa_window_t* window)
{
    if ((window->wl_display = wl_display_connect(NULL)) == NULL)
    {
        wa_logf(WA_FATAL, "Failed to connect to Wayland display!\n");
        return false;
    }

    if ((window->wl_registry = wl_display_get_registry(window->wl_display)) == NULL)
    {
        wa_logf(WA_FATAL, "Failed to get registry!\n");
        return false;
    }
    window->wl_registry_listener.global = wa_reg_glob;
    window->wl_registry_listener.global_remove = wa_reg_glob_rm;
    wl_registry_add_listener(window->wl_registry, &window->wl_registry_listener, window);
    if (wl_display_roundtrip(window->wl_display) == -1)
        wa_log(WA_ERROR, "wl_display_roundtrip() failed!\n");

    /* wl_surface */
    if ((window->wl_surface = wl_compositor_create_surface(window->wl_compositor)) == NULL)
    {
        wa_logf(WA_FATAL, "Failed to create wl_surface!\n");
        return false;
    }
    if ((window->frame_done_callback = wl_surface_frame(window->wl_surface)) == NULL)
    {
        wa_logf(WA_FATAL, "Failed to create wl_callback for wl_surface!\n");
        return false;
    }
    window->wl_frame_done_listener.done = wa_frame_done;
    wl_callback_add_listener(window->frame_done_callback, &window->wl_frame_done_listener, window);

    /* XDG Surface */
    if ((window->xdg_surface = xdg_wm_base_get_xdg_surface(window->xdg_shell, window->wl_surface)) == NULL)
    {
        wa_logf(WA_FATAL, "Failed to create xdg_surface!\n");
        return false;
    }
    window->xdg_surface_listener.configure = wa_xdg_surface_conf;
    xdg_surface_add_listener(window->xdg_surface, &window->xdg_surface_listener, window);

    /* XDG Toplevel */
    if ((window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface)) == NULL)
    {
        wa_logf(WA_FATAL, "Failed to get XDG Toplevel!\n");
        return false;
    }
    window->xdg_toplevel_listener.configure = wa_toplevel_conf;
    window->xdg_toplevel_listener.close = wa_toplevel_close;
    window->xdg_toplevel_listener.configure_bounds = wa_toplevel_conf_bounds;
    window->xdg_toplevel_listener.wm_capabilities = wa_toplevel_wm_caps;
    xdg_toplevel_add_listener(window->xdg_toplevel, &window->xdg_toplevel_listener, window);

    const char* title = (window->state.window.title) ? window->state.window.title : WA_DEFAULT_TITLE;
    xdg_toplevel_set_title(window->xdg_toplevel, title);

    const char* app_id = (window->state.window.wayland.app_id) ? window->state.window.wayland.app_id : WA_DEFAULT_APP_ID;
    xdg_toplevel_set_app_id(window->xdg_toplevel, app_id);

    wa_window_set_fullscreen(window, window->state.window.state & WA_STATE_FULLSCREEN);

    /* wl_output */
    window->wl_output_listener.geometry = wa_output_geo;
    window->wl_output_listener.mode = wa_output_mode;
    window->wl_output_listener.done = wa_output_done;
    window->wl_output_listener.scale = wa_output_scale;
    window->wl_output_listener.name = wa_output_name;
    window->wl_output_listener.description = wa_output_desc;
    for (size_t i = 0; i < window->n_outputs; i++)
        wl_output_add_listener(window->wl_outputs[i], &window->wl_output_listener, window);

    wa_init_xdg_decoration(window);

    wl_surface_commit(window->wl_surface);
    wl_display_roundtrip(window->wl_display);
    return true;
}

static bool 
wa_window_init_egl(wa_window_t* window)
{
    const EGLint context_attr[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 6,

        EGL_CONTEXT_OPENGL_PROFILE_MASK, 
        EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,

        EGL_NONE
    };
    EGLint config_attr[] = {
        EGL_RED_SIZE,       8,
        EGL_GREEN_SIZE,     8,
        EGL_BLUE_SIZE,      8,
        EGL_ALPHA_SIZE,     8,
        EGL_DEPTH_SIZE,     24,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    int n;

    if ((window->egl_display = eglGetDisplay(window->wl_display)) == EGL_NO_DISPLAY)
    {
        wa_logf(WA_FATAL, "EGL Get Display: %s\n", WA_EGL_ERROR);
        return false;
    }

    if (!eglInitialize(window->egl_display, 
                       &window->egl_major, &window->egl_minor))
    {
        wa_logf(WA_FATAL, "EGL Initialize: %s\n", WA_EGL_ERROR);
        return false;
    }

    wa_log(WA_VBOSE, "EGL Display: major: %d, minor: %d\n\tVersion: %s\n\tVendor: %s\n\tExtenstions: %s\n", 
            window->egl_major, window->egl_minor, eglQueryString(window->egl_display, EGL_VERSION),
            eglQueryString(window->egl_display, EGL_VENDOR), 
            eglQueryString(window->egl_display, EGL_EXTENSIONS));

    if (!eglBindAPI(EGL_OPENGL_API))
    {
        wa_logf(WA_ERROR, "EGL Bind API: %s\n", WA_EGL_ERROR);
        return false;
    }

    if (!eglChooseConfig(window->egl_display, config_attr, 
                         &window->egl_config, 1, &n) || n != 1)
    {
        wa_logf(WA_ERROR, "EGL Choose Config: %s\n", WA_EGL_ERROR);
        return false;
    }

    if ((window->egl_context = eglCreateContext(window->egl_display, 
                                                window->egl_config, 
                                                EGL_NO_CONTEXT, 
                                                context_attr)) == EGL_NO_CONTEXT)
    {
        wa_logf(WA_ERROR, "EGL Create Context: %s\n", WA_EGL_ERROR);
        return false;
    }

    if ((window->wl_egl_window = wl_egl_window_create(window->wl_surface, 
                                                      window->state.window.w, 
                                                      window->state.window.h)) == EGL_NO_SURFACE)
    {
        wa_logf(WA_ERROR, "wl_egl_window_create(): %s\n", WA_EGL_ERROR);
        return false;
    }

    if ((window->egl_surface = eglCreateWindowSurface(window->egl_display, 
                                                      window->egl_config, 
                                                      (EGLNativeWindowType)window->wl_egl_window, 
                                                      NULL)) == EGL_NO_SURFACE)
    {
        wa_logf(WA_ERROR, "EGL Create Window Surface: %s\n", WA_EGL_ERROR);
        return false;
    }

    if (!eglMakeCurrent(window->egl_display, window->egl_surface, 
                        window->egl_surface, window->egl_context))
    {
        wa_logf(WA_ERROR, "EGL Make current: %s\n", WA_EGL_ERROR);
        return false;
    }

    if (glewInit() != GLEW_OK)
    {
        wa_logf(WA_ERROR, "GLEW init failed: %s\n", glewGetErrorString(glGetError()));
        return false;
    }


    return true;
}

wa_window_t* 
wa_window_create(const char* title, int w, int h, bool fullscreen)
{
    wa_state_t state;
    wa_state_set_default(&state);
    state.window.title = title;
    state.window.state = (fullscreen) ? WA_STATE_FULLSCREEN : 0;
    state.window.w = w;
    state.window.h = h;

    return wa_window_create_from_state(&state);
}

wa_window_t* 
wa_window_create_from_state(wa_state_t* state)
{
    if (!state)
    {
        wa_logf(WA_FATAL, "%s() state is NULL!\n", __func__);
        return NULL;
    }
    
    wa_window_t* window = malloc(sizeof(wa_window_t));
    if (window == NULL)
    {
        wa_logf(WA_FATAL, "malloc() returned NULL!\n");
        return NULL;
    }
    memset(window, 0, sizeof(wa_window_t));

    memcpy(&window->state, state, sizeof(wa_state_t));

    if (!wa_window_init_wayland(window))
    {
        wa_window_delete(window);
        return NULL;
    }

    if (!wa_window_init_egl(window))
    {
        wa_window_delete(window);
        return NULL;
    }

    window->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wl_display_dispatch(window->wl_display);
    
    window->running = true;
    wa_log(WA_INFO, "Window \"%s\" %dx%d created\n", window->state.window.title, window->state.window.w, window->state.window.h);

    return window;
}

void 
wa_state_set_default(wa_state_t* state)
{
    if (!state)
    {
        wa_logf(WA_WARN, "state is NULL!\n");
        return;
    }

    memset(state, 0, sizeof(wa_state_t));

    state->callbacks.event = wa_default_event;
    state->callbacks.update = wa_default_update;
    state->callbacks.close = wa_default_close;
    state->callbacks.draw = wa_default_draw;
    state->callbacks.focus = wa_default_focus;
    state->callbacks.unfocus = wa_default_unfocus;
    state->window.w = 100;
    state->window.h = 100;
    state->window.title = WA_DEFAULT_TITLE;
    state->window.wayland.app_id = WA_DEFAULT_APP_ID;
}

void 
wa_window_set_callbacks(wa_window_t* window, const wa_callbacks_t* callbacks)
{
    if (!window || !callbacks)
        return;

    memcpy(&window->state.callbacks, callbacks, sizeof(wa_callbacks_t));
}

wa_state_t* 
wa_window_get_state(wa_window_t* window)
{
    return &window->state;
}

int 
wa_window_mainloop(wa_window_t* window)
{
    if (!window)
    {
        wa_logf(WA_FATAL, "%s() window is NULL!\n", __func__);
        return -1;
    }

    if (!window->state.callbacks.close || !window->state.callbacks.event || !window->state.callbacks.update)
    {
        wa_logf(WA_FATAL, "%s() state callback(s) are NULL!", __func__);
        return -1;
    }

    wa_draw(window);

    while (window->running)
    {
        if (wl_display_dispatch(window->wl_display) == -1)
        {
            int ret = wl_display_get_error(window->wl_display);
            wa_logf(WA_FATAL, "wl_display_dispatch() failed: %d\n", ret);
            return ret;
        }

        window->state.callbacks.update(window, window->state.user_data);
    }

    return 0;
}

void 
wa_window_set_fullscreen(wa_window_t* window, bool fullscreen)
{
    if (!window)
    {
        wa_logf(WA_WARN, "%s() window is NULL!\n", __func__);
        return;
    }

    if (fullscreen)
        xdg_toplevel_set_fullscreen(window->xdg_toplevel, NULL);
    else
        xdg_toplevel_unset_fullscreen(window->xdg_toplevel);
}

void 
wa_window_stop(wa_window_t* window)
{
    if (!window)
    {
        wa_logf(WA_WARN, "%s() window is NULL!\n", __func__);
        return;
    }
    window->running = false;
}

static void 
wa_window_egl_cleanup(wa_window_t* window)
{
    if (window->egl_display)
    {
        if (window->egl_surface)
            eglDestroySurface(window->egl_display, window->egl_surface);
        if (window->egl_context)
            eglDestroyContext(window->egl_display, window->egl_context);
        eglMakeCurrent(window->egl_display, 0, 0, 0);
        eglTerminate(window->egl_display);
    }
}

static void 
wa_window_xkb_cleanup(wa_window_t* window)
{
    if (window->xkb_context)
        xkb_context_unref(window->xkb_context);
    if (window->xkb_keymap)
        xkb_keymap_unref(window->xkb_keymap);
    if (window->xkb_state)
        xkb_state_unref(window->xkb_state);
}

static void 
wa_window_wayland_cleanup(wa_window_t* window)
{
    if (window->xdg_toplevel_decoration)
        zxdg_toplevel_decoration_v1_destroy(window->xdg_toplevel_decoration);
    if (window->xdg_decoration_manager)
        zxdg_decoration_manager_v1_destroy(window->xdg_decoration_manager);
    if (window->xdg_toplevel)
        xdg_toplevel_destroy(window->xdg_toplevel);
    if (window->xdg_surface)
        xdg_surface_destroy(window->xdg_surface);
    if (window->xdg_shell)
        xdg_wm_base_destroy(window->xdg_shell);
    if (window->wl_registry)
        wl_registry_destroy(window->wl_registry);
    if (window->wl_compositor)
        wl_compositor_destroy(window->wl_compositor);
    if (window->wl_surface)
        wl_surface_destroy(window->wl_surface);
    if (window->wl_keyboard)
        wl_keyboard_destroy(window->wl_keyboard);
    if (window->wl_pointer)
        wl_pointer_destroy(window->wl_pointer);
    if (window->wl_egl_window)
        wl_egl_window_destroy(window->wl_egl_window);
    for (size_t i = 0; i < window->n_outputs; i++)
        if (window->wl_outputs[i])
            wl_output_destroy(window->wl_outputs[i]);

    if (window->wl_display)
        wl_display_disconnect(window->wl_display);
}

void 
wa_window_delete(wa_window_t* window)
{
    if (!window)
        return;

    wa_window_egl_cleanup(window);
    wa_window_xkb_cleanup(window);
    wa_window_wayland_cleanup(window);
        
    free(window);

    wa_log(WA_DEBUG, "Cleanup done.\n");
}
