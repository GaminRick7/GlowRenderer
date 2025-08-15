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
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoord).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}

void main() {
    // Output 1: World position
    gPosition = vec4(FragPos, 1.0);
    gAlbedo = vec4(pow(texture(albedoMap, TexCoord).rgb, vec3(2.2)), 1.0); // Gamma correction

    float ao = texture(aoMap, TexCoord).r;
    
    // Output 2: World normal (with normal mapping)
    vec3 N;
    if (textureSize(normalMap, 0).x > 1) {
        N = getNormalFromMap();
    } else {
        N = normalize(Normal);
    }
    gNormal = vec4(N, 0.0);
    float metallic = texture(metallicMap, TexCoord).r;
    float roughness = texture(roughnessMap, TexCoord).r;
    gMetallicRoughness = vec4(metallic, roughness, 0.0, 1.0);
    gAO = vec4(ao, ao, ao, 1.0);
}