#include "bubble_bobble.h"
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <cmath>

using namespace std;
using namespace bagel;

namespace bubble_bobble
{
    constexpr Drawable BubbleBobble::makeDrawable(SDL_FRect part, SDL_FPoint size)
    {
        return Drawable{{part}, size};
    }

    void BubbleBobble::input_system() const
    {
        static const Mask mask = MaskBuilder()
            .set<Keys>()
            .set<Intent>()
            .build();

        SDL_PumpEvents();
        const bool* keys = SDL_GetKeyboardState(nullptr);

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(mask)) {
                const auto& k = e.get<Keys>();
                auto& i = e.get<Intent>();

                i.left = keys[k.left];
                i.right = keys[k.right];
                i.jump = keys[k.jump];
            }
        }
    }

    void BubbleBobble::move_system() const
    {
        static const Mask mask = MaskBuilder()
            .set<Player>()
            .set<Intent>()
            .set<Collider>()
            .build();

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(mask)) {
                const auto& i = e.get<Intent>();
                const auto& c = e.get<Collider>();

                b2Vec2 velocity = b2Body_GetLinearVelocity(c.b);
                velocity.x = 0;

                if (i.left) {
                    velocity.x = -PLAYER_SPEED;
                }

                if (i.right) {
                    velocity.x = PLAYER_SPEED;
                }

                b2Body_SetLinearVelocity(c.b, velocity);
            }
        }
    }

    bool BubbleBobble::is_player_on_platform(Entity player) const
    {
        static const Mask platformMask = MaskBuilder()
            .set<Platform>()
            .set<Transform>()
            .set<Drawable>()
            .build();

        const auto& playerTransform = player.get<Transform>();

        const float playerHalfW = 20.0f;
        const float playerHalfH = 30.0f;

        const float playerBottom = playerTransform.p.y + playerHalfH;
        const float playerLeft = playerTransform.p.x - playerHalfW;
        const float playerRight = playerTransform.p.x + playerHalfW;

        for (Entity p = Entity::first(); !p.eof(); p.next()) {
            if (p.test(platformMask)) {
                const auto& platformTransform = p.get<Transform>();
                const auto& platformDrawable = p.get<Drawable>();

                const float platformTop = platformTransform.p.y - platformDrawable.size.y / 2.0f;
                const float platformLeft = platformTransform.p.x - platformDrawable.size.x / 2.0f;
                const float platformRight = platformTransform.p.x + platformDrawable.size.x / 2.0f;

                const bool closeToTop = fabs(playerBottom - platformTop) < 9.0f;
                const bool insideX = playerRight > platformLeft && playerLeft < platformRight;

                if (closeToTop && insideX) {
                    return true;
                }
            }
        }

        return false;
    }

    void BubbleBobble::jump_system() const
    {
        static const Mask mask = MaskBuilder()
            .set<Player>()
            .set<Intent>()
            .set<Collider>()
            .set<Transform>()
            .build();

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(mask)) {
                const auto& i = e.get<Intent>();
                const auto& c = e.get<Collider>();

                b2Vec2 velocity = b2Body_GetLinearVelocity(c.b);

                if (i.jump && velocity.y >= -0.5f && is_player_on_platform(e)) {
                    velocity.y = JUMP_SPEED;
                    b2Body_SetLinearVelocity(c.b, velocity);
                }
            }
        }
    }

    void BubbleBobble::box_system()
    {
        static constexpr float BOX_STEP = 1.0f / FPS;

        static const Mask mask = MaskBuilder()
            .set<Transform>()
            .set<Collider>()
            .build();

        b2World_Step(box, BOX_STEP, 4);

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(mask)) {
                const auto t = b2Body_GetTransform(e.get<Collider>().b);

                e.get<Transform>() = {
                    {t.p.x * BOX_SCALE, t.p.y * BOX_SCALE},
                    RAD_TO_DEG * b2Rot_GetAngle(t.q)
                };

                if (e.has<Player>()) {
                    e.get<Transform>().a = 0;

                    if (e.get<Transform>().p.y > WIN_H + 80) {
                        gameOver = true;
                        e.destroy();
                        return;
                    }
                }
            }
        }
    }

    void BubbleBobble::draw_system() const
    {
        static const Mask mask = MaskBuilder()
            .set<Transform>()
            .set<Drawable>()
            .build();

        for (Entity e = Entity::first(); !e.eof(); e.next()) {
            if (e.test(mask)) {
                const auto& t = e.get<Transform>();
                const auto& d = e.get<Drawable>();

                SDL_FRect dest = {
                    t.p.x - d.size.x / 2.0f,
                    t.p.y - d.size.y / 2.0f,
                    d.size.x,
                    d.size.y
                };

                SDL_RenderTextureRotated(
                    ren,
                    tex,
                    &d.part,
                    &dest,
                    t.a,
                    nullptr,
                    SDL_FLIP_NONE
                );
            }
        }
    }

    void BubbleBobble::create_platform(float x, float y, float w, float h)
    {
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_staticBody;
        def.position = {x / BOX_SCALE, y / BOX_SCALE};

        b2BodyId body = b2CreateBody(box, &def);

        b2ShapeDef shapeDef = b2DefaultShapeDef();

        b2Polygon platformBox = b2MakeBox(
            w / 2.0f / BOX_SCALE,
            h / 2.0f / BOX_SCALE
        );

        b2CreatePolygonShape(body, &shapeDef, &platformBox);

        Entity::create().addAll(
            Platform{},
            Transform{{x, y}, 0},
            makeDrawable({156, 31, 480, 43}, {w, h}),
            Collider{body}
        );
    }

    void BubbleBobble::create_apple_wall(float x, float y, float w, float h)
    {
        b2BodyDef def = b2DefaultBodyDef();
        def.type = b2_staticBody;
        def.position = {x / BOX_SCALE, y / BOX_SCALE};

        b2BodyId body = b2CreateBody(box, &def);

        b2ShapeDef shapeDef = b2DefaultShapeDef();

        b2Polygon wallBox = b2MakeBox(
            w / 2.0f / BOX_SCALE,
            h / 2.0f / BOX_SCALE
        );

        b2CreatePolygonShape(body, &shapeDef, &wallBox);

        Entity::create().addAll(
            Platform{},
            Transform{{x, y}, 0},
            makeDrawable({13, 20, 124, 720}, {w, h}),
            Collider{body}
        );
    }

    BubbleBobble::BubbleBobble()
    {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            cout << SDL_GetError() << endl;
            return;
        }

        if (!SDL_CreateWindowAndRenderer(
            "Bubble Bobble",
            WIN_W,
            WIN_H,
            SDL_WINDOW_RESIZABLE,
            &win,
            &ren)) {
            cout << SDL_GetError() << endl;
            return;
        }

        SDL_Surface* surf = IMG_Load("res/sprite_sheet.png");

        if (surf == nullptr) {
            cout << SDL_GetError() << endl;
            return;
        }

        tex = SDL_CreateTextureFromSurface(ren, surf);

        if (tex == nullptr) {
            cout << SDL_GetError() << endl;
            SDL_DestroySurface(surf);
            return;
        }

        SDL_DestroySurface(surf);

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0, 34};

        box = b2CreateWorld(&worldDef);

        if (!b2World_IsValid(box)) {
            cout << "Failed creating Box2D world" << endl;
            return;
        }

        // ===== PLAYER START POSITION LIKE THE IMAGE =====

        b2BodyDef playerDef = b2DefaultBodyDef();
        playerDef.type = b2_dynamicBody;
        playerDef.position = {
            285.0f / BOX_SCALE,
            455.0f / BOX_SCALE
        };

        b2BodyId playerBody = b2CreateBody(box, &playerDef);

        b2ShapeDef playerShapeDef = b2DefaultShapeDef();
        playerShapeDef.density = 1.0f;

        b2Polygon playerBox = b2MakeBox(
            20.0f / BOX_SCALE,
            30.0f / BOX_SCALE
        );

        b2CreatePolygonShape(playerBody, &playerShapeDef, &playerBox);

        Entity::create().addAll(
            Player{},
            Transform{{285.0f, 455.0f}, 0},
            makeDrawable({150, 275, 115, 135}, {52, 62}),
            Collider{playerBody},
            Intent{},
            Keys{
                SDL_SCANCODE_LEFT,
                SDL_SCANCODE_RIGHT,
                SDL_SCANCODE_SPACE
            }
        );

        // ===== MAP FRAME WITH APPLES ON SIDES =====

        create_apple_wall(35, 360, 55, 660);
        create_apple_wall(1245, 360, 55, 660);

        // עליון - שלושה חלקים כמו בתמונה
        create_platform(245, 70, 330, 28);
        create_platform(640, 70, 210, 28);
        create_platform(1035, 70, 330, 28);

        // ===== MAIN MAP STRUCTURE LIKE THE IMAGE =====

        // קומה עליונה פנימית
        create_platform(385, 185, 360, 26);
        create_platform(895, 185, 360, 26);

        // קומה אמצעית עליונה
        create_platform(335, 305, 460, 26);
        create_platform(945, 305, 460, 26);

        // קומה אמצעית תחתונה - פוזיציית השחקן מתחילה כאן בצד שמאל
        create_platform(280, 425, 460, 26);
        create_platform(945, 425, 460, 26);

        // קומה נמוכה קצרה
        create_platform(175, 555, 240, 26);
        create_platform(380, 555, 90, 26);
        create_platform(900, 555, 90, 26);
        create_platform(1110, 555, 240, 26);

        // רצפה תחתונה עם פתחים לנפילה - אין רצפה מלאה כדי שיהיה Game Over
        create_platform(245, 675, 330, 28);
        create_platform(640, 675, 210, 28);
        create_platform(1035, 675, 330, 28);
    }

    BubbleBobble::~BubbleBobble()
    {
        if (b2World_IsValid(box))
            b2DestroyWorld(box);

        if (tex != nullptr)
            SDL_DestroyTexture(tex);

        if (ren != nullptr)
            SDL_DestroyRenderer(ren);

        if (win != nullptr)
            SDL_DestroyWindow(win);

        SDL_Quit();
    }

    void BubbleBobble::run()
    {
        auto start = SDL_GetTicks();
        bool quit = false;

        while (!quit && !gameOver) {
            input_system();
            move_system();
            jump_system();
            box_system();

            SDL_RenderClear(ren);
            draw_system();
            SDL_RenderPresent(ren);

            const auto end = SDL_GetTicks();

            if (end - start < GAME_FRAME) {
                SDL_Delay(GAME_FRAME - (end - start));
            }

            start += GAME_FRAME;

            SDL_Event e;

            while (SDL_PollEvent(&e)) {
                if ((e.type == SDL_EVENT_QUIT) ||
                    ((e.type == SDL_EVENT_KEY_DOWN) &&
                     (e.key.scancode == SDL_SCANCODE_ESCAPE))) {
                    quit = true;
                }
            }
        }
    }
}