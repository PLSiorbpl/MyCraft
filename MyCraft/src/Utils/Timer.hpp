#pragma once
#include <chrono>

class Timer {
public:
    explicit Timer(const int max_samples = 15) : max_samples(max_samples) { Reset(); }

    void Reset() {
        start = std::chrono::steady_clock::now();
    }

    [[nodiscard]] double ElapsedMs() const {
        return DurationMs(std::chrono::steady_clock::now() - start);
    }

    double Stop() {
        last = ElapsedMs();

        total += last;
        samples++;

        if (samples >= max_samples) {
            average = total / samples;
            total = 0.0;
            samples = 0;
        }

        return total;
    }

    [[nodiscard]] double LastMs() const {
        return last;
    }

    [[nodiscard]] double Avg() const {
        return average;
    }

private:
    static double DurationMs(const std::chrono::steady_clock::duration duration) noexcept {
        return std::chrono::duration<double, std::milli>(duration).count();
    }

    std::chrono::steady_clock::time_point start;
    int max_samples;
    int samples = 0;
    double last = 0.0;
    double average = 0.0;
    double total = 0.0;
};
