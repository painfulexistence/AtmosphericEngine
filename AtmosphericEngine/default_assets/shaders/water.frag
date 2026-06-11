#version 410 core

in vec3 v_worldPos;
in vec3 v_normal;
in vec2 v_uv;

uniform vec3  u_lightDir;
uniform vec3  u_lightColor;
uniform vec3  u_cameraPos;
uniform vec3  u_fogColor;
uniform float u_fogDensity;
uniform float u_waterLine;

// VX water colors
const vec3 DEEP_COLOR    = vec3(0.04, 0.11, 0.35); // COLOR_INDIGO
const vec3 SHALLOW_COLOR = vec3(0.686, 0.933, 0.933); // COLOR_MINT_GREEN
const float BEER_COEF    = 0.095; // VX beer_absorption_coef

out vec4 fragColor;

void main() {
    vec3 norm     = normalize(v_normal);
    vec3 viewDir  = normalize(u_cameraPos - v_worldPos);
    vec3 lightDir = normalize(u_lightDir);
    vec3 halfDir  = normalize(lightDir + viewDir);

    // Underwater camera tint (matching VX)
    if (u_cameraPos.y < u_waterLine) {
        float submerge = smoothstep(2.0, 32.0, u_waterLine - u_cameraPos.y);
        vec3 col = mix(SHALLOW_COLOR, DEEP_COLOR, submerge);
        fragColor = vec4(col, 0.9);
        return;
    }

    // Beer-Lambert depth approximation (no depth texture: use UV pattern as proxy)
    float uvDepth    = clamp(sin(v_uv.x * 12.0 + v_uv.y * 10.0) * 0.5 + 0.5, 0.0, 1.0);
    float beerFactor = max(1.0 - exp(-uvDepth * 8.0 * BEER_COEF), 0.0);
    vec3 col = mix(SHALLOW_COLOR, DEEP_COLOR, beerFactor);

    // Diffuse + specular (Blinn-Phong, matching VX)
    float diff    = max(dot(norm, lightDir), 0.0);
    float spec    = pow(max(dot(norm, halfDir), 0.0), 128.0);
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 5.0);

    col = mix(col, col * diff * u_lightColor + vec3(1.0) * spec * u_lightColor, 0.2);
    col = mix(col, vec3(1.0), fresnel * 0.5); // Fresnel highlight

    // Fog
    float dist      = length(v_worldPos - u_cameraPos);
    col = mix(u_fogColor, col, clamp(exp(-u_fogDensity * dist * dist), 0.0, 1.0));

    fragColor = vec4(col, smoothstep(0.1, 0.9, beerFactor + 0.3));
}
