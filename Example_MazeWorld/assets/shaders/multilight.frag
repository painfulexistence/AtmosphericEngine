#version 410

#define MAX_NUM_AUX_LIGHTS 6

struct Surface
{
    //Reference: http://devernay.free.fr/cours/opengl/materials.html
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
};

struct PointLight
{
    vec3 position;
    vec3 attenuation;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
};

uniform Surface surf;
uniform DirLight main_light;
uniform PointLight aux_lights[MAX_NUM_AUX_LIGHTS];
uniform int aux_light_count;
uniform vec3 cam_pos;
uniform sampler2D base_map_unit;
uniform float time;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 tex_uv;
out vec4 Color;

const float gamma = 2.2;

vec3 CalculateDirectionalLight(DirLight light)
{
    vec3 norm = normalize(frag_normal);
    vec3 viewDir = normalize(cam_pos - frag_pos);
    vec3 lightDir = normalize(-light.direction);
    vec3 halfway = normalize(lightDir + viewDir);

    vec3 ambient = light.ambient * surf.ambient;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * light.intensity * (diff * surf.diffuse);
    float spec = pow(max(dot(halfway, norm), 0.0), surf.shininess);
    vec3 specular = light.specular * light.intensity * spec * surf.specular;

    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light)
{
    vec3 norm = normalize(frag_normal);
    vec3 viewDir = normalize(cam_pos - frag_pos);
    vec3 lightDir = normalize(light.position - frag_pos);
    vec3 halfway = normalize(lightDir + viewDir);
    vec3 attn = light.attenuation;
    float dist = distance(light.position, frag_pos);
    float attenuation = 1.0 / (attn.x + attn.y * dist + attn.z * dist * dist);
    float intensity = light.intensity;

    vec3 ambient = light.ambient * surf.ambient;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * intensity * diff * surf.diffuse;
    float spec = pow(max(dot(halfway, norm), 0.0), surf.shininess);
    vec3 specular = light.specular * intensity * spec * surf.specular;

    return attenuation * (ambient + diffuse + specular);
}

void main()
{
    vec3 result = vec3(0);
    result += CalculateDirectionalLight(main_light);
    for (int i = 0; i < aux_light_count; ++i)
    {
        result += CalculatePointLight(aux_lights[i]);
    }

    result = mix(result, texture(base_map_unit, tex_uv).xyz, 0);
    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1.0);
}
