#pragma once
#include "Globals.hpp"
#include "Scripting/IL.hpp"
//#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

class Lua : IL
{
public:
    Lua();

    ~Lua();

    void Init() override;

    void Bind(const std::string& func) override;

    void Source(const std::string& file) override;

    void Run(const std::string&) override;

    void Print(const std::string&) override;

    sol::state& Env();

    void GetData(const std::string& key, sol::table& data);

private:
    sol::state _env;
};