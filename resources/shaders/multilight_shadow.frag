#version 410

#define MAX_NUM_AUX_LIGHTS 6
#define SHADOW_KERNEL_LEVEL 1
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
    mat4 ProjectionView;
};

struct PointLight
{
    vec3 position;
    vec3 attenuation;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
    mat4 ProjectionViews[6];
};


uniform Surface surf;
uniform DirLight main_light;
uniform PointLight aux_lights[MAX_NUM_AUX_LIGHTS];
uniform int aux_light_count;
uniform vec3 cam_pos;
uniform sampler2D tex_unit;
uniform sampler2D shadow_map_unit;
uniform samplerCube omni_shadow_map_unit;
uniform float time;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 tex_uv;
out vec4 Color;

const float gamma = 2.2;

vec3 SurfDiffuse(vec3 surfColor)
{
    vec3 texColor = pow(texture(tex_unit, tex_uv).rgb, vec3(gamma));
    return mix(surfColor, texColor, 0.9);
}

float ShadowBias(vec3 norm, vec3 lightDir)
{
    return 0.0025 * (1.0 - abs(1.0 - 2.0 * abs(dot(norm, lightDir))));
}

float DirectionalShadow(vec3 shadowCoords, float bias)
{  
    float depth = shadowCoords.z;
    
    float shadow = 0.0;
    if (depth <= 1.0) 
    {
        int samples = 0;
        float texelSize = 1.0 / textureSize(shadow_map_unit, 0).x;
        for(int dx = -SHADOW_KERNEL_LEVEL; dx <= SHADOW_KERNEL_LEVEL; ++dx)
        {
            for(int dy = -SHADOW_KERNEL_LEVEL; dy <= SHADOW_KERNEL_LEVEL; ++dy)
            {
                vec2 samplePoint = shadowCoords.xy + texelSize * vec2(dx, dy);
                shadow += (depth - bias >= texture(shadow_map_unit, samplePoint).r) ? 1.0 : 0.0;
                ++samples;
            }
        }
        shadow /= samples;
    }
    return shadow;
}

float PointShadow(vec3 shadowCoords, float bias)
{
    float depth = length(shadowCoords);
    
    float shadow = 0.0;
    if (depth <= 1.0) 
    {
        int samples = 0;
        float texelSize = 1.0 / textureSize(omni_shadow_map_unit, 0).x;
        for(int dx = -SHADOW_KERNEL_LEVEL; dx <= SHADOW_KERNEL_LEVEL; ++dx)
        {
            for(int dy = -SHADOW_KERNEL_LEVEL; dy <= SHADOW_KERNEL_LEVEL; ++dy)
            {
                for(int dz = -SHADOW_KERNEL_LEVEL; dz <= SHADOW_KERNEL_LEVEL; ++dz)
                {
                    vec3 samplePoint = shadowCoords + texelSize * vec3(dx, dy, dz);
                    shadow += (depth - bias >= texture(omni_shadow_map_unit, samplePoint).r) ? 1.0 : 0.0;
                    ++samples;
                }
            }
        }
        shadow /= samples;
    } 
    return shadow;
}

vec3 CalculateDirectionalLight(DirLight light, bool shadowCasting)
{
    vec3 norm = normalize(frag_normal);
    vec3 viewDir = normalize(cam_pos - frag_pos);
    vec3 lightDir = normalize(-light.direction);
    vec3 halfway = normalize(lightDir + viewDir);
    vec4 lightSpaceFragPos = light.ProjectionView * vec4(frag_pos, 1.0);
    vec3 lightSpaceFragCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;  
    
    float shadow = shadowCasting ? DirectionalShadow(lightSpaceFragCoords * 0.5 + 0.5, ShadowBias(norm, lightDir)) : 0.0;
    float intensity = light.intensity;

    vec3 ambient = light.ambient * surf.ambient;
    float diff = clamp(dot(norm, lightDir), 0.0, 1.0);
    vec3 diffuse = light.diffuse * diff * SurfDiffuse(surf.diffuse);
    float spec = pow(clamp(dot(halfway, norm), 0.0, 1.0), surf.shininess) * smoothstep(0.0, 0.2, dot(norm, lightDir));
    vec3 specular = light.specular * spec * surf.specular;

    return ambient + intensity * clamp(1.0 - shadow, 0.0, 1.0) * (diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, bool shadowCasting)
{
    vec3 norm = normalize(frag_normal);
    vec3 viewDir = normalize(cam_pos - frag_pos);
    vec3 lightDir = normalize(light.position - frag_pos);
    vec3 halfway = normalize(lightDir + viewDir);
    
    vec3 attn = light.attenuation;
    float dist = distance(light.position, frag_pos);
    float attenuation = 1.0 / (attn.x + attn.y * dist + attn.z * dist * dist);

    float shadow = shadowCasting ? PointShadow((frag_pos - light.position) / 400.0f, ShadowBias(norm, lightDir)) : 0.0;
    float intensity = light.intensity;

    vec3 ambient = light.ambient * surf.ambient;
    float diff = clamp(dot(norm, lightDir), 0.0, 1.0);
    vec3 diffuse = light.diffuse * diff * SurfDiffuse(surf.diffuse);
    float spec = pow(clamp(dot(halfway, norm), 0.0, 1.0), surf.shininess) * smoothstep(0.0, 0.2, dot(norm, lightDir));
    vec3 specular = light.specular * spec * surf.specular;
    
    return attenuation * (ambient + intensity * clamp(1.0 - shadow, 0.0, 1.0) * (diffuse + specular));
}

void main()
{   
    vec3 result = vec3(0.0);
    result += CalculateDirectionalLight(main_light, true);
    result += CalculatePointLight(aux_lights[0], true);
    result += CalculatePointLight(aux_lights[1], false);
    result += CalculatePointLight(aux_lights[2], false);
    result += CalculatePointLight(aux_lights[3], false);
    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1.0);
}
