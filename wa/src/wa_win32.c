#include "wa_win32.h"
#include "wa.h"
#include "wa_event.h"
#include <stdio.h>
#include <windows.h>

static void
wa_default_draw(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{
    glClearColor(0.5, 0.0, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

static void
wa_default_close(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{
}

static void
wa_default_event(_WA_UNUSED wa_window_t* window, _WA_UNUSED const wa_event_t* ev, _WA_UNUSED void* data)
{
}

static void
wa_default_update(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
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

wa_window_t*    
wa_window_create(const char* title, int w, int h, bool fullscreen)
{
    wa_state_t state;
    wa_state_set_default(&state);
    state.window.title = title;
    state.window.w = w;
    state.window.h = h;
    state.window.state = (fullscreen) ? WA_STATE_FULLSCREEN : 0;
    return wa_window_create_from_state(&state);
}

static void
wa_draw(wa_window_t* window)
{
    window->state.callbacks.draw(window, window->state.user_data);

    SwapBuffers(window->hdc);
}

static void
wa_resize(wa_window_t* window, int w, int h)
{
    if (w == 0 || h == 0)
        return;

    window->state.window.w = w;
    window->state.window.h = h;

    glViewport(0, 0, w, h);
}

LRESULT CALLBACK 
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    wa_window_t* window = (wa_window_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            wa_log(WA_DEBUG, "WM_DESTROY\n");
            return 0;
        case WM_SIZE:
        {
            int w = LOWORD(lparam);
            int h = HIWORD(lparam);
            wa_log(WA_INFO, "WM_SIZE: %d/%d\n", w, h);
            wa_resize(window, w, h);
            return 0;
        }
        case WM_SETFOCUS:
        {
            window->state.callbacks.focus(window, window->state.user_data);
            return 0;
        }
        case WM_KILLFOCUS:
        {
            window->state.callbacks.unfocus(window, window->state.user_data);
            return 0;
        }
        case WM_KEYDOWN:
        {
            if (wparam < KEY_COUNT)
            {
                window->key_states[wparam] = 1;
            }
            wa_log(WA_DEBUG, "KEY DOWN: %c\n", (char)wparam);

            wa_event_t ev = {
                .type = WA_EVENT_KEYBOARD,
                .keyboard.pressed = 1,
                .keyboard.sym = wparam
            };
            window->state.callbacks.event(window,
                                          &ev,
                                          window->state.user_data);

            return 0;
        }
        case WM_KEYUP:
        {
            if (wparam < KEY_COUNT)
            {
                window->key_states[wparam] = 0;
            }
            wa_log(WA_DEBUG, "KEY UP: %c\n", (char)wparam);
            wa_event_t ev = {
                .type = WA_EVENT_KEYBOARD,
                .keyboard.pressed = 0,
                .keyboard.sym = wparam
            };
            window->state.callbacks.event(window,
                                          &ev,
                                          window->state.user_data);
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

// Define the function pointer type
typedef HGLRC (APIENTRY *wglCreateContextAttribsARBProc)(HDC, HGLRC, const int *);

// Declare a global variable for the function pointer
wglCreateContextAttribsARBProc wglCreateContextAttribsARB = NULL;

wa_window_t*    
wa_window_create_from_state(wa_state_t* state)
{
    wa_window_t* window;
    const char* class_name = "class name?";

    window = calloc(1, sizeof(wa_window_t));
    memcpy(&window->state, state, sizeof(wa_state_t));
    window->instance = GetModuleHandle(NULL);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = window_proc;
    wc.hInstance = window->instance;
    wc.lpszClassName = class_name;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    window->hwnd = CreateWindowEx(
        0, 
        class_name, 
        state->window.title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT,
        NULL, 
        NULL, 
        window->instance, 
        NULL
    );
    if (window->hwnd == NULL)
    {
        wa_logf(WA_FATAL, "CreateWindowEx() return NULL!\n");
        free(window);
        return NULL;
    }

    SetWindowLongPtr(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

    ShowWindow(window->hwnd, SW_SHOWDEFAULT);
    UpdateWindow(window->hwnd);

    wa_window_set_fullscreen(window, window->state.window.state & WA_STATE_FULLSCREEN);

    HDC hdc = GetDC(window->hwnd);
    window->hdc = hdc;

    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelformat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelformat, &pfd);

    // Create a temporary OpenGL context
    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

// Retrieve the function pointer
    wglCreateContextAttribsARB = (wglCreateContextAttribsARBProc)wglGetProcAddress("wglCreateContextAttribsARB");

    if (wglCreateContextAttribsARB) 
    {
        // Define attributes for the core profile context
        int attribs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 6,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };

        HGLRC coreContext = wglCreateContextAttribsARB(hdc, 0, attribs);
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(tempContext);
        wglMakeCurrent(hdc, coreContext);
        window->hglrc = coreContext;
    }
    else
        wa_log(WA_ERROR, "NUULL?\n");

    GLenum glewInitResult = glewInit();
    if (glewInitResult != GLEW_OK) {
        fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(glewInitResult));
        exit(EXIT_FAILURE);
    }

    window->running = true;

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
}

wa_state_t*     
wa_window_get_state(wa_window_t* window)
{
    return &window->state;
}

int     
wa_window_mainloop(wa_window_t* window)
{
    MSG msg;
    while (window->running)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                wa_log(WA_DEBUG, "WM_QUIT\n");
                window->running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        wa_draw(window);
        window->state.callbacks.update(window, window->state.user_data);
    }
    window->state.callbacks.close(window, window->state.user_data);
    return 0;
}

void    
wa_window_set_fullscreen(wa_window_t* window, bool fullscreen)
{
    DWORD style;

    if (fullscreen) 
    {
        // Save the current window size and position
        GetWindowRect(window->hwnd, &window->rect);

        // Switch to fullscreen mode
        style = WS_POPUP;
        SetWindowLong(window->hwnd, GWL_STYLE, style);
        SetWindowLong(window->hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);

        // Get the screen size
        window->rect.left = 0;
        window->rect.top = 0;
        window->rect.right = GetSystemMetrics(SM_CXSCREEN);
        window->rect.bottom = GetSystemMetrics(SM_CYSCREEN);

        // Set the window to cover the entire screen
        SetWindowPos(window->hwnd, HWND_TOPMOST,
                     window->rect.left, window->rect.top,
                     window->rect.right - window->rect.left,
                     window->rect.bottom - window->rect.top,
                     SWP_NOACTIVATE | SWP_NOREDRAW | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        window->state.window.state |= WA_STATE_FULLSCREEN;
    } 
    else 
    {
        // Switch to windowed mode
        style = WS_OVERLAPPEDWINDOW;
        SetWindowLong(window->hwnd, GWL_STYLE, style);

        // Restore the window's previous size and position
        SetWindowPos(window->hwnd, HWND_TOPMOST,
                     window->rect.left, window->rect.top,
                     window->rect.right - window->rect.left,
                     window->rect.bottom - window->rect.top,
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        window->state.window.state ^= WA_STATE_FULLSCREEN;
    }
    // window->state.window.w = window->rect.right;
    // window->state.window.h = window->rect.bottom;
    //wa_resize(window, window->state.window.w, window->state.window.h);
}

void 
wa_window_stop(wa_window_t* window)
{
    window->running = false;
}

void 
wa_window_delete(wa_window_t* window)
{
    free(window);
}
