#include "application.h"

#include "platform.h"
#include "memory/ememory.h"
#include "event.h"
#include "input.h"
#include "clock.h"
#include "renderer/renderer_frontend.h"
#include "game_types.h"

#include "systems/texture_system.h"
#include "memory/linear_allocator.h"

typedef struct application_state
{
	application_config app_config;
	clock app_clock;

	f64 last_time;
	game *game_inst;

	b8 is_running;
	b8 is_suspended;
	u32 width;
	u32 height;

	linear_allocator systems_allocator;

	// Systems
	void *memory_system_state;
	u64 memory_memory_requirement;

	void *event_system_state;
	u64 event_system_memory_requirement;

	void *logger_system_state;
	u64 logger_memory_requirement;

	void *input_system_state;
	u64 input_system_memory_requirement;

	void *platform_system_state;
	u64 platform_system_memory_requirement;

	void *renderer_frontend;
	u64 renderer_memory_requirement;

	void *texture_system_state;
	u64 texture_system_memory_requirement;

} application_state;

static application_state *app_state;

b8 application_on_close(const void *sender, event_context context, event_type type);
b8 application_on_resize(const void *sender, event_context context, event_type type);
b8 application_on_key_pressed(const void *sender, event_context context, event_type type);

EAPI b8 application_initialize(game *game)
{
	memory_system_initialize();

	app_state = eallocate(sizeof(application_state), MEMORY_TYPE_SYSTEM_STATE);
	app_state->game_inst = game;
	clock_create(&app_state->app_clock);

	// Initialize all subsystems
	u64 systems_allocator_total_size = 64 * 1024;
	linear_allocator_create(systems_allocator_total_size, &app_state->systems_allocator);

	// Logger
	u64 memory_requirement = 0;
	logger_system_initialize(LOG_LEVEL_TRACE, 0, &memory_requirement);
	app_state->logger_system_state = linear_allocator_allocate(&app_state->systems_allocator, memory_requirement);
	logger_system_initialize(LOG_LEVEL_TRACE, app_state->logger_system_state, &memory_requirement);
	app_state->logger_memory_requirement = memory_requirement;

	// Input
	input_system_initialize(&memory_requirement, 0);
	app_state->input_system_state = linear_allocator_allocate(&app_state->systems_allocator, memory_requirement);
	input_system_initialize(&memory_requirement, app_state->input_system_state);
	app_state->input_system_memory_requirement = memory_requirement;
	ezero_out(app_state->input_system_state, sizeof(input_system_state));

	// Events
	event_system_initialize(&memory_requirement, 0);
	app_state->event_system_state = linear_allocator_allocate(&app_state->systems_allocator, memory_requirement);
	event_system_initialize(&memory_requirement, app_state->event_system_state);
	app_state->event_system_memory_requirement = memory_requirement;

	// Platform
	memory_requirement = 0;
	platform_system_initialize(&memory_requirement, 0, "", 0, 0);
	app_state->platform_system_state = linear_allocator_allocate(&app_state->systems_allocator, memory_requirement);
	platform_system_initialize(
		&memory_requirement,
		app_state->platform_system_state,
		"Engine",
		app_state->game_inst->app_config.window_width,
		app_state->game_inst->app_config.window_height);
	app_state->platform_system_memory_requirement = memory_requirement;

	// Renderer
	memory_requirement = 0;
	renderer_frontend_initialize(&memory_requirement, 0, "Engine", app_state->platform_system_state);
	app_state->renderer_frontend = linear_allocator_allocate(&app_state->systems_allocator, memory_requirement);
	renderer_frontend_initialize(&memory_requirement, app_state->renderer_frontend, "Engine", app_state->platform_system_state);
	app_state->renderer_memory_requirement = memory_requirement;

	// Texture system
	memory_requirement = 0;
	texture_system_config tex_config;
	tex_config.max_texture_count = 128;
	texture_system_initialize(&memory_requirement, 0, tex_config);
	app_state->texture_system_state = linear_allocator_allocate(&app_state->systems_allocator, memory_requirement);
	texture_system_initialize(&memory_requirement, app_state->texture_system_state, tex_config);
	app_state->texture_system_memory_requirement = memory_requirement;

	// Initialize game
	if (!app_state->game_inst->initialize(app_state->game_inst))
	{
		EN_FATAL("Failed to initialize game.");
		return false;
	}

	app_state->width = app_state->game_inst->app_config.window_width;
	app_state->height = app_state->game_inst->app_config.window_height;

	app_state->game_inst->on_resize(
		app_state->game_inst,
		app_state->width,
		app_state->height);

	// Register on event functions
	// OnClose
	event_system_register_event((registered_event){EVENT_TYPE_WINDOW_CLOSE, application_on_close});
	event_system_register_event((registered_event){EVENT_TYPE_WINDOW_RESIZE, application_on_resize});
	event_system_register_event((registered_event){EVENT_TYPE_KEY_PRESSED, application_on_key_pressed});

	app_state->is_suspended = false;
	return true;
}

