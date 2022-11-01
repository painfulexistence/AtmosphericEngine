#pragma once
#include "Globals.hpp"
#include "Lua.hpp"

struct Texture {
    std::string name;
    std::string path;
    
    Texture(sol::table);
};