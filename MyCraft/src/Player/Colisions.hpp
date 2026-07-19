#pragma once
#include <glm/glm.hpp>

class colisions {
    public:
    static bool isSolidAround(glm::vec3 pos, float margin = 0.25f, float height = 1.8f);

    static bool isSolidAt(glm::vec3 pos);
};