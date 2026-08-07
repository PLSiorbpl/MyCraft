#include "Models.hpp"

std::map<std::string, Model_Set> Models_cache;

glm::ivec3 rotateY(glm::ivec3 p, const uint8_t times) {
    p -= glm::ivec3(8);

    switch (times & 3) {
        case 0: break;
        case 1: p = {  p.z, p.y, -p.x }; break;
        case 2: p = { -p.x, p.y, -p.z }; break;
        case 3: p = { -p.z, p.y,  p.x }; break;
        default: break;
    }

    p += glm::ivec3(8);
    return p;
}

void Bake(const Model& model, const uint8_t directions) {
    Model_Set model_set;
    model_set.rotations.emplace(Direction::North, model);

    if (directions & (1 << static_cast<uint8_t>(Direction::South))) {
        model_set.rotations.emplace(Direction::South, Bake_Model(model, Direction::South));
    }
    if (directions & (1 << static_cast<uint8_t>(Direction::East))) {
        model_set.rotations.emplace(Direction::East, Bake_Model(model, Direction::East));
    }
    if (directions & (1 << static_cast<uint8_t>(Direction::West))) {
        model_set.rotations.emplace(Direction::West, Bake_Model(model, Direction::West));
    }

    Models_cache.emplace(model.name, model_set);
}

Model Bake_Model(Model model, const Direction direction) {
    for (auto &elem : model.elements) {
        auto north = elem.north;
        auto east = elem.east;
        auto south = elem.south;
        auto west = elem.west;

        elem.from -= glm::ivec3(8);
        elem.to -= glm::ivec3(8);

        switch (direction) {
            case Direction::South: {
                elem.from = {elem.from.x, elem.from.y, -elem.from.z};
                elem.to = {elem.to.x, elem.to.y, -elem.to.z};

                std::swap(elem.north, elem.south);
                std::swap(elem.east,  elem.west);

                if (elem.up)
                    elem.up->uvRotation = 2;
                if (elem.down)
                    elem.down->uvRotation = 2;
                break;
            }
            case Direction::West: {
                elem.from = { elem.from.z, elem.from.y, -elem.from.x };
                elem.to = { elem.to.z, elem.to.y, -elem.to.x };

                elem.north = west;
                elem.east = north;
                elem.south = east;
                elem.west = south;

                if (elem.up)
                    elem.up->uvRotation = 1;
                if (elem.down)
                    elem.down->uvRotation = 1;
                break;
            }
            case Direction::East: {
                elem.from = { -elem.from.z, elem.from.y, elem.from.x };
                elem.to = { -elem.to.z, elem.to.y, elem.to.x };

                elem.north = east;
                elem.east = south;
                elem.south = west;
                elem.west = north;

                if (elem.up)
                    elem.up->uvRotation = 3;
                if (elem.down)
                    elem.down->uvRotation = 3;
                break;
            }
            default: break;
        }
        elem.from += glm::ivec3(8);
        elem.to += glm::ivec3(8);

        const glm::ivec3 newFrom = glm::min(elem.from, elem.to);
        const glm::ivec3 newTo = glm::max(elem.from, elem.to);

        elem.from = newFrom;
        elem.to = newTo;
    }

    return model;
}
