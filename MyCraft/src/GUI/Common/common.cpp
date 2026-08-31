#include "common.hpp"

#include "Textures.hpp"

namespace gui {
    size_t Parse_Color(const std::string& text, const size_t i, const glm::vec4& base, glm::vec4& color) {
        const size_t n = text.size();
        if (i + 1 >= n) return 0;
        const char code = text[i + 1];

        if (code == 'r' || code == 'R') {
            color = base;
            return 2;
        }

        if (const int palette = Hex_Val(code); palette >= 0) {
            color = rgba(MC_Colors[palette]);
            return 2;
        }
        return 0;
    }

    size_t Parse_Hex(const std::string& text, const size_t i, glm::vec4& color) {
        const size_t n = text.size();
        size_t j = i + 1;
        uint32_t hex = 0;
        int digits = 0;
        while (j < n && digits < 6) { // #RRGGBB - exactly 6 hex digits
            const int v = Hex_Val(text[j]);
            if (v < 0) break;
            hex = (hex << 4) | static_cast<uint32_t>(v);
            digits++;
            j++;
        }
        if (digits != 6) return 0;
        color = Hex_Color(hex);
        return j - i;
    }
}
