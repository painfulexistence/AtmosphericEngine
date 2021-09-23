#pragma once
#include "Globals.hpp"
#include "Physics/PhysicsServer.hpp"
#include "Shader.hpp"
#include "mesh.hpp"
#include "geometry.hpp"

class Scene
{
private:
    std::map<uint64_t, std::unique_ptr<Geometry>> _geometries;

public:
    static std::map<std::string, std::shared_ptr<Mesh>> MeshTable;

    uint64_t CreateGhostGeometry();

    uint64_t CreateMeshGeometry(const std::string& entry, glm::mat4 modelTransform = glm::mat4(1.0f));

    bool GetGeometryModelTransform(uint64_t id, glm::mat4& transform) const;

    void SetGeometryModelTransform(uint64_t id, const glm::mat4& transform);

    bool GetGeometryModelWorldTransform(uint64_t id, glm::mat4& transform) const;

    void SetGeometryModelWorldTransform(uint64_t id, const glm::mat4& transform);

    glm::mat4 GetGeometryWorldMatrix(uint64_t id)
    {
        if (_geometries.count(id) == 0)
            throw std::runtime_error("Cannot find the geometry!");
        
        return _geometries.find(id)->second->GetWorldMatrix();
    }

    void Render(ShaderProgram& program, glm::mat4 projection, glm::mat4 view);
};