#include "Sprite.h"
#include "Game.h"
#include "GameClient.h"

#include <iostream>

int main() {
    try {
        GameClient gameClient("127.0.0.1", 12345);

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            throw std::runtime_error(SDL_GetError());
        }

        Game game("Handle Latency", 640, 480, 30);

        Sprite sprite("data/ninja.png", game.getRenderer());

        game.run(sprite);
    
        SDL_Quit();
    
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
