#pragma once
#include "globals.hpp"
#include "glm/glm.hpp"
#include "bullet_linear_math.hpp"
#include <memory>

class ShaderProgram;

class DebugDrawer : public btIDebugDraw
{
private:
    struct DebugVertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    int _mode = 3;
    GLuint vbo;
    std::vector<DebugVertex> _lines;
    std::unique_ptr<ShaderProgram> program;

public:
    GLuint vao;

    DebugDrawer();

    ~DebugDrawer();

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

    void Render();
};