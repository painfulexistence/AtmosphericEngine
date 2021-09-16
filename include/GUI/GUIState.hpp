#pragma once
#include "common.hpp"
#include "GUI/GUIWindow.hpp"

enum GUIMode 
{ 
    MINIMALIST, FULL, INVISIBLE
};

enum GUITheme 
{ 
    LIGHT, DARK 
};

enum GUIFont
{ 
    SMALL 
};

class GUIState
{
public:
    GUIState();
    GUIState(GUIMode mode, GUITheme theme, GUIFont font, float opacity = 1.0f);
    ~GUIState();
    void Render() const;
    GUIWindow CreateWindow(std::string name);
    void DestroyWindow(std::string key);
    GUIWindow* FindWindow(std::string key);
private:
    GUIMode _mode;
    GUITheme _theme;
    GUIFont _font;
    float _opacity;
    std::vector<GUIWindow> _windows;
};