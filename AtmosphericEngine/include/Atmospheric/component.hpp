#pragma once
#include "globals.hpp"

class GameObject;

class Component {
public:
    virtual ~Component(){};

    virtual std::string GetName() const = 0;

    GameObject* GetOwner() const {
        return gameObject;
    }

    virtual void OnAttach(){};
    virtual void OnDetach(){};

    GameObject* gameObject = nullptr;
};

struct Transform;

struct RigidbodyComponent;

struct Geometry;

struct Script;

// Note that the pointers are widely used here, because the byte size of the reference type is not fixed and can change by implementations
// TODO: For now, the enabled and transform both represents a component's global state. But these should be rewritten with the concept of scene hierarchy in the future
struct TransformData
{
    const Transform* current;
};

struct RigidbodyComponentData {
    bool isActive;
    const RigidbodyComponent* current;
};

struct GeometryData
{
    bool isActive;
    const Geometry* current;
};

struct ScriptData
{
    const std::string& script;
};