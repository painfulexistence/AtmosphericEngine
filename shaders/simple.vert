#version 410

uniform mat4 PV;
uniform mat4 M;

in vec3 position;
in vec3 normal;
in vec2 uv;
out vec3 frag_pos;
out vec3 frag_normal;
out vec3 flat_color;
out vec2 tex_uv;

void main()
{
    flat_color = vec3(1, 1, 1);
    frag_normal = mat3(M) * normal;
    frag_pos = vec3(M * vec4(position, 1.0));
    tex_uv = uv;
    gl_Position = PV * M * vec4(position, 1.0);
}