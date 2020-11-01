#version 410

uniform mat4 LightProjectionView;

layout(location = 0) in vec3 position;
layout(location = 3) in mat4 World;

void main()
{
    gl_Position = LightProjectionView * World * vec4(position, 1.0);
}