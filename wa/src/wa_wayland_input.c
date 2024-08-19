#include "wa_event.h"
#include "wa_wayland.h"
#include "wa_log.h"
#include "xdg-shell.h"

void 
wa_kb_map(void* data, _WA_UNUSED struct wl_keyboard* keyboard, uint32_t frmt, int fd, uint32_t size)
{
    wa_window_t* window = data;
    char* map_str = MAP_FAILED;
    printf("kb_map: frmt: %u, fd: %d, size: %u\n", frmt, fd, size);

    map_str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_str == MAP_FAILED)
    {
        close(fd);
        return;
    }

    if (window->xkb_keymap)
        xkb_keymap_unref(window->xkb_keymap);
    if (window->xkb_state)
        xkb_state_unref(window->xkb_state);

    window->xkb_keymap = xkb_keymap_new_from_string(
            window->xkb_context,
            map_str,
            XKB_KEYMAP_FORMAT_TEXT_V1,
            0
    );

    munmap(map_str, size);
    close(fd);

    if (window->xkb_keymap == NULL)
    {
        fprintf(stderr, "WA ERROR: Failed to compile keymap.\n");
        return;
    }

    window->xkb_state = xkb_state_new(window->xkb_keymap);
    if (window->xkb_state == NULL)
    {
        fprintf(stderr, "WA ERROR: Failed to create XKB state.\n");
        xkb_keymap_unref(window->xkb_keymap);
        window->xkb_keymap = NULL;
        return;
    }
}

void 
wa_kb_enter(_WA_UNUSED void* data, _WA_UNUSED struct wl_keyboard* keyboard, uint32_t serial, _WA_UNUSED struct wl_surface* surface, _WA_UNUSED struct wl_array* array)
{
    wa_log(WA_DEBUG, "kb_enter: serial: %u\n", serial);
}

void 
wa_kb_leave(_WA_UNUSED void* data, _WA_UNUSED struct wl_keyboard* keyboard, uint32_t serial, _WA_UNUSED struct wl_surface* surface)
{
    wa_log(WA_DEBUG, "kb_leave: serial: %u\n", serial);
}

void 
wa_kb_key(void* data, _WA_UNUSED struct wl_keyboard* keyboard, _WA_UNUSED uint32_t serial, _WA_UNUSED uint32_t t, uint32_t key, uint32_t state)
{
    wa_window_t* window = data;

    uint32_t keycode = key + 8;

    xkb_keysym_t sym = xkb_state_key_get_one_sym(window->xkb_state, keycode);
    wa_event_t key_event = {
        .type = WA_EVENT_KEYBOARD,
        .keyboard.pressed = state,
        .keyboard.sym = sym
    };

    xkb_keysym_get_name(sym, key_event.keyboard.name, WA_KEY_NAME_LEN);

    wa_log(WA_DEBUG, "Key pressed (%d): Sym: %u, name: '%s'\n",
           state, sym, key_event.keyboard.name);

    window->state.callbacks.event(window, &key_event, window->state.user_data);
}

