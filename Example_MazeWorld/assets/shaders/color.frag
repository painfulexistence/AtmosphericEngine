#version 410

in vec3 fragColor;
out vec4 Color;

void main()
{
    Color = vec4(fragColor, 1);
}
