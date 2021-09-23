#pragma once
#include "Globals.hpp"
#include "Assets/ShapeCollection.hpp"

class DefaultShapeCollection : public ShapeCollection
{
public:
    DefaultShapeCollection()
    {        
        Add(std::string("unit_cube"), new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)));
        Add(std::string("unit_sphere"), new btSphereShape(0.5f));
        Add(std::string("FPS_controller"), new btCapsuleShape(0.5f, 3.0f));
    }
};