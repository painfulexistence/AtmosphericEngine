#version 410 core

in vec2 v_uv;
uniform sampler2D u_scene;
uniform sampler2D u_bloom;
uniform float u_bloomStrength;
uniform float u_exposure;

out vec4 fragColor;

vec3 ACESFilm(vec3 x) {
    float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 hdr    = texture(u_scene, v_uv).rgb;
    vec3 bloom  = texture(u_bloom, v_uv).rgb;
    vec3 result = hdr + bloom * u_bloomStrength;
    result      = ACESFilm(result * u_exposure);
    result      = pow(result, vec3(1.0 / 2.2)); // gamma
    fragColor   = vec4(result, 1.0);
}
