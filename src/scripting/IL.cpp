#include "Scripting/IL.hpp"

void IL::Print(const std::string& msg)
{
    // Default print function
    fmt::print("[Script] {}", msg);
}