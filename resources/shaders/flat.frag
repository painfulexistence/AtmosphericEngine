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


void main()
{
    Color = vec4(0.9, 0.9, 0.9, 1);
}   
