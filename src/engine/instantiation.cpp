#include "instantiation.hpp"

Instantiation::Instantiation(int idx)
{
    materialIdx = idx;
}

void Instantiation::Init(Geometry*& geometry)
{
    geometry->Init();
    prefab = geometry;
    instances.push_back(prefab);
}

void Instantiation::Init(std::vector<Geometry*>& geometries)
{
    if (geometries.size() <= 0)
        throw std::runtime_error("Failed to initialize an instantiation.");

    for (Geometry*& geometry : geometries)
    {
        geometry->Init();
        instances.push_back(geometry);
    }
    prefab = geometries[0];
}
