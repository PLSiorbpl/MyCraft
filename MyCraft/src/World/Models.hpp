#pragma once
#include <optional>
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <string>

enum class Face_id : uint8_t {
    XN = 0,
    XP = 1,
    YN = 2,
    YP = 3,
    ZN = 4,
    ZP = 5
};

enum class Face_dir : uint8_t {
    XN = 1 << 0,
    XP = 1 << 1,
    YN = 1 << 2,
    YP = 1 << 3,
    ZN = 1 << 4,
    ZP = 1 << 5
};

enum class Cull : uint8_t {
    No,
    Yes,
    Force
};

struct Face {
    glm::ivec2 texture;
    glm::vec4 uv;
    Cull cull;
};

struct Element {
    glm::vec3 from;
    glm::vec3 to;

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