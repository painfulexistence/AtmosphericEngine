#include "lua.hpp"

using namespace sol;

state Lua::L = state();

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
    L.script(std::string("print('") + str + std::string("')"));
}

void Lua::Source(const std::string& path)
{
    L.script_file(path);
}

Lua::Lua() {}
