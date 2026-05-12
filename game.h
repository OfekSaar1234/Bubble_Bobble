#pragma once
#include <box2d/box2d.h>
#include <SDL3/SDL.h>

namespace bubble_bobble
{
    class Game
    {
    public:
        bool init();
        void run();
        void clean();

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* sprite_sheet = nullptr;
        b2WorldId physics_world = b2_nullWorldId;

        bool running = true;

        void handle_events();
        void update();
        void render();
    };
}