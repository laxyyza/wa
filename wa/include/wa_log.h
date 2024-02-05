#ifndef _WA_LOG_H_
#define _WA_LOG_H_

enum wa_log_level
{
    WA_FATAL,
    WA_ERROR,
    WA_INFO,
    WA_DEBUG,

    WA_LOG_LEVEL_LEN
};

void wa_log_set_level(enum wa_log_level level);
void wa_log(enum wa_log_level level, const char* format, ...);

#endif // _WA_LOG_H_
