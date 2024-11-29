#pragma once
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#define GLEW_BUILD //For Windows
#include <GL/glew.h>

#include <pch.hpp>

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