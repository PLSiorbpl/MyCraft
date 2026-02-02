#include <iostream>
#include <glad/glad.h>
#include "Gui.hpp"
#include "Common/Gui_Types.hpp"
#include <Utils/InputManager.hpp>

using namespace gui;

glm::vec4 Gui::Texture(const Texture_Id tex, const glm::vec4 &UV, uint32_t &Flags) {
    constexpr float Pixel = 1.0f / 128.0f;
    constexpr float Tile  = 16 * Pixel;

    switch (tex) {
        case Texture_Id::Block: {
            int id = UV.x;
            const int Variant = UV.y;
            Flags32::Set(Flags, static_cast<int>(FlagBit::UseTexture));
            Flags32::SetTextureId(Flags, 0);
            id %= 6;
            return {id*Tile, Variant*Tile, (id+1)*Tile, (Variant+1)*Tile};
        }
        case Texture_Id::Item: {

        }
        case Texture_Id::Gui: {
            Flags32::Set(Flags, static_cast<int>(FlagBit::UseTexture));
            Flags32::SetTextureId(Flags, 1);
            return (UV / glm::vec4(512));
        }
        case Texture_Id::Font: {
            Flags32::Set(Flags, static_cast<int>(FlagBit::UseTexture));
            Flags32::SetTextureId(Flags, 2);
            return UV;
        }
        default: {
            Flags32::Set(Flags, static_cast<int>(FlagBit::UseTexture));
            Flags32::SetTextureId(Flags, 0);
            return glm::vec4(0);
        }
    }
}

void Gui::Text(const glm::vec2& Pos, const Label& label) {
    constexpr int Cols = 16;
    constexpr glm::vec2 AtlasS = glm::vec2(128);
    constexpr int glypW = 8;
    constexpr int glypH = 9;
    glm::vec2 Cursor = Pos + label.Offset;
    for (unsigned char c : label.text) {
        if (c < ' ' || c > '~') c = '?';
        const int idx = c - ' ';
        const int col = idx % Cols;
        const int row = idx / Cols;
        const auto px1 = glm::vec2(col * glypW, row * glypH);
        const auto px2 = glm::vec2(px1.x + glypW, px1.y + glypH);
        const auto uv = glm::vec4((px1/AtlasS), px2/AtlasS);
        const auto Size = glm::vec2((glypW * label.Style.Scale), (glypH * label.Style.Scale));
        DrawRectangle(
            {.Anchor = Anch::None, .Size = Size, .Offset = Cursor},
            {.BgColor = uv, .TextureId = Texture_Id::Font}
        );
        Cursor += glm::vec2(Size.x - (label.Style.PaddingX * label.Style.Scale) - (Advance[idx] * label.Style.Scale), 0);
    }
}

bool Gui::UpdateText(TextCache& cache, const int value, const char* fmt) {
    if (cache.i == value) return false;
    cache.i = value;

    char buf[128];
    std::snprintf(buf, 64, fmt, value);
    cache.text = buf;
    return true;
}

bool Gui::UpdateText(TextCache& cache, const float value, const char* fmt) {
    if (std::abs(cache.f - value) < 1e-5f) return false;
    cache.f = value;
    char buf[128];
    std::snprintf(buf, sizeof(buf), fmt, value);
    cache.text = buf;
    return true;
}

bool Gui::UpdateText(TextCache& cache, const double value, const char* fmt) {
    if (std::abs(cache.d - value) < 1e-9) return false;
    cache.d = value;
    char buf[128];
    std::snprintf(buf, sizeof(buf), fmt, value);
    cache.text = buf;
    return true;
}

