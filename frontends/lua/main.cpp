/// AtmosLua - Love2D-style Lua Game Runtime
///
/// This is the entry point for running Lua games with the Atmospheric Engine.
/// It creates a LuaApplication that loads and executes main.lua.
///
/// Usage:
///   ./AtmosLua [options]
///
/// The runtime looks for main.lua in the following locations (in order):
///   1. ./assets/scripts/main.lua
///   2. ./assets/main.lua
///   3. ./main.lua
///
/// Example main.lua:
///   function load()
///       player = atmos.world.spawn(0, 1, 0)
///       player.name = "Player"
///   end
///
///   function update(dt)
///       if atmos.input.isKeyDown(atmos.keys.ESCAPE) then
///           atmos.quit()
///       end
///   end

#include "lua_application.hpp"

int main(int argc, char* argv[])
{
    // Parse command line arguments
    AppConfig config = {
        .windowTitle = "AtmosLua",
        .windowWidth = 1280,
        .windowHeight = 720,
        .windowResizable = true,
        .vsync = true,
        .useDefaultTextures = true,
        .useDefaultShaders = true,
    };

    // TODO: Parse command line arguments for config overrides
    // e.g., --width 1920 --height 1080 --fullscreen

    // Create and run the Lua application
    LuaApplication app(config);
    app.Run();

    return 0;
}
