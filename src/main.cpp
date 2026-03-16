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
const unsigned int PANEL_HEIGHT = 200;


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
    
    int sphere_mat = scene.add_material({0, {0.5f,0.5f,0.5f}, 0.0f});
    int ground_mat = scene.add_material({0, {0.5f,0.5f,0.5f}, 0.0f});
    
    scene.add_sphere("sphere", Sphere{glm::vec3(0.0,0.0,0.2), 0.5, sphere_mat});
    scene.add_sphere("ground", Sphere{glm::vec3(0.0,-100.5,0.0), 100.0, ground_mat});
    
    Camera camera(&compute, 90.0, glm::vec3(0.0,0.0,-1.0), glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,1.0,0.0));



    bool showAddSpherePopup = false;
    char newSphereName[64] = "";
    std::string selectedID = "";
    bool lightMode = false;
    
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
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, fb_height-PANEL_HEIGHT));

        ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        auto sphereIDs = scene.getSphereIDs();
        for (const auto& id: sphereIDs) {
            if (ImGui::Selectable(id.c_str(), selectedID == id)) {
                selectedID = id;
            }
        }

        if (ImGui::Button("Add Sphere")) {
            showAddSpherePopup = true;
            newSphereName[0] = '\0';
        }
        if (showAddSpherePopup) {
            ImGui::OpenPopup("Add New Sphere");
            showAddSpherePopup = false;
        }
        if (ImGui::BeginPopupModal("Add New Sphere", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter a name for the new sphere:");
            ImGui::InputText("##sphereName", newSphereName, IM_ARRAYSIZE(newSphereName));

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                std::string name = newSphereName;
                if (!name.empty()) {
                    int mat = scene.add_material({0, {0.5f,0.5f,0.5f}, 0.0f});
                    scene.add_sphere(name, Sphere{{0,0,0}, 0.5, mat});
                    selectedID = name;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();




        ImGui::SetNextWindowPos(ImVec2(0,fb_height-PANEL_HEIGHT));
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, PANEL_HEIGHT));

        ImGui::Begin("General", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Text("FPS: %i", fps);
        if (ImGui::Checkbox("Light Mode", &lightMode)) {
            if (lightMode) {
                ImGui::StyleColorsLight();
            } else {
                ImGui::StyleColorsDark();
            }
        }

        ImGui::End();




        ImGui::SetNextWindowPos(ImVec2(fb_width-PANEL_WIDTH,0));
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, fb_height-PANEL_HEIGHT));

        ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (!selectedID.empty()) {
            ImGui::Text("Selected Sphere: %s", selectedID.c_str());
            ImGui::Separator();

            ImGui::Text("Geometry");
            glm::vec3 pos = scene.get_sphere_pos(selectedID);
            if (ImGui::DragFloat3("Position", &pos[0], 0.1f)) {
                scene.set_sphere_pos(selectedID, pos);
            }

            float radius = scene.get_sphere_radius(selectedID);
            if (ImGui::DragFloat("Radius", &radius, 0.05f, 0.0f, 100.0f)) {
                scene.set_sphere_radius(selectedID, radius);
            }

            int mat_id = scene.get_sphere_material(selectedID);
            if (mat_id >= 0) {
                ImGui::Separator();
                ImGui::Text("Material");

                int type = scene.get_material_type(mat_id);
                if (ImGui::Combo("Type", &type, "Lambertian\0Metal\0\0")) {
                    scene.set_material_type(mat_id, type);
                    compute.resetFrame();
                }

                glm::vec3 color = scene.get_material_albedo(mat_id);
                if (ImGui::ColorEdit3("Color", &color[0])) {
                    scene.set_material_albedo(mat_id, color);
                    compute.resetFrame();
                }

                float fuzz = scene.get_material_fuzz(mat_id);
                if (ImGui::DragFloat("Fuzz", &fuzz, 0.01f, 0.0f, 1.0f)) {
                    scene.set_material_fuzz(mat_id, fuzz);
                    compute.resetFrame();
                }
            }

            ImGui::Separator();
            if (ImGui::Button("Remove Sphere")) {
                scene.delete_sphere(selectedID);
                selectedID = "";
            }
        } else {
            ImGui::Text("No sphere selected!");
        }

        ImGui::End();



        ImGui::SetNextWindowPos(ImVec2(fb_width-PANEL_WIDTH,fb_height-PANEL_HEIGHT));
        ImGui::SetNextWindowSize(ImVec2(PANEL_WIDTH, PANEL_HEIGHT));

        ImGui::Begin("Camera Settings", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        float fov = camera.getFOV();
        if (ImGui::DragFloat("FOV", &fov, 1.0f, 30.0f, 120.0f)) {
            camera.setFOV(fov);
            compute.resetFrame();
        }

        glm::vec3 campos = camera.getPos();
        if (ImGui::DragFloat3("Camera Pos", &campos[0], 0.1f)) {
            camera.setPos(campos);
            compute.resetFrame();
        }

        glm::vec3 target = camera.getTarget();
        if (ImGui::DragFloat3("Camera Target", &target[0], 0.1f)) {
            camera.setTarget(target);
            compute.resetFrame();
        }

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