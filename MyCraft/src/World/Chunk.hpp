#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <cstdint>

typedef unsigned int GLuint;
typedef int GLsizei;

class Chunk {
public:
    struct Block {
        uint8_t id;
        // Flags: Solid, Transparent, ...
        uint8_t Flags;

        explicit Block(const uint8_t id = 0, const uint8_t Flags = 0)
            : id(id), Flags(Flags) {}
    };

    struct Vertex {
        glm::vec3 position;
        uint8_t uv[2];
        uint8_t normal;
        uint8_t pad = 0;
    };

    // World Stuff
    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 256;
    static constexpr int DEPTH = 16;
    static constexpr int SIZE = WIDTH*HEIGHT*DEPTH;
    std::array<Block, SIZE> blocks;
    int chunkX, chunkZ;
    static const std::map<uint8_t, Block> BlockDefs;

    // Mesh Stuff
    bool has_terrain = false;
    bool is_edge = false;
    bool has_mesh = false;
    bool InRender = false;
    bool DirtyFlag = true;

    std::vector<Vertex> Mesh;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei indexCount = 0;

    explicit Chunk(const int X = 0, const int Y = 0) : chunkX(X), chunkZ(Y) {}

    static int index(const int x, const int y, const int z) {
        const int idx = x + z * WIDTH + y * WIDTH * DEPTH;

        return idx;
    }

    [[nodiscard]] const Block& get(const int x, const int y, const int z) const noexcept {
        return blocks[index(x, y, z)];
    }

    void set(const int x, const int y, const int z, const Block& block) {
        blocks.at(index(x, y, z)) = block;
    }

    void setID(const int x, const int y, const int z, const uint8_t id) {
        blocks.at(index(x, y, z)).id = id;
    }

    void SendData();

    void RemoveData();
};