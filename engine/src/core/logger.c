#include <stdio.h>
#include <stdarg.h>

#include "logger.h"
#include "platform.h"
#include "memory/ememory.h"

#define LOG_MESSAGE_MAX_LENGTH 2048

void log_message(log_level level, const char* message, ...) {
		i8 buffer[LOG_MESSAGE_MAX_LENGTH];
		const i8* level_strings[6] = { "[TRACE]: ", "[INFO]: ", "[DEBUG]: " , "[WARN]: " , "[ERROR]: " , "[FATAL]: " };
		
		ezero_out(buffer, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH);
		
		va_list arg_ptr;
		va_start(arg_ptr, message);
		vsprintf_s(buffer, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH, message, arg_ptr);
		va_end(arg_ptr);
		
		i8 out_message[LOG_MESSAGE_MAX_LENGTH];
		ezero_out(out_message, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH);
		// Append the message severity to the buffer
		if (level < 6) {
			sprintf_s(out_message, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH, "%s%s\n", level_strings[level], buffer);
		}
		else {
			level = LOG_LEVEL_TRACE;
		}
		platform_log_message(level, out_message);
}