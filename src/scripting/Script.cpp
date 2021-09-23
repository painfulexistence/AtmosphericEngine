#include "Scripting/Script.hpp"
#include "Scripting/Lua.hpp"

Script::Script()
{
    // Note that upcast can be implicit
    // But the child class must be able to access at least one virtual member function of its parent, i.e. publicly inherit its parent
    // Otherwise segmentation fault may happen!
    this->_L = dynamic_cast<IL*>(new Lua());
}

Script::~Script()
{
    delete dynamic_cast<Lua*>(this->_L);
}

void Script::Init(MessageBus* mb, Application* app)
{
    Server::Init(mb, app);
    
    this->_L->Init();
    this->_L->Run("init()");
    //L->Bind("get_cursor_uv", &Input::GetCursorUV, &input);
    //L->Bind("get_key_down", &Input::GetKeyDown, &input);
    //L->Bind("check_errors", &GraphicsServer::CheckErrors, &graphics);
}

void Script::Process(float dt)
{
    //this->_L->Run(fmt::format("update({})", dt));
}

void Script::OnMessage(Message msg)
{

}

void Script::Print(const std::string& msg)
{
    this->_L->Print(msg);
}

template<typename IL> auto Script::GetData(const std::string& key)
{
    return dynamic_cast<IL*>(this->_L)->GetData(key);
}

template<> void Script::GetData(const std::string& key, sol::table& data)
{
    dynamic_cast<Lua*>(this->_L)->GetData(key, data);
}
