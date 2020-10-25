#pragma once
#include "../common.hpp"
#include "geometry.hpp"

struct Instantiation 
{
    Instantiation(int);

    void Init(const std::shared_ptr<Geometry>&);

    void Init(const std::vector<std::shared_ptr<Geometry>>&);

    int materialIdx;
    
    std::shared_ptr<Geometry> prefab;
    
    std::vector<std::shared_ptr<Geometry>> instances;
};
