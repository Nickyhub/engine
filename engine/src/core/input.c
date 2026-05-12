#include "input.h"
#include "memory/ememory.h"
#include "event.h"
#include "logger.h"

static input_system_state *state_ptr;

b8 input_system_initialize(u64 *memory_requirement, void *state)
{
	if (memory_requirement && !state)
	{
		*memory_requirement = sizeof(input_system_state);
		return true;
	}

	state_ptr = state;
	EN_INFO("Input system initialized.");
	return true;
}

void input_system_shutdown(void *state)
{
	if (state_ptr == state)
	{
		state_ptr = 0;
	}
}

b8 input_process_key_input(key_code code, b8 pressed)
{
	if (state_ptr->is_key_pressed[code] != pressed)
	{
		state_ptr->is_key_pressed[code] = pressed;

		event_context c = {0};
		c.u_32[0] = code;
		if (pressed)
		{
			event_system_trigger_event(0, c, EVENT_TYPE_KEY_PRESSED);
		}
		else
		{
			event_system_trigger_event(0, c, EVENT_TYPE_KEY_RELEASED);
		}
		return true;
	}
	return false;
}

b8 input_process_mouse_input(mouse_code code, b8 pressed)
{
	if (state_ptr->is_mouse_pressed[code] != pressed)
	{
		state_ptr->is_mouse_pressed[code] = pressed;

		event_context c;
		c.u_32[0] = code;
		if (pressed)
		{
			event_system_trigger_event(0, c, EVENT_TYPE_MOUSE_CLICKED);
		}
		else
		{
			event_system_trigger_event(0, c, EVENT_TYPE_MOUSE_RELEASED);
		}
		return true;
	}
	return false;
}

b8 input_process_mouse_scrolled(i32 z_delta)
{
	event_context c;
	c.i_32[0] = z_delta;
	state_ptr->mouse_scrolled = z_delta;
	event_system_trigger_event(0, c, EVENT_TYPE_MOUSE_SCROLLED);
}

void input_process_mouse_pos(u32 x, u32 y) {
	state_ptr->last_mouse_pos.x = x;
	state_ptr->last_mouse_pos.y = y;
}

i32 input_mouse_scrolled()
{
	return state_ptr->mouse_scrolled;
}

void input_system_update()
{
	// Copy over the current key and mouse states to the last key and mouse states
	// Should be called at the end of the main loop
	input_system_state *s = state_ptr;
	ecopy(&s->is_key_pressed[0], &s->was_key_pressed[0], sizeof(s->is_key_pressed));
	ecopy(&s->is_mouse_pressed[0], &s->was_mouse_pressed[0], sizeof(s->is_mouse_pressed));
}

EAPI b8 input_is_mouse_button_pressed(mouse_code code)
{
	return state_ptr->is_mouse_pressed[code];
}

EAPI b8 input_was_mouse_button_pressed(mouse_code code)
{
	return state_ptr->was_mouse_pressed[code];
}

EAPI b8 input_is_key_pressed(key_code code)
{
	return state_ptr->is_key_pressed[code];
}

EAPI b8 input_is_key_up(key_code code) {
	return !state_ptr->is_key_pressed[code];
}

EAPI b8 input_was_key_pressed(key_code code)
{
	return state_ptr->was_key_pressed[code];
}

EAPI vec2_u input_get_latest_mouse_pos() {
	return state_ptr->last_mouse_pos;
}