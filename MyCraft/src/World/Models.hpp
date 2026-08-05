#pragma once
#include <optional>
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <string>

enum class Face_dir : uint8_t {
    West = 1 << 0,
    East = 1 << 1,
    Down = 1 << 2,
    Up = 1 << 3,
    North = 1 << 4,
    South = 1 << 5
};

enum class Cull : uint8_t {
    No,
    Yes,
    Force
};

struct Face {
    glm::ivec2 texture;
    glm::vec4 uv;
    Cull cull = Cull::No;
};

struct Element {
    glm::ivec3 from;
    glm::ivec3 to;

    std::optional<Face> north;
    std::optional<Face> south;
    std::optional<Face> east;
    std::optional<Face> west;
    std::optional<Face> up;
    std::optional<Face> down;
};

struct Model {
    std::string name;
    std::vector<Element> elements;
    uint8_t occlusionMask = 0;
};

extern std::map<std::string, Model> Models_cache;