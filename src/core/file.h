#pragma once
#include <stdio.h>
#include "defines.h"

typedef enum file_mode {
	FILE_MODE_READ = 0x01,
	FILE_MODE_WRITE = 0x02,
} file_mode;

typedef struct efile {
	FILE* handle;
	
	b8 is_open;
	u32 size;
	const char* name;
} efile;

b8 file_open(efile* file, const char* filename, file_mode mode, b8 is_binary);

void file_close(efile* file);

b8 file_read_all_bytes(efile* file, char* buffer);

void file_read_line(char* line_buffer);

b8 file_exists(const char* filename);