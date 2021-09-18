#include "Scripting/Script.hpp"
#include "Scripting/Lua.hpp"

Script::Script()
{

}

Script::~Script()
{

}

void Script::Init(MessageBus* mb, Application* app)
{
    ConnectBus(mb);
    this->_app = app;

    Lua::Lib();
    //Lua::L.set_function("get_cursor_uv", &Input::GetCursorUV, &input);
    //Lua::L.set_function("get_key_down", &Input::GetKeyDown, &input);
    //Lua::L.set_function("check_errors", &Renderer::CheckErrors, &renderer);
}

void Script::HandleMessage(Message msg)
{

}