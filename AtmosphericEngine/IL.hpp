#pragma once
#include "Globals.hpp"

class IL
{
public:
    virtual void Init() = 0;

    virtual void Bind(const std::string& func) = 0;

    virtual void Source(const std::string&) = 0;

    virtual void Run(const std::string&) = 0;

    virtual void Print(const std::string&);

    template<typename T> void GetData(const std::string& key, T& data);
};