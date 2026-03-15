#pragma once

#include <glad/glad.h>
#include <iostream>

class Compute {
public:
    Compute(const char* source, unsigned int X, unsigned int Y) {
        unsigned int compute_shader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute_shader, 1, &source, nullptr);
        glCompileShader(compute_shader);
        
        int _success;
        glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &_success);
        if (!_success) {
            char log[512];
            glGetShaderInfoLog(compute_shader, 512, NULL, log);
            std::cout << "COMPUTE SHADER COMPILE ERROR:\n" << log << std::endl;
        }
        
        ID = glCreateProgram();
        glAttachShader(ID, compute_shader);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &_success);
        if (!_success) {
            char log[512];
            glGetProgramInfoLog(ID, 512, NULL, log);
            std::cout << "COMPUTE SHADER LINK ERROR:\n" << log << std::endl;
        }

        groupsX = X;
        groupsY = Y;
    }

    void Use() {
        glUseProgram(ID);
    }

    unsigned int GetUniformLocation(const char* u) {
        return glGetUniformLocation(ID, u);
    }

    void Dispatch() {
        Use();
        glDispatchCompute(groupsX, groupsY, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
private:
    unsigned int ID, groupsX, groupsY;
};