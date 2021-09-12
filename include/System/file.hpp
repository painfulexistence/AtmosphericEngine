#pragma once
#include <fstream>
#include "../common.hpp"

class File
{
private:
    std::string _filename;

public:
    File(const std::string&);

    std::string GetContent() const;
};