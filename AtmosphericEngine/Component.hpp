#pragma once
#include "Globals.hpp"

class GameObject;

class Component
{
public:
    GameObject* gameObject = nullptr;

    virtual ~Component() {};

    virtual std::string GetName() const = 0;
};

struct Transform;

struct Impostor;

struct Geometry;

struct Script;

// Note that the pointers are widely used here, because the byte size of the reference type is not fixed and can change by implementations
// TODO: For now, the enabled and transform both represents a component's global state. But these should be rewritten with the concept of scene hierarchy in the future
struct TransformData
{
    const Transform* current;
};

struct ImpostorData
{
    bool isActive;
    const Impostor* current;
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