#ifndef _GameClient_H
#define _GameClient_H

#include "Game.h"
#include "Queue.h"
#include "Packet.h"
#include "Transceiver.h"

#include <memory>

class Renderer;
class Clock;

class GameClient : public Game {
public:
    GameClient(unsigned int frameRate, const char *address, unsigned short port, Renderer& renderer);

private:
    void handleWillUpdateWorld(const Clock& clock) override;
    void handleDidUpdateWorld(const Clock& clock) override;
    void finishFrame() override;

    void processIncomingPackets(const Clock& clock);
    void renderFrame();

    class State {
    public:
        explicit State(GameClient* gameClient);
        virtual ~State() = default;
        virtual void handleIncomingPacket(Packet* packet) = 0;
        virtual void sendOutgoingPackets(const Clock& clock) = 0;

    protected:
        GameClient* gameClient_;
    };

    void setState(std::shared_ptr<State>& newState);

    class Connecting : public State {
    public:
        explicit Connecting(GameClient* gameClient);
        void handleIncomingPacket(Packet* packet) override;
        void sendOutgoingPackets(const Clock& clock) override;

    private:
        void sendHello(const Clock& clock);
        void handleWelcome(Packet* packet);

        float lastHelloTime_;
    };

    class Connected : public State {
    public:
        explicit Connected(GameClient* gameClient);
        void handleIncomingPacket(Packet* packet) override;
        void sendOutgoingPackets(const Clock& clock) override;

    private:
        void sendInput();

        float lastInputTime_;
    };

    std::shared_ptr<State> currentState;
    std::shared_ptr<State> nextState;

    Queue<Packet> packetPool_;
    Queue<Packet> incomingPackets_;
    Transceiver transceiver_;

    boost::asio::ip::udp::endpoint serverEndpoint_;

    unsigned int playerId_;
};

#endif  // _GameClient_H
