#pragma once
#include "globals.hpp"
#include "bullet_linear_math.hpp"

class ShaderProgram;

class PhysicsDebugDrawer : public btIDebugDraw
{
private:
    int _mode = DebugDrawModes::DBG_DrawWireframe;

public:
    GLuint vao;

    PhysicsDebugDrawer();

    ~PhysicsDebugDrawer();

    virtual void setDebugMode(int mode) override
    {
        _mode = mode;
    };

    virtual int getDebugMode() const override
    {
        return _mode;
    };

    virtual void reportErrorWarning(const char*) override;

    virtual void drawLine(const btVector3&, const btVector3&, const btVector3&) override;

    virtual void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override;

    virtual void draw3dText(const btVector3&, const char*) override;
};