#include "GUI/GUI.hpp"
#include "GUI/GUIState.hpp"
#include "GUI/GUIWindow.hpp"
#include "GUI/GUIElement.hpp"

GUI::GUI()
{
    this->_state = new GUIState();
}

GUI::~GUI()
{
    delete this->_state;
}

void GUI::Init(MessageBus* mb, Framework* fw)
{
    ConnectBus(mb);
    this->_fw = fw;
}

void GUI::HandleMessage(Message msg)
{

}

void GUI::Render()
{    
    this->_state->Render();
}