std::string Gui::Format(const char* fmt, ...) {
    char buffer[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    return buffer;
}

glm::vec4 Gui::Color(const glm::vec4& color, uint32_t &Flags) {
    Flags32::Clear(Flags, static_cast<int>(FlagBit::UseTexture));
    return color;
}

void Gui::DrawRectangle(const Layout& layout, const BoxStyle& style) {
    uint32_t Flags = 0;
    const glm::vec2 Pos = Anchor(layout);
    const glm::vec2 Size = layout.Size;

    glm::vec4 UV;
    if (style.TextureId == Texture_Id::None) {
        // Solid Color
        UV = Color(style.BgColor, Flags);
        backend.PushToMesh({Pos, UV, Flags});
        backend.PushToMesh({Pos + glm::vec2(Size.x, 0.0f), UV, Flags});
        backend.PushToMesh({Pos + glm::vec2(0.0f, Size.y), UV, Flags});

        backend.PushToMesh({Pos + Size, UV, Flags});
        backend.PushToMesh({Pos + glm::vec2(Size.x, 0.0f), UV, Flags});
        backend.PushToMesh({Pos + glm::vec2(0.0f, Size.y), UV, Flags});
    } else {
        // Texture
        UV = Texture(style.TextureId, style.BgColor, Flags);
        backend.PushToMesh({Pos, {UV.x, UV.y, 0}, Flags});
        backend.PushToMesh({Pos + glm::vec2(Size.x, 0.0f), {UV.z, UV.y, 0}, Flags});
        backend.PushToMesh({Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.w, 0}, Flags});

        backend.PushToMesh({Pos + Size, {UV.z, UV.w, 0}, Flags});
        backend.PushToMesh({Pos + glm::vec2(Size.x, 0.0f), {UV.z, UV.y, 0}, Flags});
        backend.PushToMesh({Pos + glm::vec2(0.0f, Size.y), {UV.x, UV.w, 0}, Flags});
    }
}

void Gui::ProgressBar(const Layout& layout, const ProgressStyle& style, const Label* label) {
    const glm::vec2 Pos = Anchor(layout);
    const float Progress = glm::clamp(style.Progress, 0.0f, 1.0f);
    const float filledWidth = layout.Size.x * Progress;

    DrawRectangle(
        {.Anchor = layout.Anchor, .Size = {filledWidth, layout.Size.y}, .Offset = layout.Offset},
        {.BgColor = style.BgColor, .TextureId = style.TextureId}
    );
    if (label != nullptr) {
        const glm::vec2 TextPos = AnchorText(Pos, layout.Size, *label);
        Text(TextPos, *label);
    }
}

