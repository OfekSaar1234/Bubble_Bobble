#include "Game.h"
#include "bubble_bobble.h"
#include <SDL3_image/SDL_image.h>
#include <ctime>

namespace bubble_bobble
{
    bool Game::init()
    {
        SDL_srand(time(nullptr));

        if (!SDL_Init(SDL_INIT_VIDEO))
            return false;

        if (!SDL_CreateWindowAndRenderer(
            "Bubble Bobble",
            1280,
            720,
            0,
            &window,
            &renderer
        ))
            return false;

        sprite_sheet = IMG_LoadTexture(renderer, "res/sprite_sheet.png");

        if (!sprite_sheet)
            return false;

        b2WorldDef world_def = b2DefaultWorldDef();
        world_def.gravity = {0.0f, 10.0f};
        physics_world = b2CreateWorld(&world_def);

        game_init(physics_world);

        return true;
    }

    void Game::run()
    {
        if (!init())
            return;

        while (running && !is_game_over())
        {
            handle_events();
            update();
            render();

            SDL_Delay(16);
        }

        clean();
    }

    void Game::handle_events()
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }
    }

    void Game::update()
    {
        const bool* keyboard_state = SDL_GetKeyboardState(nullptr);

        input_system(keyboard_state);
        enemy_ai_system();
        enemy_spawn_system(physics_world);
        shooting_bubble_system(keyboard_state, physics_world);
        movement_system();
        physics_system(physics_world);
        sensor_events_system(physics_world);
        player_visual_system();
        bubble_cleanup_system();

    }

    void Game::render()
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_system(renderer, sprite_sheet);
        score_system(renderer, sprite_sheet);

        SDL_RenderPresent(renderer);
    }

    void Game::clean()
    {
        if (sprite_sheet)
            SDL_DestroyTexture(sprite_sheet);

        if (renderer)
            SDL_DestroyRenderer(renderer);

        if (window)
            SDL_DestroyWindow(window);

        SDL_Quit();
    }
}