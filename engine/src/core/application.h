#pragma once

#include "event.h"
#include "clock.h"
#include "platform.h"
#include "renderer/renderer_frontend.h"

struct game;

typedef struct application_config {
	unsigned int target_ticks_per_second;
	unsigned int window_width;
	unsigned int window_height;
	const char* name;
} application_config;

typedef struct systems_config {
	unsigned int width;
	unsigned int height;
	const char* name;
} systems_config;

typedef struct systems {
	platform_state platform;
	renderer_frontend renderer_frontend;
} systems;

EAPI b8 application_initialize(struct game* game);

b8 application_run();

void application_shutdown();