bool Gui::TextInput(const Layout &layout, const TextInputStyle &style, Label& label, Animation_State<glm::vec2>* state) {
    const int id = ID;
    ID += 1;
    bool clicked = false;
    bool hover;
    glm::vec2 Pos;

    if (state != nullptr) {
        Pos = Anchor({layout.Anchor, state->inter.getValue(), layout.Offset});
        hover = MouseInRect(Pos, state->inter.getValue());
    } else {
        Pos = Anchor(layout);
        hover = MouseInRect(Pos, layout.Size);
    }

    const glm::vec4 Col = hover ? style.style.HoverColor : style.style.BgColor;

    if (hover && InputManager::MouseState[GLFW_MOUSE_BUTTON_1] && ActiveId == -1) {
        // Select widget
        ActiveId = id;
    }

    if (!InputManager::MouseState[GLFW_MOUSE_BUTTON_1]) {
        if (hover && ActiveId == id) {
            // Clicked
            if (state != nullptr) {
                if (state->state != State::Click) {
                    state->state = State::Click;
                    state->inter.setEasing(ease::easing::EaseOutBack);
                    state->inter.setValue(layout.Size + glm::vec2(5));
                    state->inter.setDuration(0.2);
                } else {
                    state->state = State::Click;
                    state->inter.setEasing(ease::easing::EaseOutBack);
                    state->inter.setValue(layout.Size);
                    state->inter.setDuration(0.1);
                }
            } else {
                clicked = true;
            }
        }
    }

    if (!hover && InputManager::MouseState[GLFW_MOUSE_BUTTON_1] && ActiveId == id) {
        ActiveId = -1;
        InputManager::InputActive = false;
    }

    if (state != nullptr) {
        if (state->inter.ended) {
            if (state->state == State::Click) {
                InputManager::InputActive = true;
                clicked = true;
            } else if (hover) {
                state->state = State::Hover;
                state->inter.setEasing(ease::easing::EaseOutSine);
                state->inter.setValue(layout.Size + glm::vec2(2));
                state->inter.setDuration(0.1);
            } else {
                state->state = State::Idle;
                state->inter.setEasing(ease::easing::EaseOutSine);
                state->inter.setValue(layout.Size);
                state->inter.setDuration(0.1);
            }
        }
    }

    // Input Logic
    if (InputManager::InputActive && !InputManager::charBuffer.empty()) {
        if (style.max_chars > label.text.size()) {

            const char c = InputManager::charBuffer.front();

            if (style.Input_Mode == 0) {
                label.text += c;
            } else if (style.Input_Mode == 1 && std::isdigit(c)) {
                label.text += c;
            }
        }
        InputManager::charBuffer.pop_front();
    }

    if (!label.text.empty()) {
        if (InputManager::keysState[GLFW_KEY_BACKSPACE]) {
            label.text.pop_back();
            InputManager::keysState[GLFW_KEY_BACKSPACE] = false;
        }
    }

    if (state != nullptr) {
        DrawRectangle({layout.Anchor, state->inter.getValue(), layout.Offset},{.BgColor = Col, .TextureId = style.style.TextureId});
    } else {
        DrawRectangle(layout,{.BgColor = Col, .TextureId = style.style.TextureId});
    }

    if (!label.text.empty()) {
        if (InputManager::InputActive) {
            label.text.push_back(style.Cursor);
            glm::vec2 TextPos;
            if (state != nullptr) {
                TextPos = AnchorText(Pos, state->inter.getValue(), label);
            } else {
                TextPos = AnchorText(Pos, layout.Size, label);
            }
            Text(TextPos, label);
            label.text.pop_back();
        } else {
            glm::vec2 TextPos;
            if (state != nullptr) {
                TextPos = AnchorText(Pos, state->inter.getValue(), label);
            } else {
                TextPos = AnchorText(Pos, layout.Size, label);
            }
            Text(TextPos, label);
        }
    } else {
        glm::vec2 TextPos;
        if (state != nullptr) {
            TextPos = AnchorText(Pos, state->inter.getValue(), {style.Default_str, label.Style, label.Offset, label.anchor});
        } else {
            TextPos = AnchorText(Pos, layout.Size, {style.Default_str, label.Style, label.Offset, label.anchor});
        }
        Text(TextPos, {style.Default_str, label.Style, label.Offset, label.anchor});
    }
    return clicked;
}


glm::vec2 Gui::Anchor(const Layout& layout) const {
    const float width = game_settings.Scaled_w;
    const float height = game_settings.Scaled_h;
    const glm::vec2 Size = layout.Size;
    const glm::vec2 Offset = layout.Offset;
    switch (layout.Anchor) {
        case Anch::BottomLeft:   return glm::vec2(0, height - Size.y) + Offset;
        case Anch::BottomRight:  return glm::vec2(width - Size.x, height - Size.y) + Offset;
        case Anch::TopLeft:      return glm::vec2(0, 0) + Offset;
        case Anch::TopRight:     return glm::vec2(width - Size.x, 0) + Offset;
        case Anch::Center:       return glm::vec2((width - Size.x)/2, (height - Size.y)/2) + Offset;
        case Anch::BottomCenter: return glm::vec2((width-Size.x)/2, height-Size.y) + Offset;
        case Anch::TopCenter:    return glm::vec2((width-Size.x)/2, 0) + Offset;
        case Anch::LeftCenter:   return glm::vec2(0, (height - Size.y)/2) + Offset;
        case Anch::RightCenter:  return glm::vec2(width-Size.x, (height - Size.y)/2) + Offset;
        case Anch::None:         return Offset;
    }
    return glm::vec2(0);
}

