#include "clock.h"
#include "platform.h"

void clock_create(clock* c) {
	c->elapsed_ms = 0;
	c->elapsed_sec = 0;
	c->start_time = 0;
}

void clock_start(clock* c) {
	c->elapsed_ms = 0.0;
	c->start_time = platform_get_absolute_time();
}

double clock_get_elapsed_ms(clock* c) {
	c->elapsed_ms = platform_get_absolute_time() - c->start_time;
	return c->elapsed_ms;
}

double clock_get_elapsed_sec(clock* c) {
	c->elapsed_sec = (platform_get_absolute_time() - c->start_time) / 1000.0;
	return c->elapsed_sec;
}
