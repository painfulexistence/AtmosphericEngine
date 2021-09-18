#include "Scripting/IL.hpp"

void IL::Print(const std::string& msg)
{
    fmt::print("[Script] {}", msg);
}