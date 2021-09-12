#pragma once
#include "../common.hpp"

struct Texture {
    std::string name;
    std::string path;
    
    Texture(sol::table);
};