#pragma once
#include <glad/glad.h>
#include <Utils/Globals.hpp>

class Bloom {
private:
    GLuint fullscreenVAO = 0;

    GLuint brightFBO = 0;
    GLuint brightTex = 0;

    GLuint blurFBO[2] = {};
    GLuint blurTex[2] = {};

    Sh extractShader = {};
    Sh blurShader = {};
    Sh combineShader = {};

    bool horizontal = true;

public:
    void Initialize(int width, int height);
    void Extract(GLuint sceneTex) const;
    void Blur();
    void Combine(GLuint sceneTex) const;
};