#include "bubble_bobble.h"
using namespace bagel;

namespace bubble_bobble
{
    // **** Entities ****
    ent_type create_player(float x, float y)
    {
        Entity player = Entity::create();
        player.addAll(
            Position{x, y},
            Movement{0.0f, 0.0f},
            Sound{-1},
            Drawing{-1},
            InputControl{true},
            BubbleShooter{0},
            Score{0}
        );
        return player.entity();
    }

    ent_type create_bubble(float x, float y, float velocity_x, float velocity_y)
    {
        Entity bubble = Entity::create();
        bubble.addAll(
            Position{x, y},
            Movement{velocity_x, velocity_y},
            Drawing{-1},
            Sound{-1}
        );
        return bubble.entity();
    }

    ent_type create_enemy(float x, float y)
    {
        Entity enemy = Entity::create();
        enemy.addAll(
            Position{x, y},
            Movement{0.0f, 0.0f},
            Drawing{-1},
            Damage{1}
        );
        return enemy.entity();
    }

    ent_type create_pressure_enemy(float x, float y)
    {
        Entity pressure_enemy = Entity::create();
        pressure_enemy.addAll(
            Position{x, y},
            Movement{0.0f, 0.0f},
            Drawing{-1},
            Damage{1}
        );
        return pressure_enemy.entity();
    }

    ent_type create_trapped_enemy(float x, float y)
    {
        Entity trapped_enemy = Entity::create();
        trapped_enemy.addAll(
            Position{x, y},
            Movement{0.0f, 0.0f},
            Drawing{-1},
            Jump{}
        );
        return trapped_enemy.entity();
    }

    ent_type create_fruit(float x, float y, int points)
    {
        Entity fruit = Entity::create();
        fruit.addAll(
            Position{x, y},
            Drawing{-1},
            Collection{},
            Score{points},
            Sound{-1}
        );
        return fruit.entity();
    }

    ent_type create_bounds(float x, float y)
    {
        Entity bounds = Entity::create();
        bounds.addAll(
            Position{x, y},
            Drawing{-1}
        );
        return bounds.entity();
    }

    ent_type create_map(float x, float y)
    {
        Entity map = Entity::create();
        map.addAll(
            Position{x, y},
            Drawing{-1}
        );
        return map.entity();
    }

    ent_type create_score_display(float x, float y)
    {
        Entity score_display = Entity::create();
        score_display.addAll(
            Position{x, y},
            Drawing{-1},
            Score{0}
        );
        return score_display.entity();
    }
}