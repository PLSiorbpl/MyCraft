#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "World/Chunk.hpp"

class Mesh {
    public:
    static void GenerateMesh(Chunk& chunk);

    static void MeshZFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& world, const Element &elem, Block *block, int dir);
    static void MeshYFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& world, const Element &elem, Block *block, int dir);
    static void MeshXFace(std::vector<Chunk::Vertex> *vertices, const glm::vec3& world, const Element &elem, Block *block, int dir);

    static void getUVs(std::array<glm::vec2, 4>& outUV, const Face& face);
    static bool shouldRender(std::optional<Face> face, bool visible);
};