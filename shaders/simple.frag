#version 330

uniform sampler2D tex;
uniform vec3 light_color;

in vec3 flat_color;
in vec2 tex_uv;
out vec4 Color;

void main()
{
    Color = 0.5 * texture(tex, tex_uv) * vec4(flat_color, 1.0);
}