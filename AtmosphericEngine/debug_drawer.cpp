#include "debug_drawer.hpp"


DebugDrawer::DebugDrawer()
{
    glGenBuffers(1, &vao);
    glGenBuffers(1, &vbo);
}

void DebugDrawer::reportErrorWarning(const char* text)
{

}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    Line l;
    l.from = from;
    l.to = to;
    l.color = color;

    _lines.push_back(l);
}

void DebugDrawer::drawContactPoint(const btVector3& point, const btVector3& normal, btScalar distance, int lifeTime, const btVector3& color)
{
    Line l;
    l.from = point;
    l.to = point + distance * normal;
    l.color = color;

    _lines.push_back(l);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{

}

void DebugDrawer::Render()
{
    glBindVertexArray(vao);
    //glDrawArrays();
}