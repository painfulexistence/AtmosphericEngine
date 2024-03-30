#include "Script.hpp"
#include <string>

Script::Script()
{
    this->_env = sol::state();
}

Script::~Script()
{

}

void Script::Init(MessageBus* mb, Application* app)
{
    Server::Init(mb, app);

    this->_env.open_libraries();
    Source("./assets/config.lua");
    Source("./assets/manifest.lua");
    Source("./assets/main.lua");

    Run("init()");
    //Bind("get_cursor_uv", &Input::GetCursorUV, &app->input);
    //Bind("get_key_down", &Input::GetKeyDown, &app->input);
    //Bind("check_errors", &GraphicsServer::CheckErrors, &app->graphics);
}

void Script::Process(float dt)
{
    Run(fmt::format("update({})", dt));
}

void Script::OnMessage(Message msg)
{

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
