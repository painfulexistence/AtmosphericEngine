#include "Scripting/Lua.hpp"
#include "OS/File.hpp"

sol::state Lua::L = sol::state();

void Lua::Lib()
{
    L.open_libraries();
}

void Lua::Run(const std::string& str)
{
    L.script(str);
}

void Lua::Print(const std::string& str)
{
    L.script(std::string("print('[Script] ") + str + std::string("')"));
}

void Lua::Source(const std::string& path)
{
    L.script_file(path);
}