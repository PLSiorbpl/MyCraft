#pragma once
#include "Gui_Types.hpp"
#include "glm/glm.hpp"
#include "Utils/Globals.hpp"

namespace gui {
    size_t Parse_Color(const std::string& text, size_t i, const glm::vec4& base, glm::vec4& color);
    size_t Parse_Hex(const std::string& text, size_t i, glm::vec4& color);

    inline glm::vec4 rgba(const uint64_t color) {
        const float a = (color > 0xFFFFFFu) ? (color & 0xFF) / 255.0f : 1.0f;
        return {((color >> (color > 0xFFFFFFu ? 24 : 16)) & 0xFF) / 255.0f,
            ((color >> (color > 0xFFFFFFu ? 16 :  8)) & 0xFF) / 255.0f,
            ((color >> (color > 0xFFFFFFu ?  8 :  0)) & 0xFF) / 255.0f,
            a
        };
    }

    inline glm::vec4 Hex_Color(const uint32_t hex) {
        return {((hex >> 16) & 0xFF) / 255.0f, ((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f, 1.0f};
    }

    inline int Hex_Val(const char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    inline uint32_t PackRGBA(const glm::vec4& c) {
        return (static_cast<uint32_t>(c.r * 255.0f) << 0) | (static_cast<uint32_t>(c.g * 255.0f) << 8)
             | (static_cast<uint32_t>(c.b * 255.0f) << 16) | (static_cast<uint32_t>(c.a * 255.0f) << 24);
    }

    inline int wrap(const int value, const int max) {
        return (value % max + max) % max;
    }

    inline bool MouseInRect(const glm::vec2 &Pos, const glm::vec2 &Size) {
        return (game_settings.Mouse.x >= Pos.x && game_settings.Mouse.y >= Pos.y && game_settings.Mouse.x <= Pos.x+Size.x && game_settings.Mouse.y <= Pos.y+Size.y);
    }

    template <typename Func>
    void ForEach_Glyph(const Label& label, Func&& func) {
        const std::string& text = label.text;
        glm::vec4 color = label.Style.Color;
        uint32_t packed = PackRGBA(color);
        for (size_t i = 0; i < text.size(); ) {
            if (text[i] == '&' && i + 1 < text.size()) {
                if (text[i + 1] == '&') {
                    func(static_cast<unsigned char>('&'), packed);
                    i += 2;
                    continue;
                }
                glm::vec4 next{};
                if (const size_t consumed = Parse_Color(text, i, label.Style.Color, next)) {
                    color = next;
                    packed = PackRGBA(color);
                    i += consumed;
                    continue;
                }
            }
            if (text[i] == '#') {
                if (i + 1 < text.size() && text[i + 1] == '#') {
                    func(static_cast<unsigned char>('#'), packed);
                    i += 2;
                    continue;
                }
                glm::vec4 next{};
                if (const size_t consumed = Parse_Hex(text, i, next)) {
                    color = next;
                    packed = PackRGBA(color);
                    i += consumed;
                    continue;
                }
            }
            auto c = static_cast<unsigned char>(text[i]);
            if (c < ' ' || c > '~') c = '?';
            func(c, packed);
            i += 1;
        }
    }
}
