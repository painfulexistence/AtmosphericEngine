#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

struct Vertex
{
public:
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    Vertex(const glm::vec3& pos = glm::vec3(0, 0, 0), const glm::vec2& uv = glm::vec2(0, 0), const glm::vec3& normal = glm::vec3(0, 0, 0), const glm::vec3& tangent = glm::vec3(0, 0, 0), const glm::vec3& bitangent = glm::vec3(0, 0, 0))
    {
        this->position = pos;
        this->uv = uv;
        this->normal = normal;
        this->tangent = tangent;
        this->bitangent = bitangent;
    }
};