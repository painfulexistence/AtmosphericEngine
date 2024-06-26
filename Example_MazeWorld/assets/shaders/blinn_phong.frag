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
uniform sampler2D base_map_unit;
uniform float time;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 tex_uv;
out vec4 Color;

const float lightPower = 1.0;
const float gamma = 2.2;

void main()
{
    vec3 norm = normalize(frag_normal);
    vec3 viewDir = normalize(cam_pos - frag_pos);
    vec3 lightDir = normalize(-main_light.direction);
    vec3 halfway = normalize(lightDir + viewDir);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 surfColor = mix(surf.diffuse, texture(base_map_unit, tex_uv).xyz, 0.5);

    vec3 ambient = main_light.ambient * surf.ambient;
    vec3 diffuse = main_light.diffuse * lightPower * (diff * surfColor);
    vec3 specular = main_light.specular * lightPower * pow(max(dot(halfway, norm), 0.0), surf.shininess) * surf.specular;

    vec3 result = ambient + diffuse + specular;
    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1.0);
}
