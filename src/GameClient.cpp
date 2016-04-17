#include "GameClient.h"
#include "Renderer.h"
#include "Clock.h"

GameClient::GameClient(unsigned int frameRate, Renderer& renderer)
: Game(frameRate, renderer) {
}

void GameClient::handleWillUpdateWorld(const Clock& clock) {
    inputHandler_.update(clock.getGameTime());
}

void GameClient::handleDidUpdateWorld(const Clock&) {
    renderer_.clear(0.25f, 0.25f, 0.25f);
    world_.draw(renderer_);
    renderer_.present();
}
