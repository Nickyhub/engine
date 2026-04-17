#include "application.h"
#include "containers/darray.h"
#include "logger.h"

int main() {
	application_config config;
	config.name = "Engine";
	config.window_height = 1080;
	config.window_width = 1920;
	config.target_ticks_per_second = 60;

	application_state app_state;

	if(!application_initialize(config)) {
		EN_FATAL("Failed to initialize application. Shutting down.");
		return false;
	}
	application_run();
	return 0;
} 