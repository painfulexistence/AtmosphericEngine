#pragma once
#include "Globals.hpp"
#include "Messaging.hpp"
#include "Framework.hpp"
#include "Scripting/IL.hpp"
#include "Scripting/Lua.hpp"

class Script : public Messagable
{
public:
    Script();
    
    ~Script();
    
    void Init(MessageBus* mb, Application* app);
    
    void Process(float dt);

    void Print(const std::string& msg);
    
    sol::state& LuaEnv(); // TODO: Remove this because it will expose the scripting language implementation

    void OnMessage(Message msg) override;

private:
    Application* _app;
    IL* _L;
};