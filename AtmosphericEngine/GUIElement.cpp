#include "GUIElement.hpp"

GUIElement::GUIElement(std::string label, void* value, imguiFx fx)
{
    this->_label = label;
    this->_value = value;
    this->_fx = fx;
}

GUIElement::~GUIElement()
{
    
}

void GUIElement::Render()
{
    this->_fx(this->_label, this->_value);
}

GUIElement GUIElement::Text(std::string label, std::string* value)
{
    return GUIElement(label, value, ImguiFx::Text);
}

GUIElement GUIElement::Color(std::string label, ImVec4* value)
{
    return GUIElement(label, value, ImguiFx::Color);
}

GUIElement GUIElement::Button(std::string label, bool* value)
{
    return GUIElement(label, value, ImguiFx::Button);
}

GUIElement GUIElement::Slider(std::string label, float* value)
{
    return GUIElement(label, value, ImguiFx::Slider);
}

