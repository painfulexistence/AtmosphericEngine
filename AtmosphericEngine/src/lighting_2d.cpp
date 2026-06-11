#include "lighting_2d.hpp"
#include "graphics_server.hpp"
#include <algorithm>
#include <cmath>

void LightingSystem2D::Apply(GraphicsServer* gfx, int screenW, int screenH) const {
    // ---------------------------------------------------------------------------
    // 1. Dark ambient overlay
    // ---------------------------------------------------------------------------
    // Darkness = 1 - ambient brightness (use max channel so even coloured ambient
    // keeps some visibility)
    float ambientBrightness = std::max({ambientR * ambientA,
                                        ambientG * ambientA,
                                        ambientB * ambientA});
    float darkness = std::max(0.0f, 1.0f - ambientBrightness * 1.5f);

    if (darkness > 0.01f) {
        gfx->DrawQuad(0, 0, (float)screenW, (float)screenH, 0.0f,
                      glm::vec4(ambientR * 0.05f,
                                ambientG * 0.05f,
                                ambientB * 0.05f,
                                darkness));
    }

    // ---------------------------------------------------------------------------
    // 2. Soft light circles per point-light
    // ---------------------------------------------------------------------------
    // We approximate point lights by drawing a set of concentric translucent
    // quads at decreasing radii, blending from the light colour outward to
    // transparent.  This is far cheaper than a GPU post-process but gives a
    // convincing warm-glow halo effect.
    static const int RINGS = 6;

    for (const auto& light : lights) {
        float clampedIntensity = std::min(light.intensity, 2.0f);

        for (int k = 0; k < RINGS; k++) {
            float t    = (float)k / (float)RINGS;          // 0 = centre, 1 = edge
            float rad  = light.radius * (1.0f - t * 0.7f); // inner rings smaller
            float alpha = clampedIntensity * (1.0f - t) * 0.18f;
            if (alpha < 0.005f) continue;

            // Draw as a square approximation (circle drawing via glm is costly)
            float hw = rad;
            float hh = rad;
            gfx->DrawQuad(
                light.x - hw, light.y - hh,
                hw * 2.0f, hh * 2.0f,
                0.0f,
                glm::vec4(light.r, light.g, light.b, alpha)
            );
        }
    }
}