void application_shutdown()
{
	texture_system_shutdown(app_state->texture_system_state);
	renderer_frontend_shutdown();
	event_system_shutdown(app_state->event_system_state);
	input_system_shutdown(app_state->input_system_state);
	platform_system_shutdown(app_state->platform_system_state);	
	logger_system_shutdown(app_state->logger_system_state);
	
	linear_allocator_destroy(&app_state->systems_allocator);
	efree(app_state, sizeof(application_state), MEMORY_TYPE_SYSTEM_STATE);
	memory_system_shutdown();
	app_state = 0;
}

b8 application_run()
{
	app_state->is_running = true;

	clock_start(&app_state->app_clock);

	print_memory_stats();
	unsigned long frame_count = 0;
	render_packet p = {0};
	p.delta_time = 0;
	app_state->last_time = platform_get_absolute_time();
	while (app_state->is_running)
	{
		f64 current_time = platform_get_absolute_time();
		f64 delta_time = current_time - app_state->last_time;
		p.delta_time = delta_time;

		platform_pump_messages();
		if (!app_state->is_suspended)
		{
			
			if (!app_state->game_inst->update(app_state->game_inst, (f32)delta_time))
			{
				EN_FATAL("Failed to update game, shutting off.");
				app_state->is_running = false;
				break;
			}
			
			if (!app_state->game_inst->render(app_state->game_inst, (f32)delta_time))
			{
				EN_FATAL("Failed to render game, shutting off.");
				app_state->is_running = false;
				break;
			}
			
			if (!renderer_frontend_draw_frame(&p))
			{
				// Swapchain is likely rebooting and we need to acquire a
				// new image from the swapchain before ending the frame and calling
				// vkQueueSubmit/Present. Therefore skip DrawFrame
				// and EndFrame and acquire will be called again in BeginFrame
				continue;
			}
			
			if (clock_get_elapsed_sec(&app_state->app_clock) >= 1.0)
			{
				EN_DEBUG("Frames per second: %u", frame_count);
				clock_start(&app_state->app_clock);
				frame_count = 0;
			}

			input_system_update();

			app_state->last_time = current_time;
			frame_count++;
		}
	}
	application_shutdown();
}

b8 application_on_close(const void *sender, event_context context, event_type type)
{
	if (!app_state->is_running)
		return true;
	app_state->is_running = false;
	return true;
}

b8 application_on_resize(const void *sender, event_context context, event_type type)
{
	u32 width = context.u_32[0];
	u32 height = context.u_32[1];

	if (width != app_state->width || height != app_state->height)
	{
		app_state->width = width;
		app_state->height = height;
		if (width == 0 || height == 0)
		{
			EN_INFO("Window minimized suspending application.");
			app_state->is_suspended = true;
			return true;
		}
		else
		{
			app_state->game_inst->on_resize(app_state->game_inst, width, height);
			renderer_frontend_on_resized(width, height);
			app_state->is_suspended = false;
			EN_INFO("Window resized to: Width: %lu, Height: %lu", width, height);

			return true;
		}
	}
	return false;
}

b8 application_on_key_pressed(const void *sender, event_context context, event_type type)
{
	EN_DEBUG("Key Pressed: %c", context.i_32[0]);
	if (context.i_32[0] == KEY_ESCAPE)
	{
		app_state->is_running = false;
	}
	return true;
}