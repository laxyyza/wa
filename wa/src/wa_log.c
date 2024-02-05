#include "wa_log.h"

#include <stdio.h>
#include <stdarg.h>

#define WA_LOG_LEN 256

/* Terminal colors */
#define FG_RED "\033[91m"
#define BG_RED "\033[101m"
#define FG_DEFAULT "\033[39m"
#define BG_DEFAULT "\033[49m"

#define C_DEFAULT FG_DEFAULT BG_DEFAULT

static const char* wa_log_str[WA_LOG_LEVEL_LEN] = {
    "WA [" FG_RED "FATAL" FG_DEFAULT "]",
    "WA [" FG_RED "ERROR" FG_DEFAULT "]",
    "WA [INFO]",
    "WA [DEBUG]",
};

static const char* wa_log_color[WA_LOG_LEVEL_LEN] = {
    BG_RED,             /* WA_FATAL */
    FG_RED,             /* WA_ERROR */
    FG_DEFAULT,         /* WA_INFO */
    FG_DEFAULT          /* WA_DEBUG */
};

static enum wa_log_level wa_level = WA_DEBUG;

void wa_log_set_level(enum wa_log_level level)
{
    wa_level = level;
}

void wa_log(enum wa_log_level level, const char* format, ...)
{
    if (wa_level < level)
        return;

    char output[WA_LOG_LEN];
    FILE* file = (level <= WA_ERROR) ? stderr : stdout;

    va_list args;
    va_start(args, format);

    vsprintf(output, format, args);

    va_end(args);

    fprintf(file, "%s:\t%s", wa_log_str[level], wa_log_color[level]);

    fprintf(file, "%s%s%s", wa_log_color[level], output, C_DEFAULT);

    fflush(file);
}
