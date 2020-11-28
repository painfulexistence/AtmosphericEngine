#version 410

uniform mat4 LightProjectionView;

layout(location = 0) in vec3 position;
layout(location = 3) in mat4 World;

out vec3 frag_pos;

void main()
{
    frag_pos = vec3(World * vec4(position, 1.0));

    gl_Position = LightProjectionView * World * vec4(position, 1.0);
}