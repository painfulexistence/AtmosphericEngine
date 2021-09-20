#include "Scripting/Lua.hpp"

Lua::Lua()
{
    this->_env = sol::state();
}

Lua::~Lua()
{

}

void Lua::Init()
{
    this->_env.open_libraries();
    Source("./resources/config.lua");
    Source("./resources/manifest.lua");
    Source("./resources/main.lua");
}

void Lua::Bind(const std::string& func)
{
    //this->_env.set_function(func);
}

void Lua::Source(const std::string& path)
{
    this->_env.script_file(path);
}

void Lua::Run(const std::string& script)
{
    this->_env.script(script);
}

void Lua::Print(const std::string& msg)
{
    Run(fmt::format("print('[Script] {}')", msg));
}

sol::state& Lua::Env()
{
    return this->_env;
}