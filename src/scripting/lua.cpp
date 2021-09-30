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
    Source("./assets/config.lua");
    Source("./assets/manifest.lua");
    Source("./assets/main.lua");
}

void Lua::Bind(const std::string& func)
{
    //this->_env.set_function(func);
}

void Lua::Source(const std::string& file)
{
    this->_env.script_file(file);
}

void Lua::Run(const std::string& script)
{
    this->_env.script(script);
}

void Lua::Print(const std::string& msg)
{
    Run(fmt::format("print('[Script] {}')", msg));
}

const sol::table& Lua::GetData(const std::string& key)
{
    return this->_env.globals()[key];
}

void Lua::GetData(const std::string& key, sol::table& data)
{
    data = this->_env.globals()[key];
}