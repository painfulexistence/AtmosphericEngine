#version 410

uniform mat4 ProjectionView;
uniform mat4 World;

layout(location = 0) in vec3 position;

void main()
{
    vec3 frag_pos = vec3(World * vec4(position, 1.0));
    gl_Position = ProjectionView * vec4(frag_pos, 1.0);
}