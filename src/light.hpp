#ifndef LIGHT_H
#define LIGHT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <assert.h>

class Light {
    public:
        Light(glm::vec3 position, glm::vec3 color = glm::vec3(1.0, 1.0, 1.0), glm::vec3 direction = glm::vec3(0, 0, 0)) {
            _position = position;
            _color = color;
            _direction = direction;
        }

        glm::vec3 getPosition() {
            return _position;
        }
        void setPosition(glm::vec3 position) {
            _position = position;
        }

        glm::vec3 getColor() {
            return _color;
        }
        void setColor(glm::vec3 color) {
            _color = color;
        }

        glm::vec3 getDirection() {
            return _direction;
        }
        void setDirection(glm::vec3 direction) {
            _direction = direction;
        }

    private:
        glm::vec3 _position;
        glm::vec3 _direction; //(0, 0, 0) -> Point Light Source
        glm::vec3 _color;
};

#endif