#pragma once

enum class Direction : uint8_t {
    West = 0, // -X
    East = 1, // +X
    Down = 2, // -Y
    Up = 3, // +Y
    North = 4, // -Z
    South = 5,  // +Z
    All,
};

inline Direction Opposite(const Direction dir) {
    switch (dir) {
        case Direction::West: return Direction::East;
        case Direction::East: return Direction::West;
        case Direction::Down: return Direction::Up;
        case Direction::Up: return Direction::Down;
        case Direction::North: return Direction::South;
        case Direction::South: return Direction::North;
        default: return Direction::All;
    }
}

inline bool IsHorizontal(const Direction dir) {
    return dir == Direction::West ||
           dir == Direction::East ||
           dir == Direction::North ||
           dir == Direction::South;
}

inline bool IsVertical(const Direction dir) {
    return dir == Direction::Up ||
           dir == Direction::Down;
}