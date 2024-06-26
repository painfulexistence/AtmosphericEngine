#version 410

uniform mat4 ProjectionView;
uniform mat4 LightProjectionView;
uniform float time;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 5) in mat4 World;

out vec3 frag_pos;
out vec4 light_space_frag_pos;
out vec3 frag_normal;
out vec2 tex_uv;


float random(vec2 st)
{
    return fract(sin(dot(st, vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = smoothstep(0., 1., f);

    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

vec3 skew(vec3 v)
{
    float x = v.x + (noise(vec2(time / 9.4, time / 2.7)) - 0.5 * cos(time));
    float y = v.y + (noise(vec2(pow(time, 1.1), pow(time, 0.9))) + abs(sin(time))) - abs(v.x * v.z / 100);
    float z = v.z + (noise(vec2(x, y)) - 0.5 * cos(time));
    return vec3(x, y, z);
}

void main()
{
    frag_pos = vec3(World * vec4(position, 1.0));
    frag_normal = mat3(World) * normal;
    tex_uv = uv;
    light_space_frag_pos = LightProjectionView * vec4(frag_pos, 1.0);

    gl_Position = ProjectionView * World * vec4(skew(position), 1.0);
}