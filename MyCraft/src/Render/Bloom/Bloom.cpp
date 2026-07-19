#include "Bloom.hpp"
#include "Shader_Utils/Shader.hpp"

/*
 *
 *
 * I know Imated is looking at this comment :skull:
 *
 *
*/

void Bloom::Initialize(const int width, const int height) {
    extractShader.Shader = Shader::Create_Shader("MyCraft/shaders/FullScreen_vert.glsl", "MyCraft/shaders/Bloom/Extract_frag.glsl");
    blurShader.Shader = Shader::Create_Shader("MyCraft/shaders/FullScreen_vert.glsl", "MyCraft/shaders/Bloom/Blur_frag.glsl");
    combineShader.Shader = Shader::Create_Shader("MyCraft/shaders/FullScreen_vert.glsl", "MyCraft/shaders/Bloom/Combine_frag.glsl");

    glGenVertexArrays(1, &fullscreenVAO);

    // Bright Texture
    glGenTextures(1, &brightTex);
    glBindTexture(GL_TEXTURE_2D, brightTex);

    // Full resolution for stability
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                 width, height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Bright FBO
    glGenFramebuffers(1, &brightFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           brightTex,
                           0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Bright FBO ERROR\n";

    // Blur Textures
    glGenTextures(2, blurTex);

    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, blurTex[i]);

        // Quarter resolution for performance and "free blur"
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
                     width/video_settings.Blur_Scale, height/video_settings.Blur_Scale, 0,
                     GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Blur FBO
    glGenFramebuffers(2, blurFBO);

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               blurTex[i],
                               0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Blur FBO ERROR\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// FULL Resolution Pass
void Bloom::Extract(const GLuint sceneTex) const {
    glViewport(0, 0, game_settings.width, game_settings.height);
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(extractShader.Shader);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, sceneTex);

    Shader::Set_Int(extractShader.Shader, "scene", 4);
    Shader::Set_Float(extractShader.Shader, "Threshold", video_settings.Extract_Threshold);

    glBindVertexArray(fullscreenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Quarter Resolution Pass (Performance)
void Bloom::Blur() {
    glViewport(0, 0, game_settings.width/video_settings.Blur_Scale, game_settings.height/video_settings.Blur_Scale);
    horizontal = true;
    bool firstPass = true;

    glUseProgram(blurShader.Shader);

    for (int i = 0; i < video_settings.Blur_Passes; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);

        Shader::Set_Int(blurShader.Shader, "horizontal", horizontal);
        glActiveTexture(GL_TEXTURE4);
        Shader::Set_Int(blurShader.Shader, "image", 4);

        if (firstPass)
            glBindTexture(GL_TEXTURE_2D, brightTex);
        else
            glBindTexture(GL_TEXTURE_2D, blurTex[!horizontal]);

        glBindVertexArray(fullscreenVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        horizontal = !horizontal;
        if (firstPass)
            firstPass = false;
    }
}

// FUll Resolution Render to Screen
void Bloom::Combine(const GLuint sceneTex) const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, game_settings.width, game_settings.height);

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(combineShader.Shader);

    // Scene
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    Shader::Set_Int(combineShader.Shader, "scene", 4);

    // Bloom blur
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, blurTex[1]); // final ping-pong result
    Shader::Set_Int(combineShader.Shader, "bloom", 5);

    Shader::Set_Float(combineShader.Shader, "Exposure", video_settings.Exposure);

    glBindVertexArray(fullscreenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


