#include "platform.h"

#include <windowsx.h>
#include <stdio.h>

#include "memory/ememory.h"
#include "event.h"
#include "input.h"
#include "renderer/vulkan/vulkan_utils.h"

static platform_state *state;

b8 platform_create(const char *name, u16 width, u16 height, platform_state *out_platform)
{
	state = eallocate(sizeof(platform_state), MEMORY_TYPE_SYSTEM_STATE);

	const char *class_name = "WindowClassName";
	// Get current hInstance
	HINSTANCE h_instance = GetModuleHandleA(0);
	// Load up icon for the window
	DWORD error = GetLastError();

	// Fill out window class
	WNDCLASSA wc = {0};
	wc.lpfnWndProc = handleWin32Messages;
	wc.hInstance = h_instance;
	wc.lpszClassName = class_name;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	DWORD windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
	DWORD windowExStyle = WS_EX_APPWINDOW;

	windowStyle |= WS_MAXIMIZEBOX;
	windowStyle |= WS_MINIMIZEBOX;
	windowStyle |= WS_THICKFRAME;

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;

	AdjustWindowRectEx(&rect, windowStyle, 0, windowExStyle);

	// Calculate actual width and height based on the rect
	int actual_width, actual_height;
	actual_width = rect.right - rect.left;
	actual_height = rect.bottom - rect.top;
	// Register window class for the OS
	if (!RegisterClassA(&wc))
	{
		// Log error
		error = GetLastError();
		return false;
	}

	state->window_handle = CreateWindowExA(
		windowExStyle,
		class_name,
		name,
		windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, actual_width, actual_height,
		NULL,
		NULL,
		h_instance,
		NULL);

	if (!state->window_handle)
	{
		error = GetLastError();
		EN_ERROR("Window handle not created. Cannot continue application. Error Code: %lu", error);
		return false;
	}

	state->window_hinstance = h_instance;
	state->height = height;
	state->width = width;
	QueryPerformanceFrequency(&state->performance_frequency);
	ShowWindow(state->window_handle, SW_NORMAL);
	*out_platform = *state;
	return true;
}

void platform_shutdown()
{
	EN_DEBUG("Shutting down platform.");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
	efree(state, sizeof(platform_state), MEMORY_TYPE_SYSTEM_STATE);
}

const char *platform_get_vulkan_extensions() { return "VK_KHR_win32_surface"; }

f64 platform_get_absolute_time()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (double)counter.QuadPart / state->performance_frequency.QuadPart;
}

void platform_sleep(long ms)
{
	if (ms > 0)
	{
		Sleep(ms);
	}
}

void platform_log_message(log_level level, const char *message, ...)
{
	u32 colour = 0;
	switch (level)
	{
	case LOG_LEVEL_TRACE:
		colour = 5;
		break;
	case LOG_LEVEL_INFO:
		colour = 4;
		break;
	case LOG_LEVEL_DEBUG:
		colour = 3;
		break;
	case LOG_LEVEL_WARN:
		colour = 2;
		break;
	case LOG_LEVEL_ERROR:
		colour = 1;
		break;
	case LOG_LEVEL_FATAL:
		colour = 0;
		break;
	default:
		colour = 0;
		break;
	}

	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
	static int levels[6] = {64, 4, 6, 1, 2, 8};
	if (colour < 6)
	{
		SetConsoleTextAttribute(console_handle, levels[colour]);
	}
	OutputDebugStringA(message);
	size_t length = strlen(message);
	LPDWORD numberWritten = 0;
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, numberWritten, 0);
}

void platform_pump_messages()
{
	MSG msg;
	while (PeekMessageA(&msg, state->window_handle, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK handleWin32Messages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_EXITSIZEMOVE:
	{
		if (LOWORD(lParam) > 0 && HIWORD(lParam) > 0)
		{
			event_context c = {0};
			c.u_32[0] = LOWORD(lParam);
			c.u_32[1] = HIWORD(lParam);
			event_system_trigger_event(0, c, EVENT_TYPE_WINDOW_RESIZE);
		}
		break;
	}
	case WM_CLOSE:
	case WM_DESTROY:
	{
		event_context c = {0};
		event_system_trigger_event(0, c, EVENT_TYPE_WINDOW_CLOSE);
		PostQuitMessage(0);
		return 0;
	}

	case WM_MOVE:
	{
		event_context c = {0};
		c.u_32[0] = (int)(short)LOWORD(lParam); // horizontal position
		c.u_32[1] = (int)(short)HIWORD(lParam);
		event_system_trigger_event(0, c, EVENT_TYPE_WINDOW_MOVED);
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		event_context c = {0};
		c.u_32[0] = GET_X_LPARAM(lParam);
		c.u_32[1] = GET_Y_LPARAM(lParam);
		event_system_trigger_event(0, c, EVENT_TYPE_MOUSE_MOVED);
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		b8 pressed = uMsg == WM_LBUTTONDOWN || uMsg == WM_RBUTTONDOWN || uMsg == WM_MBUTTONDOWN;
		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		{
			input_process_mouse_input(MOUSE_CODE_LEFT, pressed);
			break;
		}
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		{
			input_process_mouse_input(MOUSE_CODE_RIGHT, pressed);
			break;
		}
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		{
			input_process_mouse_input(MOUSE_CODE_MIDDLE, pressed);
			break;
		}
		}
	}
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	{
		event_context c = {0};
		c.i_32[0] = (i32)wParam;

		// TODO handle alt, shift... keys as well
		b8 pressed = uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN;
		input_process_key_input((key_code)wParam, pressed);
	}
	case WM_MOUSEWHEEL:
	{
		event_context c = {0};
		c.i_32[0] = GET_WHEEL_DELTA_WPARAM(wParam);
		c.u_32[1] = GET_X_LPARAM(lParam);
		c.u_32[2] = GET_Y_LPARAM(lParam);
		input_process_mouse_scrolled((i32)GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	}
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
