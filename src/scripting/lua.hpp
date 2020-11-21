#pragma once
#include "../common.hpp"
#include "../io/file.hpp"
#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"

class Lua
{
public:
    static sol::state L;

    static void Lib();

    static void Run(const std::string&);

    static void Print(const std::string&);

    static void Source(const std::string&);

    Lua();

    void Load();

    void FixedUpdate();

    void Update(float dt);

    void Render(float dt);
};