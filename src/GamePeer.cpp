#include "GamePeer.h"
#include "Renderer.h"
#include "Protocol.h"
#include "Packet.h"
#include "Clock.h"
#include "Logging.h"

GamePeer::GamePeer(unsigned int frameRate, Renderer& renderer, unsigned short port)
: Game(frameRate, renderer)
, inputHandler_(30)
, latencyEstimator_(10)
, currentState(nullptr)
, nextState(nullptr)
, peerRegistry_()
, playerId_(1)
, objectId_(PROTOCOL_INVALID_OBJECT_ID)
, bufferedQueue_(1000)
, latencyEmulator_(bufferedQueue_, 150)
, transceiver_(port, latencyEmulator_)
, masterPeerEndpoint_() {
    currentState.reset(new GamePeer::Accepting{this});
}

GamePeer::GamePeer(unsigned int frameRate, Renderer& renderer, const char* address, unsigned short port)
: Game(frameRate, renderer)
, inputHandler_(30)
, latencyEstimator_(10)
, currentState(nullptr)
, nextState(nullptr)
, peerRegistry_()
, playerId_(PROTOCOL_INVALID_PLAYER_ID)
, objectId_(PROTOCOL_INVALID_OBJECT_ID)
, bufferedQueue_(1000)
, latencyEmulator_(bufferedQueue_, 150)
, transceiver_(latencyEmulator_)
, masterPeerEndpoint_(boost::asio::ip::address::from_string(address), port) {
    currentState.reset(new GamePeer::Connecting{this});
}

void GamePeer::handleWillUpdateWorld(const Clock& clock) {
    currentState->handleWillUpdateWorld(clock);
}

void GamePeer::handleDidUpdateWorld(const Clock& clock) {
    processIncomingPackets(clock);
    renderFrame();
    currentState->sendOutgoingPackets(clock);
}

void GamePeer::handleEvent(SDL_Event &event, bool& running) {
    switch(event.type) {
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
        } else {
            inputHandler_.handleInput(KeyAction::Down, event.key.keysym.sym);
        }
        break;
    case SDL_KEYUP:
        inputHandler_.handleInput(KeyAction::Up, event.key.keysym.sym);
        break;
    default:
        break;
    }
}

void GamePeer::finishFrame() {
    if (nextState) {
        currentState = nextState;
        nextState.reset();
    }
}

void GamePeer::setState(StatePtr& newState) {
    nextState = newState;
}

void GamePeer::processIncomingPackets(const Clock& clock) {
    auto packet = bufferedQueue_.dequeue();
    if (packet) {
        uint32_t magicNumber = 0;
        packet->read(magicNumber);
        if (magicNumber == PROTOCOL_MAGIC_NUMBER) {
            unsigned char protocolVersion = 0;
            packet->read(protocolVersion);
            if (protocolVersion == PROTOCOL_VERSION) {
                currentState->handleIncomingPacket(packet, clock);
            } else {
                WARN("Received a packet from server with invalid protocol version {0}.", protocolVersion);
            }
        } else {
            WARN("Received an invalid packet from {0}.", packet->getEndpoint());
        }
        bufferedQueue_.push(packet);
    }
}

void GamePeer::renderFrame() {
    renderer_.clear(0.25f, 0.25f, 0.25f);
    world_.draw(renderer_);
    renderer_.present();
}

GamePeer::State::State(GamePeer* gamePeer)
: gamePeer_(gamePeer) {
}

GamePeer::Accepting::Accepting(GamePeer* gamePeer)
: State(gamePeer)
, nextPlayerId_(gamePeer_->playerId_ + 1) {
}

void GamePeer::Accepting::handleIncomingPacket(Packet* packet, const Clock&) {
    unsigned char packetType = PROTOCOL_PACKET_TYPE_INVALID;
    packet->read(packetType);
    switch (packetType) {
    case PROTOCOL_PACKET_TYPE_HELLO:
        handleHello(packet);
        break;
    default:
        WARN("Received a packet with unexpected packet type {0} from {1}.", static_cast<unsigned int>(packetType), packet->getEndpoint());
        break;
    }
}

