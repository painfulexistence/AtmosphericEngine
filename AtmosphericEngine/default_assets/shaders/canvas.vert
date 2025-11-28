#version 410

uniform mat4 Projection;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 color;
layout(location = 3) in int index;

flat out int texIndex;
out vec2 texUV;
out vec4 fragColor;

void main() {
    texIndex = index;
    texUV = uv;
    fragColor = color;
    // Layer is used for sorting on CPU, not needed in shader
    gl_Position = Projection * vec4(position, 0.0, 1.0);
}