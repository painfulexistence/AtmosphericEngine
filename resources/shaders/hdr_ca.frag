#version 410

uniform sampler2D color_map_unit;
uniform float exposure = 1.0;

in vec2 tex_uv;
out vec4 Color;

const float gamma = 2.2;

vec3 GetPixel(vec2 uv)
{
    float u = uv.x;
    float v = uv.y;
    float du = (u - 0.5) * (u - 0.5) * 0.01;
    float dv = (v - 0.5) * (v - 0.5) * 0.01;

    float r = texture(color_map_unit, vec2(u - 2 * du, v + 4 * dv)).r;
    float g = texture(color_map_unit, vec2(u + 1 * du, v - 1 * dv)).g;
    float b = texture(color_map_unit, vec2(u + 5 * du, v - 3 * dv)).b;
    return vec3(r, g, b);
}

vec3 CalculateToneMapping(vec3 hdrColor)
{
    return vec3(1.0) - exp(-hdrColor * exposure);
}

void main()
{
    vec3 hdrColor = GetPixel(tex_uv);
    hdrColor = pow(hdrColor, vec3(gamma));
    vec3 result = CalculateToneMapping(hdrColor);
    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1.0);
}