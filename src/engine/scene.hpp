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
private:
    Program* _program;
    std::vector<Material*> _materials;
    std::list<Instantiation*> _instantiations;

public:
    Scene(Program*& program);
    
    ~Scene();

    void Init();

    void Create(Instantiation*&);

    void Update(float time);

    void Render();    
};
