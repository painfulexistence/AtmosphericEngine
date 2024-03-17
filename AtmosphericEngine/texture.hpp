#pragma once
#include "Globals.hpp"
#include "Script.hpp"

struct Texture {
    std::string name;
    std::string path;
    
    Texture(sol::table);
};