#pragma once
#include "Globals.hpp"
#include "Resources/MeshCollection.hpp"

class DefaultMeshCollection : public MeshCollection
{
public:
    DefaultMeshCollection()
    {
        MeshCollection();
        
        Model sphereModel = Model();
        sphereModel.AsSphere();
        Add("sphere", sphereModel);

        Model cubeModel = Model();
        cubeModel.AsCube();
        Add("cube", cubeModel);
    }
};