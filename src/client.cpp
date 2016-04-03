#include "Sprite.h"
#include "Game.h"

#include <boost/asio.hpp>

#include <thread>
#include <iostream>

class GameClient {
public:
    GameClient(const char *address, unsigned short port);
    ~GameClient();

    void send();

private:
    boost::asio::io_service io_service;
    boost::asio::io_service::work work;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint endpoint;
    std::thread thread;
    enum { MaxLength = 1024 };
    char data[MaxLength];
};

GameClient::GameClient(const char *address, unsigned short port)
: io_service()
, work(io_service)
, socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0))
, endpoint(boost::asio::ip::address::from_string(address), port)
, thread([this] () { io_service.run(); }) {
    send();
}

void GameClient::send() {
    socket.async_send_to(boost::asio::buffer(data, MaxLength), endpoint,
        [this] (const boost::system::error_code &ec, std::size_t bytesTransferred) {
            if (ec) {
                std::cerr << ec.message() << std::endl;
            } else {
                std::cout << bytesTransferred << " bytes transferred" << std::endl;
            }
        });
}

GameClient::~GameClient() {
    io_service.stop();
    thread.join();
}

int main() {
    try {
        GameClient gameClient("127.0.0.1", 12345);

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            throw std::runtime_error(SDL_GetError());
        }

        Game game("Handle Latency", 640, 480);

        Sprite sprite("data/ninja.png", game.getRenderer());

        game.run(sprite);
    
        SDL_Quit();
    
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
