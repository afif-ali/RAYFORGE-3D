#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <chrono>

#include "screen_tex.h"
#include "compute.h"

const unsigned int SCREEN_WIDTH = 640;
const unsigned int SCREEN_HEIGHT = 480;

int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window;
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "RAYFORGE 3D", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    unsigned int tex;
    glGenTextures(1, &tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCREEN_WIDTH, SCREEN_HEIGHT);
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    unsigned int compute_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute_shader, 1, &csSource, nullptr);
    glCompileShader(compute_shader);
    
    int _success;
    glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &_success);
    if (!_success) {
        char log[512];
        glGetShaderInfoLog(compute_shader, 512, NULL, log);
        std::cout << "COMPUTE SHADER COMPILE ERROR:\n" << log << std::endl;
    }
    
    unsigned int shader_program = glCreateProgram();
    glAttachShader(shader_program, compute_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &_success);
    if (!_success) {
        char log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, log);
        std::cout << "COMPUTE SHADER LINK ERROR:\n" << log << std::endl;
    }
    
    unsigned int groupsX = (unsigned int)std::ceil(SCREEN_WIDTH/16.0);
    unsigned int groupsY = (unsigned int)std::ceil(SCREEN_HEIGHT/16.0);
    
    ScreenTexture screen_tex;
    screen_tex.usePROG();
    glUniform1i(screen_tex.getTexUniform(), 0);
    
    while (!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::steady_clock::now();

        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shader_program);
        glDispatchCompute(groupsX, groupsY, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        screen_tex.usePROG();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        
        screen_tex.useVAO();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glfwSwapBuffers(window);
        glfwPollEvents();

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> dt = end-start;
        std::cout << "\rDelta Time: " << dt.count();
    }

    glfwTerminate();
    return 0;
}