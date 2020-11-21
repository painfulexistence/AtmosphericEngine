#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "physics/BulletMinimal.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstdint>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <memory>
#include <utility>
#include <stdexcept>

#define PI 3.1416
#define SCREEN_W 1440
#define SCREEN_H 900
#define SHADOW_W 4096
#define SHADOW_H 4096
#define VSYNC_ON true
#define MAX_NUM_AUX_LIGHTS 6
#define NUM_MAP_TEXS 6
#define CAMERA_ANGULAR_OFFSET 0.05
#define CAMERA_SPEED 15
#define CAMERA_VERTICAL_SPEED 8

#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2
#define AREA_LIGHT 3

enum Axis {
    UP, DOWN,
    BACK, FRONT,
    RIGHT, LEFT
};