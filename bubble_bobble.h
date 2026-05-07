#pragma once

#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include "bagel.h"

namespace bubble_bobble
{
    using Transform = struct {
        SDL_FPoint p;
        float a;
    };

    using Drawable = struct {
        SDL_FRect part;
        SDL_FPoint size;
    };

    using Intent = struct {
        bool left = false;
        bool right = false;
        bool jump = false;
    };

    using Keys = struct {
        SDL_Scancode left;
        SDL_Scancode right;
        SDL_Scancode jump;
    };

    using Collider = struct {
        b2BodyId b;
    };

    using Player = struct {};
    using Platform = struct {};

    class BubbleBobble
    {
    public:
        BubbleBobble();
        ~BubbleBobble();

        void run();
        bool valid() const { return b2World_IsValid(box); }

    private:
        static constexpr int WIN_W = 1280;
        static constexpr int WIN_H = 720;

        static constexpr int FPS = 60;
        static constexpr Uint64 GAME_FRAME = 1000 / FPS;

        static constexpr float RAD_TO_DEG = 57.2958f;
        static constexpr float BOX_SCALE = 10.0f;

        static constexpr float PLAYER_SPEED = 18.0f;
        static constexpr float JUMP_SPEED = -34.0f;

        void input_system() const;
        void move_system() const;
        void jump_system() const;
        void box_system();
        void draw_system() const;

        bool is_player_on_platform(bagel::Entity player) const;

        void create_platform(float x, float y, float w, float h);
        void create_apple_wall(float x, float y, float w, float h);

        static constexpr Drawable makeDrawable(SDL_FRect part, SDL_FPoint size);

        SDL_Texture* tex = nullptr;
        SDL_Renderer* ren = nullptr;
        SDL_Window* win = nullptr;
        b2WorldId box = b2_nullWorldId;

        bool gameOver = false;
    };
}