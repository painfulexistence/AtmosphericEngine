#pragma once
#include "Globals.hpp"
#include "GUI/ImguiFx.hpp"
#include "GUI/ImGui.hpp"

enum GUIElementType
{
    TEXT, COLOR, BUTTON, SLIDER, NONE
};

class GUIElement
{
public:
    GUIElement(std::string label, void* value, imguiFx fx);
    
    ~GUIElement();

    void Render();

    static GUIElement Text(std::string label, std::string* value);
    
    static GUIElement Color(std::string label, ImVec4* value);
    
    static GUIElement Button(std::string label, bool* value);

    static GUIElement Slider(std::string label, float* value);

private:
    std::string _label;
    void* _value;
    imguiFx _fx;
};