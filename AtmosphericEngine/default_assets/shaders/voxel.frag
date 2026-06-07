#version 410 core

in vec3 v_worldPos;
in vec3 v_normal;
in vec2 v_uv;
flat in uint v_voxelId;
flat in uint v_faceId;

uniform vec3  u_lightDir;     // world-space direction TOWARD the light
uniform vec3  u_lightColor;
uniform vec3  u_ambientColor;
uniform vec3  u_fogColor;
uniform float u_fogDensity;
uniform vec3  u_cameraPos;

out vec4 fragColor;

// Fallback palette (index 0 unused; IDs are 1-based)
const vec3 VOXEL_COLORS[9] = vec3[](
    vec3(0.0),               // 0 – air (never drawn)
    vec3(0.45, 0.30, 0.15),  // 1 – dirt
    vec3(0.28, 0.55, 0.18),  // 2 – grass
    vec3(0.50, 0.50, 0.52),  // 3 – stone
    vec3(0.20, 0.38, 0.76),  // 4 – water
    vec3(0.88, 0.78, 0.48),  // 5 – sand
    vec3(0.48, 0.28, 0.08),  // 6 – wood
    vec3(0.14, 0.52, 0.14),  // 7 – leaves
    vec3(0.92, 0.96, 1.00)   // 8 – snow
);

void main() {
    vec3 norm = normalize(v_normal);

    // Directional diffuse
    float diff = max(dot(norm, normalize(u_lightDir)), 0.0);

    // Per-voxel-type base color
    uint id = clamp(v_voxelId, 0u, 8u);
    vec3 baseColor = VOXEL_COLORS[id];

    // Slight top-face brightness boost (AO approximation)
    float topBoost = (v_faceId == 0u) ? 1.0 : (v_faceId == 1u ? 0.6 : 0.85);
    baseColor *= topBoost;

    vec3 ambient = u_ambientColor * baseColor;
    vec3 diffuse = diff * u_lightColor * baseColor;
    vec3 color   = ambient + diffuse;

    // Exponential fog
    float dist      = length(v_worldPos - u_cameraPos);
    float fogFactor = clamp(exp(-u_fogDensity * dist), 0.0, 1.0);
    color = mix(u_fogColor, color, fogFactor);

    fragColor = vec4(color, 1.0);
}
