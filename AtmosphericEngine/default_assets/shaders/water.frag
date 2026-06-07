#version 410 core

in vec3 v_worldPos;
in vec3 v_normal;
in vec2 v_uv;

uniform vec3  u_lightDir;
uniform vec3  u_lightColor;
uniform vec3  u_cameraPos;
uniform vec3  u_fogColor;
uniform float u_fogDensity;

out vec4 fragColor;

void main() {
    vec3 norm     = normalize(v_normal);
    vec3 viewDir  = normalize(u_cameraPos - v_worldPos);
    vec3 lightDir = normalize(u_lightDir);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);

    // Specular (Blinn-Phong)
    vec3  halfVec = normalize(lightDir + viewDir);
    float spec    = pow(max(dot(norm, halfVec), 0.0), 128.0);

    // Deep/shallow water color blend from UV (simulated depth)
    vec3 deepColor    = vec3(0.05, 0.15, 0.45);
    vec3 shallowColor = vec3(0.18, 0.52, 0.72);
    float depth       = clamp(sin(v_uv.x * 6.0 + v_uv.y * 5.0) * 0.5 + 0.5, 0.0, 1.0);
    vec3 baseColor    = mix(deepColor, shallowColor, depth * 0.4);

    vec3 color = baseColor * (0.2 + diff * 0.6) * u_lightColor
               + vec3(1.0) * spec * 0.8;

    // Fog
    float dist      = length(v_worldPos - u_cameraPos);
    float fogFactor = clamp(exp(-u_fogDensity * dist), 0.0, 1.0);
    color = mix(u_fogColor, color, fogFactor);

    fragColor = vec4(color, 0.82);
}
