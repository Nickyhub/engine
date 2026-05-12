#include <stdio.h>
#include <stdarg.h>

#include "logger.h"
#include "platform.h"
#include "memory/ememory.h"

#define LOG_MESSAGE_MAX_LENGTH 2048

typedef struct logger_system_state
{
	log_level level;
} logger_system_state;

static logger_system_state *state_ptr;

b8 logger_system_initialize(log_level level, void *state, u64 *memory_requirement)
{
	if (memory_requirement && !state)
	{
		*memory_requirement = sizeof(logger_system_state);
		return true;
	}

	state_ptr = state;
	state_ptr->level = level;

	EN_INFO("Logger initialized.");
	return true;
}

void logger_system_shutdown(void *state)
{
	if (state_ptr == state)
	{
		state_ptr = 0;
	}
}

void log_message(log_level level, const char *message, ...)
{
	if (level >= state_ptr->level)
	{
		i8 buffer[LOG_MESSAGE_MAX_LENGTH];
		const i8 *level_strings[6] = {"[TRACE]: ", "[INFO]: ", "[DEBUG]: ", "[WARN]: ", "[ERROR]: ", "[FATAL]: "};

		ezero_out(buffer, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH);

		va_list arg_ptr;
		va_start(arg_ptr, message);
		vsprintf_s(buffer, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH, message, arg_ptr);
		va_end(arg_ptr);

		i8 out_message[LOG_MESSAGE_MAX_LENGTH];
		ezero_out(out_message, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH);
		// Append the message severity to the buffer
		if (level < 6)
		{
			sprintf_s(out_message, sizeof(i8) * LOG_MESSAGE_MAX_LENGTH, "%s%s\n", level_strings[level], buffer);
		}
		else
		{
			level = LOG_LEVEL_TRACE;
		}
		platform_log_message(level, out_message);
	}
}