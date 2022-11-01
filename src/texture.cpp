#include "texture.hpp"
#include "Lua.hpp"

Texture::Texture(sol::table t)
{
    name = t.get_or("name", std::string("unnamed"));
    path = t.get_or("path", std::string("assets/textures/nonexistent"));
}