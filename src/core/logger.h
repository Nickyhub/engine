#pragma once

#include "defines.h"

typedef enum log_level {
	LOG_LEVEL_TRACE,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
} log_level;

void log_message(log_level log_level, const char* message, ...);
u8 get_log_level();
void set_log_level(log_level log_level);

#define EN_TRACE(message, ...) log_message(LOG_LEVEL_TRACE, message, ##__VA_ARGS__)
#define EN_INFO(message, ...) log_message(LOG_LEVEL_INFO, message, ##__VA_ARGS__)
#define EN_DEBUG(message, ...) log_message(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__)
#define EN_WARN(message, ...) log_message(LOG_LEVEL_WARN, message, ##__VA_ARGS__)
#define EN_ERROR(message, ...) log_message(LOG_LEVEL_ERROR, message, ##__VA_ARGS__)
#define EN_FATAL(message, ...) log_message(LOG_LEVEL_FATAL, message, ##__VA_ARGS__)