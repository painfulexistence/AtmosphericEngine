#include "scene.hpp"

Scene::Scene() 
{

}

Scene::~Scene() {}

void Scene::AddChild(Entity* entity)
{
    _entities.push_back(entity);
}

void Scene::Render()
{
    for (Entity* entity : _entities) {
    }
}