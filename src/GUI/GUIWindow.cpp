#include "GUI/GUIWindow.hpp"
#include "GUI/GUIDocument.hpp"
#include "Framework/ImGui.hpp"

GUIWindow::GUIWindow()
{
    this->_name = "";
    this->_document = new GUIDocument(this);
}

GUIWindow::GUIWindow(std::string name)
{
    this->_name = name;
    this->_document = new GUIDocument(this);
}

GUIWindow::GUIWindow(std::string name, std::vector<GUIElement> elements)
{
    this->_name = name;
    this->_document = new GUIDocument(this);
    for (auto elm : elements)
    {
        this->_document->AddElement(elm);
    }
}

GUIWindow::~GUIWindow()
{
    delete this->_document;
}

void GUIWindow::Render()
{
    ImGui::Begin(this->_name.c_str());
    this->_document->Render();
    ImGui::End();
}

void GUIWindow::OnChange(std::vector<GUIElement> elements)
{
    // TODO: diff algorithm
    this->_document->Overwrite(elements);
}
