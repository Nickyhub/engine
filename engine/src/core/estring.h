#pragma once
#include "defines.h"

b8 string_compare(const char* str0, const char* str1);

i32 string_format(char* dest, const char* format, ...);

i32 string_format_v(char* dest, const char* format, void* va_listp);

b8 strings_equali(const char *str0, const char *str1);