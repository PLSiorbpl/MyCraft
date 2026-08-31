#pragma once
#include <glm/glm.hpp>

namespace gui {
    constexpr glm::vec4 Button_Big = {0, 0, 150, 20};
    constexpr glm::vec4 Button_Big_Active = {0, 21, 150, 41};
    constexpr glm::vec4 Button_Small = {151, 0, 221, 20};
    constexpr glm::vec4 Button_Small_Active = {151, 21, 221, 41};

    constexpr glm::vec4 Slot_Round = {222, 0, 244, 22};
    constexpr glm::vec4 Slot_Active = {222, 23, 244, 45};
    constexpr glm::vec4 Slot = {222, 46, 244, 68};

    constexpr glm::vec4 Slider_H = {245, 0, 252, 16};
    constexpr glm::vec4 Slider_H_Active = {245, 17, 252, 33};

    constexpr uint32_t MC_Colors[16] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
        0xAA0000, 0xAA00AA, 0xFFAA00, 0xAAAAAA,
        0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
        0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
    };
}
