#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <list>

#define SCREEN_W 1000
#define SCREEN_H 1000
#define FRAMERATE_SMOOTHNESS 0.8

#define ATTR_PROJECTION_VIEW_MATRIX 0
#define ATTR_WORLD_MATRIX 4