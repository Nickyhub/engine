#pragma once
#include "defines.h"
#include "containers/darray.h"

#define MAX_REGISTERED_EVENT_CALLBACKS 512

typedef enum event_type {
	EVENT_TYPE_MOUSE_MOVED,
	EVENT_TYPE_MOUSE_CLICKED,
	EVENT_TYPE_MOUSE_RELEASED,
	EVENT_TYPE_MOUSE_SCROLLED,
	EVENT_TYPE_KEY_DOWN,
	EVENT_TYPE_KEY_RELEASED,
	EVENT_TYPE_KEY_PRESSED,
	EVENT_TYPE_WINDOW_RESIZE,
	EVENT_TYPE_WINDOW_CLOSE,
	EVENT_TYPE_WINDOW_MOVED,
	
	EVENT_TYPE_MAX,
} event_type;

typedef union event_context {
	u8 u_8[16];
	u16 u_16[8];
	u32 u_32[4];
	i32 i_32[4];
	u64 u_64[2];
	i64 i_64[2];
	f32 f_32[4];
} event_context;

typedef b8 (*pfnOnEvent)(const void* sender, event_context context, event_type type);

typedef struct registered_event {
	event_type type;
	pfnOnEvent callback;
} registered_event;

void registered_event_create(event_type type, pfnOnEvent callback, registered_event* out_event);

typedef struct event_system_state {
	registered_event* registered_events; // darray
	u16 current_event_id_slot;
} event_system_state;

b8 event_system_initialize();

void event_system_register_event(registered_event reg_event);

void event_system_trigger_event(void* sender, event_context context, event_type type);

void event_system_shutdown();