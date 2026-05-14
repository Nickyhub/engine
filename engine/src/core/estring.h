#pragma once
#include "defines.h"
#include "math/math_types.h"

b8 string_compare(const char* str0, const char* str1);

i32 string_format(char* dest, const char* format, ...);

i32 string_format_v(char* dest, const char* format, void* va_listp);

b8 strings_equali(const char *str0, const char *str1);

i32 string_length(const char* str);

char* string_copy(char* dest, const char* source);

char* string_ncopy(char* dest, const char* source, i64 length);

char* string_trim(char* str);

void string_mid(char* dest, const char* source, i32 start, i32 length);

i32 string_index_of(char* str, char c);

b8 string_to_vec4(char* str, vec4* out_vector);

b8 string_to_vec3(char* str, vec3* out_vector);

b8 string_to_vec2(char* str, vec2* out_vector);


b8 string_to_f32(char* str, f32* f);

b8 string_to_f64(char* str, f64* f);

b8 string_to_i8(char* str, i8* i);
b8 string_to_i16(char* str, i16* i);
b8 string_to_i32(char* str, i32* i);
b8 string_to_i64(char* str, i64* i);

b8 string_to_u8(char* str, u8* u);
b8 string_to_u16(char* str, u16* u);
b8 string_to_u32(char* str, u32* u);
b8 string_to_u64(char* str, u64* u);

b8 string_to_bool(char* str, b8* b);