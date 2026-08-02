#pragma once
#include <algorithm>

#include "glm/glm.hpp"

#include "Models.hpp"

class Block {
public:
    virtual ~Block() = default;
    [[nodiscard]] virtual Block* clone() const = 0;

    Model *model;
    glm::ivec2 uv;
    bool is_solid;
    bool is_transparent;

    virtual void onPlace(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual bool isPowered(const glm::ivec3& pos, const glm::ivec2 &chunk, const bool is_source = false) { return false; }
    virtual void onRemove(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onActivate(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onNeighborChanged(const glm::ivec3& pos, const glm::ivec2 &chunk) {}

    virtual std::string get_name() { if (model) return model->name; else return "No Model!"; }

    virtual glm::vec3 get_overlay() { return {1.0f, 1.0f, 1.0f}; }
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
        model = &Models_cache["Air"];
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
        model = &Models_cache["Stone"];
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
        model = &Models_cache["Grass"];
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
        uv = glm::ivec2(2, 2);
        model = &Models_cache["Dirt"];
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
        uv = glm::ivec2(3, 0);
        model = &Models_cache["Iron"];
    }

    void onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;
    void onRemove(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;
    bool isPowered(const glm::ivec3 &pos, const glm::ivec2 &chunk, const bool is_source) override { return powered; }

private:
    bool powered = true;
};

class Wool : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Wool(*this);
    }

    Wool() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(4, 0);
        model = &Models_cache["Wool"];
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
        uv = glm::ivec2(5, 0);
        model = &Models_cache["Water"];
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
        uv = glm::ivec2(6, 0);
        model = &Models_cache["Lamp"];
    }

    void onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) override { onNeighborChanged(pos, chunk); }
    void onNeighborChanged(const glm::ivec3& pos, const glm::ivec2 &chunk) override;

    std::string get_name() override { if (model) return model->name + (lit ? ": ON" : ": OFF"); else return "No Model!"; }

private:
    bool lit = false;
};

class Redstone_dust : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Redstone_dust(*this);
    }

    Redstone_dust() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(7, 0);
        model = &Models_cache["Redstone_dust"];
    }

    void onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;
    void onRemove(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;
    bool isPowered(const glm::ivec3 &pos, const glm::ivec2 &chunk, bool is_source) override;
    void onNeighborChanged(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;

    std::string get_name() override { if (model) return model->name + (powered ? ": ON" : ": OFF"); else return "No Model!"; }

    glm::vec3 get_overlay() override {
        float p = std::clamp((float)powered, 0.0f, 1.0f);

        float r = p * 0.6f + (p > 0.0f ? 0.4f : 0.3f);
        float g = std::clamp(p * p * 0.7f - 0.5f * p, 0.0f, 1.0f);
        float b = std::clamp(p * p * 0.6f - 0.7f * p, 0.0f, 1.0f);

        return {r, g, b};
    }

private:
    bool powered = false;
    bool updated = false;
};