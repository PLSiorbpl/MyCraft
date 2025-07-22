#pragma once
#include <cstdint>

struct Block {
    int8_t id;
    bool transparent;
    bool solid;
    uint8_t light;

    Block(uint8_t id = 0, bool transparent = false, bool solid = false, uint8_t light = 0);
};
