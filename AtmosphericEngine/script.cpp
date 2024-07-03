#include "script.hpp"
#include <string>

Script* Script::_instance = nullptr;

Script::Script()
{
    if (_instance != nullptr)
        throw std::runtime_error("Script is already initialized!");

    _env = sol::state();

    _instance = this;
}

Script::~Script()
{

}

void Script::Init(Application* app)
{
    Server::Init(app);

    this->_env.open_libraries();
    Source("./assets/config.lua");
    Source("./assets/manifest.lua");
    Source("./assets/main.lua");

    Run("init()");
    //Bind("get_cursor_uv", &Input::GetCursorUV, Input::Get());
    //Bind("get_key_down", &Input::GetKeyDown, Input::Get());
    //Bind("check_errors", &GraphicsServer::CheckErrors, GraphicsServer::Get());
}

void Script::Process(float dt)
{
    Run(fmt::format("update({})", dt));
}

// TODO: an extra argument is needed here
void Script::Bind(const std::string& func)
{
    //this->_env.set_function(func);
}

void Script::Source(const std::string& file)
{
    this->_env.script_file(file);
}

void Script::Run(const std::string& script)
{
    this->_env.script(script);
}

void Script::Print(const std::string& msg)
{
    Run(fmt::format("print('[Script] {}')", msg));
}

sol::table Script::GetData(const std::string& key)
{
    return this->_env.globals()[key];
}

void Script::GetData(const std::string& key, sol::table& data)
{
    data = this->_env.globals()[key];
}
