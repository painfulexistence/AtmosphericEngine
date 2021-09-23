#pragma once
#include "Globals.hpp"
#include "Graphics/mesh.hpp"

class MeshCollection
{
public:
    MeshCollection()
    {

    }

    void Add(const std::string& key, Mesh mesh)
    {
        this->_table.insert({key, mesh});
    }
    
    Mesh* Find(const std::string& key)
    {
        return &(this->_table.find(key)->second);
    }

protected:
    std::map<const std::string&, Mesh> _table;
};