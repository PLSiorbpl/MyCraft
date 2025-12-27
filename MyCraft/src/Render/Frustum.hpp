#pragma once
#include <glm/glm.hpp>

class Frustum {
    public:
    struct Plane {
        glm::vec3 normal;
        float d;
    };
    
    struct Frust {
        Plane planes[6];
    };

    static Frust ExtractFrustum(const glm::mat4& clip);

    static bool IsAABBVisible(const Frust& f, const glm::vec3& min, const glm::vec3& max);
};