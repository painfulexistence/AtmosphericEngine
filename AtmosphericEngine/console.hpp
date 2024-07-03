#pragma once
#include "globals.hpp"
#include "server.hpp"

/// Logging system
class Console : public Server
{
private:
    static Console* _instance;

public:
    static Console* Get()
    {
        return _instance;
    }

    Console();

    ~Console();
};