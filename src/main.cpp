#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>
#include <glm/glm.hpp>

#include "screen_tex.h"
#include "compute.h"

const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 900;


struct Sphere {
    glm::vec3 pos;
    float radius;
};


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

    glfwSwapInterval(0);

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
    


    std::vector<Sphere> spheres = {
        { glm::vec3(-1.0,0.0,-1.0), 0.5},
        { glm::vec3(2.0,2.0,-5.0), 1.5},
        { glm::vec3(0.0,-100.5,1.0), 100.0 }
    };
    int count = static_cast<int>(spheres.size());

    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "sphere_count"), count);
    
    std::vector<glm::vec3> positions;
    std::vector<float> radii;
    positions.reserve(count);
    radii.reserve(count);

    for (auto &s: spheres) {
        positions.push_back(s.pos);
        radii.push_back(s.radius);
    }

    glUniform3fv(glGetUniformLocation(shader_program, "sphere_pos"), count, &positions[0][0]);
    glUniform1fv(glGetUniformLocation(shader_program, "sphere_rad"), count, &radii[0]);




    auto last_time = std::chrono::steady_clock::now();
    int frame = 0;
    while (!glfwWindowShouldClose(window))
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - last_time;
        last_time = now;
        int fps = 1.0/delta.count();
        std::string title = "FPS: " + std::to_string(fps);
        glfwSetWindowTitle(window, title.c_str());

        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shader_program);
        glUniform1i(glGetUniformLocation(shader_program, "frame"), frame);
        glDispatchCompute(groupsX, groupsY, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        screen_tex.usePROG();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        
        screen_tex.useVAO();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame++;
    }

    glfwTerminate();
    return 0;
}