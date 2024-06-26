#version 410

uniform mat4 ProjectionView;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in mat4 World;

uniform sampler2D normal_map_unit;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 tex_uv;

void main()
{
    vec3 T = normalize(vec3(World * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(World * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(World * vec4(normal, 0.0)));
    mat3 TBN = mat3(T, B, N);
    frag_pos = vec3(World * vec4(position, 1.0));
    frag_normal = normalize(TBN * (texture(normal_map_unit, uv).rgb * 2.0 - 1.0)); // FIXME: frag_normal cannot be interpolated like this
    tex_uv = uv;

    gl_Position = ProjectionView * vec4(frag_pos, 1.0);
}

//Note: Uniform location qualifiers only available in version 4.3 or after