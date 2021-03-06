#include "SpaceShip.h"
#include "Renderer.h"
#include "InputHandler.h"
#include "Packet.h"
#include "Protocol.h"

SpaceShip::SpaceShip(const Renderer &renderer, const Vector2d& position)
: playerId_(PROTOCOL_INVALID_PLAYER_ID)
, sprite_("data/ship.png", renderer)
, thrustOn_(false)
, angle_(0)
, position_(position)
, velocity_(0, 0)
, lookat_(0, -1)
, acceleration_(0) {
}

SpaceShip::SpaceShip(const Renderer &renderer)
: SpaceShip(renderer, Vector2d(0, 0)) {
}

void SpaceShip::update(float elapsed) {
    angle_ *= 0.99f;
    lookat_.rotate(angle_ * elapsed);
    acceleration_ = thrustOn_ ? 50.0f : 0.0f;
    velocity_ *= 0.99f;
    if (velocity_.length() < 0.8f) {
        velocity_.reset();
    }
    velocity_ += (elapsed * (acceleration_ * lookat_));
    position_ += (elapsed * velocity_);
}

void SpaceShip::draw(Renderer &renderer) {
    renderer.setDrawColor(1, 1, 1, 1);
    renderer.drawLine(position_, position_ + (30 * lookat_));

    renderer.setDrawColor(1, 0, 0, 1);
    renderer.drawLine(position_, position_ + velocity_);

    sprite_.draw(
        static_cast<int>(position_.getX() - static_cast<float>(sprite_.getWidth()) / 2.0f),
        static_cast<int>(position_.getY() - static_cast<float>(sprite_.getHeight()) / 2.0f),
        -angle(Vector2d{0, -1}, lookat_),
        renderer);
}

void SpaceShip::write(Packet* packet) {
    packet->write(playerId_);
    position_.write(packet);
    velocity_.write(packet);
    lookat_.write(packet);
    packet->write(angle_);
    packet->write(thrustOn_);
}

void SpaceShip::read(Packet* packet) {
    packet->read(playerId_);
    position_.read(packet);
    velocity_.read(packet);
    lookat_.read(packet);
    packet->read(angle_);
    packet->read(thrustOn_);
}

uint32_t SpaceShip::getClassId() const {
    return ClassId;
}

void SpaceShip::setPlayerId(uint32_t playerId) {
    playerId_ = playerId;
}

uint32_t SpaceShip::getPlayerId() const {
    return playerId_;
}

void SpaceShip::rotate(float angle) {
    angle_ += angle;
}

void SpaceShip::thrust(bool onOff) {
    thrustOn_ = onOff;
}

const Vector2d& SpaceShip::getPosition() const {
    return position_;
}

unsigned int SpaceShip::getWidth() const {
    return sprite_.getWidth();
}

unsigned int SpaceShip::getHeight() const {
    return sprite_.getHeight();
}

const Vector2d& SpaceShip::getLookAt() const {
    return lookat_;
}

GameObjectPtr SpaceShip::createInstance(const Renderer& renderer) {
    return GameObjectPtr(new SpaceShip(renderer));
}
