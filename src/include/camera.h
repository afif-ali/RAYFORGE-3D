#pragma once



#include "compute.h"
#include <glm/glm.hpp>


class Camera {
public:
    Camera(Compute* _comp, float _fov, glm::vec3 _pos, glm::vec3 _target, glm::vec3 _up) {
        comp = _comp;
        setFOV(_fov);
        setPos(_pos);
        setTarget(_target);
        setUp(_up);
    }

    void setFOV(float _fov) {
        fov = _fov;
        glUniform1f(comp->GetUniformLocation("fov"), fov);
    }

    void setPos(glm::vec3 _pos) {
        pos = _pos;
        glUniform3f(comp->GetUniformLocation("lookfrom"), pos.x, pos.y, pos.z);
    }

    void setTarget(glm::vec3 _target) {
        target = _target;
        glUniform3f(comp->GetUniformLocation("lookat"), target.x, target.y, target.z);
    }

    void setUp(glm::vec3 _up) {
        up = _up;
        glUniform3f(comp->GetUniformLocation("vup"), up.x, up.y, up.z);
    }

    float getFOV() {
        return fov;
    }

    glm::vec3 getPos()
    {
        return pos;
    }

    glm::vec3 getTarget()
    {
        return target;
    }

    glm::vec3 getUp()
    {
        return up;
    }

private:
    Compute* comp;
    float fov;
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;
};