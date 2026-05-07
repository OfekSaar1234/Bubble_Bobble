#pragma once
#include "bagel.h"
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
        int sprite_id = -1;
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

    // **** Entities ****
    ent_type  create_player(float x, float y);
    ent_type  create_bubble(float x, float y, float velocity_x, float velocity_y);
    ent_type  create_enemy(float x, float y);
    ent_type  create_pressure_enemy(float x, float y);
    ent_type  create_trapped_enemy(float x, float y);
    ent_type  create_fruit(float x, float y, int points);
    ent_type  create_bounds(float x, float y);
    ent_type  create_map(float x, float y);
    ent_type  create_score_display(float x, float y);
}

// Storage Specializations

// Packed Storage (StackStorage)
template <> struct bagel::Storage<bubble_bobble::Position> final : bagel::NoInstance {
    using type = bagel::StackStorage<bubble_bobble::Position>;
};

template <> struct bagel::Storage<bubble_bobble::Drawing> final : bagel::NoInstance {
    using type = bagel::StackStorage<bubble_bobble::Drawing>;
};

// Tagged Storage (No memory footprint)
template <> struct bagel::Storage<bubble_bobble::Jump> final : bagel::NoInstance {
    using type = bagel::TaggedStorage<bubble_bobble::Jump>;
};

template <> struct bagel::Storage<bubble_bobble::Collection> final : bagel::NoInstance {
    using type = bagel::TaggedStorage<bubble_bobble::Collection>;
};