#version 410

uniform mat4 ProjectionView;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 5) in mat4 World;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 tex_uv;

void main()
{
    frag_pos = vec3(World * vec4(position, 1.0));
    frag_normal = mat3(World) * normal;
    tex_uv = uv;

    gl_Position = ProjectionView * World * vec4(position, 1.0);
}