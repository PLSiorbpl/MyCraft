#include "Frustum.hpp"

Frustum::Frust Frustum::ExtractFrustum(const glm::mat4& clip) {
    Frust f;

    // Left
    f.planes[0].normal.x = clip[0][3] + clip[0][0];
    f.planes[0].normal.y = clip[1][3] + clip[1][0];
    f.planes[0].normal.z = clip[2][3] + clip[2][0];
    f.planes[0].d = clip[3][3] + clip[3][0];

    // Right
    f.planes[1].normal.x = clip[0][3] - clip[0][0];
    f.planes[1].normal.y = clip[1][3] - clip[1][0];
    f.planes[1].normal.z = clip[2][3] - clip[2][0];
    f.planes[1].d = clip[3][3] - clip[3][0];

    // Bottom
    f.planes[2].normal.x = clip[0][3] + clip[0][1];
    f.planes[2].normal.y = clip[1][3] + clip[1][1];
    f.planes[2].normal.z = clip[2][3] + clip[2][1];
    f.planes[2].d = clip[3][3] + clip[3][1];

    // Top
    f.planes[3].normal.x = clip[0][3] - clip[0][1];
    f.planes[3].normal.y = clip[1][3] - clip[1][1];
    f.planes[3].normal.z = clip[2][3] - clip[2][1];
    f.planes[3].d = clip[3][3] - clip[3][1];

    // Near
    f.planes[4].normal.x = clip[0][3] + clip[0][2];
    f.planes[4].normal.y = clip[1][3] + clip[1][2];
    f.planes[4].normal.z = clip[2][3] + clip[2][2];
    f.planes[4].d = clip[3][3] + clip[3][2];

    // Far
    f.planes[5].normal.x = clip[0][3] - clip[0][2];
    f.planes[5].normal.y = clip[1][3] - clip[1][2];
    f.planes[5].normal.z = clip[2][3] - clip[2][2];
    f.planes[5].d = clip[3][3] - clip[3][2];

    // Normalize all
    for (int i = 0; i < 6; i++) {
        float len = glm::length(f.planes[i].normal);
        f.planes[i].normal /= len;
        f.planes[i].d /= len;
    }

    return f;
}

bool Frustum::IsAABBVisible(const Frust& f, const glm::vec3& min, const glm::vec3& max) {
    for (int i = 0; i < 6; i++) {
        const auto& p = f.planes[i];

        glm::vec3 v = min;

        if (p.normal.x >= 0) v.x = max.x;
        if (p.normal.y >= 0) v.y = max.y;
        if (p.normal.z >= 0) v.z = max.z;

        if (glm::dot(p.normal, v) + p.d < 0)
            return false;
    }
    return true;
}