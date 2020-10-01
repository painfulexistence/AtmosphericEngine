#pragma once
#include "globals.h"
#include <iostream>
#include <assert.h>

class Light 
{
    private:
    glm::vec3 _position;
    glm::vec3 _direction;
    glm::vec3 _color;
    
    public:
    Light(glm::vec3 position, glm::vec3 color);
    void setPosition(glm::vec3 position);
    void setColor(glm::vec3 color);
    void setDirection(glm::vec3 direction);
    
    glm::vec3 getPosition() { return _position; }
    glm::vec3 getColor() { return _color; }
    glm::vec3 getDirection() { return _direction; }
};