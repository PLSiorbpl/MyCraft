#pragma once

class Block {
public:
    virtual ~Block() = default;

    bool is_solid;
    bool is_transparent;
    glm::ivec2 uv;
};



class Air : public Block {
public:
    Air() {
        is_solid = false;
        is_transparent = true;
        uv = glm::ivec2(0, 0);
    }

};

class Stone : public Block {
public:
    Stone() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(0, 0);
    }

};

class Grass : public Block {
public:
    Grass() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(1, 0);
    }

};

class Dirt : public Block {
public:
    Dirt() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(2, 0);
    }

};

class Iron : public Block {
public:
    Iron() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(3, 0);
    }

};

class Wool : public Block {
public:
    Wool() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(4, 0);
    }

};

class Water : public Block {
public:
    Water() {
        is_solid = true;
        is_transparent = false;
        uv = glm::ivec2(5, 0);
    }

};