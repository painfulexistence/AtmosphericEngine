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
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Surface surf;
uniform Light light;
uniform vec3 cam_pos;
uniform sampler2D tex;
uniform float time;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 tex_uv;
out vec4 Color;


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

void main()
{   
    vec2 uv = mix(tex_uv, vec2(noise(vec2(frag_pos.x * frag_pos.y, frag_pos.z * frag_pos.y)), noise(vec2(frag_pos.z * frag_pos.x, noise(frag_pos.zy)))), 0.1);

    vec3 norm = normalize(frag_normal);
    vec3 viewDir = normalize(cam_pos - frag_pos);
    vec3 lightDir = normalize(light.position - frag_pos);
    // Light type switch (when length(normalize(light.direction)) = 1, it means directional light)
    //lightDir = mix(lightDir, -normalize(light.direction), length(normalize(light.direction)));
    vec3 reflectDir = reflect(-lightDir, norm);

    vec3 ambient = light.ambient * surf.ambient;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * surf.diffuse);
    vec3 specular = light.specular * pow(max(dot(viewDir, reflectDir), 0.0), surf.shininess * 128) * surf.specular;

    vec3 result = mix((ambient + diffuse + specular), texture(tex, uv).xyz, 0.1);
    Color = vec4(result.x, result.y, result.z, 1);
}
