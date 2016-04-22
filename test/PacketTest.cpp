#include "Packet.h"
#include "catch.hpp"

TEST_CASE("Packet has correct default values", "[Packet]") {
    Packet packet(8);
    REQUIRE(packet.getCapacity() == 8);
}

TEST_CASE("Packet can be read and written", "[Packet]") {
    Packet packet(8);

    const uint32_t value = 0x12345678;
    REQUIRE_NOTHROW(packet.write(reinterpret_cast<const uint8_t*>(&value) + 0, sizeof(uint8_t)));
    REQUIRE_NOTHROW(packet.write(reinterpret_cast<const uint8_t*>(&value) + 1, sizeof(uint8_t)));
    REQUIRE_NOTHROW(packet.write(reinterpret_cast<const uint8_t*>(&value) + 2, sizeof(uint8_t)));
    REQUIRE_NOTHROW(packet.write(reinterpret_cast<const uint8_t*>(&value) + 3, sizeof(uint8_t)));

    packet.reset();

    uint32_t output{};
    REQUIRE_NOTHROW(packet.read(&output, sizeof(uint32_t)));

    REQUIRE(output == value);
}

TEST_CASE("Exception is thrown when writing out of bounds", "[Packet]") {
    Packet packet(2);

    uint32_t value = 0;
    REQUIRE_THROWS_AS(packet.write(&value, sizeof(uint32_t)), std::out_of_range);
}

TEST_CASE("Size can be set", "[Packet]") {
    Packet packet(2);

    REQUIRE(packet.getSize() == 0);
    REQUIRE_NOTHROW(packet.setSize(1));
    REQUIRE(packet.getSize() == 1);
    REQUIRE_NOTHROW(packet.setSize(2));
    REQUIRE(packet.getSize() == 2);
    REQUIRE_THROWS_AS(packet.setSize(3), std::out_of_range);
}

TEST_CASE("BufferReader can be read", "[BufferReader]") {
    Packet packet(32);

    SECTION("Read one byte") {
        const uint8_t value = 0x12;
        packet.write(&value, sizeof(uint8_t));
        packet.reset();
        uint8_t t{};
        packet.read(t);
        REQUIRE(t == 0x12);
        REQUIRE(packet.getSize() == 1);
    }

    SECTION("Read two bytes") {
        const uint16_t value = 0x3412;
        packet.write(&value, sizeof(uint16_t));
        packet.reset();
        uint16_t t{};
        packet.read(t);
        REQUIRE(t == 0x1234);
        REQUIRE(packet.getSize() == 2);
    }

    SECTION("Read four bytes") {
        const uint32_t value = 0x78563412;
        packet.write(&value, sizeof(uint32_t));
        packet.reset();
        uint32_t t{};
        packet.read(t);
        REQUIRE(t == 0x12345678);
        REQUIRE(packet.getSize() == 4);
    }

    SECTION("Read eight bytes") {
        const uint64_t value = 0xF0DEBC9A78563412;
        packet.write(&value, sizeof(uint64_t));
        packet.reset();
        uint64_t t{};
        packet.read(t);
        REQUIRE(t == 0x123456789ABCDEF0);
        REQUIRE(packet.getSize() == 8);
    }
}

TEST_CASE("Packet has correct default values 2", "[Packet]") {
    Packet packet(32);
    REQUIRE(packet.getCapacity() == 32);
    REQUIRE(packet.getSize() == 0);
}

TEST_CASE("Packet can be cleared", "[Packet]") {
    Packet packet(32);

    const uint8_t value = 0x12;
    packet.write(value);

    REQUIRE(*packet.getData() == value);
    REQUIRE(packet.getSize() == 1);

    packet.clear();
    packet.reset();

    REQUIRE(*packet.getData() == 0x00);
    REQUIRE(packet.getSize() == 0);
}

TEST_CASE("Packet can be written", "[Packet]") {
    Packet packet(32);

    SECTION("Write single byte") {
        const uint8_t value = 0x12;
        packet.write(value);
        REQUIRE(packet.getSize() == 1);
        REQUIRE(*packet.getData() == 0x12);
    }

    SECTION("Write two bytes") {
        const uint16_t value = 0x1234;
        packet.write(value);
        REQUIRE(packet.getSize() == 2);
        REQUIRE(*(reinterpret_cast<const uint16_t*>(packet.getData())) == 0x3412);
    }

    SECTION("Write four bytes") {
        const uint32_t value = 0x12345678;
        packet.write(value);
        REQUIRE(packet.getSize() == 4);
        REQUIRE(*(reinterpret_cast<const uint32_t*>(packet.getData())) == 0x78563412);
    }

    SECTION("Write eight bytes") {
        const uint64_t value = 0x123456789ABCDEF0;
        packet.write(value);
        REQUIRE(packet.getSize() == 8);
        REQUIRE(*(reinterpret_cast<const uint64_t*>(packet.getData())) == 0xF0DEBC9A78563412);
    }
}
