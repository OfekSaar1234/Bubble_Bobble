#pragma once
#include "bagel.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
using namespace bagel;

namespace bubble_bobble
{
    // **** Components ****
    struct Movement
    {
        // preferred storage: sparse
        // reason: Only a subset of entities move, so sparse storage avoids wasting memory on static entities.
        float velocity_x = 0.0f;
        float velocity_y = 0.0f;
    };


    struct Position
    {
        // preferred storage: packed
        // reason: Most entities have a position and systems frequently iterate over them.
        float x = 0.0f;
        float y = 0.0f;
    };


    struct Sound
    {
        // preferred storage: sparse
        // reason: Only some entities trigger sounds, so sparse storage saves memory.
        int sound_id = -1;
    };


    struct Score
    {
        // preferred storage: sparse
        // reason: Only specific entities use score, so sparse storage avoids unnecessary data.
        int points = 0;
    };


    struct Drawing
    {
        // preferred storage: packed
        // reason: Most entities need to be rendered, so packed storage allows efficient iteration.
        // The part of the sprite sheet we want to draw
        SDL_FRect sprite;

        // The size on the screen
        float width = 0.0f;
        float height = 0.0f;
    };

    struct InputControl
    {
        // preferred storage: sparse
        // reason: Only player-controlled entities need input handling.
        bool enabled = true;
    };

    struct BubbleShooter
    {
        // preferred storage: sparse
        // reason: Only the player has shooting ability, so sparse storage is sufficient.
        int cooldown = 0;
    };

    struct Jump
    {
        // preferred storage: tag
        // reason: The existence of the component is enough to mark jump interaction.
    };

    struct Bubble
    {
        int lifetime = 240;
    };

    struct TrappedEnemy
    {
    };
    struct Enemy
    {
        int relocate_timer = 600; // 10 seconds * 60 FPS
    };
    struct Player
    {
        int open_mouth_timer = 0;
        int lives = 3;
        int invincible_timer = 0;
        int eating_timer = 0;
    };
    struct Collection
    {
        // preferred storage: tag
        // reason: The component only marks that an entity can be collected.
    };

    struct Damage
    {
        // preferred storage: sparse
        // reason: Only enemies or harmful entities use damage.
        int damage_value = 1;
    };

    struct LevelChanger
    {
        // preferred storage: sparse
        // reason: Only a few entities control level progression.
        bool can_move_to_next_level = false;
    };

    struct Direction
    {
        int dir = 1; // 1 = right, -1 = left
    };
    struct PhysicsBody
    {
        b2BodyId body = b2_nullBodyId;
        b2ShapeId shape = b2_nullShapeId;

    };

    // **** Sprites ****
    extern const SDL_FRect GREEN_PLAYER;
    extern const SDL_FRect BLUE_PLAYER;
    extern const SDL_FRect GREEN_PLAYER_OPEN;
    extern const SDL_FRect ENEMY_PURPLE;
    extern const SDL_FRect BUBBLE;
    extern const SDL_FRect APPLE;
    extern const SDL_FRect BANANA;
    extern const SDL_FRect BEER;
    extern const SDL_FRect PLATFORM;
    extern const SDL_FRect BOUNDS_WALL;
    extern const SDL_FRect LIFE_ICON;
    extern const SDL_FRect TRAPPED_ENEMY;
    extern const SDL_FRect BOUNDS_TILE;
    extern const SDL_FRect DIGITS[10];



    // **** Systems ****
    void movement_system();
    void physics_system(b2WorldId physics_world);
    void input_system(const bool* keyboard_state);
    void shooting_bubble_system(const bool* keyboard_state,b2WorldId physics_world);
    void capture_system(b2WorldId physics_world);
    void jump_system(b2WorldId physics_world);
    void collection_system(b2WorldId physics_world);
    void damage_system(b2WorldId physics_world);
    void score_system(SDL_Renderer* renderer, SDL_Texture* sprite_sheet);
    void render_system(SDL_Renderer* renderer, SDL_Texture* sprite_sheet);
    void bubble_cleanup_system();
    void player_visual_system();
    void destroy_game_entity(Entity e);
    void game_init(b2WorldId physics_world);
    void sensor_events_system(b2WorldId physics_world);
    void enemy_spawn_system(b2WorldId physics_world);
    void enemy_ai_system();


    void sound_system();
    void level_progression_system();

    // **** Entities ****
    ent_type  create_player(float x, float y, b2WorldId physics_world);
    ent_type  create_bubble(float x, float y, float velocity_x, float velocity_y, b2WorldId physics_world);
    ent_type  create_enemy(float x, float y, b2WorldId physics_world);
    ent_type  create_pressure_enemy(float x, float y);
    ent_type  create_trapped_enemy(float x, float y, b2WorldId physics_world);
    ent_type  create_fruit(float x, float y, int points,b2WorldId physics_world);
    ent_type  create_platform(float x,float y,float width,float height,b2WorldId physics_world);
    ent_type  create_bounds(float x, float y, float width, float height,b2WorldId physics_world);
    ent_type  create_map(float x, float y, b2WorldId physics_world);
    ent_type  create_score_display(float x, float y);
    bool is_game_over();
}

    // **** Storage Types ****

    // frequently iterated every frame
    template <>
    struct bagel::Storage<bubble_bobble::Position> final : NoInstance
    {
        using type = PackedStorage<bubble_bobble::Position>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::Drawing> final : NoInstance
    {
        using type = PackedStorage<bubble_bobble::Drawing>;
    };

    // physics and movement are accessed constantly
    template <>
    struct bagel::Storage<bubble_bobble::Movement> final : NoInstance
    {
        using type = PackedStorage<bubble_bobble::Movement>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::PhysicsBody> final : NoInstance
    {
        using type = PackedStorage<bubble_bobble::PhysicsBody>;
    };

    // only some entities have these
    template <>
    struct bagel::Storage<bubble_bobble::Sound> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::Sound>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::InputControl> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::InputControl>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::BubbleShooter> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::BubbleShooter>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::Damage> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::Damage>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::Direction> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::Direction>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::Player> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::Player>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::Score> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::Score>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::LevelChanger> final : NoInstance
    {
        using type = SparseStorage<bubble_bobble::LevelChanger>;
    };

    // tags only
    template <>
    struct bagel::Storage<bubble_bobble::Jump> final : NoInstance
    {
        using type = TaggedStorage<bubble_bobble::Jump>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::Collection> final : NoInstance
    {
        using type = TaggedStorage<bubble_bobble::Collection>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::Bubble> final : bagel::NoInstance
    {
        using type = bagel::SparseStorage<bubble_bobble::Bubble>;
    };
    template <>
    struct bagel::Storage<bubble_bobble::Enemy> final : bagel::NoInstance
    {
        using type = bagel::StackStorage<bubble_bobble::Enemy>;
    };

    template <>
    struct bagel::Storage<bubble_bobble::TrappedEnemy> final : NoInstance
    {
        using type = TaggedStorage<bubble_bobble::TrappedEnemy>;
    };