#pragma once

#include <array>
#include <atomic>

template<typename T, size_t N>
class TsQueue {
    public:
    std::array<T, N> buf;
    std::atomic<size_t> head = 0;
    std::atomic<size_t> tail = 0;

    bool push(const T& v) {
        size_t h = head.load();
        size_t next = (h + 1) % N;
        if (next == tail.load()) return false;
        buf[h] = v;
        head.store(next);
        return true;
    }

    bool pop(T& out) {
        size_t t = tail.load();
        if (t == head.load()) return false;
        out = buf[t];
        tail.store((t + 1) % N);
        return true;
    }
};