#pragma once
#include "Globals.hpp"
#include "Framework.hpp"
#include "Scripting/IL.hpp"
#include "Scripting/Lua.hpp"

class Script : public Server
{
public:
    Script();
    
    ~Script();
    
    void Init(MessageBus* mb, Application* app);
    
    void Process(float dt) override;

    void Print(const std::string& msg);

    template<typename T> void GetData(const std::string& key, T& data);
    
    sol::state& LuaEnv(); // TODO: Remove this because it will expose the scripting language implementation

    void OnMessage(Message msg) override;

private:
    Application* _app;
    IL* _L;
};