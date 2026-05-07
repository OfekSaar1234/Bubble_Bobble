#include "Game.h"

int main() {
    BubbleBobbleGame game;
    if (game.valid()) {
        game.run();
    }
    return 0;
}