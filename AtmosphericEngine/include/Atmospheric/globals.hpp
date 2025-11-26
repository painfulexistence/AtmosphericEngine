#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
// #define GLEW_BUILD // build GLEW as dynamic library
// #include <GL/glew.h>
#include <glad/glad.h>
#endif

// #include <pch.hpp>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <fmt/core.h>

#define CAMERA_ANGULAR_OFFSET 0.05
#define CAMERA_SPEED 15
#define CAMERA_VERTICAL_SPEED 8
#define PI 3.1416
#define GRAVITY 9.8
#define FIXED_TIME_STEP 1.0 / 60.0

enum Axis {
    UP, DOWN,
    BACK, FRONT,
    RIGHT, LEFT
};

enum class ShapeType {
    Cube,
    Sphere,
    Capsule,
    Cylinder,
    Cone,
    Plane,
    Custom,
};

struct CubeShapeData {
    glm::vec3 size;
};

struct SphereShapeData {
    float radius;
};

struct CapsuleShapeData {
    float radius;
    float height;
};

struct CylinderShapeData {
    float radius;
    float height;
};

struct ConeShapeData {
    float radius;
    float height;
};

struct Shape {
    ShapeType type;
    union {
        CubeShapeData cubeData;
        SphereShapeData sphereData;
        CapsuleShapeData capsuleData;
        CylinderShapeData cylinderData;
        ConeShapeData coneData;
    } data;
};