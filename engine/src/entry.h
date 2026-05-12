#pragma once

#include "core/application.h"
#include "containers/darray.h"
#include "core/logger.h"

#include "game_types.h"
#include "memory/ememory.h"

// Created in entry.c in the game
extern b8 create_game(game* out_game);

int main(void)
{
	memory_system_initialize();

	// Request the game instance from the application.
	game game_inst;
	if (!create_game(&game_inst))
	{
		EN_FATAL("Could not create game!");
		return -1;
	}

	// Ensure the function pointers exist.
	if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize)
	{
		EN_FATAL("The game's function pointers must be assigned!");
		return -2;
	}

	// Initialization.
	if (!application_initialize(&game_inst))
	{
		EN_FATAL("Application failed to create!.");
		return 1;
	}

	// Begin the game loop.
	if (!application_run())
	{
		EN_INFO("Application did not shutdown gracefully.");
		return 2;
	}


	return 0;
}