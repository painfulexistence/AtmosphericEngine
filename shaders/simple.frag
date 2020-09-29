#version 330

uniform sampler2D tex;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 light_dir;
uniform vec3 view_pos;

in vec3 frag_pos;
in vec3 frag_normal;
in vec3 flat_color;
in vec2 tex_uv;
out vec4 Color;

void main()
{   
    vec3 final_light_dir = (1 - length(light_dir)) * normalize(light_pos - frag_pos) + length(light_dir) * light_dir;

    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * light_color;

    float diff = max(dot(frag_normal, final_light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    float specularStrength = 0.5;
    int shininess = 16;
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-final_light_dir, frag_normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
    vec3 specular = specularStrength * spec * light_dir;

    vec3 result = vec3(texture(tex, tex_uv)) * (ambient + diffuse + specular) * flat_color;
    Color = vec4(1- result.x, 1- result.y, 1- result.z, 1);
}