#pragma once
#include "../common.hpp"

class Geometry 
{
private:
    glm::mat4 _model = glm::mat4(1.0f);
    glm::mat4 _m2w = glm::mat4(1.0f);

public:
    glm::mat4 GetModelTransform() const { return _model; }

    void SetModelTransform(glm::mat4 model) { _model = model; }

    glm::mat4 GetModelWorldTransform() const { return _m2w; }

    void SetModelWorldTransform(glm::mat4 m2w) { _m2w = m2w; }

    glm::mat4 GetWorldMatrix() const { return _m2w * _model; }
};