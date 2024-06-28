## Devlog
- 2024/03/22 - considering using glad+sdl2 instead of glew+glfw
- 2024/06/24 - upgraded dear imgui so that it can work on Apple M3 machines
- 2024/06/26 - fixed an issue where the error is not thrown when the framebuffer is incomplete (missing "throw" keyword)
- 2024/06/26 - fixed an issue where glGetString(GL_VERSION) returns null and all textures are missing and not loadable by switching to system-wide GLEW and back
- 2024/06/26 - found out that passing vec3(0, 0, 0) to shadow pass World matrix can cause an interesting bug where the shadows move autonomously
- 2024/06/27 - implemented normal mapping (normals flipped)
- 2024/06/28 - fixed flipped normals
- 2024/06/28 - it turns out that the bottleneck is light calculation; turning aux lights off can make the game run at a reasonable speed
- TODO: use std::filesystem to get the correct asset loading path