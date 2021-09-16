#pragma once
#include "common.hpp"
#include "Scripting/Lua.hpp"

struct Texture {
    std::string name;
    std::string path;
    
    Texture(sol::table);
};