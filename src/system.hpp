#pragma once
#include "Common.hpp"

class System 
{
private:
    static int SystemCount;

protected:
    int _id;
    std::string _name;

public:
    System(std::string name)
    {
        if (SystemCount == NULL)
        {
            SystemCount = 0;
        }
        _id = ++SystemCount;
        _name = name;
    }

    ~System()
    {
        _id = 0;
    }

    Register()
    {

    }

    Unregister()
    {

    }
};