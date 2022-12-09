#include "IL.hpp"

void IL::Print(const std::string& msg)
{
    // Default print function
    fmt::print("[Script] {}", msg);
}

template<typename T> void IL::GetData(const std::string& key, T& data)
{
    throw std::runtime_error("Accessor of the requested type is not implemented!");
}

