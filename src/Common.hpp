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
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <memory>
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
#define FIXED_TIME_STEP 1.0 / 60.0
//#define FRAMERATE_SMOOTHNESS 0.8

//#define ATTR_PROJECTION_VIEW_MATRIX 0
//#define ATTR_WORLD_MATRIX 4

#define DIR_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2
#define AREA_LIGHT 3