glm::vec2 Gui::MeasureText(const Label& label) {
    constexpr int glypW = 8;
    constexpr int glypH = 9;
    glm::vec2 TextSize = {0, glypH * label.Style.Scale};
    for (unsigned char c : label.text) {
        if (c < ' ' || c > '~') c = '?';
        const int idx = c - ' ';
        const auto Size = glm::vec2(glypW * label.Style.Scale, glypH * label.Style.Scale);
        TextSize.x += Size.x - (label.Style.PaddingX * label.Style.Scale) - (Advance[idx] * label.Style.Scale);
    }
    return TextSize;
}

glm::vec2 Gui::AnchorText(const glm::vec2& Pos, const glm::vec2& Size, const Label& label) {
    const glm::vec2 TextSize = MeasureText(label);
    constexpr glm::vec2 TextOffset = {0.0f, -1.0f};
    const glm::vec2 Offset = Pos + label.Offset + TextOffset;
    switch (label.anchor) {
        case Anch::BottomLeft:   return glm::vec2(0, Size.y - TextSize.y) + Offset;
        case Anch::BottomRight:  return glm::vec2(Size.x - TextSize.x, Size.y - TextSize.y) + Offset;
        case Anch::TopLeft:      return glm::vec2(0, 0) + Offset;
        case Anch::TopRight:     return glm::vec2(Size.x - TextSize.x, 0) + Offset;
        case Anch::Center:       return glm::vec2((Size.x - TextSize.x)/2, (Size.y - TextSize.y)/2) + Offset;
        case Anch::BottomCenter: return glm::vec2((Size.x-TextSize.x)/2, Size.y-TextSize.y) + Offset;
        case Anch::TopCenter:    return glm::vec2((Size.x-TextSize.x)/2, 0) + Offset;
        case Anch::LeftCenter:   return glm::vec2(0, (Size.y - TextSize.y)/2) + Offset;
        case Anch::RightCenter:  return glm::vec2(Size.x-TextSize.x, (Size.y - TextSize.y)/2) + Offset;
        case Anch::None:         return Offset;
    }
    return glm::vec2(0);
}

bool Gui::Button(const Layout &layout, const ButtonStyle &style, const Label &label, Animation_State<glm::vec2> *state) {
    const int id = ID;
    ID += 1;
    bool clicked = false;
    bool hover;
    glm::vec2 Pos;

    if (state != nullptr) {
        Pos = Anchor({layout.Anchor, state->inter.getValue(), layout.Offset});
        hover = MouseInRect(Pos, state->inter.getValue());
    } else {
        Pos = Anchor(layout);
        hover = MouseInRect(Pos, layout.Size);
    }

    const glm::vec4 Col = hover ? style.HoverColor : style.BgColor;
    
    if (hover && InputManager::MouseState[GLFW_MOUSE_BUTTON_1] && ActiveId == -1) {
        ActiveId = id;
    }
    if (!InputManager::MouseState[GLFW_MOUSE_BUTTON_1]) {
        if (ActiveId == id && hover) {
            if (state != nullptr) {
                state->state = State::Click;
                state->inter.setEasing(ease::easing::EaseOutBack);
                state->inter.setValue(layout.Size + glm::vec2(5));
                state->inter.setDuration(0.1);
            } else {
                clicked = true;
            }
        }

        if (ActiveId == id)
            ActiveId = -1;
    }

    if (state != nullptr) {
        if (state->inter.ended) {
            if (state->state == State::Click) {
                clicked = true;
            } else if (hover) {
                state->state = State::Hover;
                state->inter.setEasing(ease::easing::EaseOutSine);
                state->inter.setValue(layout.Size + glm::vec2(2));
                state->inter.setDuration(0.1);
            } else {
                state->state = State::Idle;
                state->inter.setEasing(ease::easing::EaseOutSine);
                state->inter.setValue(layout.Size);
                state->inter.setDuration(0.1);
            }
        }
    }

    if (state != nullptr) {
        const glm::vec2 TextPos = AnchorText(Pos, state->inter.getValue(), label);
        DrawRectangle({layout.Anchor, state->inter.getValue(), layout.Offset},{.BgColor = Col, .TextureId = style.TextureId});
        Text(TextPos, label);
    } else {
        const glm::vec2 TextPos = AnchorText(Pos, layout.Size, label);
        DrawRectangle(layout,{.BgColor = Col, .TextureId = style.TextureId});
        Text(TextPos, label);
    }

    return clicked;
}

