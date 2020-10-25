#pragma once
#include "../common.hpp"

class Debugger : public btIDebugDraw 
{
private:
    struct Line 
    {
        btVector3 from;
        btVector3 to;
        btVector3 color;
    };
    struct Point
    {
        btVector3 p;
        btVector3 color;
    };
    int _mode = 3;
    GLuint vao, vbo;
    std::vector<Line> _lines;
    std::vector<Point> _points;

public:
    Debugger();
        
    void setDebugMode(int mode) override
    {
        _mode = mode;
    };
    
    int getDebugMode() const override
    {
        return _mode;
    };

    void reportErrorWarning(const char*) override;

    void drawLine(const btVector3&, const btVector3&, const btVector3&) override;
    
    void drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) override;
        
    void draw3dText(const btVector3&, const char*) override;

    void Render();
};