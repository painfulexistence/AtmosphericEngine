#version 410

struct Surface
{
    //Reference: http://devernay.free.fr/cours/opengl/materials.html
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
};

uniform Surface surf;
uniform Light main_light;
uniform vec3 cam_pos;
uniform sampler2D tex_unit;
uniform float time;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 tex_uv;
out vec4 Color;

const float lightPower = 1.0;
const float gamma = 2.2;


float random(vec2 st)
{
    return fract(sin(dot(st, vec2(12.9898, 78.233))) * 43758.5453);
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

vec2 noise2d(vec2 p)
{
    return vec2(noise(vec2(p.x, p.y)), noise(vec2(p.y, p.x)));
}

vec3 Iceborne(vec3 result)
{
    return vec3(result.x * noise(frag_pos.xy), result.y + noise(frag_pos.yz), result.z / noise(frag_pos.zx));
}

vec3 Childhood(vec3 result)
{
    return vec3(result.x + noise(frag_pos.xy), result.y / noise(frag_pos.yz), result.z / noise(frag_pos.zx));
}

vec2 DistortUV(vec2 uv)
{
    return uv + noise2d(uv + noise2d(uv + noise2d(vec2(frag_pos.x * frag_pos.y, frag_pos.z * frag_pos.y))));
}

void main()
{
    vec3 norm = normalize(frag_normal);
    vec3 viewDir = normalize(cam_pos - frag_pos);
    vec3 lightDir = normalize(-main_light.direction);
    lightDir = mix(lightDir, -normalize(main_light.direction), length(normalize(main_light.direction)));
    vec3 halfway = normalize(lightDir + viewDir);

    float diff = max(dot(norm, lightDir), 0.0);
    tex_uv = DistortUV(tex_uv); // Distort UV
    vec3 surfColor = mix(surf.diffuse, texture(tex_unit, tex_uv).xyz, 0.1);

    vec3 ambient = main_light.ambient * 3.0 * surf.ambient;
    vec3 diffuse = main_light.diffuse * lightPower * (diff * surfColor);
    vec3 specular = main_light.specular * lightPower * pow(max(dot(halfway, norm), 0.0), surf.shininess) * surf.specular;

    vec3 result = ambient + diffuse + specular;
    result = Childhood(result); // Distort color
    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1);
}
