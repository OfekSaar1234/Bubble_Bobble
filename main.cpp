#include "bubble_bobble.h"

int main()
{
    bubble_bobble::BubbleBobble game;

    if (game.valid()) {
        game.run();
    }

    return 0;
}