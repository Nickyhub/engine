#pragma once

typedef struct clock {
	double elapsed_ms;
	double elapsed_sec;
	double start_time;
} clock;

void clock_create(clock* c);

void clock_start(clock* c);

double clock_get_elapsed_sec(clock* c);

double clock_get_elapsed_ms(clock* c);
