#pragma once
#include "Globals.hpp"
#include "Physics/BulletCollision.hpp"

class ShapeCollection
{
public:
    ShapeCollection()
    {

    }
    
    void Add(const std::string& key, btCollisionShape* shape)
    {
        this->_table.insert({key, shape});
    }
    
    btCollisionShape* Find(const std::string& key)
    {
        return this->_table.find(key)->second;
    }

protected:
    std::map<const std::string&, btCollisionShape*> _table;
};