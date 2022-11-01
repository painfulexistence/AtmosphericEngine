#pragma once
#include "Globals.hpp"
#include "IL.hpp"
//#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

class Lua : public IL
{
public:
    Lua();

    ~Lua();

    void Init() override;

    void Bind(const std::string& func) override;

    void Source(const std::string& file) override;

    void Run(const std::string&) override;

    void Print(const std::string&) override;

    const sol::table& GetData(const std::string& key);

    void GetData(const std::string& key, sol::table& data);

private:
    sol::state _env;
};