#include "application.h"

#include "platform.h"
#include "memory/ememory.h"
#include "event.h"
#include "input.h"
#include "clock.h"
#include "renderer/renderer_frontend.h"
#include "game_types.h"

typedef struct application_state
{
	systems systems;
	application_config app_config;
	clock app_clock;

	f64 last_time;
	game *game_inst;

	b8 is_running;
	b8 is_suspended;
	u32 width;
	u32 height;
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

	input_system_initialize();

	if (!event_system_initialize())
	{
		EN_FATAL("Cannot initialize event system. Shutting down.");
		return false;
	}

	if (!platform_create(
			"Engine",
			app_state->game_inst->app_config.window_width,
			app_state->game_inst->app_config.window_height,
			&app_state->systems.platform))
	{
		EN_FATAL("Failed to create platform. Shutting down.");
		return false;
	}

	if (!renderer_frontend_initialize("Engine", &app_state->systems.platform))
	{
		EN_FATAL("Failed to initialize renderer. Shutting down.");
		return false;
	}

	// initialize game
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
	renderer_frontend_shutdown();
	event_system_shutdown();
	input_system_shutdown();
	platform_shutdown();

	efree(app_state, sizeof(application_state), MEMORY_TYPE_SYSTEM_STATE);
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
		input_system_update();
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
	if(context.i_32[0] == KEY_ESCAPE) {
		app_state->is_running = false;
	}
	return true;
}