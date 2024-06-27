#version 410

#define MAX_NUM_AUX_LIGHTS 6
#define SHADOW_KERNEL_LEVEL 1
struct Surface
{
    //Phong model (reference: http://devernay.free.fr/cours/opengl/materials.html)
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    //PBR model
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float intensity;
    int cast_shadow;
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
    int cast_shadow;
    mat4 ProjectionViews[6];
};

uniform Surface surf;
uniform DirLight main_light;
uniform PointLight aux_lights[MAX_NUM_AUX_LIGHTS];
uniform int aux_light_count;
uniform vec3 cam_pos;
uniform sampler2D base_map_unit;
uniform sampler2D normal_map_unit;
uniform sampler2D shadow_map_unit;
uniform samplerCube omni_shadow_map_unit;
uniform float time;

in vec3 frag_pos;
in vec2 tex_uv;
in mat3 TBN;
out vec4 Color;

const float PI = 3.1415927;
const float gamma = 2.2;

vec3 BlinnPhongBRDF(vec3 norm, vec3 lightDir, vec3 viewDir);

vec3 CookTorranceBRDF(vec3 norm, vec3 lightDir, vec3 viewDir);

float TrowbridgeReitzGGX(float nh, float r);

float SmithsSchlickGGX(float nv, float nl, float r);

vec3 FresnelSchlick(float nh, vec3 f0);

vec3 SurfaceColor(vec3 base);

float ShadowBias(vec3 norm, vec3 lightDir);

float DirectionalShadow(vec3 shadowCoords, float bias);

float PointShadow(vec3 shadowCoords, float bias);

vec3 CalculateDirectionalLight(DirLight light, vec3 norm, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    vec4 lightSpaceFragPos = light.ProjectionView * vec4(frag_pos, 1.0);
    vec3 lightSpaceFragCoords = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
    float shadow = light.cast_shadow * DirectionalShadow(lightSpaceFragCoords * 0.5 + 0.5, ShadowBias(norm, lightDir));
    vec3 radiance = light.diffuse * clamp(1.0 - shadow, 0.0, 1.0);

    return CookTorranceBRDF(norm, lightDir, viewDir) * radiance * clamp(dot(norm, lightDir), 0.0, 1.0);
}

vec3 CalculatePointLight(PointLight light, vec3 norm, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - frag_pos);

    float dist = distance(light.position, frag_pos);
    float attenuation = 1.0 / (dist * dist);
    float shadow = light.cast_shadow * PointShadow((frag_pos - light.position) / 400.0f, ShadowBias(norm, lightDir));
    vec3 radiance = attenuation * light.diffuse * clamp(1.0 - shadow, 0.0, 1.0);

    return CookTorranceBRDF(norm, lightDir, viewDir) * radiance * clamp(dot(norm, lightDir), 0.0, 1.0);
}

vec3 BlinnPhongBRDF(vec3 norm, vec3 lightDir, vec3 viewDir)
{
    // NOTES: the light.specular/light.diffuse term was extracted out to render equation
    vec3 halfway = normalize(lightDir + viewDir);
    float nl = clamp(dot(norm, lightDir), 0.0, 1.0);
    float nh = clamp(dot(norm, halfway), 0.0, 1.0);
    vec3 diffuse = nl * SurfaceColor(surf.diffuse);
    vec3 specular = pow(nh, surf.shininess) * smoothstep(0.0, 0.2, dot(norm, lightDir)) * surf.specular;
    return diffuse + specular;
}

vec3 CookTorranceBRDF(vec3 norm, vec3 lightDir, vec3 viewDir)
{
    vec3 halfway = normalize(lightDir + viewDir);
    float nv = clamp(dot(norm, viewDir), 0.0, 1.0);
    float nl = clamp(dot(norm, lightDir), 0.0, 1.0);
    float nh = clamp(dot(norm, halfway), 0.0, 1.0);
    float D = TrowbridgeReitzGGX(nh, surf.roughness + 0.01);
    float G = SmithsSchlickGGX(nv, nl, surf.roughness + 0.01);
    vec3 surfColor = SurfaceColor(surf.albedo);
    vec3 F = FresnelSchlick(nh, mix(vec3(0.04), surfColor, surf.metallic));
    vec3 specular = D * F * G / max(4.0 * nv * nl, 0.0001);
    vec3 kd = (1.0 - surf.metallic) * (vec3(1.0) - F);
    vec3 diffuse = kd * surfColor / PI;
    return diffuse + specular;
}

float TrowbridgeReitzGGX(float nh, float r)
{
    float r2 = r * r;
    float nh2 = nh * nh;
    float nhr2 = (nh2 * (r2 - 1) + 1) * (nh2 * (r2 - 1) + 1);
    return r2 / (PI * nhr2);
}

float SmithsSchlickGGX(float nv, float nl, float r)
{
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    float ggx1 = nv / (nv * (1.0 - k) + k);
    float ggx2 = nl / (nl * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float nh, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - nh, 5.0);
}

vec3 SurfaceColor(vec3 base)
{
    vec3 texColor = pow(texture(base_map_unit, tex_uv).rgb, vec3(gamma));
    return base * texColor;
}

float ShadowBias(vec3 norm, vec3 lightDir)
{
    return 0.0025 * (1.0 - abs(1.0 - 2.0 * abs(dot(norm, lightDir)))); //maximize when dot(norm, lightDir) = 0.5
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

void main()
{
    vec3 texNorm = texture(normal_map_unit, tex_uv).rgb * 2.0 - 1.0;
    vec3 norm = normalize(TBN * texNorm);
    vec3 viewDir = normalize(cam_pos - frag_pos);

    vec3 result = vec3(0.0);
    result += CalculateDirectionalLight(main_light, norm, viewDir);
    result += CalculatePointLight(aux_lights[0], norm, viewDir);
    result += CalculatePointLight(aux_lights[1], norm, viewDir);
    result += CalculatePointLight(aux_lights[2], norm, viewDir);
    result += CalculatePointLight(aux_lights[3], norm, viewDir);
    result += vec3(0.2) * (1.0 - surf.ao) * SurfaceColor(surf.albedo);

    result = pow(result, vec3(1.0 / gamma));

    Color = vec4(result, 1.0);
}
