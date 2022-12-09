#pragma once
#include "Globals.hpp"
#include "GUIElement.hpp"
#include "ImGui.hpp"

class GUIWindow;

class GUIDocument
{
public:
    GUIDocument(GUIWindow* window);
    ~GUIDocument();
    void AddElement(GUIElement element);
    void AddElement(std::string label, GUIElementType type = GUIElementType::NONE,  std::string* text = nullptr);
    void AddElement(std::string label, GUIElementType type = GUIElementType::NONE, ImVec4* color = nullptr);
    void AddElement(std::string label, GUIElementType type = GUIElementType::NONE, float* number = nullptr);
    void RemoveElement(std::string key);
    void Render();
    void Overwrite(std::vector<GUIElement> elements);

private:
    GUIWindow* _window;
    std::vector<GUIElement> _elements;
};