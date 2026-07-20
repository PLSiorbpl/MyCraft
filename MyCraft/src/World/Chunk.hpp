#pragma once
#include <array>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <cstdint>

#include "Block.hpp"

typedef unsigned int GLuint;
typedef int GLsizei;

enum class block_type : uint8_t {
    Air,
    Stone,
    Grass ,
    Dirt,
    Iron,
    Wool,
    Water,

    _count
};

extern std::array<std::unique_ptr<Block>, static_cast<int>(block_type::_count)> block_cache;
void init_block_state();

class Chunk {
public:
    struct block {
        block_type id = block_type::Air;
        uint16_t state = 0; // 0 - global  1-...

        explicit block(const block_type id = block_type::Air)
            : id(id) {}
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
    std::array<block, SIZE> blocks;
    int chunkX, chunkZ;

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

    [[nodiscard]] const block& get(const int x, const int y, const int z) const noexcept {
        return blocks[index(x, y, z)];
    }

    [[nodiscard]] Block *get_state(const int x, const int y, const int z) const noexcept {
        const auto &b = blocks[index(x, y, z)];
        if (b.state == 0) {
            return block_cache[static_cast<size_t>(b.id)].get();
        }

        return block_cache[static_cast<size_t>(b.id)].get();
    }

    void set(const int x, const int y, const int z, const block& block) {
        blocks.at(index(x, y, z)) = block;
    }

    void SendData();

    void RemoveData();
};