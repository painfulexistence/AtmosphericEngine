#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

uniform mat4 u_model;
uniform mat4 u_viewProj;
uniform float u_time;

out vec3 v_worldPos;
out vec3 v_normal;
out vec2 v_uv;

void main() {
    vec3 pos = aPos;
    // Simple sine wave displacement
    pos.y += sin(pos.x * 0.3 + u_time * 1.2) * 0.25
           + sin(pos.z * 0.4 + u_time * 0.9) * 0.20;

    v_worldPos = vec3(u_model * vec4(pos, 1.0));
    v_normal   = vec3(0.0, 1.0, 0.0); // simplified; could derive from displacement
    v_uv       = aUV + vec2(u_time * 0.02, u_time * 0.015);

    gl_Position = u_viewProj * vec4(v_worldPos, 1.0);
}
