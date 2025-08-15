#version 410 core

// Input from vertex shader
in vec2 TexCoord;

// Output final color
out vec4 FragColor;

// G-Buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMetallicRoughness;
uniform sampler2D gAO;

// Camera position
uniform vec3 viewPos;

// Light properties
uniform vec3 lightPositions[2];
uniform vec3 lightColors[2];
uniform int numLights;

void main() {
    // Read data from G-Buffer
    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 Normal = texture(gNormal, TexCoord).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoord).rgb;
    vec2 MetallicRoughness = texture(gMetallicRoughness, TexCoord).rg;
    float Metallic = MetallicRoughness.x;
    float Roughness = MetallicRoughness.y;
    float AO = texture(gAO, TexCoord).r;
    
    // Calculate lighting for each light
    vec3 lighting = vec3(0.0);
    
    for(int i = 0; i < numLights; i++) {
        // Calculate light direction and distance
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        float distance = length(lightPositions[i] - FragPos);
        
        // Simple attenuation
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        // Diffuse lighting
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColors[i] * Albedo;
        
        // Add to total lighting
        lighting += (diffuse) * attenuation;
    }
    
    // Output final color
    FragColor = vec4(lighting, 1.0);
}