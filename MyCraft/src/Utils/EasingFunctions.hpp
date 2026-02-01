#pragma once

namespace ease {
    float linear(float t);

    float InOutCubic(float t);

    float OutCubic(float t);

    float SineOut(float t);

    float BackOut(float t);

    enum class easing {
        // Time related
        Linear,
        // Default
        EaseInOutCubic,
        // Opening
        EaseOutCubic,
        // Hover
        EaseOutSine,
        // Click
        EaseOutBack
    };

    float GetEase(float t, easing ease);
}
