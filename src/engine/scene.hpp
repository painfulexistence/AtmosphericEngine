#pragma once
#include "../common.hpp"
#include "geometry.hpp"
#include "instantiation.hpp"
#include "material.hpp"
#include "program.hpp"
#include "camera.hpp"
#include "light.hpp"


class Scene 
{
    std::shared_ptr<Program> _program;
    std::vector<Material> _materials;
    std::list<std::shared_ptr<Instantiation>> _instantiations;

public:
    Scene(const std::shared_ptr<Program>&);
    
    ~Scene();

    void Init();

    void Create(const std::shared_ptr<Instantiation>&);

    void Update(float time);

    void Render();    
};
