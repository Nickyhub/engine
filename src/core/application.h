#pragma once

#include "event.h"
#include "clock.h"
#include "platform.h"
#include "renderer/renderer_frontend.h"

typedef struct application_config {
	unsigned int target_ticks_per_second;
	unsigned int window_width;
	unsigned int window_height;
	const char* name;
} application_config;

//class Platform;

typedef struct systems_config {
	unsigned int width;
	unsigned int height;
	const char* name;
} systems_config;

typedef struct systems {
	platform_state platform;
	renderer_frontend renderer_frontend;
} systems;

typedef struct application_state {
	systems systems;
	application_config app_config;
	b8 running;
	clock app_clock;
	f64 last_time;
} application_state;

b8 application_initialize(application_config config);

b8 application_run();

void application_shutdown();

