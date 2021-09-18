#include "GUI/GUIState.hpp"
#include "GUI/GUIWindow.hpp"
#include "GUI/ImGui.hpp"

GUIState::GUIState()
{
    this->_mode = GUIMode::MINIMALIST;
    this->_theme = GUITheme::DARK;
    this->_font = GUIFont::SMALL;
    this->_opacity = 1.0;
}

GUIState::GUIState(GUIMode mode, GUITheme theme, GUIFont font, float opacity)
{
    this->_mode = mode;
    this->_theme = theme;
    this->_font = font;
    this->_opacity = opacity;
}

GUIState::~GUIState()
{
    
}

void GUIState::Render() const
{
    if (this->_mode == GUIMode::INVISIBLE)
        return;

    // Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    for (auto win : _windows)
        win.Render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GUIWindow* GUIState::CreateWindow(std::string name)
{
    auto window = new GUIWindow(name);
    this->_windows.push_back(*window);
    return window;
}

void GUIState::DestroyWindow(std::string key)
{

}

GUIWindow* GUIState::FindWindow(std::string key)
{
    return nullptr;
}

