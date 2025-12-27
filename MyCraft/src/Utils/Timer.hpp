#pragma once

#include <chrono>

class Timer {
public:
    Timer() { Reset(); }

    void Reset() {
        start = std::chrono::steady_clock::now();
    }

    double ElapsedMs() const {
        return std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start
        ).count();
    }

private:
    std::chrono::steady_clock::time_point start;
};
