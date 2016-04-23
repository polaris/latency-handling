#include "Protocol.h"
#include "Packet.h"
#include "MoveList.h"

static void createBasicPacket(Packet* packet, boost::asio::ip::udp::endpoint& endpoint, unsigned char packetType) {
    packet->clear();
    packet->setEndpoint(endpoint);
    packet->write(PROTOCOL_MAGIC_NUMBER);
    packet->write(PROTOCOL_VERSION);
    packet->write(packetType);
}

void createHelloPacket(Packet* packet, boost::asio::ip::udp::endpoint& endpoint) {
    createBasicPacket(packet, endpoint, PROTOCOL_PACKET_TYPE_HELLO);
}

void createWelcomePacket(Packet* packet, boost::asio::ip::udp::endpoint& endpoint) {
    createBasicPacket(packet, endpoint, PROTOCOL_PACKET_TYPE_WELCOME);
}

void createInputPacket(Packet* packet, boost::asio::ip::udp::endpoint& endpoint, const MoveList& moveList) {
    createBasicPacket(packet, endpoint, PROTOCOL_PACKET_TYPE_INPUT);
    moveList.write(packet);
}
