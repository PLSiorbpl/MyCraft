#pragma once
#include "glm/glm.hpp"

class Block {
public:
    virtual ~Block() = default;
    [[nodiscard]] virtual Block* clone() const = 0;

    bool is_solid;
    bool is_transparent;
    glm::ivec2 uv;

    virtual void onPlace(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onRemove(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onActivate(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onNeighborChanged(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
};



class Air : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Air(*this);
    }

    Air() {
        is_solid = false;
        is_transparent = true;
        uv = glm::ivec2(0, 0);
    }

private:
};

class Stone : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Stone(*this);
    }

    Stone() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(1, 0);
    }

private:
};

class Grass : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Grass();
    }

    Grass() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(2, 0);
    }

private:
};

class Dirt : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Dirt(*this);
    }

    Dirt() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(3, 0);
    }

private:
};

class Iron : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Iron(*this);
    }

    Iron() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(4, 0);
    }

    void onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;
    void onRemove(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;

private:
};

class Wool : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Wool(*this);
    }

    Wool() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(5, 0);
    }

private:
};

class Water : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Water(*this);
    }

    Water() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(6, 0);
    }

private:
};

class Lamp : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Lamp(*this);
    }

    Lamp() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(7, 0);
    }

    void onActivate(const glm::ivec3& pos, const glm::ivec2 &chunk) override {
        if (!lit) {
            uv = {0, 4};
            lit = true;
        } else {
            uv = {7, 0};
            lit = false;
        }
    }

private:
    bool lit = false;
};