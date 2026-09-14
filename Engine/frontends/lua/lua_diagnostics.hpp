#pragma once

#include <functional>
#include <string>

/// Diagnostics shared by the Lua frontend (LuaApplication, ScriptableComponent,
/// the runtime's host bridge).
///
/// Every script error funnels through ReportLuaError(): it always goes to
/// stderr, and the host may install a sink to surface it elsewhere — the web
/// playground shows it in the page next to the editor.
using LuaErrorSink = std::function<void(const std::string& context, const std::string& message)>;

void ReportLuaError(const std::string& context, const std::string& message);
void SetLuaErrorSink(LuaErrorSink sink);

/// "Halted" means a script callback raised. Every callback (update(), component
/// ticks, collision handlers) stays silent until the next LuaApplication::
/// ReloadScript() clears the flag, so a broken update() reports its error once
/// instead of sixty times a second — the scene freezes on its last good frame.
bool IsLuaHalted();
void SetLuaHalted(bool halted);
