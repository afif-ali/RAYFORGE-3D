#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


class ScreenTexture {
public:
    ScreenTexture(int width, int height) {
        initShaderProgram();
        initGeometry();
        initTex(width, height);

        glUniform1i(glGetUniformLocation(PROG, "screenTex"), 0);
    }

    void usePROG() {
        glUseProgram(PROG);
    }

    void useVAO() {
        glBindVertexArray(VAO);
    }

    void Draw() {
        usePROG();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        
        useVAO();
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

private:
    unsigned int VAO, VBO, PROG, tex;

    const char *vertexShaderSource = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aUV;
        out vec2 TexCoords;
        void main() {
            TexCoords = aUV;
            gl_Position = vec4(aPos, 1.0);
        }
    )";
    const char *fragmentShaderSource = R"(
        #version 460 core
        in vec2 TexCoords;
        out vec4 FragColor;
        uniform sampler2D screenTex;
        void main() {
            FragColor = texture(screenTex, TexCoords);
        }
    )";


    void initShaderProgram() {
        int  success;
        char infoLog[512];

        unsigned int vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        unsigned int fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if(!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        PROG = glCreateProgram();
        glAttachShader(PROG, vertexShader);
        glAttachShader(PROG, fragmentShader);
        glLinkProgram(PROG);
        glGetProgramiv(PROG, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(PROG, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void initGeometry() {
        float vertices[] = {
            -1.0, -1.0,  0.0,       0.0, 0.0,
            -1.0,  1.0,  0.0,       0.0, 1.0,
             1.0,  1.0,  0.0,       1.0, 1.0,

             1.0,  1.0,  0.0,       1.0, 1.0,
             1.0, -1.0,  0.0,       1.0, 0.0,
            -1.0, -1.0,  0.0,       0.0, 0.0,
        };

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void initTex(int width, int height) {
        glGenTextures(1, &tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
        glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    }
};