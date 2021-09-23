#pragma once
#include "Globals.hpp"
#include "Assets/MeshCollection.hpp"

class DefaultMeshCollection : public MeshCollection
{
public:
    DefaultMeshCollection()
    {
        MeshCollection();
        
        Mesh sphereMesh = Mesh();
        sphereMesh.AsSphere();
        Add("sphere", sphereMesh);

        Mesh cubeMesh = Mesh();
        cubeMesh.AsCube();
        Add("cube", cubeMesh);
    }
};