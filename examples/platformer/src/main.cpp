#include "game.hpp"
#include <iostream>

int main() {
    try {
        // Create and run the game
        Game game;
        game.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}