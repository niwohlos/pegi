#ifndef FORMAT_HPP
#define FORMAT_HPP

#include <cstdarg>
#include <cstdio>


static inline char *format(const char *format, ...) __attribute__((format(printf, 1, 2)));

static inline char *format(const char *format, ...)
{
    char *str;
    va_list va;

    va_start(va, format);
    vasprintf(&str, format, va);
    va_end(va);

    return str;
}

#endif
