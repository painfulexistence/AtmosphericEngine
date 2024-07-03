#include "physics_debug_drawer.hpp"
#include "fmt/core.h"
#include "shader.hpp"
#include <memory>

DebugDrawer::DebugDrawer()
{
    glGenBuffers(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));

    glBindVertexArray(0);

    program = std::make_unique<ShaderProgram>("assets/shaders/physics_debug.vert", "assets/shaders/physics_debug.frag");
}

DebugDrawer::~DebugDrawer()
{
    glDeleteBuffers(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void DebugDrawer::reportErrorWarning(const char* text)
{
    fmt::print("Bullet Warning: {}\n", text);
}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    _lines.push_back({
        glm::vec3(from.getX(), from.getY(), from.getZ()),
        glm::vec3(color.getX(), color.getY(), color.getZ())
    });
    _lines.push_back({
        glm::vec3(to.getX(), to.getY(), to.getZ()),
        glm::vec3(color.getX(), color.getY(), color.getZ())
    });
}

void DebugDrawer::drawContactPoint(const btVector3& point, const btVector3& normal, btScalar distance, int lifeTime, const btVector3& color)
{
    _lines.push_back({
        glm::vec3(point.getX(), point.getY(), point.getZ()),
        glm::vec3(color.getX(), color.getY(), color.getZ())
    });
    _lines.push_back({
        glm::vec3(point.getX() + distance * normal.getX(), point.getY() + distance * normal.getY(), point.getZ() + distance * normal.getZ()),
        glm::vec3(color.getX(), color.getY(), color.getZ())
    });
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
    // TODO
}

void DebugDrawer::Render()
{
    if (_lines.size() == 0)
        return;

    glDisable(GL_DEPTH_TEST);

    program->Activate();

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, _lines.size() * sizeof(DebugVertex), _lines.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, _lines.size() / 2);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}