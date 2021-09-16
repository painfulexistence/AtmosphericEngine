#pragma once
//#define SOL_ALL_SAFETIES_ON 1
#include "sol/sol.hpp"
#include "common.hpp"

class Lua
{
public:
    static sol::state L;

    static void Lib();

    static void Run(const std::string&);

    static void Print(const std::string&);

    static void Source(const std::string&);
};