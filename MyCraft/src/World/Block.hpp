#pragma once
#include <algorithm>
#include "glm/glm.hpp"

#include "Models.hpp"
#include "Direction.hpp"

class Block {
public:
    virtual ~Block() = default;
    [[nodiscard]] virtual Block* clone() const = 0;
    [[nodiscard]] virtual bool needsState() const { return false; }

    Model *model;
    glm::ivec2 uv;
    bool is_solid;
    bool is_transparent;

    virtual void onPlace(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual uint8_t getPower(const glm::ivec3& pos, const glm::ivec2 &chunk, const Direction dir) { return 0; }
    virtual bool conductsPower() { return true; }
    virtual void onRemove(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onInteraction(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onInstantUpdate(const glm::ivec3& pos, const glm::ivec2 &chunk) {}
    virtual void onTickUpdate(const glm::ivec3& pos, const glm::ivec2 &chunk) {}

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

    bool conductsPower() override { return false; }

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

    uint8_t getPower(const glm::ivec3 &pos, const glm::ivec2 &chunk, const Direction dir) override { return 16; }
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

    bool conductsPower() override { return false; }

private:
};

class Lamp : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Lamp(*this);
    }
    [[nodiscard]] bool needsState() const override { return true; }

    Lamp() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(6, 0);
        model = &Models_cache["Lamp"];
    }

    void onPlace(const glm::ivec3 &pos, const glm::ivec2 &chunk) override { onInstantUpdate(pos, chunk); }
    void onInstantUpdate(const glm::ivec3& pos, const glm::ivec2 &chunk) override;

    std::string get_name() override { if (model) return model->name + (lit ? ": ON" : ": OFF"); else return "No Model!"; }

private:
    bool lit = false;
};

class Redstone_dust : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Redstone_dust(*this);
    }
    [[nodiscard]] bool needsState() const override { return true; }

    Redstone_dust() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(7, 0);
        model = &Models_cache["Redstone_dust"];
    }

    uint8_t getPower(const glm::ivec3 &pos, const glm::ivec2 &chunk, const Direction dir) override { return power; }
    bool conductsPower() override { return false; }
    void onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;

    std::string get_name() override { if (model) return model->name + " " + std::to_string(power); else return "No Model!"; }

    glm::vec3 get_overlay() override {
        float p = std::clamp(power / 15.0f, 0.0f, 1.0f);

        float r = p * 0.6f + (p > 0.0f ? 0.4f : 0.3f);
        float g = std::clamp(p * p * 0.7f - 0.5f * p, 0.0f, 1.0f);
        float b = std::clamp(p * p * 0.6f - 0.7f * p, 0.0f, 1.0f);

        return {r, g, b};
    }

private:
    uint8_t power = false;
};

class Repeater : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Repeater(*this);
    }
    [[nodiscard]] bool needsState() const override { return true; }

    Repeater() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(7, 1);
        model = &Models_cache["Repeater Off"];
    }

    bool conductsPower() override { return false; }
    uint8_t getPower(const glm::ivec3 &pos, const glm::ivec2 &chunk, const Direction dir) override { if (powered && Opposite(dir) == direction_) return 16; return 0; }
    void onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;
    void onTickUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;

    std::string get_name() override { if (model) return model->name + " delay:" + std::to_string(delay); else return "No Model!"; }

private:
    Direction direction_ = Direction::North;
    bool powered = false;
    uint8_t delay = 20;
    bool scheduled = false;
};

class Redstone_Torch : public Block {
public:
    [[nodiscard]] Block* clone() const override {
        return new Redstone_Torch(*this);
    }
    [[nodiscard]] bool needsState() const override { return true; }

    Redstone_Torch() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(7, 3);
        model = &Models_cache["Redstone Torch Off"];
    }

    bool conductsPower() override { return false; }
    uint8_t getPower(const glm::ivec3 &pos, const glm::ivec2 &chunk, const Direction dir) override { if (!powered && Opposite(dir) == direction_) return 16; return 0; }
    void onInstantUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;
    void onTickUpdate(const glm::ivec3 &pos, const glm::ivec2 &chunk) override;

private:
    Direction direction_ = Direction::North;
    bool powered = false;
    uint8_t delay = 20;
    bool scheduled = false;
};