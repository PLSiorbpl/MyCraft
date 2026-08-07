#pragma once
#include <optional>
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <string>

#include "Direction.hpp"

enum class Face_mask : uint8_t {
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
    glm::ivec2 texture = {7, 7};
    glm::vec4 uv = {0, 0, 16, 16};
    Cull cull = Cull::No;
};

struct Element {
    glm::ivec3 from = {0, 0, 0};
    glm::ivec3 to = {16, 16, 16};

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

struct Model_Set {
    std::map<Direction, Model> rotations;

    [[nodiscard]] Model *get(const Direction dir) {
        const auto it = rotations.find(dir);
        if (it != rotations.end())
            return &it->second;
        else {
            std::printf("No model variant: %s\n", Direction_to_String(dir).c_str());
            if (rotations.empty())
                std::printf("\tUnknown model!\n");
            std::printf("\tModel: %s\n", rotations.at(Direction::North).name.c_str());
        }

        if (rotations.empty())
            std::printf("No model variant: %s", Direction_to_String(dir).c_str());

        return &rotations.at(Direction::North);
    }
};

glm::ivec3 rotateY(glm::ivec3 p, uint8_t times);

void Bake(const Model& model, uint8_t directions);
Model Bake_Model(Model model, Direction direction);

extern std::map<std::string, Model_Set> Models_cache;