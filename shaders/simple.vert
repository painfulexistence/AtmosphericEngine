#version 330

uniform mat4 mvp;

in vec3 position;
in vec3 color;
in vec2 uv;
out vec3 flat_color;
out vec2 tex_uv;

void main()
{
    flat_color = color;
    tex_uv = uv;
    gl_Position = mvp * vec4(position, 1.0);
}