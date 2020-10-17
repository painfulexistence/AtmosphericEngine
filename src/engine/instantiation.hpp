#pragma once
#include "../common.hpp"
#include "geometry.hpp"

struct Instantiation 
{
    Instantiation(int);

    void Init(Geometry*&);

    void Init(std::vector<Geometry*>&);

    int materialIdx;
    
    Geometry* prefab;
    
    std::vector<Geometry*> instances;
};
