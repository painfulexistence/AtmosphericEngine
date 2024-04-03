#pragma once

#define GLEW_BUILD //For Windows
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>

#include <cstdlib>
#include <ctime>
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <stack>
#include <memory>
#include <thread>
#include <functional>
#include <optional>
#include <utility>
#include <stdexcept>

#include "graphics_config.hpp"
#include "runtime_config.hpp"

#define CAMERA_ANGULAR_OFFSET 0.05
#define CAMERA_SPEED 15
#define CAMERA_VERTICAL_SPEED 8
#define PI 3.1416
#define GRAVITY 9.8
#define FIXED_TIME_STEP 1.0 / 60.0

#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2
#define AREA_LIGHT 3

enum Axis {
    UP, DOWN,
    BACK, FRONT,
    RIGHT, LEFT
};