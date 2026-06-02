#pragma once
#include <cstdint>
#include <glm/glm.hpp>

// Abstract offscreen render target (framebuffer + color/depth attachments).
// Each graphics backend provides a concrete implementation.
//
// Usage pattern:
//   target->Begin();
//   target->Clear({0, 0, 0, 1});
//   // ... draw calls ...
//   target->End();
//   uint32_t tex = target->GetTextureID(); // use as texture in next pass
class IGPURenderTarget {
public:
    virtual ~IGPURenderTarget() = default;

    // Bind this target and save the previously bound target.
    virtual void Begin() = 0;
    // Restore the previously saved target.
    virtual void End() = 0;
    // Clear color, and depth/stencil if the target was created with them.
    virtual void Clear(const glm::vec4& color = glm::vec4(0.0f)) = 0;

    // Color attachment as a backend texture handle cast to uint32_t.
    // For OpenGL this is a GLuint texture ID, usable with glBindTexture.
    virtual uint32_t GetTextureID() const = 0;
    // Depth attachment handle, or 0 if not created with a depth attachment.
    virtual uint32_t GetDepthTextureID() const = 0;

    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;
    virtual glm::vec2 GetSize() const = 0;

    virtual bool IsValid() const = 0;

    // Recreate internal resources at the new dimensions.
    virtual void Resize(int width, int height) = 0;
};
