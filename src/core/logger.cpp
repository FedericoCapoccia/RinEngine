#include "logger.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace logger {

enum log_level_t {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
};

static const char* log_level_tag[] = {
    "err",
    "warn",
    "info",
    "debug",
};

static const char* log_level_color[] = {
    "\x1b[31m", // Red
    "\x1b[33m", // Yellow
    "\x1b[32m", // Green
    "\x1b[36m", // Cyan
};

static const char* ansi_reset = "\x1b[0m";

static void log_output(log_level_t level, const char* message, va_list args)
{

    va_list args_copy;
    va_copy(args_copy, args);
    int buf_len = vsnprintf(nullptr, 0, message, args_copy) + 1;
    va_end(args_copy);

    if (buf_len < 0) {
        va_end(args);
        return;
    }

    char* formatted = (char*)malloc(buf_len);
    if (formatted == nullptr) {
        va_end(args);
        return;
    }

    vsnprintf(formatted, (size_t)buf_len + 1, message, args);
    va_end(args);

    FILE* output = (level == LOG_LEVEL_ERROR || level == LOG_LEVEL_WARN) ? stderr : stdout;
    fprintf(output, "%s[%s]\t%s%s\n", log_level_color[level], log_level_tag[level], formatted, ansi_reset);
    free(formatted);
}

void error(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    log_output(LOG_LEVEL_ERROR, message, args);
}

void warn(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    log_output(LOG_LEVEL_WARN, message, args);
}

void info(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    log_output(LOG_LEVEL_INFO, message, args);
}

void debug(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    log_output(LOG_LEVEL_DEBUG, message, args);
}

}
