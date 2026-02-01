#include <cmath>
#include "EasingFunctions.hpp"

namespace ease {
    constexpr float PI = 3.14159265f;

    float linear(const float t) {
        return t;
    }

    float InOutCubic(float t) {
        if (t < 0.5f) {
            return 4.0f * t * t * t;
        }
        const float x = -2.0f * t + 2.0f;
        return 1.0f - (x * x * x) * 0.5f;
    }

    float OutCubic(const float t) {
        const float x = 1.0f - t;
        return 1.0f - x * x * x;
    }

    float SineOut(const float t) {
        return std::sin((t * PI) / 2);
    }

    inline float BackOut(const float t) {
        constexpr float c1 = 0.4f;
        constexpr float c3 = c1 + 1.0f;

        const float x = t - 1.0f;
        return 1.0f + c3 * x * x * x + c1 * x * x;
    }

    float GetEase(const float t, const easing ease) {
        switch (ease) {
            default:
                return linear(t);
            case easing::Linear:
                return linear(t);
            case easing::EaseInOutCubic:
                return InOutCubic(t);
            case easing::EaseOutCubic:
                return OutCubic(t);
            case easing::EaseOutSine:
                return SineOut(t);
            case easing::EaseOutBack:
                return BackOut(t);
        }
    }
}