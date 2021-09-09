#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "physics/BulletMinimal.h"
#include "scripting/lua.hpp"

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstdint>
#include <iostream>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <memory>
#include <utility>
#include <stdexcept>

#define PI 3.1416
#define SCREEN_W 1440
#define SCREEN_H 810
#define SHADOW_W 2048
#define SHADOW_H 2048
#define MAX_AUX_SHADOWS 0
#define SHADOW_CASCADES 3
#define VSYNC_ON true
#define NUM_MAP_UNITS 6
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