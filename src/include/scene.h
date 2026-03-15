# pragma once



#include <unordered_map>
#include <glm/glm.hpp>
#include <iostream>

#include "compute.h"



struct Sphere {
    glm::vec3 pos;
    float radius;
};

class Scene {
public:
    Scene(Compute* comp) {
        _comp = comp;
    }

    void add_sphere(std::string ID, Sphere s) {
        spheres[ID] = s;
        count++;
        Update();
    }

    void delete_sphere(std::string ID) {
        spheres.erase(ID);
        count--;
        Update();
    }

    void set_sphere_pos(std::string ID, glm::vec3 pos) {
        spheres[ID].pos = pos;
        Update();
    }

    void set_sphere_radius(std::string ID, float radius) {
        spheres[ID].radius = radius;
        Update();
    }

    glm::vec3 get_sphere_pos(std::string ID) {
        return spheres[ID].pos;
    }

    float get_sphere_radius(std::string ID) {
        return spheres[ID].radius;
    }

    // MISSING: get pos, get/set rad

    
    private:
    Compute* _comp;
    std::unordered_map<std::string, Sphere> spheres;
    int count;
    std::vector<glm::vec3> positions;
    std::vector<float> radii;

    void Update() {
        count = static_cast<int>(spheres.size());

        positions.clear();
        radii.clear();
        positions.reserve(count);
        radii.reserve(count);
        for (auto& [id, s] : spheres) {
            positions.push_back(s.pos);
            radii.push_back(s.radius);
        }

        _comp->Use();
        glUniform1i(_comp->GetUniformLocation("sphere_count"), count);
        glUniform3fv(_comp->GetUniformLocation("sphere_pos"), count, &positions[0][0]);
        glUniform1fv(_comp->GetUniformLocation("sphere_rad"), count, &radii[0]);
    
        _comp->resetFrame();
    }
};