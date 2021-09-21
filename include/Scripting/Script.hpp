#pragma once
#include "Globals.hpp"
#include "Framework.hpp"
#include "Scripting/IL.hpp"

class Script : public Server
{
public:
    Script();
    
    ~Script();
    
    void Init(MessageBus* mb, Application* app);
    
    void Process(float dt) override;
    
    void OnMessage(Message msg) override;

    void Print(const std::string& msg);

    template<typename T> void GetData(const std::string& key, T& data);

private:
    Application* _app;
    IL* _L;
};