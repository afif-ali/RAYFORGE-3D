#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <chrono>
#include <vector>
#include <glm/glm.hpp>

#include "screen_tex.h"
#include "compute.h"
#include "ray_tracing.h"

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


    
    unsigned int groupsX = (unsigned int)std::ceil(SCREEN_WIDTH/16.0);
    unsigned int groupsY = (unsigned int)std::ceil(SCREEN_HEIGHT/16.0);
    
    ScreenTexture screen_tex(SCREEN_WIDTH, SCREEN_HEIGHT);
    Compute compute(csSource, groupsX, groupsY);
    
    


    std::vector<Sphere> spheres = {
        { glm::vec3(-1.0,0.0,-1.0), 0.5},
        { glm::vec3(2.0,2.0,-5.0), 1.5},
        { glm::vec3(0.0,-100.5,1.0), 100.0 }
    };
    
    int count = static_cast<int>(spheres.size());
    std::vector<glm::vec3> positions;
    std::vector<float> radii;
    
    positions.reserve(count);
    radii.reserve(count);
    
    for (auto &s: spheres) {
        positions.push_back(s.pos);
        radii.push_back(s.radius);
    }
    
    compute.Use();
    glUniform1i(compute.GetUniformLocation("sphere_count"), count);
    glUniform3fv(compute.GetUniformLocation("sphere_pos"), count, &positions[0][0]);
    glUniform1fv(compute.GetUniformLocation("sphere_rad"), count, &radii[0]);




    auto last_time = std::chrono::steady_clock::now();
    int frame = 0;
    while (!glfwWindowShouldClose(window))
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - last_time;
        last_time = now;
        int fps = 1.0/delta.count();
        std::clog << "\rFPS " << fps << "       ";

        glClear(GL_COLOR_BUFFER_BIT);
        
        compute.Use();
        glUniform1i(compute.GetUniformLocation("frame"), frame);

        
        compute.Dispatch();
        screen_tex.Draw();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        frame++;
    }

    glfwTerminate();
    return 0;
}