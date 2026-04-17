#include "file.h"
#include "logger.h"

#include <sys/stat.h>

b8 file_open(efile* file, const char* filename, file_mode mode, b8 is_binary)
{
	const char* mode_string;
	if (file_exists(filename)) {
		file->name = filename;
		if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
			mode_string = is_binary ? "w+b" : "w+";
		}
		else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
			mode_string = is_binary ? "rb" : "r";
		}
		else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
			mode_string = is_binary ? "wb" : "w";
		}
		else {
			EN_ERROR("Invalid mode passed while trying to open file: '%s'", filename);
			return false;
		}

		// Try to open file
		fopen_s(&file->handle, filename, mode_string);
		if (!file->handle) {
			EN_ERROR("Error opening file: '%s'.", filename);
			return false;
		}

		// If file could be opened, determine size of the file.
		rewind(file->handle);
		fseek(file->handle, 0, SEEK_END);
		file->size = ftell(file->handle);
		rewind(file->handle);

		file->is_open = true;
		file->name = filename;
		return true;
	}
	else {
		EN_ERROR("Tried to open non existent file: %s", filename);
		file->is_open = false;
		return false;
	}
}

b8 file_read_all_bytes(efile* file, char* buffer) {
	if (file->is_open) {
		fread(buffer, 1, file->size, file->handle);
		return true;
	}
	else {
		EN_ERROR("Tried to read from file that has not been opened. Open file first: %s.", file->name);
		return false;
	}
}

void file_close(efile* file) {
	if(file->handle) {
		fclose(file->handle);
		file->handle = 0;
	}
}

b8 file_exists(const char* filename) {
	#ifdef _MSC_VER
		struct _stat buffer;
		return _stat(filename, &buffer) == 0;
	#else
		struct stat buffer;
		return stat(filename, &buffer) == 0;
	#endif
}