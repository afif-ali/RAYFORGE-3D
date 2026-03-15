# pragma once



#include <unordered_map>
#include <glm/glm.hpp>
#include <iostream>

#include "compute.h"



struct Sphere {
    glm::vec3 pos;
    float radius;
    int material_id;
};

struct Material {
    int type;
    glm::vec3 albedo;
    float fuzz;
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

    int add_material(Material m) {
        materials.push_back(m);
        return materials.size()-1;
    }
    
private:
    Compute* _comp;
    std::unordered_map<std::string, Sphere> spheres;
    std::vector<Material> materials;
    int count;
    
    void Update() {
        count = static_cast<int>(spheres.size());
        std::vector<glm::vec3> positions;
        std::vector<float> radii;
        std::vector<int> sphere_materials;

        positions.reserve(count);
        radii.reserve(count);
        for (auto& [id, s] : spheres) {
            positions.push_back(s.pos);
            radii.push_back(s.radius);
            sphere_materials.push_back(s.material_id);
        }
        
        std::vector<int> material_types;
        std::vector<glm::vec3> material_albedo;
        std::vector<float> material_fuzz;

        for (auto& m : materials) {
            material_types.push_back(m.type);
            material_albedo.push_back(m.albedo);
            material_fuzz.push_back(m.fuzz);
        }

        _comp->Use();
        glUniform1i(_comp->GetUniformLocation("sphere_count"), count);
        glUniform3fv(_comp->GetUniformLocation("sphere_pos"), count, &positions[0][0]);
        glUniform1fv(_comp->GetUniformLocation("sphere_rad"), count, &radii[0]);
        glUniform1iv(_comp->GetUniformLocation("sphere_mat"), count, sphere_materials.data());

        glUniform1i(_comp->GetUniformLocation("material_count"), materials.size());
        glUniform1iv(_comp->GetUniformLocation("material_type"), materials.size(), material_types.data());
        glUniform3fv(_comp->GetUniformLocation("material_albedo"), materials.size(), &material_albedo[0][0]);
        glUniform1fv(_comp->GetUniformLocation("material_fuzz"), materials.size(), material_fuzz.data());
    
        _comp->resetFrame();
    }
};