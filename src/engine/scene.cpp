#include "scene.hpp"

std::map<std::string, std::shared_ptr<Mesh>> Scene::MeshTable = std::map<std::string, std::shared_ptr<Mesh>>();

std::uint64_t Scene::CreateGhostGeometry()
{
    std::uint64_t id = _geometries.size() + 1;
    std::unique_ptr<Geometry> geo = std::make_unique<Geometry>();
    _geometries.insert({id, std::move(geo)});

    return id;
}

std::uint64_t Scene::CreateMeshGeometry(const std::string& entry, glm::mat4 modelTransform)
{
    if (MeshTable.count(entry) == 0)
        throw std::runtime_error("Cannot create geometry!");

    std::uint64_t id = _geometries.size() + 1;
    std::unique_ptr<Geometry> geo = std::make_unique<Geometry>();
    geo->SetModelTransform(modelTransform);
    _geometries.insert({id, std::move(geo)});
    MeshTable.find(entry)->second->AddInstance(id);

    return id;
}

bool Scene::GetGeometryModelTransform(std::uint64_t id, glm::mat4& transform) const
{
    if (_geometries.count(id) == 0)
        return false;
    
    transform = _geometries.find(id)->second->GetModelTransform();
    return true;
}

void Scene::SetGeometryModelTransform(std::uint64_t id, const glm::mat4& transform)
{
    if (_geometries.count(id) == 0)
        return;
    
    _geometries.find(id)->second->SetModelTransform(transform);
}

bool Scene::GetGeometryModelWorldTransform(std::uint64_t id, glm::mat4& transform) const
{
    if (_geometries.count(id) == 0)
        return false;
    
    transform = _geometries.find(id)->second->GetModelWorldTransform();
    return true;
}

void Scene::SetGeometryModelWorldTransform(std::uint64_t id, const glm::mat4& transform)
{
    if (_geometries.count(id) == 0)
        return;
    
    _geometries.find(id)->second->SetModelWorldTransform(transform);
}

void Scene::Render(Program& program)
{
    for (const auto& entry : MeshTable)
    {
        const auto& mesh = entry.second;
        const auto& instances = mesh->GetInstances();
        std::list<uint64_t> culled = instances;
        //TODO: Cull
        std::vector<glm::mat4> wms(culled.size());
        
        int k = 0;
        for (auto it = culled.cbegin(); it != culled.cend(); ++it, ++k)
        {
            if (_geometries.count(*it) == 0)
                continue;
            wms[k] = _geometries.find(*it)->second->GetWorldMatrix();
        }
        
        mesh->Render(program, wms);
    }
}
