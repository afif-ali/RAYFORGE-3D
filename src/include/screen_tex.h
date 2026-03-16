#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


class ScreenTexture {
public:
    ScreenTexture(int width, int height) {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        resize(width, height);
    }

    unsigned int id() {
        return tex;
    }

    void resize(int width, int height) {
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    }

private:
    unsigned int tex;
};