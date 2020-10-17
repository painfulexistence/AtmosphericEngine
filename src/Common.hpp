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

#define SCREEN_W 1000
#define SCREEN_H 1000
#define CAMERA_ANGULAR_OFFSET 0.05
#define CAMERA_SPEED 20
#define CAMERA_VERTICAL_SPEED 8
//#define FRAMERATE_SMOOTHNESS 0.8

//#define ATTR_PROJECTION_VIEW_MATRIX 0
//#define ATTR_WORLD_MATRIX 4
