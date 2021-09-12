#include "Physics/Debugger.hpp"


Debugger::Debugger()
{
    glGenBuffers(1, &vao);
    glGenBuffers(1, &vbo);
}

void Debugger::reportErrorWarning(const char* text)
{
    std::cout << text << std::endl;
}

void Debugger::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    Line l;
    l.from = from;
    l.to = to;
    l.color = color;

    _lines.push_back(l);
}
    
void Debugger::drawContactPoint(const btVector3& point, const btVector3& normal, btScalar distance, int lifeTime, const btVector3& color)
{   
    Line l;
    l.from = point;
    l.to = point + distance * normal;
    l.color = color;

    _lines.push_back(l);
}
    
void Debugger::draw3dText(const btVector3& location, const char* textString)
{

}

void Debugger::Render()
{
    glBindVertexArray(vao);
    //glDrawArrays();
}