#include "GUIDocument.hpp"
#include "GUIWindow.hpp"
#include "GUIElement.hpp"

GUIDocument::GUIDocument(GUIWindow* window)
{
    this->_window = window;
}

GUIDocument::~GUIDocument()
{

}

void GUIDocument::AddElement(GUIElement element)
{
    this->_elements.push_back(element);
}

void GUIDocument::AddElement(std::string label, GUIElementType type, std::string* text)
{
    switch (type)
    {
        case GUIElementType::BUTTON:
            AddElement(GUIElement::Button(label, nullptr));
            break;
        case GUIElementType::TEXT:
        default:
            AddElement(GUIElement::Text(label, text));
            break;
    }
}

void GUIDocument::AddElement(std::string label, GUIElementType type, ImVec4* color)
{
    switch (type)
    {
        case GUIElementType::COLOR:
        default:
            AddElement(GUIElement::Color(label, color));
            break;
    }
}

void GUIDocument::AddElement(std::string label, GUIElementType type, float* number)
{
    switch (type)
    {
        case GUIElementType::SLIDER:
        default:
            AddElement(GUIElement::Slider(label, number));
            break;
    }
}

void GUIDocument::RemoveElement(std::string key)
{

}

void GUIDocument::Render()
{

}

void GUIDocument::Overwrite(std::vector<GUIElement> elements)
{
    this->_elements.swap(elements);
}