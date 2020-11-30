#version 410

uniform sampler2D color_map_unit;
uniform float exposure = 0.5;

in vec2 tex_uv;
out vec4 Color;

const float gamma = 2.2;

vec3 CalculateToneMapping(vec3 hdrColor)
{
    //hdrColor = hdrColor / (hdrColor + vec3(1.0)); //Reinhard
    return vec3(1.0) - exp(-hdrColor * exposure);
}

void main()
{
    vec3 hdrCcolor = pow(texture(color_map_unit, tex_uv).rgb, vec3(2.2));
    vec3 result = CalculateToneMapping(hdrCcolor);
    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1.0);
}