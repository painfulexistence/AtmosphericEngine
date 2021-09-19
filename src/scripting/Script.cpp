#include "Scripting/Script.hpp"
#include "Scripting/Lua.hpp"

Script::Script()
{
    this->_L = (IL*)new Lua();
}

Script::~Script()
{
    delete reinterpret_cast<Lua*>(this->_L);
}

void Script::Init(MessageBus* mb, Application* app)
{
    ConnectBus(mb);
    this->_app = app;

    this->_L->Init();
    this->_L->Run("init()");
    //L->Bind("get_cursor_uv", &Input::GetCursorUV, &input);
    //L->Bind("get_key_down", &Input::GetKeyDown, &input);
    //L->Bind("check_errors", &Renderer::CheckErrors, &renderer);
}

void Script::Process(float dt)
{
    //this->_L->Run(fmt::format("Update({})", dt));
}

void Script::Print(const std::string& msg)
{
    this->_L->Print(msg);
}


void Script::OnMessage(Message msg)
{

}

sol::state& Script::LuaEnv()
{
    return reinterpret_cast<Lua*>(this->_L)->Env();
}