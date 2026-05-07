#pragma once
#include <SDL3/SDL.h>
#include "bagel.h"
#include "bubble_bobble.h"

class BubbleBobbleGame {
public:
    BubbleBobbleGame();
    ~BubbleBobbleGame();

    void run();
    bool valid() const { return ren != nullptr; }

private:
    static constexpr int WIN_W = 800;
    static constexpr int WIN_H = 600;
    static constexpr int FPS = 60;
    static constexpr Uint64 GAME_FRAME = 1000 / FPS;

    // Systems
    void input_system() const;
    void movement_system() const;
    void render_system() const;

    SDL_Texture* tex = nullptr;
    SDL_Renderer* ren = nullptr;
    SDL_Window* win = nullptr;
};