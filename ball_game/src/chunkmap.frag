#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPosition;  
in vec2 TexCoord;
in float Height;
in float chunkHeight;
  
uniform vec3 lightPosition; 
uniform vec3 viewPosition;
uniform vec3 lightColor;
uniform sampler2D ourTexture;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(lightPosition - FragPosition);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;
            
    // specular
    float specularStrength = 0.5;

    vec3 viewDirection = normalize(viewPosition - FragPosition);
    vec3 reflectDirection = reflect(-lightDirection, norm);

    float spec = 0.0; 
    if (dot(norm, viewDirection) > 0.0) { // Check if normal is pointing towards view direction
        spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32); // Calculate specular highlight
    }
    vec3 specular = specularStrength * spec * lightColor;

    // Normalize height to range [0, 1]
    float normalizedHeight = (Height + chunkHeight) / (2.0 * chunkHeight);

    // Discrete color bands based on normalized height
    vec3 color;
    if (normalizedHeight < 0.2) {
        color = vec3(0.0, 0.0, 1.0); // Blue for lowest heights
    } else if (normalizedHeight < 0.4) {
        color = vec3(1.0, 1.0, 0.0); // Yellow for medium heights
    } else if (normalizedHeight < 0.6) {
        color = vec3(0.0, 1.0, 0.0); // Green for low heights
    } else if (normalizedHeight < 0.8) {
        color = vec3(0.6, 0.3, 0.0); // Brown for high heights
    } else {
        color = vec3(1.0, 1.0, 1.0); // White for highest heights
    }

    // Determine steepness and apply gray color for steep angles
    vec3 up = vec3(0.0, 1.0, 0.0);
    float steepness = dot(norm, up);
    if (steepness < 0.50 && color != vec3(1.0,1.0,1.0)) { // Adjust the threshold as needed
        color = vec3(0.5, 0.5, 0.5); // Gray for steep angles
    }

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 1.0);
} 