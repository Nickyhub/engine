#pragma once

#include <WinDef.h>

#include "logger.h"

typedef struct platform_state {
	HWND window_handle;
	HINSTANCE window_hinstance;
	LARGE_INTEGER performance_frequency;

	u16 width;
	u16 height;
} platform_state;

b8 platform_system_initialize(u64 *memory_requirement, void *state, const char *name, u16 width, u16 height);
void platform_system_shutdown(void *state);
void platform_pump_messages();
void platform_log_message(log_level log_level, const char* message, ...);
f64 platform_get_absolute_time();
void platform_sleep(long ms);

const char* platform_get_vulkan_extensions();
LRESULT CALLBACK handleWin32Messages(HWND hwnd, UINT uMsg, WPARAM wPara, LPARAM lParam);
