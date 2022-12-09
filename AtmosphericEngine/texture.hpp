#pragma once
#include "Globals.hpp"
#include "lua.hpp"

struct Texture {
    std::string name;
    std::string path;
    
    Texture(sol::table);
};