#pragma once
#include "Globals.hpp"
#include "Graphics/Model.hpp"

class MeshCollection
{
public:
    MeshCollection()
    {

    }

    void Add(const std::string& key, Model model)
    {
        this->_table.insert({key, model});
    }
    
    Model* Find(const std::string& key)
    {
        return &(this->_table.find(key)->second);
    }

protected:
    std::map<const std::string&, Model> _table;
};