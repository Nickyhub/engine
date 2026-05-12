#include <string.h>
#include <stdio.h>
#include "estring.h"
#include <stdarg.h>

#ifndef _MSC_VER
#include <strings.h>
#endif

#include "memory/ememory.h"

b8 string_compare(const char *str0, const char *str1)
{
    return strcmp(str0, str1) == 0;
}

i32 string_format(char *dest, const char *format, ...)
{
    if (dest)
    {
        __builtin_va_list arg_ptr;
        va_start(arg_ptr, format);
        i32 written = string_format_v(dest, format, arg_ptr);
        va_end(arg_ptr);
        return written;
    }
    return -1;
}

i32 string_format_v(char *dest, const char *format, void *va_listp)
{
    if (dest)
    {
        // Big, but can fit on the stack.
        char buffer[32000];
        i32 written = vsnprintf(buffer, 32000, format, va_listp);
        buffer[written] = 0;
        ecopy(buffer, dest, written + 1);

        return written;
    }
    return -1;
}

// Case-insensitive string comparison. True if the same, otherwise false.
b8 strings_equali(const char *str0, const char *str1)
{
#ifdef __GNUC__
    return strcasecmp(str0, str1) == 0;
#elif defined _MSC_VER
    return _strcmpi(str0, str1) == 0;
#endif
}