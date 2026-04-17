#include "event.h"
#include "defines.h"
#include "logger.h"
#include "memory/ememory.h"

static event_system_state *system_state;

void registered_event_create(event_type type, pfnOnEvent callback, registered_event *out_event)
{
	out_event->callback = callback;
	out_event->type = type;
}

b8 event_system_initialize()
{
	system_state = eallocate(sizeof(event_system_state), MEMORY_TYPE_SYSTEM_STATE);
	system_state->current_event_id_slot = 0;
	ezero_out(system_state, sizeof(event_system_state));
	system_state->registered_events = darray_create_size(registered_event, MAX_REGISTERED_EVENT_CALLBACKS);
}

void event_system_register_event(registered_event reg_event)
{
	if (reg_event.callback && system_state->current_event_id_slot < MAX_REGISTERED_EVENT_CALLBACKS - 1)
	{
		darray_push_back(system_state->registered_events, reg_event);
		system_state->current_event_id_slot++;
	}
	else
	{
		EN_WARN("event_system_register_event was called with invalid callback or now slots are available.");
	}
}

void event_system_trigger_event(void *sender, event_context context, event_type type)
{
	if (type != EVENT_TYPE_MAX)
	{
		for (u32 i = 0; i < darray_length(system_state->registered_events); i++)
		{
			if (system_state->registered_events[i].type == type && system_state->registered_events[i].callback)
			{
				// Get the callback and call it with the appropriate context
				system_state->registered_events[i].callback(sender, context, type);
			}
		}
	}
	else
	{
		EN_ERROR("FireEvent was called with invalid event type.");
	}
}

void event_system_shutdown()
{
	darray_destroy(system_state->registered_events);
	efree(system_state, sizeof(event_system_state), MEMORY_TYPE_SYSTEM_STATE);
}