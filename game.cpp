#include "Game.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>

using namespace std;
using namespace bagel;
using namespace bubble_bobble;

BubbleBobbleGame::BubbleBobbleGame() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        cout << SDL_GetError() << endl;
        return;
    }

    if (!SDL_CreateWindowAndRenderer("Bubble Bobble", WIN_W, WIN_H, 0, &win, &ren)) {
        cout << SDL_GetError() << endl;
        return;
    }

    // Replace with your actual sprite sheet name
    SDL_Surface *surf = IMG_Load("res/bomberman.png");
    if (surf != nullptr) {
        tex = SDL_CreateTextureFromSurface(ren, surf);
        SDL_DestroySurface(surf);
    }

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

    // Spawn initial entities
    create_player(100.0f, 100.0f);
    create_enemy(400.0f, 100.0f);
}

BubbleBobbleGame::~BubbleBobbleGame() {
    if (tex != nullptr) SDL_DestroyTexture(tex);
    if (ren != nullptr) SDL_DestroyRenderer(ren);
    if (win != nullptr) SDL_DestroyWindow(win);
    SDL_Quit();
}

void BubbleBobbleGame::input_system() const {
    static const Mask mask = MaskBuilder()
        .set<InputControl>()
        .set<Movement>()
        .build();

    SDL_PumpEvents();
    const bool* keys = SDL_GetKeyboardState(nullptr);

    for (Entity e = Entity::first(); !e.eof(); e.next()) {
        if (e.test(mask)) {
            const auto& input = e.get<InputControl>();
            auto& mov = e.get<Movement>();

            if (input.enabled) {
                mov.velocity_x = 0;
                mov.velocity_y = 0;
                if (keys[SDL_SCANCODE_RIGHT]) mov.velocity_x = 2.0f;
                if (keys[SDL_SCANCODE_LEFT]) mov.velocity_x = -2.0f;
                if (keys[SDL_SCANCODE_UP]) mov.velocity_y = -2.0f;
                if (keys[SDL_SCANCODE_DOWN]) mov.velocity_y = 2.0f;
            }
        }
    }
}

void BubbleBobbleGame::movement_system() const {
    static const Mask mask = MaskBuilder()
        .set<Position>()
        .set<Movement>()
        .build();

    for (Entity e = Entity::first(); !e.eof(); e.next()) {
        if (e.test(mask)) {
            auto& pos = e.get<Position>();
            const auto& mov = e.get<Movement>();

            pos.x += mov.velocity_x;
            pos.y += mov.velocity_y;
        }
    }
}

void BubbleBobbleGame::render_system() const {
    static const Mask mask = MaskBuilder()
        .set<Position>()
        .set<Drawing>()
        .build();

    for (Entity e = Entity::first(); !e.eof(); e.next()) {
        if (e.test(mask)) {
            const auto& pos = e.get<Position>();

            // Temporary rendering logic until you map your sprite sheet
            SDL_FRect dest = { pos.x, pos.y, 32.0f, 32.0f };

            if (tex) {
                SDL_FRect src = {0, 0, 16, 16}; // Placeholder
                SDL_RenderTexture(ren, tex, &src, &dest);
            } else {
                // Fallback colored square if image fails to load
                SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
                SDL_RenderFillRect(ren, &dest);
            }
        }
    }
}

void BubbleBobbleGame::run() {
    auto start = SDL_GetTicks();
    bool quit = false;

    while (!quit) {
        // 1. Update Systems
        input_system();
        movement_system();

        // 2. Render
        SDL_SetRenderDrawColor(ren, 50, 50, 150, 255); // Blue-ish background
        SDL_RenderClear(ren);
        render_system();
        SDL_RenderPresent(ren);

        // 3. Frame Timing
        const auto end = SDL_GetTicks();
        if (end - start < GAME_FRAME) {
            SDL_Delay(GAME_FRAME - (end - start));
        }
        start += GAME_FRAME;

        // 4. Events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if ((e.type == SDL_EVENT_QUIT) ||
                ((e.type == SDL_EVENT_KEY_DOWN) && (e.key.scancode == SDL_SCANCODE_ESCAPE))) {
                quit = true;
            }
        }
    }
}