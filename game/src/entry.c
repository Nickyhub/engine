#include <entry.h>

#include "game.h"

#include <entry.h>

#include <memory/ememory.h>

// Define the function to create a game
b8 create_game(game* out_game) {
    // Application configuration.
    out_game->app_config.window_width = 1920;
    out_game->app_config.window_height = 1080;
    out_game->app_config.name = "Kohi Engine Testbed";
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->initialize = game_initialize;
    out_game->on_resize = game_on_resize;

    // Create the game state.
    out_game->state = eallocate(sizeof(game_state), MEMORY_TYPE_GAME);
    ezero_out(out_game->state, sizeof(game_state));
    return TRUE;
}