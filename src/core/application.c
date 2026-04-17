#include "application.h"

#include "platform.h"
#include "memory/ememory.h"
#include "event.h"
#include "input.h"
#include "clock.h"
#include "renderer/renderer_frontend.h"

static application_state *app_state;

b8 application_on_close(const void *sender, event_context context, event_type type);
b8 application_on_resize(const void *sender, event_context context, event_type type);
b8 application_on_key_pressed(const void *sender, event_context context, event_type type);

b8 application_initialize(application_config config)
{
	memory_system_initialize();

	app_state = eallocate(sizeof(application_state), MEMORY_TYPE_SYSTEM_STATE);

	app_state->app_config.name = config.name;
	app_state->app_config.window_height = config.window_height;
	app_state->app_config.window_width = config.window_width;
	app_state->app_config.target_ticks_per_second = config.target_ticks_per_second;
	clock_create(&app_state->app_clock);

	input_system_initialize();
	
	if (!event_system_initialize())
	{
		EN_FATAL("Cannot initialize event system. Shutting down.");
		return false;
	}

	if (!platform_create(
			"Engine",
			app_state->app_config.window_width,
			app_state->app_config.window_height,
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

	// Register on event functions
	// OnClose
	event_system_register_event((registered_event){EVENT_TYPE_WINDOW_CLOSE, application_on_close});
	event_system_register_event((registered_event){EVENT_TYPE_WINDOW_RESIZE, application_on_resize});
	event_system_register_event((registered_event){EVENT_TYPE_KEY_PRESSED, application_on_key_pressed});
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

	memory_system_shutdown();
}

b8 application_run()
{
	app_state->running = true;

	clock_start(&app_state->app_clock);

	print_memory_stats();
	unsigned long frame_count = 0;
	render_packet p = {0};
	p.delta_time = 0;
	while (app_state->running)
	{
		f64 current_time = platform_get_absolute_time();
		f64 delta_time = current_time - app_state->last_time;
		p.delta_time = delta_time;

		platform_pump_messages();
		input_system_update();

		if (renderer_frontend_draw_frame(&p))
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
	application_shutdown();
}

b8 application_on_close(const void *sender, event_context context, event_type type)
{
	if(!app_state->running) return true;
	app_state->running = false;
	return true;
}

b8 application_on_resize(const void *sender, event_context context, event_type type)
{
	if(!renderer_frontend_on_resized(context.u_32[0], context.u_32[1])) {
		EN_ERROR("Renderer frontend failed to resize.");
		return false;
	}
	return true;
}

b8 application_on_key_pressed(const void *sender, event_context context, event_type type)
{
	EN_DEBUG("Key Pressed: %c", context.i_32[0]);
	return true;
}