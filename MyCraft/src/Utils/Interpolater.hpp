#pragma once
#include <chrono>
#include "EasingFunctions.hpp"

/*
    original video: when i find it
 */

template <typename T>
struct Interpolate {
    T start{};
    T end{};

    float start_time{};

    float speed = 1.0f;

    bool ended = true;

    ease::easing function = ease::easing::EaseInOutCubic;

    explicit Interpolate(const T &init_value = {}) : start{init_value}, end{start} {}

    static float CurrentTime() {
        const auto now = std::chrono::steady_clock::now();
        const auto duration = now.time_since_epoch();

        const auto seconds = std::chrono::duration_cast<std::chrono::duration<float>>(duration);
        return seconds.count();
    }

    [[nodiscard]] float elapsedSeconds() const {
        return CurrentTime() - start_time;
    }

    void setValue(const T &new_value) {
        ended = false;
        start = getValue();
        end = new_value;
        start_time = CurrentTime();
    }

    T getValue() {
        const float elapsed = elapsedSeconds();
        const float t = elapsed * speed;

        if (t >= 1.0f) {
            ended = true;
            return end;
        }

        const T delta = end - start;
        return start + delta * ease::GetEase(t, function);
    }

    void setDuration(const float duration) {
        speed = 1.0f / duration;
    }

    void setEasing(const ease::easing e) {
        function = e;
    }

    explicit operator T() const {
        return getValue();
    }

    void operator=(const T &new_value) {
        setValue(new_value);
    }

    void ForceSet(const T &start_, const T &end_) {
        ended = false;
        start = start_;
        end = end_;
        start_time = CurrentTime();
        if (start_ == end_)
            ended = true;
    }
};