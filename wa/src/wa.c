#include "wa.h"
#include "wa_log.h"
#include <GL/glew.h>

void 
wa_print_opengl(void)
{
    wa_log(WA_INFO, "OpenGL vendor: %s\n",
           glGetString(GL_VENDOR));
    wa_log(WA_INFO, "OpenGL renderer: %s\n",
           glGetString(GL_RENDERER));
    wa_log(WA_INFO, "OpenGL version: %s\n",
           glGetString(GL_VERSION));
    wa_log(WA_INFO, "OpenGL GLSL version: %s\n",
           glGetString(GL_SHADING_LANGUAGE_VERSION));
}