void 
wa_kb_mod(void* data, 
          _WA_UNUSED struct wl_keyboard* keyboard, 
          _WA_UNUSED uint32_t serial,
          uint32_t mods_depressed, uint32_t mods_latched, 
          uint32_t mods_locked, uint32_t group)
{
    wa_window_t* window = data;

    xkb_state_update_mask(window->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void 
wa_kb_rep(_WA_UNUSED void* data, _WA_UNUSED struct wl_keyboard* keyboard, int32_t rate, int32_t del)
{
    wa_log(WA_DEBUG, "kb_rep: rate: %u, del: %d\n", rate, del);
}

void 
wa_point_enter(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, _WA_UNUSED uint32_t serial, 
               _WA_UNUSED struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    wa_log(WA_DEBUG, "WA: point_enter(surxy: %dx%d)\n", surface_x, surface_y);
}

void 
wa_point_leave(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
               _WA_UNUSED uint32_t serial, _WA_UNUSED struct wl_surface* surface)
{
    wa_log(WA_DEBUG, "WA: point_leave()\n");
}

void 
wa_point_move(void* data, _WA_UNUSED struct wl_pointer* pointer, _WA_UNUSED uint32_t time, 
              wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    wa_window_t* window = data;
    const int x = wl_fixed_to_int(surface_x);
    const int y = wl_fixed_to_int(surface_y);

    //printf("WA: point_move(time: %u) %dx%d\n", time, x, y);

    wa_event_t event = {
        .type = WA_EVENT_POINTER,
        .pointer.x = x,
        .pointer.y = y,
    };

    window->state.callbacks.event(window, &event, window->state.user_data);
}

void 
wa_point_button(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
                uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    wa_log(WA_DEBUG, "WA: point_button(serial: %u, time: %u) button: %u, state: %u\n", 
           serial, time, button, state);
}

void 
wa_point_axis(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
              uint32_t time, uint32_t axis_type, wl_fixed_t value)
{
    wa_log(WA_DEBUG, "WA: point_axis(time: %u) type: %u, val: %u\n", time, axis_type, value);
}

void 
wa_point_frame(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer)
{
}

void 
wa_point_axis_src(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
                       uint32_t axis_src)
{
    wa_log(WA_DEBUG, "WA: point_axis_src() %u\n", axis_src);
}

void 
wa_point_axis_stop(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
                   uint32_t time, uint32_t axis)
{
    wa_log(WA_DEBUG, "WA: point_axis_stop(time: %u) %u\n", time, axis);
}

void 
wa_point_axis_discrete(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
                       uint32_t axis, int32_t discrete)
{
    wa_log(WA_DEBUG, "WA: point_axis_discrete() axis: %u, discrete: %d\n", axis, discrete);
}

void 
wa_point_axis120(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
                 uint32_t axis_type, int value)
{
    wa_log(WA_DEBUG, "WA: point_axis120() type: %u, value: %d\n", axis_type, value);
}

void 
wa_point_axis_dir(_WA_UNUSED void* data, _WA_UNUSED struct wl_pointer* pointer, 
                  uint32_t axis_type, uint32_t dir)
{
    wa_log(WA_DEBUG, "WA: point_axis_dir() type: %u, dir: %u\n", axis_type, dir);
}

void 
wa_seat_cap(void* data, struct wl_seat* seat, uint32_t cap)
{
    wa_window_t* window = data;
    if (cap & WL_SEAT_CAPABILITY_KEYBOARD)
    {
        window->wl_keyboard = wl_seat_get_keyboard(seat);

        window->wl_keyboard_listener.keymap = wa_kb_map;
        window->wl_keyboard_listener.enter = wa_kb_enter;
        window->wl_keyboard_listener.leave = wa_kb_leave;
        window->wl_keyboard_listener.key = wa_kb_key;
        window->wl_keyboard_listener.modifiers = wa_kb_mod;
        window->wl_keyboard_listener.repeat_info = wa_kb_rep;

        wl_keyboard_add_listener(window->wl_keyboard, &window->wl_keyboard_listener, data);
    }
    if (cap & WL_SEAT_CAPABILITY_POINTER)
    {
        window->wl_pointer = wl_seat_get_pointer(seat);

        window->wl_pointer_listener.enter = wa_point_enter;
        window->wl_pointer_listener.leave = wa_point_leave;
        window->wl_pointer_listener.motion = wa_point_move;
        window->wl_pointer_listener.button = wa_point_button;
        window->wl_pointer_listener.axis = wa_point_axis;
        window->wl_pointer_listener.frame = wa_point_frame;
        window->wl_pointer_listener.axis_source = wa_point_axis_src;
        window->wl_pointer_listener.axis_stop = wa_point_axis_stop;
        window->wl_pointer_listener.axis_discrete = wa_point_axis_discrete;
        window->wl_pointer_listener.axis_value120 = wa_point_axis120;
        window->wl_pointer_listener.axis_relative_direction = wa_point_axis_dir;

        wl_pointer_add_listener(window->wl_pointer, &window->wl_pointer_listener, window);
    }

    wa_log(WA_DEBUG, "wl_seat cap: 0x%X\n", cap);
}

void 
wa_seat_name(_WA_UNUSED void* data, _WA_UNUSED struct wl_seat* seat, const char* name)
{
    wa_log(WA_DEBUG, "wl_seat name: '%s'\n", name);
}
