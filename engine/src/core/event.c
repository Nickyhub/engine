#include "event.h"
#include "defines.h"
#include "logger.h"
#include "memory/ememory.h"

static event_system_state *state_ptr;

void registered_event_create(event_type type, pfnOnEvent callback, registered_event *out_event)
{
	out_event->callback = callback;
	out_event->type = type;
}

b8 event_system_initialize(u64 *memory_requirement, void *state)
{
	if (memory_requirement && !state)
	{
		*memory_requirement = sizeof(event_system_state);
		return true;
	}

	state_ptr = state;
	state_ptr->current_event_id_slot = 0;
	state_ptr->registered_events = darray_create_size(registered_event, MAX_REGISTERED_EVENT_CALLBACKS);
	EN_INFO("Event system initialized.");
	return true;
}

void event_system_register_event(registered_event reg_event)
{
	if (reg_event.callback && state_ptr->current_event_id_slot < MAX_REGISTERED_EVENT_CALLBACKS - 1)
	{
		darray_push_back(state_ptr->registered_events, reg_event);
		state_ptr->current_event_id_slot++;
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
		for (u32 i = 0; i < darray_length(state_ptr->registered_events); i++)
		{
			if (state_ptr->registered_events[i].type == type && state_ptr->registered_events[i].callback)
			{
				// Get the callback and call it with the appropriate context
				state_ptr->registered_events[i].callback(sender, context, type);
			}
		}
	}
	else
	{
		EN_ERROR("FireEvent was called with invalid event type.");
	}
}

void event_system_shutdown(void *state)
{
	darray_destroy(state_ptr->registered_events);
	state_ptr = 0;
}