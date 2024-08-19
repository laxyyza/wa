#include "wa.h"
#include "wa_event.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <GL/glew.h>
#include <xkbcommon/xkbcommon.h>
#include <signal.h>

typedef struct 
{
    uint32_t shader_program;
    uint32_t VBO;
    uint32_t VAO;
    wa_window_t* window;
    wa_state_t state;
} app_t;

app_t* app_ptr;

float vertices[] = {
/*      vertices                color           */
   -0.8f, -0.8f, 
    0.8f, -0.8f,
    0.8f,  0.8f,
   -0.8f,  0.8f,
};

uint8_t indices[] = {
    0, 1, 2,
    2, 3, 0
};

size_t
fsize(int fd)
{
    struct stat stat;
    fstat(fd, &stat);
    return stat.st_size;
}

const char*
readfile(const char* filepath)
{
    char* buf;
    size_t size;
    int fd = open(filepath, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return NULL;
    }
    size = fsize(fd);
    buf = calloc(size + 1, sizeof(char));
    if (read(fd, buf, size) == -1)
        perror("read");
    close(fd);
    return buf;
}

GLuint
compile_shader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char info[512];
        glGetShaderInfoLog(shader, 512, NULL, info);
        fprintf(stderr, "ERROR:SHADER_COMPILE of type: %s\n%s\n",
                (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT",
                info);
    }
    return shader;
}

GLuint
create_shader_program(void)
{
    const char* vert_src = readfile("sandbox/vert.glsl");
    const char* frag_src = readfile("sandbox/frag.glsl");

    GLuint vert_shader = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint frag_shader = compile_shader(GL_FRAGMENT_SHADER, frag_src);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glLinkProgram(shader_program);

    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char info[512];
        glGetProgramInfoLog(shader_program, 512, NULL, info);
        printf("ERROR:PROGRAM_LINKING_ERROR: %s\n",
               info);
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    free((void*)vert_src);
    free((void*)frag_src);

    return shader_program;
}

static void
app_draw(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* data)
{
    // app_t* app = data;
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // glUseProgram(app->shader_program);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

    // glBindVertexArray(app->VAO);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
    // glBindVertexArray(0);
}

static void
app_event(wa_window_t* window, const wa_event_t* ev, _WA_UNUSED void* data)
{
    if (ev->type == WA_EVENT_KEYBOARD)
    {
        if (ev->keyboard.pressed)
        {
            if (ev->keyboard.sym == XKB_KEY_f)
            {
                wa_state_t* state = wa_window_get_state(window);
                wa_window_set_fullscreen(window, !(state->window.state & WA_STATE_FULLSCREEN));
            }
        }
    }
}

static void
app_update(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* user_data)
{

}

static void
app_close(_WA_UNUSED wa_window_t* window, _WA_UNUSED void* user_data)
{

}

static void
sighandle(_WA_UNUSED int signum)
{
    wa_window_stop(app_ptr->window);
}

// Define the debug callback
void opengl_debug_callback(GLenum source, GLenum type, GLuint id,
                                    GLenum severity, _WA_UNUSED GLsizei length,
                                    const GLchar *message, _WA_UNUSED const void *data) {
    app_t* app = (app_t*)data;
    wa_window_stop(app->window);
    fprintf(stderr, "OpenGL Debug Message:\n");
    fprintf(stderr, "Source: 0x%x\n", source);
    fprintf(stderr, "Type: 0x%x\n", type);
    fprintf(stderr, "ID: %d\n", id);
    fprintf(stderr, "Severity: 0x%x\n", severity);
    fprintf(stderr, "Message: %s\n", message);
    fprintf(stderr, "----------------\n");

}

int main(void)
{
    int ret;
    app_t app = {0};
    app_ptr = &app;
    wa_state_t* state = &app.state;
    state->callbacks.draw = app_draw;
    state->user_data = &app;
    state->callbacks.event = app_event;
    state->window.state = WA_STATE_FULLSCREEN;
    state->window.title = "WA Test";
    state->window.wayland.app_id = "yes";
    state->window.w = 600;
    state->window.h = 400;
    state->callbacks.update = app_update;
    state->callbacks.close = app_close;

    signal(SIGINT, sighandle);

    if ((app.window = wa_window_create_from_state(state)) == NULL)
        return -1;

        // Enable debug output
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback(opengl_debug_callback, &app);

    // GLuint VBO, VAO;
    // glGenVertexArrays(1, &VAO);
    // glGenBuffers(1, &VBO);
    //
    // glBindVertexArray(VAO);

    uint32_t buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    uint32_t ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(uint8_t), indices, GL_STATIC_DRAW);


    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    // glEnableVertexAttribArray(1);

    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    app.shader_program = create_shader_program();
    // app.VAO = VAO;
    // app.VBO = VBO;
    glUseProgram(app.shader_program);

    ret = wa_window_mainloop(app.window);

    glDeleteProgram(app.shader_program);
    wa_window_delete(app.window);

    printf("exit code: %d\n", ret);

    return ret;
}
