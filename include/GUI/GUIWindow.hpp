#pragma once
#include "common.hpp"
#include "GUI/GUIElement.hpp"

class GUIDocument;

class GUIWindow
{
public:
    GUIWindow();
    
    GUIWindow(std::string name);
    
    GUIWindow(std::string name, std::vector<GUIElement> elements);
    
    ~GUIWindow();
    
    void OnChange(std::vector<GUIElement> elements);
    
    void Render();

private:
    std::string _name;
    GUIDocument* _document;
};