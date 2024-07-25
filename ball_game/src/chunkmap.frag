#version 410 core

//in vec2 TexCoord;
//in vec3 Normal;
//in vec3 Position;

out vec4 FragColor;

//uniform sampler2D texture_sampler;
//uniform vec3 camera_position;

void main() {
    //vec3 normal = normalize(Normal);
    //vec3 view_dir = normalize(camera_position - Position);

    //vec4 tex_color = texture(texture_sampler, TexCoord);
    //vec3 ambient = vec3(0.1) * tex_color.xyz;
    //vec3 diffuse = max(dot(normal, view_dir), 0.0) * tex_color.xyz;

    FragColor = vec4(1.0,1.0,1.0,1.0);
}