#include "text_component.hpp"
#include "application.hpp"
#include "batch_renderer_2d.hpp"
#include "game_object.hpp"
#include "graphics_server.hpp"

TextComponent::TextComponent(GameObject* gameObject, const TextProps& props) : CanvasDrawable(gameObject) {
    _text = props.text;
    _fontPath = props.fontPath;
    _fontID = props.fontID;
    _fontSize = props.fontSize;
    _size = props.size;
    _pivot = props.pivot;
    _color = props.color;
    _hAlign = props.hAlign;
    _vAlign = props.vAlign;
    _layer = props.layer;
    _zOrder = props.zOrder;
}

std::string TextComponent::GetName() const {
    return std::string("TextComponent");
}

void TextComponent::OnAttach() {
    auto* graphics = gameObject->GetApp()->GetGraphicsServer();
    graphics->RegisterCanvasDrawable(this);

    // If fontPath is provided and fontID is not set, load the font
    if (_fontID == 0 && !_fontPath.empty()) {
        // Load at a base size for quality (we'll scale when rendering)
        _fontBaseSize = 48.0f;
        _fontID = graphics->LoadFont(_fontPath, _fontBaseSize);
    }
}

void TextComponent::OnDetach() {
    // Note: We don't unload the font here because it might be shared
    // Font cleanup should be handled by the user or FontManager
}

void TextComponent::Draw(BatchRenderer2D* renderer) {
    if (_text.empty() || _fontID == 0) return;
    if (!gameObject->isActive) return;

    auto* graphics = GraphicsServer::Get();
    if (!graphics) return;

    // Calculate scale factor: desired fontSize / base font size
    float scale = _fontSize / _fontBaseSize;

    // Measure the text to determine its actual size
    glm::vec2 textSize = graphics->MeasureText(_fontID, _text, scale);

    // Get world transform (includes parent transforms)
    glm::mat4 worldTransform = gameObject->GetTransform();
    glm::vec3 worldPos = glm::vec3(worldTransform[3]);

    // Calculate alignment offset within the bounding box
    float alignOffsetX = 0.0f;
    float alignOffsetY = 0.0f;

    switch (_hAlign) {
    case TextHAlignment::Left:
        alignOffsetX = 0.0f;
        break;
    case TextHAlignment::Center:
        alignOffsetX = (_size.x - textSize.x) * 0.5f;
        break;
    case TextHAlignment::Right:
        alignOffsetX = _size.x - textSize.x;
        break;
    }

    switch (_vAlign) {
    case TextVAlignment::Top:
        alignOffsetY = 0.0f;
        break;
    case TextVAlignment::Center:
        alignOffsetY = (_size.y - textSize.y) * 0.5f;
        break;
    case TextVAlignment::Bottom:
        alignOffsetY = _size.y - textSize.y;
        break;
    }

    // Apply pivot offset (pivot is 0,0 = top-left, 1,1 = bottom-right)
    float pivotOffsetX = -_pivot.x * _size.x;
    float pivotOffsetY = -_pivot.y * _size.y;

    // Final text position
    float textX = worldPos.x + pivotOffsetX + alignOffsetX;
    float textY = worldPos.y + pivotOffsetY + alignOffsetY;

    // Get font info for rendering
    Font* font = graphics->MeasureText(_fontID, "", 1.0f).x >= 0 ? nullptr : nullptr;// Dummy call

    // We need to render character by character using the batch renderer
    // Access font data through GraphicsServer's internal font manager
    // Since we can't directly access _fontManager, we use GraphicsServer::RenderBufferedText pattern
    // But for component-based rendering, we need to render directly here.

    // Use GraphicsServer's DrawText which buffers the text command
    // However, that renders in screen space. We need to render at our world position.

    // For simplicity in this demo implementation, we'll queue the text to be rendered
    // Note: This works because CanvasPass calls Draw() then RenderBufferedText()
    graphics->DrawText(_fontID, _text, textX, textY, scale, _color);
}
