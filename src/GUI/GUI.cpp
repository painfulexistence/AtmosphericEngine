#include "GUI/GUI.hpp"
#include "GUI/GUIState.hpp"
#include "GUI/GUIWindow.hpp"
#include "GUI/GUIElement.hpp"

/*
ImGui::Begin("General Graphics");

ImGui::Text("OpenGL version: %s", glGetString(GL_VERSION));
ImGui::Text("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));

GLint depth, stencil;
glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth);    
glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil);    
ImGui::Text("Depth bits: %d", depth);
ImGui::Text("Stencil bits: %d", stencil);

GLint maxVertUniforms, maxFragUniforms;
glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertUniforms);
glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragUniforms);
ImGui::Text("Max vertex uniforms: %d bytes", maxVertUniforms / 4);
ImGui::Text("Max fragment uniforms: %d bytes", maxFragUniforms / 4);

GLint maxVertUniBlocks, maxFragUniBlocks;
glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertUniBlocks);
glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxFragUniBlocks);
ImGui::Text("Max vertex uniform blocks: %d", maxVertUniBlocks);
ImGui::Text("Max fragment uniform blocks: %d", maxFragUniBlocks);

GLint maxElementIndices, maxElementVertices;
glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxElementIndices);
glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxElementVertices);
ImGui::Text("Max element indices: %d", maxElementIndices);
ImGui::Text("Max element vertices: %d", maxElementVertices);

ImGui::End();

ImGui::Begin("Realtime Rendering");

ImGui::ColorEdit3("Clear color", (float*)&clearColor);
ImGui::Text("Entities count: %lu", entities.size());
ImGui::Text("Draw time: %.3f s/frame", dt);
ImGui::Text("Frame rate: %.1f FPS", 1.0f / dt);
//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

ImGui::End();
*/

GUI::GUI()
{
    this->_state = new GUIState();
    //auto win = this->_state->CreateWindow("Realtime Rendering"); //TODO: this will cause the simulation thread to never finish 
}

GUI::~GUI()
{
    delete this->_state;
}

void GUI::Init(MessageBus* mb, Application* app)
{
    ConnectBus(mb);
    this->_app = app;
}

void GUI::Render(float dt)
{    
    this->_state->Render();
}

void GUI::HandleMessage(Message msg)
{

}