void GamePeer::Accepting::handleHello(Packet* packet) {
    if (!gamePeer_->peerRegistry_.isRegistered(packet->getEndpoint())) {
        INFO("HELLO received from new peer at {0}", packet->getEndpoint());
        auto invitePacket = gamePeer_->bufferedQueue_.pop();
        if (invitePacket) {
            const auto playerId = nextPlayerId_++;

            createInvitePacket(invitePacket, playerId, packet->getEndpoint(), gamePeer_->peerRegistry_);
            gamePeer_->transceiver_.sendTo(invitePacket);
            INFO("Sending INVITE to peer at {0}.", packet->getEndpoint());

            gamePeer_->peerRegistry_.forEachPeer([this, packet, playerId] (const boost::asio::ip::udp::endpoint& peerEndpoint) {
                auto introPacket = gamePeer_->bufferedQueue_.pop();
                if (introPacket) {
                    createIntroPacket(introPacket, playerId, packet->getEndpoint(), peerEndpoint);
                    gamePeer_->transceiver_.sendTo(introPacket);
                    INFO("Sending INTRO to peer at {0}.", peerEndpoint);
                } else {
                    WARN("Failed to send INTRO to peer at {0}: empty packet pool", peerEndpoint);
                }
            });

            gamePeer_->peerRegistry_.add(packet->getEndpoint());

            if (gamePeer_->peerRegistry_.getCount() == 3) {
                auto newState = StatePtr(new Waiting{gamePeer_});
                gamePeer_->setState(newState);
            }
        } else {
            WARN("Failed to send INVITE to new peer: empty packet pool.");
        }
    } else {
        WARN("HELLO from a registered peer {0}.", packet->getEndpoint());
    }
}

void GamePeer::Accepting::sendOutgoingPackets(const Clock&) {
}

GamePeer::Connecting::Connecting(GamePeer* gamePeer)
: State(gamePeer)
, lastHelloTime_(0) {
}

void GamePeer::Connecting::handleIncomingPacket(Packet* packet, const Clock&) {
    unsigned char packetType = PROTOCOL_PACKET_TYPE_INVALID;
    packet->read(packetType);
    switch (packetType) {
    case PROTOCOL_PACKET_TYPE_INVITE:
        handleInvite(packet);
        break;
    default:
        WARN("Received a packet with unexpected packet type {0} from {1}.", static_cast<unsigned int>(packetType), packet->getEndpoint());
        break;
    }
}

void GamePeer::Connecting::handleInvite(Packet* packet) {
    packet->read(gamePeer_->playerId_);

    DEBUG("INVITE received from master peer at {0}. My player ID is {1}.", packet->getEndpoint(), gamePeer_->playerId_);

    gamePeer_->peerRegistry_.add(packet->getEndpoint());

    int32_t peerCount = 0;
    packet->read(peerCount);
    for (int32_t i = 0; i < peerCount; ++i) {
        std::string address;
        packet->read(address);
        unsigned short port = 0;
        packet->read(port);
        const boost::asio::ip::udp::endpoint peerEndpoint(boost::asio::ip::address::from_string(address), port);
        gamePeer_->peerRegistry_.add(peerEndpoint);
        DEBUG("Adding peer at {0}:{1}", address, port);
    }

    auto newState = StatePtr(new Waiting{gamePeer_});
    gamePeer_->setState(newState);
}

void GamePeer::Connecting::sendOutgoingPackets(const Clock& clock) {
    sendHello(clock);
}

void GamePeer::Connecting::sendHello(const Clock& clock) {
    const auto now = clock.getTime();
    if (now > lastHelloTime_ + PROTOCOL_HELLO_INTERVAL) {
        auto packet = gamePeer_->bufferedQueue_.pop();
        if (packet) {
            createHelloPacket(packet, gamePeer_->masterPeerEndpoint_);
            INFO("Send HELLO to {0}.", packet->getEndpoint());
            gamePeer_->transceiver_.sendTo(packet);
        } else {
            WARN("Failed to send HELLO to server: empty packet pool.");
        }
        lastHelloTime_ = now;
    }
}

GamePeer::Waiting::Waiting(GamePeer* gamePeer)
: State(gamePeer) {
}

void GamePeer::Waiting::handleIncomingPacket(Packet* packet, const Clock&) {
    unsigned char packetType = PROTOCOL_PACKET_TYPE_INVALID;
    packet->read(packetType);
    switch (packetType) {
    case PROTOCOL_PACKET_TYPE_INTRO:
        handleIntro(packet);
        break;
    default:
        WARN("Received a packet with unexpected packet type {0} from {1}.", static_cast<unsigned int>(packetType), packet->getEndpoint());
        break;
    }
}

void GamePeer::Waiting::handleIntro(Packet* packet) {
    int32_t playerId = 0;
    packet->read(playerId);
    DEBUG("INTRO received from master peer at {0}. New peers player ID is {1}.", packet->getEndpoint(), playerId);
    std::string address;
    packet->read(address);
    unsigned short port = 0;
    packet->read(port);
    const boost::asio::ip::udp::endpoint peerEndpoint(boost::asio::ip::address::from_string(address), port);
    gamePeer_->peerRegistry_.add(peerEndpoint);
    DEBUG("Adding peer at {0}:{1}", address, port);
}

void GamePeer::Waiting::sendOutgoingPackets(const Clock&) {
}

GamePeer::Playing::Playing(GamePeer* gamePeer)
: State(gamePeer) {
}

void GamePeer::Playing::handleWillUpdateWorld(const Clock&) {
}

void GamePeer::Playing::handleIncomingPacket(Packet*, const Clock&) {
}

void GamePeer::Playing::sendOutgoingPackets(const Clock&) {
}
