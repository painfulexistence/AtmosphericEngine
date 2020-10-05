#version 410

uniform mat4 ProjectionView;
uniform mat4 World;
uniform float time;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 tex_uv;

void main()
{
    frag_normal = mat3(World) * normal;
    frag_pos = vec3(World * vec4(position, 1.0));
    tex_uv = uv;
    gl_Position = ProjectionView * World * vec4(position, 1.0);
}

//Note: Uniform location qualifiers only available in version 4.3 or after