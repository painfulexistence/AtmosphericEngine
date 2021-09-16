#pragma once
#include "Framework/ImGui.hpp"

/*
//Example ImGui code
bool showDemo = false;
bool showClosable = false;

ImGui::ShowDemoWindow(&showDemo);
if (showClosable)
{
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Test Window", &showClosable);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    if (ImGui::Button("Button"))
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    if (ImGui::Button("Close Me"))
        showClosable = false;
    ImGui::End();
}
*/
typedef std::function<bool(std::string, void*)> imguiFx;

namespace ImguiFx
{
    // Not type safe
    static imguiFx Text = [](std::string label, void* value) 
    { 
        ImGui::Text(label.c_str(), value);
        return true; 
    };

    // Not type safe
    static imguiFx Slider = [](std::string label, void* value) 
    { 
        bool isActive = ImGui::SliderFloat(label.c_str(), (float*)value, 0.0f, 1.0f);
        return isActive;
    };

    // Not type safe
    static imguiFx Color = [](std::string label, void* value) 
    { 
        bool isActive = ImGui::ColorEdit3(label.c_str(), (float*)value);
        return isActive;
    };

    // Not type safe
    static imguiFx Button = [](std::string label, void* value) 
    {
        bool isActive = ImGui::Button(label.c_str(), ImVec2(20.0f, 60.0f));
        return isActive;
    };
}