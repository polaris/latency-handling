#ifndef _GamePeer_H
#define _GamePeer_H

#include "Game.h"
#include "InputHandler.h"
#include "LatencyEstimator.h"
#include "BufferedQueue.h"
#include "LatencyEmulator.h"
#include "Transceiver.h"

#include <memory>

class GamePeer : public Game {
public:
    GamePeer(unsigned int frameRate, Renderer& renderer, unsigned short port);

    GamePeer(unsigned int frameRate, Renderer& renderer, const char* masterAddress, unsigned short port);

private:
    void handleWillUpdateWorld(const Clock& clock) override;
    void handleDidUpdateWorld(const Clock& clock) override;
    void handleEvent(SDL_Event &event, bool& running) override;
    void finishFrame() override;

    class State {
    public:
        explicit State(GamePeer* gamePeer);
        virtual ~State() = default;
        State(const GamePeer::State&) = delete;
        GamePeer::State& operator =(const GamePeer::State&) = delete;
        virtual void handleWillUpdateWorld(const Clock&) {}
        virtual void handleIncomingPacket(Packet* packet, const Clock& clock) = 0;
        virtual void sendOutgoingPackets(const Clock& clock) = 0;

    protected:
        GamePeer* gamePeer_;
    };

    using StatePtr = std::shared_ptr<State>;

    void setState(StatePtr& newState);

    class Accepting : public State {
    public:
        explicit Accepting(GamePeer* gamePeer);
        void handleWillUpdateWorld(const Clock&) override;
        void handleIncomingPacket(Packet* packet, const Clock& clock) override;
        void sendOutgoingPackets(const Clock& clock) override;
    };

    class Connecting : public State {
    public:
        explicit Connecting(GamePeer* gamePeer);
        void handleWillUpdateWorld(const Clock&) override;
        void handleIncomingPacket(Packet* packet, const Clock& clock) override;
        void sendOutgoingPackets(const Clock& clock) override;
    };

    class Waiting : public State {
    public:
        explicit Waiting(GamePeer* gamePeer);
        void handleWillUpdateWorld(const Clock&) override;
        void handleIncomingPacket(Packet* packet, const Clock& clock) override;
        void sendOutgoingPackets(const Clock& clock) override;
    };

    class Playing : public State {
    public:
        explicit Playing(GamePeer* gamePeer);
        void handleWillUpdateWorld(const Clock&) override;
        void handleIncomingPacket(Packet* packet, const Clock& clock) override;
        void sendOutgoingPackets(const Clock& clock) override;
    };

    InputHandler inputHandler_;
    LatencyEstimator latencyEstimator_;

    StatePtr currentState;
    StatePtr nextState;

    BufferedQueue bufferedQueue_;
    LatencyEmulator latencyEmulator_;
    Transceiver transceiver_;
};

#endif	// _GamePeer_H
