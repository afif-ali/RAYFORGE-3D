#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <chrono>

#include "screen_tex.h"
#include "compute.h"
#include "ray_tracing.h"
#include "scene.h"
#include "camera.h"

const unsigned int SCREEN_WIDTH = 1600;
const unsigned int SCREEN_HEIGHT = 900;

const unsigned int PANEL_WIDTH = 300;


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
    glfwSetWindowSizeLimits(window, 720, 360, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    glfwSwapInterval(0);




    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");



    
    unsigned int groupsX = (unsigned int)std::ceil(SCREEN_WIDTH/16.0);
    unsigned int groupsY = (unsigned int)std::ceil(SCREEN_HEIGHT/16.0);
    int render_w = SCREEN_WIDTH;
    int render_h = SCREEN_HEIGHT;
    
    ScreenTexture screen_tex(SCREEN_WIDTH, SCREEN_HEIGHT);
    Compute compute(csSource, groupsX, groupsY);
    

    Scene scene(&compute);
    
    int ground = scene.add_material({0, {0.8f,0.8f,0.0f}, 0.0f});
    int center = scene.add_material({0, {0.1f,0.2f,0.5f}, 0.0f});
    int right = scene.add_material({1, {0.8f,0.8f,0.8f}, 0.0f});
    int left = scene.add_material({1, {0.8f,0.6f,0.2f}, 0.2f});
    
    scene.add_sphere("ground", Sphere{ glm::vec3(0.0,-100.5,0.0), 100.0, ground});
    scene.add_sphere("center", Sphere{glm::vec3(0.0,0.0,0.2), 0.5, center});
    scene.add_sphere("right", Sphere{glm::vec3(-1.0,0.0,0.0), 0.5, right});
    scene.add_sphere("left", Sphere{glm::vec3(1.0,0.0,0.0), 0.5, left});
    
    Camera camera(&compute, 90.0, glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,1.0,0.0));

    
    auto last_time = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window))
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> delta = now - last_time;
        last_time = now;
        int fps = (int)(1.0/delta.count());


        int fb_width, fb_height;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);

        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        compute.sendFrame();
        compute.Dispatch();

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, fb_height));

        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("RAYFORGE 3D");
        ImGui::Separator();
        ImGui::Text("FPS: %d", fps);
        ImGui::Spacing();
        ImGui::Text("Scene tools will go here");

        ImGui::End();




        ImGui::SetNextWindowPos(ImVec2(fb_width-PANEL_WIDTH,0));
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, fb_height));

        ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);


        ImGui::Text("RAYFORGE 3D");
        ImGui::Separator();
        ImGui::Text("FPS: %d", fps);
        ImGui::Spacing();
        ImGui::Text("Scene tools will go here");

        ImGui::End();



        ImGui::SetNextWindowPos(ImVec2(PANEL_WIDTH, 0));
        ImGui::SetNextWindowSize(ImVec2(fb_width - 2*PANEL_WIDTH, fb_height));

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImVec2 size = ImGui::GetContentRegionAvail();
        int new_w = (int)size.x;
        int new_h = (int)size.y;
        if (new_w != render_w || new_h != render_h)
        {
            std::cout << "resize\n";
            render_w = new_w;
            render_h = new_h;
            screen_tex.resize(render_w, render_h);
            compute.resize( (unsigned int)std::ceil(render_w / 16.0),
                            (unsigned int)std::ceil(render_h / 16.0));
            compute.resetFrame();
        }

        ImGui::Image((ImTextureID)(intptr_t)screen_tex.id(), size);
        ImGui::End();



        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
        compute.moveFrame();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}