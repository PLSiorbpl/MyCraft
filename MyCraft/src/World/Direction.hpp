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

inline Direction GetDirection(const glm::vec3& dir) {
    const float ax = std::abs(dir.x);
    const float ay = std::abs(dir.y);
    const float az = std::abs(dir.z);

    //if (ay > ax && ay > az)
    //    return dir.y > 0 ? Direction::Up : Direction::Down;
    if (ax > az)
        return dir.x > 0 ? Direction::East : Direction::West;
    return dir.z > 0 ? Direction::South : Direction::North;
}

inline std::string Direction_to_String(const Direction dir) {
    switch (dir) {
        case Direction::West: return "West";
        case Direction::East: return "East";
        case Direction::Down: return "Down";
        case Direction::Up: return "Up";
        case Direction::North: return "North";
        case Direction::South: return "South";
        default: return "All";
    }
}

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

constexpr Direction Right(Direction dir) {
    switch (dir) {
        case Direction::North: return Direction::East;
        case Direction::East:  return Direction::South;
        case Direction::South: return Direction::West;
        case Direction::West:  return Direction::North;
        default:               return dir; // Up, Down, All
    }
}

constexpr Direction Left(Direction dir) {
    switch (dir) {
        case Direction::North: return Direction::West;
        case Direction::West:  return Direction::South;
        case Direction::South: return Direction::East;
        case Direction::East:  return Direction::North;
        default:               return dir;
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