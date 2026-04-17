#include "input.h"
#include "memory/ememory.h"
#include "event.h"

static input_system_state* input_state;

void input_system_initialize() {
	input_state = eallocate(sizeof(input_system_state), MEMORY_TYPE_SYSTEM_STATE);
}


void input_system_shutdown() {
	efree(input_state, sizeof(input_system_state), MEMORY_TYPE_SYSTEM_STATE);
}

b8 input_is_key_pressed(key_code code) {
	return input_state->is_key_pressed[code];
}

b8 input_was_key_pressed(key_code code) {
	return input_state->was_key_pressed[code];
}


b8 input_process_key_input(key_code code, b8 pressed) {
	if (input_state->is_key_pressed[code] != pressed) {
		input_state->is_key_pressed[code] = pressed;

		event_context c = {0};
		c.u_32[0] = code;
		if (pressed) {
			event_system_trigger_event(0, c, EVENT_TYPE_KEY_PRESSED);
		}
		else {
			event_system_trigger_event(0, c, EVENT_TYPE_KEY_RELEASED);
		}
		return true;
	}
	return false;
}

b8 input_is_mouse_button_pressed(mouse_code code) {
	return input_state->is_mouse_pressed[code];
}

b8 input_was_mouse_button_pressed(mouse_code code) {
	return input_state->was_mouse_pressed[code];
}

b8 input_process_mouse_input(mouse_code code, b8 pressed) {
	if (input_state->is_mouse_pressed[code] != pressed) {
		input_state->is_mouse_pressed[code] = pressed;

		event_context c;
		c.u_32[0] = code;
		if (pressed) {
			event_system_trigger_event(0, c, EVENT_TYPE_MOUSE_CLICKED);
		}
		else {
			event_system_trigger_event(0, c, EVENT_TYPE_MOUSE_RELEASED);
		}
		return true;
	}
	return false;
}

b8 input_process_mouse_scrolled(i32 z_delta) {
	event_context c;
	c.i_32[0] = z_delta;
	input_state->mouse_scrolled = z_delta;
	event_system_trigger_event(0, c, EVENT_TYPE_MOUSE_SCROLLED);
}

i32 input_mouse_scrolled() {
	return input_state->mouse_scrolled;
}

void input_system_update() {
	// Copy over the current key and mouse states to the last key and mouse states
	// Should be called at the end of the main loop
	input_system_state* s = input_state;
	ecopy(&s->is_key_pressed[0], &s->was_key_pressed[0], sizeof(s->is_key_pressed));
	ecopy(&s->is_mouse_pressed[0], &s->was_mouse_pressed[0], sizeof(s->is_mouse_pressed));
}