glm::vec4 Gui::Gradient(float x, const glm::vec4& a, const glm::vec4& b) {
    x = glm::clamp(x, 0.0f, 1.0f);
    return glm::mix(a, b, x);
}

glm::vec4 Gui::Gradient(float x, const glm::vec4& a, const glm::vec4& b, const glm::vec4& c) {
    x = glm::clamp(x, 0.0f, 1.0f);
    if (x < 0.5f)
        return glm::mix(a, b, x * 2.0f);
    return glm::mix(b, c, (x - 0.5f) * 2.0f);
}

std::vector<std::string> Gui::chat;

const int Gui::Advance[('~'-' ')+1] = {
    /* ' ' */ 1,
    /* '!' */ 4,
    /* '"' */ 2,
    /* '#' */ 0,
    /* '$' */ 0,
    /* '%' */ 0,
    /* '&' */ -1,
    /* ''' */ 3,
    /* '(' */ 2,
    /* ')' */ 2,
    /* '*' */ 2,
    /* '+' */ 2,
    /* ',' */ 3,
    /* '-' */ 2,
    /* '.' */ 4,
    /* '/' */ 1,
    /* '0' */ 0,
    /* '1' */ 0,
    /* '2' */ 0,
    /* '3' */ 0,
    /* '4' */ 0,
    /* '5' */ 0,
    /* '6' */ 0,
    /* '7' */ 0,
    /* '8' */ 0,
    /* '9' */ 0,
    /* ':' */ 4,
    /* ';' */ 4,
    /* '<' */ 2,
    /* '=' */ 2,
    /* '>' */ 2,
    /* '?' */ 0,
    /* '@' */ 0,
    /* 'A' */ 0,
    /* 'B' */ 0,
    /* 'C' */ 0,
    /* 'D' */ 0,
    /* 'E' */ 1,
    /* 'F' */ 1,
    /* 'G' */ 0,
    /* 'H' */ 0,
    /* 'I' */ 2,
    /* 'J' */ 1,
    /* 'K' */ 1,
    /* 'L' */ 1,
    /* 'M' */ 0,
    /* 'N' */ 0,
    /* 'O' */ -1,
    /* 'P' */ 0,
    /* 'Q' */ -1,
    /* 'R' */ 0,
    /* 'S' */ 0,
    /* 'T' */ 0,
    /* 'U' */ 0,
    /* 'V' */ 0,
    /* 'W' */ 0,
    /* 'X' */ 0,
    /* 'Y' */ 0,
    /* 'Z' */ 0,
    /* '[' */ 2,
    /* '\' */ 1,
    /* ']' */ 2,
    /* '^' */ 0,
    /* '_' */ 0,
    /* '`' */ 3,
    /* 'a' */ 0,
    /* 'b' */ 1,
    /* 'c' */ 2,
    /* 'd' */ 1,
    /* 'e' */ 1,
    /* 'f' */ 1,
    /* 'g' */ 1,
    /* 'h' */ 2,
    /* 'i' */ 4,
    /* 'j' */ 2,
    /* 'k' */ 1,
    /* 'l' */ 4,
    /* 'm' */ 0,
    /* 'n' */ 2,
    /* 'o' */ 1,
    /* 'p' */ 1,
    /* 'q' */ 1,
    /* 'r' */ 1,
    /* 's' */ 1,
    /* 't' */ 2,
    /* 'u' */ 1,
    /* 'v' */ 0,
    /* 'w' */ 0,
    /* 'x' */ 0,
    /* 'y' */ 1,
    /* 'z' */ 0,
    /* '{' */ 3,
    /* '|' */ 3,
    /* '}' */ 3,
    /* '~' */ -1
};
