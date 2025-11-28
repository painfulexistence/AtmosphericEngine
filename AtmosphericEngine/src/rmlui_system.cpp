#include "rmlui_system.hpp"
#include <spdlog/spdlog.h>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#else
// Linux clipboard support could be added via X11 or Wayland
#endif

RmlUiSystem::RmlUiSystem() {
    auto now = std::chrono::high_resolution_clock::now();
    m_start_time = std::chrono::duration<double>(now.time_since_epoch()).count();
}

RmlUiSystem::~RmlUiSystem() {
}

double RmlUiSystem::GetElapsedTime() {
    auto now = std::chrono::high_resolution_clock::now();
    double current_time = std::chrono::duration<double>(now.time_since_epoch()).count();
    return current_time - m_start_time;
}

bool RmlUiSystem::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    switch (type) {
        case Rml::Log::LT_ALWAYS:
        case Rml::Log::LT_ERROR:
            spdlog::error("[RmlUi] {}", message);
            break;
        case Rml::Log::LT_WARNING:
            spdlog::warn("[RmlUi] {}", message);
            break;
        case Rml::Log::LT_INFO:
            spdlog::info("[RmlUi] {}", message);
            break;
        case Rml::Log::LT_DEBUG:
            spdlog::debug("[RmlUi] {}", message);
            break;
        default:
            spdlog::trace("[RmlUi] {}", message);
            break;
    }
    return true;
}

void RmlUiSystem::SetMouseCursor(const Rml::String& cursor_name) {
    // Platform-specific cursor setting would go here
    // For now, we'll just log it
    spdlog::trace("[RmlUi] Set cursor: {}", cursor_name);
}

void RmlUiSystem::SetClipboardText(const Rml::String& text) {
#ifdef _WIN32
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hMem) {
            memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
        CloseClipboard();
    }
#else
    // Clipboard not implemented for this platform
    spdlog::trace("[RmlUi] Set clipboard text (not implemented on this platform)");
#endif
}

void RmlUiSystem::GetClipboardText(Rml::String& text) {
#ifdef _WIN32
    if (OpenClipboard(nullptr)) {
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData) {
            char* pszText = static_cast<char*>(GlobalLock(hData));
            if (pszText) {
                text = pszText;
            }
            GlobalUnlock(hData);
        }
        CloseClipboard();
    }
#else
    // Clipboard not implemented for this platform
    text = "";
#endif
}

void RmlUiSystem::ActivateKeyboard(Rml::Vector2f caret_position, float line_height) {
    // On mobile platforms, this would show the virtual keyboard
    // For desktop, this is typically a no-op
}

void RmlUiSystem::DeactivateKeyboard() {
    // On mobile platforms, this would hide the virtual keyboard
}
