#pragma once
#include "Globals.hpp"
#include "Server.hpp"
//#define SOL_ALL_SAFETIES_ON 1
#define SOL_LUA_VERSION 504
#include "sol/sol.hpp"


class Script : public Server
{
public:
    Script();
    
    ~Script();
    
    void Init(MessageBus* mb, Application* app);
    
    void Process(float dt) override;
    
    void OnMessage(Message msg) override;

    void Bind(const std::string& func);

    void Source(const std::string& file);

    void Run(const std::string&);

    void Print(const std::string& msg);

    const sol::table& GetData(const std::string& key);

    void GetData(const std::string& key, sol::table& data);

private:
    Application* _app;
    sol::state _env;
};