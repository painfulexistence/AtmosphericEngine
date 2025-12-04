#include "../lua_application.hpp"
#include "../font_manager.hpp"
#include "Atmospheric/renderer.hpp"

// Keep track of current draw color
static glm::vec4 s_CurrentColor = glm::vec4(1.0f);

// Global font manager instance
static FontManager s_FontManager;

void BindGraphicsAPI(sol::state& lua, GraphicsServer* graphics)
{
    sol::table atmos = lua["atmos"];
    sol::table gfx = atmos.create("graphics");

    // Get main camera
    gfx["getMainCamera"] = [graphics]() -> CameraComponent* {
        return graphics->GetMainCamera();
    };

    // Get main light
    gfx["getMainLight"] = [graphics]() -> LightComponent* {
        return graphics->GetMainLight();
    };

    // Screen dimensions (via Window)
    gfx["getScreenSize"] = []() -> std::tuple<int, int> {
        auto size = Window::Get()->GetSize();
        return std::make_tuple(size.width, size.height);
    };

    gfx["getWidth"] = []() {
        return Window::Get()->GetSize().width;
    };

    gfx["getHeight"] = []() {
        return Window::Get()->GetSize().height;
    };

    // Shader management
    gfx["reloadShaders"] = []() {
        AssetManager::Get().ReloadShaders();
    };

    // ===== 2D Immediate Mode Drawing =====
    // Note: These use the BatchRenderer2D and should be called within proper scene bounds

    // Set current draw color
    gfx["setColor"] = sol::overload(
        [](float r, float g, float b) {
            s_CurrentColor = glm::vec4(r, g, b, 1.0f);
        },
        [](float r, float g, float b, float a) {
            s_CurrentColor = glm::vec4(r, g, b, a);
        },
        [](const glm::vec4& color) {
            s_CurrentColor = color;
        }
    );

    gfx["getColor"] = []() -> glm::vec4 {
        return s_CurrentColor;
    };

    // Draw colored rectangle
    gfx["drawRect"] = sol::overload(
        [graphics](float x, float y, float w, float h) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawQuad(glm::vec2(x, y), glm::vec2(w, h), s_CurrentColor);
        },
        [graphics](float x, float y, float w, float h, float rotation) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawRotatedQuad(glm::vec2(x, y), glm::vec2(w, h), rotation, s_CurrentColor);
        }
    );

    // Draw textured sprite
    gfx["drawSprite"] = sol::overload(
        // Draw with texture ID at position
        [graphics](uint32_t texID, float x, float y) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawQuad(glm::vec2(x, y), glm::vec2(64, 64), texID, s_CurrentColor);
        },
        // Draw with texture ID, position, and size
        [graphics](uint32_t texID, float x, float y, float w, float h) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawQuad(glm::vec2(x, y), glm::vec2(w, h), texID, s_CurrentColor);
        },
        // Draw with texture ID, position, size, and rotation
        [graphics](uint32_t texID, float x, float y, float w, float h, float rotation) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawRotatedQuad(glm::vec2(x, y), glm::vec2(w, h), rotation, texID, s_CurrentColor);
        }
    );

    // ===== Text Rendering =====

    // Load a font (returns FontID)
    gfx["loadFont"] = [](const std::string& path, float size) -> FontID {
        return s_FontManager.LoadFont(path, size);
    };

    // Unload a font
    gfx["unloadFont"] = [](FontID id) {
        s_FontManager.UnloadFont(id);
    };

    // Draw text with current color
    // drawText(font, text, x, y)           -- uses base font size
    // drawText(font, text, x, y, scale)    -- scale relative to base size
    gfx["drawText"] = sol::overload(
        [graphics](FontID fontID, const std::string& text, float x, float y) {
            Font* font = s_FontManager.GetFont(fontID);
            if (!font) return;

            auto* batch = graphics->renderer->GetBatchRenderer();
            float cursorX = x;
            float scale = 1.0f;

            for (char c : text) {
                const Glyph* glyph = s_FontManager.GetGlyph(fontID, static_cast<int>(c));
                if (!glyph) continue;

                float drawX = cursorX + glyph->xOffset * scale;
                float drawY = y + glyph->yOffset * scale + font->ascent * scale;
                float drawW = glyph->width * scale;
                float drawH = glyph->height * scale;

                if (drawW > 0 && drawH > 0) {
                    // Create UV coordinates for this glyph
                    glm::vec2 uvs[4] = {
                        {glyph->u0, glyph->v0},  // top-left
                        {glyph->u1, glyph->v0},  // top-right
                        {glyph->u1, glyph->v1},  // bottom-right
                        {glyph->u0, glyph->v1}   // bottom-left
                    };

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(drawX, drawY, 0.0f));
                    transform = glm::scale(transform, glm::vec3(drawW, drawH, 1.0f));
                    batch->DrawQuad(transform, font->textureID, uvs, s_CurrentColor);
                }

                cursorX += glyph->advance * scale;
            }
        },
        [graphics](FontID fontID, const std::string& text, float x, float y, float scale) {
            Font* font = s_FontManager.GetFont(fontID);
            if (!font) return;

            auto* batch = graphics->renderer->GetBatchRenderer();
            float cursorX = x;

            for (char c : text) {
                const Glyph* glyph = s_FontManager.GetGlyph(fontID, static_cast<int>(c));
                if (!glyph) continue;

                float drawX = cursorX + glyph->xOffset * scale;
                float drawY = y + glyph->yOffset * scale + font->ascent * scale;
                float drawW = glyph->width * scale;
                float drawH = glyph->height * scale;

                if (drawW > 0 && drawH > 0) {
                    glm::vec2 uvs[4] = {
                        {glyph->u0, glyph->v0},
                        {glyph->u1, glyph->v0},
                        {glyph->u1, glyph->v1},
                        {glyph->u0, glyph->v1}
                    };

                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(drawX, drawY, 0.0f));
                    transform = glm::scale(transform, glm::vec3(drawW, drawH, 1.0f));
                    batch->DrawQuad(transform, font->textureID, uvs, s_CurrentColor);
                }

                cursorX += glyph->advance * scale;
            }
        }
    );

    // Measure text dimensions
    gfx["measureText"] = sol::overload(
        [](FontID fontID, const std::string& text) -> std::tuple<float, float> {
            glm::vec2 size = s_FontManager.MeasureText(fontID, text, 1.0f);
            return std::make_tuple(size.x, size.y);
        },
        [](FontID fontID, const std::string& text, float scale) -> std::tuple<float, float> {
            glm::vec2 size = s_FontManager.MeasureText(fontID, text, scale);
            return std::make_tuple(size.x, size.y);
        }
    );

    // Get font line height
    gfx["getFontHeight"] = [](FontID fontID, float scale) -> float {
        Font* font = s_FontManager.GetFont(fontID);
        return font ? font->lineHeight * scale : 0.0f;
    };

    // Begin/End 2D scene (for manual control)
    gfx["begin2D"] = [graphics]() {
        auto* batch = graphics->renderer->GetBatchRenderer();
        // Use orthographic projection matching screen size
        auto size = Window::Get()->GetSize();
        glm::mat4 proj = glm::ortho(0.0f, (float)size.width, (float)size.height, 0.0f, -1.0f, 1.0f);
        batch->BeginScene(proj);
    };

    gfx["end2D"] = [graphics]() {
        auto* batch = graphics->renderer->GetBatchRenderer();
        batch->EndScene();
    };

    // ===== CameraComponent usertype =====
    lua.new_usertype<CameraComponent>("CameraComponent",
        sol::no_constructor,

        "gameObject", sol::readonly(&CameraComponent::gameObject)
    );

    // ===== LightComponent usertype =====
    lua.new_usertype<LightComponent>("LightComponent",
        sol::no_constructor,

        "gameObject", sol::readonly(&LightComponent::gameObject),

        // Direct access to public members
        "intensity", &LightComponent::intensity,
        "ambient", &LightComponent::ambient,
        "diffuse", &LightComponent::diffuse,
        "specular", &LightComponent::specular,
        "castShadow", &LightComponent::castShadow
    );

    // ===== Asset management =====
    sol::table assets = atmos.create("assets");

    assets["createCubeMesh"] = [](const std::string& name, float size) -> Mesh* {
        return AssetManager::Get().CreateCubeMesh(name, size);
    };

    assets["createSphereMesh"] = [](const std::string& name, float radius, int segments) -> Mesh* {
        return AssetManager::Get().CreateSphereMesh(name, radius, segments);
    };

    assets["createPlaneMesh"] = [](const std::string& name, float width, float height) -> Mesh* {
        return AssetManager::Get().CreatePlaneMesh(name, width, height);
    };

    assets["getMesh"] = [](const std::string& name) -> Mesh* {
        return AssetManager::Get().GetMesh(name);
    };

    assets["loadTexture"] = [](const std::string& path) {
        AssetManager::Get().LoadTextures({path});
    };

    // Get texture by name (returns GL texture ID for use with drawSprite)
    assets["getTexture"] = [](const std::string& name) -> uint32_t {
        return AssetManager::Get().GetTexture(name);
    };

    // ===== Mesh usertype =====
    lua.new_usertype<Mesh>("Mesh",
        sol::no_constructor,

        "setMaterial", [](Mesh* mesh, int index) {
            auto& materials = AssetManager::Get().GetMaterials();
            if (index >= 0 && index < static_cast<int>(materials.size())) {
                mesh->SetMaterial(materials[index]);
            }
        }
    );
}
