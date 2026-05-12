#include "bubble_bobble.h"
#include <cstdint>

using namespace bagel;

namespace bubble_bobble
{
    const SDL_FRect GREEN_PLAYER = {170, 275, 95, 125};
    const SDL_FRect BLUE_PLAYER = {645, 275, 95, 125};
    const SDL_FRect GREEN_PLAYER_OPEN = {515, 265, 105, 135};
    const SDL_FRect ENEMY_PURPLE = {175, 470, 75, 95};
    const SDL_FRect TRAPPED_ENEMY_BROWN = {1112, 460, 110, 115};
    const SDL_FRect LIFE_ICON = {940, 805, 75, 75};
    const SDL_FRect BUBBLE = {575, 630, 95, 95};
    const SDL_FRect APPLE = {1140, 645, 75, 80};
    const SDL_FRect BANANA = {1235, 635, 105, 95};
    const SDL_FRect BEER = {1290, 485, 80, 105};
    const SDL_FRect PLATFORM = {150, 20, 500, 60};
    const SDL_FRect BOUNDS_WALL = {20, 15, 120, 730};
    const SDL_FRect BOUNDS_TILE = {10, 20, 55, 55};
    const SDL_FRect GAME_OVER_TEXT = {55, 865, 660, 150};
    const SDL_FRect YOU_WIN_TEXT = {845, 865, 575, 150};
    const SDL_FRect DIGITS[10] = {
        {925, 128, 45, 55},    // 0
        {1000, 128, 35, 55},   // 1
        {1075, 128, 45, 55},   // 2
        {1150, 128, 45, 55},   // 3
        {1225, 128, 45, 55},   // 4
        {925, 205, 45, 55},    // 5
        {1000, 205, 45, 55},   // 6
        {1075, 205, 45, 55},   // 7
        {1150, 205, 45, 55},   // 8
        {1225, 205, 45, 55}    // 9
    };

    const SDL_FRect FRUIT_SPRITES[] = {APPLE,BANANA,BEER};

    constexpr uint64_t PLAYER_CATEGORY   = 0x0001;
    constexpr uint64_t PLATFORM_CATEGORY = 0x0002;
    constexpr uint64_t WALL_CATEGORY     = 0x0004;

    constexpr float PLAYER_SPEED = 7.0f;
    constexpr float ENEMY_SPEED = 4.5f;
    constexpr float JUMP_HEIGHT = -11.0f;
    constexpr float SCALE = 30.0f;
    static bool game_over= false;
    static int enemies_created = 0;
    constexpr int MAX_ENEMIES = 8;
    constexpr int ENEMY_RELOCATE_TIME = 600;

    float random_between(float min, float max)
    {
        return min + SDL_randf() * (max - min);
    }

