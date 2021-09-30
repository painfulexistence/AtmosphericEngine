#version 410

uniform sampler2D color_map_unit;
uniform float exposure = 1.0;

in vec2 tex_uv;
out vec4 Color;

const float gamma = 2.2;

vec3 ReinhardToneMap(vec3 hdrColor)
{
    hdrColor *= exposure;
    return hdrColor / (hdrColor + vec3(1.0));
}

vec3 ExponentialToneMap(vec3 hdrColor)
{
    hdrColor *= exposure;
    return vec3(1.0) - exp(-hdrColor);
}

vec3 HableToneMap(vec3 hdrColor)
{
    hdrColor *= exposure;
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    vec3 color = ((hdrColor * (A * hdrColor + C * B) + D * E) / (hdrColor * (A * hdrColor + B) + D * F)) - E / F;
    vec3 whiteScale = vec3(1.0) / (((vec3(11.2) * (A * vec3(11.2) + C * B) + D * E) / (vec3(11.2) * (A * vec3(11.2) + B) + D * F)) - E / F);
    return color * whiteScale;
}

vec3 ACESFilmicToneMap(vec3 hdrColor)
{
    hdrColor *= exposure;
    float A = 2.51;
    float B = 0.03;
    float C = 2.43;
    float D = 0.59;
    float E = 0.14;
    return clamp((hdrColor * (A * hdrColor + B)) / (hdrColor * (C * hdrColor + D) + E), 0.0, 1.0);
}

void main()
{
    vec3 hdrColor = pow(texture(color_map_unit, tex_uv).rgb, vec3(gamma));
    vec3 result = ExponentialToneMap(hdrColor);
    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1.0);
}