#version 410

flat in int texIndex;
in vec2 texUV;
in vec4 fragColor;

out vec4 Color;

uniform sampler2D Textures[32];

void main() {
    // TODO: use texture index
    vec4 texColor = texture(Textures[0], texUV);
    Color = vec4(texColor.rgb * fragColor.rgb, texColor.a);
}