    // **** Systems ****
    void movement_system()
    {
        static const Mask mask = MaskBuilder{}
        .set<Movement>()
        .set<PhysicsBody>()
        .build();

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (!e.test(mask))
                continue;
            if (e.has<Bubble>())
                continue;

            Movement& movement = e.get<Movement>();

            PhysicsBody& physics = e.get<PhysicsBody>();

            b2Vec2 velocity =b2Body_GetLinearVelocity(physics.body);
            if (e.has<Player>())
            {
                b2Filter filter = b2Shape_GetFilter(physics.shape);

                if (velocity.y < -0.1f)
                {
                    filter.maskBits =UINT64_MAX &~PLATFORM_CATEGORY;
                }
                else
                {
                    filter.maskBits =UINT64_MAX;
                }

                b2Shape_SetFilter(physics.shape, filter);
            }

            velocity.x = movement.velocity_x;

            b2Body_SetLinearVelocity(
                physics.body,
                velocity
            );
        }
    }
    void input_system(const bool* keyboard_state)
    {
        Mask input_mask = MaskBuilder{}
        .set<InputControl>()
        .set<Movement>()
        .set<Direction>()
        .set<Player>()
        .set<PhysicsBody>()
        .build();

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (!e.test(input_mask))
                continue;

            InputControl& input = e.get<InputControl>();
            Movement& movement = e.get<Movement>();
            Direction& direction = e.get<Direction>();
            PhysicsBody& physics = e.get<PhysicsBody>();

            if (!input.enabled)
                continue;

            movement.velocity_x = 0.0f;

            if (keyboard_state[SDL_SCANCODE_LEFT]) {
                movement.velocity_x = PLAYER_SPEED * -1.0f;
                direction.dir = -1;
            }

            if (keyboard_state[SDL_SCANCODE_RIGHT] ) {
                movement.velocity_x = PLAYER_SPEED;
                direction.dir = 1;
            }
            b2Vec2 velocity = b2Body_GetLinearVelocity(physics.body);
            velocity.x = movement.velocity_x;
            b2Body_SetLinearVelocity(physics.body, velocity);

            if (keyboard_state[SDL_SCANCODE_SPACE] && fabs(velocity.y) < 0.1f)
            {
                b2Vec2 velocity = b2Body_GetLinearVelocity(physics.body);

                if (velocity.y > -0.1f && velocity.y < 0.001f)
                {
                    velocity.y = JUMP_HEIGHT;
                    b2Body_SetLinearVelocity(physics.body, velocity);
                }
            }


        }
    }
    void shooting_bubble_system(const bool* keyboard_state, b2WorldId physics_world)
    {
        Mask shooting_mask = MaskBuilder{}
        .set<Position>()
        .set<BubbleShooter>()
        .set<Player>()
        .set<Direction>()
        .build();

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (!e.test(shooting_mask))
                continue;

            Position& position = e.get<Position>();
            BubbleShooter& shooter = e.get<BubbleShooter>();
            Direction& direction = e.get<Direction>();

            if (shooter.cooldown > 0)
                shooter.cooldown--;

            if (keyboard_state[SDL_SCANCODE_Z] && shooter.cooldown == 0)
            {
                create_bubble(
                    position.x + direction.dir * 70.0f,
                    position.y + 35.0f,
                    direction.dir * 7.0f,
                    0.0f,
                    physics_world
                );
                Player& player = e.get<Player>();
                player.open_mouth_timer = 12;

                shooter.cooldown = 25;
            }
        }
    }
    void capture_system(b2WorldId physics_world)
    {
        Mask enemy_mask = MaskBuilder{}
        .set<Enemy>()
        .set<PhysicsBody>()
        .set<Position>()
        .build();

        Mask bubble_mask = MaskBuilder{}
        .set<Bubble>()
        .set<PhysicsBody>()
        .build();

        const b2SensorEvents sensor_events =
            b2World_GetSensorEvents(physics_world);

        if (sensor_events.beginCount > 0)
        {
            SDL_Log("Sensor events: %d", sensor_events.beginCount);
        }

        for (int i = 0; i < sensor_events.beginCount; i++)
        {
            b2SensorBeginTouchEvent event =
                sensor_events.beginEvents[i];

            b2ShapeId sensor_shape = event.sensorShapeId;
            b2ShapeId visitor_shape = event.visitorShapeId;

            b2BodyId sensor_body =
                b2Shape_GetBody(sensor_shape);

            b2BodyId visitor_body =
                b2Shape_GetBody(visitor_shape);

            for (Entity enemy = Entity::first(); !enemy.eof(); enemy.next())
            {
                if (!enemy.test(enemy_mask))
                    continue;

                PhysicsBody& enemy_physics =
                    enemy.get<PhysicsBody>();

                if (!B2_ID_EQUALS(enemy_physics.body, sensor_body))
                    continue;

                for (Entity bubble = Entity::first(); !bubble.eof(); bubble.next())
                {
                    if (!bubble.test(bubble_mask))
                        continue;

                    PhysicsBody& bubble_physics =
                        bubble.get<PhysicsBody>();

                    if (!B2_ID_EQUALS(bubble_physics.body, visitor_body))
                        continue;

                    Position& enemy_position =
                        enemy.get<Position>();

                    create_trapped_enemy(enemy_position.x,enemy_position.y, physics_world);

                    destroy_game_entity(bubble);
                    destroy_game_entity(enemy);

                    return;
                }
            }
        }
    }
    void jump_system(b2WorldId physics_world)
    {
        Mask player_mask = MaskBuilder{}
        .set<Player>()
        .set<Position>()
        .build();

        Mask trapped_mask = MaskBuilder{}
        .set<TrappedEnemy>()
        .set<Position>()
        .build();

        for (Entity player = Entity::first(); !player.eof(); player.next())
        {
            if (!player.test(player_mask))
                continue;

            Position& player_pos = player.get<Position>();

            for (Entity trapped = Entity::first(); !trapped.eof(); trapped.next())
            {
                if (!trapped.test(trapped_mask))
                    continue;

                Position& trapped_pos = trapped.get<Position>();

                float dx = player_pos.x - trapped_pos.x;
                float dy = player_pos.y - trapped_pos.y;

                float distance_squared = dx * dx + dy * dy;

                if (distance_squared < 80.0f * 80.0f) {
                    create_fruit(trapped_pos.x,trapped_pos.y,100,physics_world);

                    destroy_game_entity(trapped);
                    break;
                }
            }
        }
    }
    void collection_system(b2WorldId physics_world)
    {
        Mask fruit_mask = MaskBuilder{}
        .set<Collection>()
        .set<PhysicsBody>()
        .set<Score>()
        .build();

        Mask player_mask = MaskBuilder{}
        .set<Player>()
        .set<PhysicsBody>()
        .set<Score>()
        .build();

        const b2SensorEvents sensor_events =
            b2World_GetSensorEvents(physics_world);

        for (int i = 0; i < sensor_events.beginCount; i++)
        {
            b2SensorBeginTouchEvent event =
                sensor_events.beginEvents[i];

            b2BodyId sensor_body =
                b2Shape_GetBody(event.sensorShapeId);

            b2BodyId visitor_body =
                b2Shape_GetBody(event.visitorShapeId);

            for (Entity fruit = Entity::first(); !fruit.eof(); fruit.next())
            {
                if (!fruit.test(fruit_mask))
                    continue;

                PhysicsBody& fruit_physics =
                    fruit.get<PhysicsBody>();

                if (!B2_ID_EQUALS(fruit_physics.body, sensor_body))
                    continue;

                for (Entity player = Entity::first(); !player.eof(); player.next())
                {
                    if (!player.test(player_mask))
                        continue;

                    PhysicsBody& player_physics =
                        player.get<PhysicsBody>();

                    if (!B2_ID_EQUALS(player_physics.body, visitor_body))
                        continue;

                    Score& player_score =
                        player.get<Score>();

                    Score& fruit_score =
                        fruit.get<Score>();

                    player_score.points +=
                        fruit_score.points;

                    Player& player_component =
                        player.get<Player>();

                    player_component.eating_timer = 20;

                    SDL_Log(
                        "Score: %d",
                        player_score.points
                    );

                    destroy_game_entity(fruit);

                    return;
                }
            }
        }
    }
    void damage_system(b2WorldId physics_world)
{
    Mask enemy_mask = MaskBuilder{}
        .set<Enemy>()
        .set<PhysicsBody>()
        .build();

    Mask player_mask = MaskBuilder{}
        .set<Player>()
        .set<PhysicsBody>()
        .set<Position>()
        .build();

    const b2SensorEvents sensor_events =
        b2World_GetSensorEvents(physics_world);

    for (int i = 0; i < sensor_events.beginCount; i++)
    {
        b2SensorBeginTouchEvent event =
            sensor_events.beginEvents[i];

        b2BodyId sensor_body =
            b2Shape_GetBody(event.sensorShapeId);

        b2BodyId visitor_body =
            b2Shape_GetBody(event.visitorShapeId);

        for (Entity enemy = Entity::first(); !enemy.eof(); enemy.next())
        {
            if (!enemy.test(enemy_mask))
                continue;

            PhysicsBody& enemy_physics = enemy.get<PhysicsBody>();

            if (!B2_ID_EQUALS(enemy_physics.body, sensor_body))
                continue;

            for (Entity player = Entity::first(); !player.eof(); player.next())
            {
                if (!player.test(player_mask))
                    continue;

                PhysicsBody& player_physics = player.get<PhysicsBody>();

                if (!B2_ID_EQUALS(player_physics.body, visitor_body))
                    continue;

                Player& player_component = player.get<Player>();

                if (player_component.invincible_timer > 0)
                    return;

                player_component.lives--;
                player_component.invincible_timer = 90;

                SDL_Log("Player hit! Lives: %d", player_component.lives);

                Position& player_position = player.get<Position>();
                player_position.x = 300.0f;
                player_position.y = 430.0f;

                b2Body_SetTransform(
                    player_physics.body,
                    {300.0f / SCALE, 430.0f / SCALE},
                    b2Body_GetRotation(player_physics.body)
                );
                b2Body_SetLinearVelocity(player_physics.body,{0.0f, 0.0f});

                if (player_component.lives <= 0)
                    SDL_Log("Game Over");

                return;
            }
        }
    }
}
    void player_visual_system()
    {
        Mask player_mask = MaskBuilder{}
        .set<Player>()
        .set<Drawing>()
        .build();

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (!e.test(player_mask))
                continue;

            Drawing& drawing = e.get<Drawing>();
            Player& player = e.get<Player>();
            if (player.invincible_timer > 0)
                player.invincible_timer--;

            if (player.open_mouth_timer > 0)
            {
                drawing.sprite = GREEN_PLAYER_OPEN;
                player.open_mouth_timer--;
            }
            else if (player.eating_timer > 0)
            {
                player.eating_timer--;
                drawing.sprite = GREEN_PLAYER_OPEN;
            }
            else
            {
                drawing.sprite = GREEN_PLAYER;
            }
        }
    }
    void score_system(SDL_Renderer* renderer, SDL_Texture* sprite_sheet)
    {
        Mask player_mask = MaskBuilder{}
        .set<Player>()
        .set<Score>()
        .build();

        int score = 0;

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (e.test(player_mask))
            {
                score = e.get<Score>().points;
                break;
            }
        }

        int digits[6] = {0};
        int count = 0;

        if (score == 0)
        {
            digits[count++] = 0;
        }
        else
        {
            while (score > 0 && count < 6)
            {
                digits[count++] = score % 10;
                score /= 10;
            }
        }

        float x = 50.0f;
        float y = 60.0f;

        for (int i = count - 1; i >= 0; --i)
        {
            int digit = digits[i];

            SDL_FRect dst = {
                x,
                y,
                32.0f,
                40.0f
            };

            SDL_RenderTexture(
                renderer,
                sprite_sheet,
                &DIGITS[digit],
                &dst
            );

            x += 34.0f;
        }

        int lives = 0;

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (!e.test(player_mask))
                continue;

            lives = e.get<Player>().lives;
            break;
        }

        for (int i = 0; i < lives; i++)
        {
            SDL_FRect dst = {
                1040.0f + i * 60.0f,
                40.0f,
                45.0f,
                45.0f
            };

            SDL_RenderTexture(
                renderer,
                sprite_sheet,
                &LIFE_ICON,
                &dst
            );
        }
    }
    void bubble_cleanup_system()
    {
        static const Mask bubble_mask = MaskBuilder{}
        .set<Bubble>()
        .build();

        Entity bubble_to_destroy{{-1}};

        for (Entity bubble = Entity::first();
             !bubble.eof();
             bubble.next())
        {
            if (!bubble.test(bubble_mask))
                continue;

            Bubble& bubble_component =
                bubble.get<Bubble>();

            bubble_component.lifetime--;

            if (bubble_component.lifetime <= 0)
            {
                bubble_to_destroy = bubble;
                break;
            }
        }

        if (bubble_to_destroy.entity().id != -1)
        {
            destroy_game_entity(
                bubble_to_destroy
            );
        }
    }
    void render_system(SDL_Renderer* renderer, SDL_Texture* sprite_sheet)
{
    Mask render_mask = MaskBuilder{}
        .set<Position>()
        .set<Drawing>()
        .build();

    Mask player_mask = MaskBuilder{}
        .set<Player>()
        .build();

    // draw everything except player
    for (Entity e = Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(render_mask))
            continue;

        if (e.test(player_mask))
            continue;

        Position& position = e.get<Position>();
        Drawing& drawing = e.get<Drawing>();

        SDL_FRect dst = {
            position.x,
            position.y,
            drawing.width,
            drawing.height
        };

        SDL_FlipMode flip = SDL_FLIP_NONE;

        if (e.has<Direction>())
        {
            Direction& direction = e.get<Direction>();

            if (direction.dir == -1)
                flip = SDL_FLIP_HORIZONTAL;
        }

        SDL_RenderTextureRotated(
            renderer,
            sprite_sheet,
            &drawing.sprite,
            &dst,
            0.0,
            nullptr,
            flip
        );
    }

    // draw player last
    for (Entity e = Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(render_mask))
            continue;

        if (!e.test(player_mask))
            continue;

        Position& position = e.get<Position>();
        Drawing& drawing = e.get<Drawing>();

        SDL_FRect dst = {
            position.x,
            position.y,
            drawing.width,
            drawing.height
        };

        SDL_FlipMode flip = SDL_FLIP_NONE;

        if (e.has<Direction>())
        {
            Direction& direction = e.get<Direction>();

            if (direction.dir == -1)
                flip = SDL_FLIP_HORIZONTAL;
        }

        SDL_RenderTextureRotated(
            renderer,
            sprite_sheet,
            &drawing.sprite,
            &dst,
            0.0,
            nullptr,
            flip
        );
    }
}
    void physics_system(b2WorldId physics_world)
    {
        b2World_Step(physics_world, 1.0f / 60.0f, 4);

        Mask physics_mask = MaskBuilder{}
        .set<Position>()
        .set<PhysicsBody>()
        .set<Drawing>()
        .build();

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (!e.test(physics_mask))
                continue;

            Position& position = e.get<Position>();
            PhysicsBody& physics = e.get<PhysicsBody>();

            b2Vec2 body_position = b2Body_GetPosition(physics.body);

            Drawing& drawing = e.get<Drawing>();

            position.x = body_position.x * SCALE - drawing.width / 2.0f;

            position.y = body_position.y * SCALE - drawing.height / 2.0f;
        }
    }
    void destroy_game_entity(Entity e)
    {
        if (e.has<PhysicsBody>())
        {
            PhysicsBody& physics = e.get<PhysicsBody>();

            if (B2_IS_NON_NULL(physics.body))
            {
                b2DestroyBody(physics.body);
                physics.body = b2_nullBodyId;
            }
        }

        e.destroy();
    }
    void sensor_events_system(b2WorldId physics_world)
{
    Mask enemy_mask = MaskBuilder{}
        .set<Enemy>()
        .set<PhysicsBody>()
        .set<Position>()
        .build();

    Mask bubble_mask = MaskBuilder{}
        .set<Bubble>()
        .set<PhysicsBody>()
        .build();

    Mask player_mask = MaskBuilder{}
        .set<Player>()
        .set<PhysicsBody>()
        .set<Position>()
        .set<Score>()
        .build();

    Mask collection_mask = MaskBuilder{}
        .set<Collection>()
        .set<PhysicsBody>()
        .set<Score>()
        .build();

    Mask trapped_mask = MaskBuilder{}
        .set<TrappedEnemy>()
        .set<PhysicsBody>()
        .set<Position>()
        .build();

    const b2SensorEvents sensor_events =
        b2World_GetSensorEvents(physics_world);

    Entity bubble_to_destroy{{-1}};
    Entity enemy_to_destroy{{-1}};
    Entity fruit_to_destroy{{-1}};
    Entity trapped_to_destroy{{-1}};

    bool should_create_fruit = false;
    float fruit_x = 0.0f;
    float fruit_y = 0.0f;


    bool should_create_trapped_enemy = false;
    float trapped_x = 0.0f;
    float trapped_y = 0.0f;

    for (int i = 0; i < sensor_events.beginCount; i++)
    {
        b2SensorBeginTouchEvent event =
            sensor_events.beginEvents[i];

        b2BodyId sensor_body =
            b2Shape_GetBody(event.sensorShapeId);

        b2BodyId visitor_body =
            b2Shape_GetBody(event.visitorShapeId);

        // Enemy sensor touched something
        for (Entity enemy = Entity::first(); !enemy.eof(); enemy.next())
        {
            if (!enemy.test(enemy_mask))
                continue;

            PhysicsBody& enemy_physics =
                enemy.get<PhysicsBody>();

            if (!B2_ID_EQUALS(enemy_physics.body, sensor_body))
                continue;

            // Bubble touched Enemy sensor
            for (Entity bubble = Entity::first(); !bubble.eof(); bubble.next())
            {
                if (!bubble.test(bubble_mask))
                    continue;

                PhysicsBody& bubble_physics =
                    bubble.get<PhysicsBody>();

                if (!B2_ID_EQUALS(bubble_physics.body, visitor_body))
                    continue;

                Position& enemy_position =
                    enemy.get<Position>();

                trapped_x = enemy_position.x;
                trapped_y = enemy_position.y;

                bubble_to_destroy = bubble;
                enemy_to_destroy = enemy;
                should_create_trapped_enemy = true;
            }

            // Player touched Enemy sensor
            for (Entity player = Entity::first(); !player.eof(); player.next())
            {
                if (!player.test(player_mask))
                    continue;

                PhysicsBody& player_physics =
                    player.get<PhysicsBody>();

                if (!B2_ID_EQUALS(player_physics.body, visitor_body))
                    continue;

                Player& player_component =
                    player.get<Player>();

                if (player_component.invincible_timer > 0)
                    continue;

                player_component.lives--;
                player_component.invincible_timer = 90;

              //  Position& player_position =player.get<Position>();

              //  player_position.x = 120.0f;
              //  player_position.y = 500.0f;

                b2Body_SetTransform(
                    player_physics.body,
                    {390.0f / SCALE, 85.0f / SCALE},
                    b2Body_GetRotation(player_physics.body)
                );

                b2Body_SetLinearVelocity(
                    player_physics.body,
                    {0.0f, 0.0f}
                );

                if (player_component.lives <= 0)
                    game_over = true;
            }
        }

        // Collection sensor touched Player
        for (Entity fruit = Entity::first(); !fruit.eof(); fruit.next())
        {
            if (!fruit.test(collection_mask))
                continue;

            PhysicsBody& fruit_physics =
                fruit.get<PhysicsBody>();

            if (!B2_ID_EQUALS(fruit_physics.body, sensor_body))
                continue;

            for (Entity player = Entity::first(); !player.eof(); player.next())
            {
                if (!player.test(player_mask))
                    continue;

                PhysicsBody& player_physics =
                    player.get<PhysicsBody>();

                if (!B2_ID_EQUALS(player_physics.body, visitor_body))
                    continue;

                Score& player_score =
                    player.get<Score>();

                Score& fruit_score =
                    fruit.get<Score>();

                player_score.points += fruit_score.points;

                Player& player_component =
                    player.get<Player>();

                player_component.eating_timer = 20;
                fruit_to_destroy = fruit;
            }
        }
        for (Entity trapped = Entity::first(); !trapped.eof(); trapped.next())
        {
            if (!trapped.test(trapped_mask))
                continue;

            PhysicsBody& trapped_physics =
                trapped.get<PhysicsBody>();

            if (!B2_ID_EQUALS(trapped_physics.body, sensor_body))
                continue;

            for (Entity player = Entity::first(); !player.eof(); player.next())
            {
                if (!player.test(player_mask))
                    continue;

                PhysicsBody& player_physics =
                    player.get<PhysicsBody>();

                if (!B2_ID_EQUALS(player_physics.body, visitor_body))
                    continue;

                Position& trapped_position =
                    trapped.get<Position>();

                fruit_x = random_between(100.0f, 1100.0f);
                fruit_y = random_between(100.0f, 580.0f);

                should_create_fruit = true;
                trapped_to_destroy = trapped;
            }
        }
    }

    if (should_create_trapped_enemy)
    {
        create_trapped_enemy(trapped_x,trapped_y,physics_world);
    }
    if (should_create_fruit)
    {
        create_fruit(fruit_x,fruit_y,100,physics_world);
    }

    if (bubble_to_destroy.entity().id != -1)
        destroy_game_entity(bubble_to_destroy);

    if (enemy_to_destroy.entity().id != -1)
        destroy_game_entity(enemy_to_destroy);

    if (fruit_to_destroy.entity().id != -1)
        destroy_game_entity(fruit_to_destroy);
    if (trapped_to_destroy.entity().id != -1)
        destroy_game_entity(trapped_to_destroy);
}
    void enemy_ai_system()
    {
        static const Mask mask = MaskBuilder{}
        .set<Enemy>()
        .set<Movement>()
        .set<PhysicsBody>()
        .build();

        for (Entity enemy = Entity::first(); !enemy.eof(); enemy.next())
        {
            if (!enemy.test(mask))
                continue;

            Movement& movement = enemy.get<Movement>();
            Enemy& enemy_component = enemy.get<Enemy>();
            PhysicsBody& physics = enemy.get<PhysicsBody>();

            if (SDL_randf() < 0.01f)
            {
                if (SDL_randf() < 0.5f)
                    movement.velocity_x = ENEMY_SPEED;
                else
                    movement.velocity_x = ENEMY_SPEED * -1.0f;
            }

            enemy_component.relocate_timer--;

            if (enemy_component.relocate_timer <= 0)
            {
                const float enemy_width = 50.0f;
                const float enemy_height = 60.0f;

                float positions[][2] = {
                    {250.0f, 480.0f},
                    {700.0f, 480.0f},
                    {300.0f, 350.0f},
                    {850.0f, 350.0f},
                    {520.0f, 230.0f},
                    {900.0f, 230.0f}
                };

                int index = static_cast<int>(SDL_randf() * 6.0f);

                float x = positions[index][0];
                float y = positions[index][1];

                b2Body_SetTransform(
                    physics.body,
                    {
                        (x + enemy_width / 2.0f) / SCALE,
                        (y + enemy_height / 2.0f) / SCALE
                    },
                    b2Body_GetRotation(physics.body)
                );

                b2Body_SetLinearVelocity(
                    physics.body,
                    {0.0f, 0.0f}
                );

                enemy_component.relocate_timer = ENEMY_RELOCATE_TIME;
            }
        }
    }
    bool is_game_over()
    {
        return game_over;
    }
    void enemy_spawn_system(b2WorldId physics_world)
    {
        static int spawn_timer = 180;

        spawn_timer--;

        if (spawn_timer > 0)
            return;

        int enemy_count = 0;

        static const Mask enemy_mask = MaskBuilder{}
        .set<Enemy>()
        .build();

        for (Entity e = Entity::first(); !e.eof(); e.next())
        {
            if (e.test(enemy_mask))
                enemy_count++;
        }

        if (enemy_count < MAX_ENEMIES && enemies_created < MAX_ENEMIES)
        {
            float x = random_between(100.0f, 1100.0f);
            float y = random_between(80.0f, 500.0f);

            create_enemy(x, y, physics_world);
            enemies_created++;
        }

        spawn_timer = static_cast<int>(
            random_between(180.0f, 420.0f)
        );
    }


    void sound_system() {}
    void level_progression_system() {}

    void game_init(b2WorldId physics_world)
    {
        create_player(390.0f, 85.0f, physics_world);
        // screen bounds
        create_bounds(0.0f, 0.0f, 40.0f, 720.0f, physics_world);
        create_bounds(1240.0f, 0.0f, 40.0f, 720.0f, physics_world);
        create_bounds(0.0f, 0.0f, 1280.0f, 40.0f, physics_world);
        create_bounds(0.0f, 680.0f, 1280.0f, 40.0f, physics_world);

        // platforms
        create_platform(200.0f, 540.0f, 360.0f, 40.0f, physics_world);
        create_platform(700.0f, 540.0f, 360.0f, 40.0f, physics_world);

        create_platform(220.0f, 410.0f, 300.0f, 40.0f, physics_world);
        create_platform(760.0f, 410.0f, 300.0f, 40.0f, physics_world);

        create_platform(170.0f, 280.0f, 280.0f, 40.0f, physics_world);
        create_platform(600.0f, 280.0f, 280.0f, 40.0f, physics_world);
        create_platform(950.0f, 280.0f, 280.0f, 40.0f, physics_world);

        create_platform(360.0f, 160.0f, 560.0f, 40.0f, physics_world);

        // enemies
        create_enemy(300.0f, 200.0f, physics_world);
        create_enemy(700.0f, 200.0f, physics_world);
        create_enemy(920.0f, 330.0f, physics_world);
        enemies_created+=3;

        create_score_display(30.0f, 50.0f);
    }
    // **** Entities ****
    ent_type create_player(float x, float y, b2WorldId physics_world)
    {
        Entity player = Entity::create();

        b2BodyDef body_def = b2DefaultBodyDef();
        body_def.type = b2_dynamicBody;
        body_def.position = {x / 30.0f, y / 30.0f};

        b2BodyId body = b2CreateBody(physics_world, &body_def);

        b2Polygon box = b2MakeBox(30.0f / SCALE,37.5f / SCALE);
        b2ShapeDef shape_def = b2DefaultShapeDef();
        shape_def.filter.categoryBits =PLAYER_CATEGORY;
        shape_def.filter.maskBits =UINT64_MAX;
        shape_def.density = 1.0f;  // צפיפות (כובד)
        shape_def.material.friction = 0.3f; // חיכוך
        shape_def.enableSensorEvents = true;


        b2ShapeId shape = b2CreatePolygonShape(body, &shape_def, &box);
        player.addAll(
            Position{x, y},
            Movement{0.0f, 0.0f},
            Sound{-1},
            Drawing{GREEN_PLAYER, 60.0f, 75.0f},
            InputControl{true},
            BubbleShooter{0},
            Score{0},
            Direction{1},
            PhysicsBody{body, shape},
            Player{}
        );

        return player.entity();
    }

    ent_type create_bubble(float x, float y, float velocity_x, float velocity_y, b2WorldId physics_world)
    {
        Entity bubble = Entity::create();
        b2BodyDef body_def = b2DefaultBodyDef();

        body_def.type = b2_dynamicBody;
        body_def.position = {
            x / SCALE,
            y / SCALE
        };

        body_def.gravityScale = 0.0f;

        b2BodyId body = b2CreateBody(physics_world,&body_def);
        b2Circle circle;
        circle.center = {0.0f, 0.0f};
        circle.radius = 25.0f / SCALE;

        b2ShapeDef shape_def = b2DefaultShapeDef();

        shape_def.density = 0.1f;
        shape_def.material.friction = 0.0f;
        shape_def.material.restitution = 1.0f;
        shape_def.enableSensorEvents = true;

        b2Vec2 velocity = {velocity_x, velocity_y};
        b2Body_SetLinearVelocity(body, velocity);

        b2CreateCircleShape(
            body,
            &shape_def,
            &circle
        );

        bubble.addAll(
            Position{x, y},
            Movement{velocity_x, velocity_y},
            Drawing{BUBBLE, 60.0f, 60.0f},
            Sound{-1},
            Bubble{240},
            PhysicsBody{body ,b2_nullShapeId}
        );
        return bubble.entity();
    }

    ent_type create_enemy(float x, float y, b2WorldId physics_world)
    {
        Entity enemy = Entity::create();

        b2BodyDef body_def = b2DefaultBodyDef();

        body_def.type = b2_dynamicBody;

        body_def.position = {
            x / SCALE,
            y / SCALE
        };

       // body_def.fixedRotation = true;

        b2BodyId body = b2CreateBody(physics_world,&body_def);

        b2Polygon box = b2MakeBox(25.0f / SCALE,30.0f / SCALE);

        b2ShapeDef shape_def = b2DefaultShapeDef();

        shape_def.density = 1.0f;
        shape_def.material.friction = 0.3f;

        b2CreatePolygonShape(
            body,
            &shape_def,
            &box
        );
        b2Polygon sensor_box = b2MakeBox(35.0f / SCALE,40.0f / SCALE);

        b2ShapeDef sensor_def = b2DefaultShapeDef();

        sensor_def.isSensor = true;
        sensor_def.enableSensorEvents = true;

        b2CreatePolygonShape(
            body,
            &sensor_def,
            &sensor_box
        );

        enemy.addAll(
            Position{x, 430.0f},
            Movement{2.0f, 0.0f},
            Drawing{ENEMY_PURPLE, 50.0f, 60.0f},
            Damage{1},
            Enemy{600},
            PhysicsBody{body, b2_nullShapeId}
        );

        return enemy.entity();
    }

    ent_type create_pressure_enemy(float x, float y)
    {
        Entity pressure_enemy = Entity::create();

        pressure_enemy.addAll(
            Position{x, y},
            Movement{0.0f, 0.0f},
            Drawing{ENEMY_PURPLE, 70.0f, 80.0f},
            Damage{1},
            Enemy{}
        );

        return pressure_enemy.entity();
    }

    ent_type create_trapped_enemy(float x, float y, b2WorldId physics_world)
    {
        Entity trapped_enemy = Entity::create();
        b2BodyDef body_def = b2DefaultBodyDef();

        body_def.type = b2_staticBody;

        body_def.position = {
            x / SCALE,
            y / SCALE
        };

        b2BodyId body = b2CreateBody(
            physics_world,
            &body_def
        );

        b2Circle circle;
        circle.center = {0.0f, 0.0f};
        circle.radius = 40.0f / SCALE;

        b2ShapeDef shape_def = b2DefaultShapeDef();

        shape_def.isSensor = true;
        shape_def.enableSensorEvents = true;

        b2CreateCircleShape(
            body,
            &shape_def,
            &circle
        );
        trapped_enemy.addAll(
            Position{x, y},
            Movement{0.0f, 0.0f},
            Drawing{TRAPPED_ENEMY_BROWN, 90.0f, 90.0f},
            TrappedEnemy{},
            Jump{},
            PhysicsBody{body, b2_nullShapeId}
        );

        return trapped_enemy.entity();
    }

    ent_type create_fruit(float x, float y, int points,b2WorldId physics_world)
    {
        Entity fruit = Entity::create();
        b2BodyDef body_def = b2DefaultBodyDef();

        body_def.type = b2_staticBody;

        body_def.position = {
            x / SCALE,
            y / SCALE
        };

        b2BodyId body = b2CreateBody(
            physics_world,
            &body_def
        );
        b2Polygon fruit_box = b2MakeBox(25.0f / SCALE,25.0f / SCALE
);

        b2ShapeDef shape_def = b2DefaultShapeDef();

        shape_def.isSensor = true;
        shape_def.enableSensorEvents = true;

        b2CreatePolygonShape(
            body,
            &shape_def,
            &fruit_box
        );
        const int fruit_count = sizeof(FRUIT_SPRITES) / sizeof(FRUIT_SPRITES[0]);
        int index = static_cast<int>(SDL_randf() * fruit_count);

        SDL_FRect fruit_sprite =FRUIT_SPRITES[index];

        fruit.addAll(
            Position{x, y},
            Drawing{fruit_sprite, 55.0f, 55.0f},
            Collection{},
            Score{points},
            Sound{-1},
            PhysicsBody{body, b2_nullShapeId}
        );

        return fruit.entity();
    }

    ent_type create_bounds(float x,float y,float width,float height,b2WorldId physics_world)
    {
        Entity bounds = Entity::create();

        b2BodyDef body_def = b2DefaultBodyDef();
        body_def.type = b2_staticBody;

        body_def.position = {
            (x + width / 2.0f) / SCALE,
            (y + height / 2.0f) / SCALE
        };

        b2BodyId body = b2CreateBody(
            physics_world,
            &body_def
        );

        b2Polygon box = b2MakeBox(
            width / 2.0f / SCALE,
            height / 2.0f / SCALE
        );

        b2ShapeDef shape_def = b2DefaultShapeDef();
        shape_def.filter.categoryBits =WALL_CATEGORY;
        shape_def.filter.maskBits =UINT64_MAX;

        b2CreatePolygonShape(
            body,
            &shape_def,
            &box
        );

        bounds.addAll(
            PhysicsBody{body}
        );

        const float tile_size = 40.0f;

        for (float tile_y = y; tile_y < y + height; tile_y += tile_size)
        {
            for (float tile_x = x; tile_x < x + width; tile_x += tile_size)
            {
                Entity tile = Entity::create();

                tile.addAll(
                    Position{tile_x, tile_y},
                    Drawing{BOUNDS_TILE, tile_size, tile_size}
                );
            }
        }

        return bounds.entity();
    }

    ent_type create_map(float x, float y, b2WorldId physics_world)
    {
        Entity map = Entity::create();
        b2BodyDef body_def = b2DefaultBodyDef();
        body_def.type = b2_staticBody;
        body_def.position = {
            (x + 200.0f) / SCALE,
            (y + 20.0f) / SCALE
        };

        b2BodyId body = b2CreateBody(physics_world, &body_def);

        b2Polygon box = b2MakeBox(
            200.0f / SCALE,
            20.0f / SCALE
        );

        b2ShapeDef shape_def = b2DefaultShapeDef();
        b2CreatePolygonShape(body, &shape_def, &box);

        map.addAll(
            Position{x, y},
            Drawing{PLATFORM, 400.0f, 40.0f},
            PhysicsBody{body}
        );

        return map.entity();
    }

    ent_type create_platform(float x,float y,float width,float height,b2WorldId physics_world)
    {
        Entity platform = Entity::create();

        b2BodyDef body_def = b2DefaultBodyDef();
        body_def.type = b2_staticBody;

        body_def.position = {
            (x + width / 2.0f) / SCALE,
            (y + height / 2.0f) / SCALE
        };

        b2BodyId body = b2CreateBody(
            physics_world,
            &body_def
        );

        b2Polygon box = b2MakeBox(
            width / 2.0f / SCALE,
            height / 2.0f / SCALE
        );

        b2ShapeDef shape_def = b2DefaultShapeDef();
        shape_def.filter.categoryBits =PLATFORM_CATEGORY;
        shape_def.filter.maskBits =UINT64_MAX;

        b2CreatePolygonShape(
            body,
            &shape_def,
            &box
        );

        platform.addAll(
            PhysicsBody{body}
        );

        const float tile_size = 40.0f;

        for (float tile_x = x; tile_x < x + width; tile_x += tile_size)
        {
            Entity tile = Entity::create();

            tile.addAll(
                Position{tile_x, y},
                Drawing{PLATFORM, tile_size, height}
            );
        }

        return platform.entity();
    }

    ent_type create_score_display(float x, float y)
    {
        Entity score_display = Entity::create();

        score_display.addAll(
            Position{x, y},
            Score{0}
        );

        return score_display.entity();
    }
}