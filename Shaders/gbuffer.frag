#version 410 core

// Inputs from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

// Outputs to multiple render targets
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gMetallicRoughness;
layout (location = 4) out vec4 gAO;

// Texture samplers - using the names that Mesh class sets
uniform sampler2D diffuse0;  // Albedo texture
uniform sampler2D specular0; // Roughness texture (using specular slot)
uniform sampler2D normal0;   // Normal map

void main() {
    // Output 1: World position
    gPosition = vec4(FragPos, 1.0);
    
    // Output 2: World normal (with normal mapping)
    vec3 N = normalize(Normal);
    if (texture(normal0, TexCoord).a > 0.1) {
        // Use the TBN matrix from vertex shader for normal mapping
        N = normalize(TBN * (texture(normal0, TexCoord).rgb * 2.0 - 1.0));
    }
    gNormal = vec4(N, 1.0);
    
    // Output 3: Albedo color
    gAlbedo = texture(diffuse0, TexCoord);
    
    // Output 4: Metallic and roughness
    // For now, use roughness from specular texture and set metallic to 0.5
    float metallic = 0.5; // Default metallic value
    float roughness = texture(specular0, TexCoord).r;
    gMetallicRoughness = vec4(metallic, roughness, 0.0, 1.0);
    gAO = vec4(1.0, 1.0, 1.0, 1.0);
}