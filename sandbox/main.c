#include "wa.h"
#include "wa_event.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <GL/glew.h>
#include <xkbcommon/xkbcommon.h>

typedef struct 
{
    GLuint shader_program;
    GLuint VBO;
    GLuint VAO;
} app_t;

float vertices[] = {
/*      vertices                color           */
    0.0f,  0.8f,  0.0f,     1.0f, 0.0f, 0.0f,
   -0.8f, -0.8f, 0.0f,      0.0f, 1.0f, 0.0f,
    0.8f, -0.8f, 0.0f,      0.0f, 0.0f, 1.0f
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
app_draw(_WA_UNUSED wa_window_t* window, void* data)
{
    app_t* app = data;
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->shader_program);

    glBindVertexArray(app->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
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
                wa_window_set_fullscreen(window, !state->window.fullscreen);
            }
        }
    }
}

int main(void)
{
    int ret;
    wa_window_t* window;
    app_t app = {0};
    
    if ((window = wa_window_create("Test", 900, 600, false)) == NULL)
        return -1;
    wa_state_t* state = wa_window_get_state(window);
    state->callbacks.draw = app_draw;
    state->user_data = &app;
    state->callbacks.event = app_event;

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    app.shader_program = create_shader_program();
    app.VAO = VAO;
    app.VBO = VBO;

    ret = wa_window_mainloop(window);

    wa_window_delete(window);

    printf("exit code: %d\n", ret);

    return ret;
}
