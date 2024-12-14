#include "console.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "script.hpp"

Console* Console::_instance = nullptr;

Console::Console()
{
    if (_instance != nullptr)
        throw std::runtime_error("Console is already initialized!");

    _instance = this;
}

Console::~Console()
{

}

void Console::Init(Application* app)
{
    Server::Init(app);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("console", console_sink);
    spdlog::set_default_logger(logger);
}

void Console::Process(float dt)
{

}

void Console::DrawImGui(float dt)
{
    if (ImGui::CollapsingHeader("Console", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Log Level:");
        if (ImGui::RadioButton("Info", spdlog::level::info == spdlog::default_logger()->level())) {
            spdlog::default_logger()->set_level(spdlog::level::info);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Warn", spdlog::level::warn == spdlog::default_logger()->level())) {
            spdlog::default_logger()->set_level(spdlog::level::warn);
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Error", spdlog::level::err == spdlog::default_logger()->level())) {
            spdlog::default_logger()->set_level(spdlog::level::err);
        }

        ImGui::Separator();

        ImGui::BeginChild("Log", ImVec2(0, 300));
        ImGui::Text("Log:");
        ImGui::EndChild();

        ImGui::Separator();

        // TODO: command palette
        static char command[256] = "";
        if (ImGui::InputText("Command", command, IM_ARRAYSIZE(command), ImGuiInputTextFlags_EnterReturnsTrue)) {
            ExecuteCommand(command);
            command[0] = '\0';
        }
    }
}

void Console::Info(const std::string& message)
{
#if RUNTIME_LOG_ON
    spdlog::info(message);
#endif
}

void Console::Warn(const std::string& message)
{
#if RUNTIME_LOG_ON
    spdlog::warn(message);
#endif
}

void Console::Error(const std::string& message)
{
#if RUNTIME_LOG_ON
    spdlog::error(message);
#endif
}

void Console::RegisterCommand(const std::string& cmd, std::function<void(const std::vector<std::string>&)> callback)
{
    _commands[cmd] = callback;
}

void Console::ExecuteCommand(const std::string& cmd)
{
    auto it = _commands.find(cmd);
    if (it != _commands.end()) {
        it->second({});
    }

    Script::Get()->Run(cmd);
}