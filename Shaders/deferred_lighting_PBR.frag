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

// Light uniforms - using a more reasonable limit that fits within OpenGL constraints
uniform vec3 lightPositions[64];
uniform vec3 lightColors[64];
uniform int numLights;

// Spotlight uniforms
uniform vec3 spotLightPositions[64];
uniform vec3 spotLightDirections[64];
uniform vec3 spotLightColors[64];
uniform float spotLightInnerCutoffs[64];
uniform float spotLightOuterCutoffs[64];
uniform int numSpotLights;

// Directional light uniforms
uniform vec3 dirLightDirection;
uniform vec3 dirLightColor;
uniform bool hasDirLight;

// PBR constants
const float PI = 3.14159265359;
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Calculate PBR lighting contribution for a given light direction and radiance
vec3 calculatePBRContribution(vec3 L, vec3 radiance, vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0)
{
    vec3 H = normalize(V + L);
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
    vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // Energy conservation
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  

    // Scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // Return outgoing radiance
    return (kD * albedo / PI + specular) * radiance * NdotL;
}
void main() {
    // Read data from G-Buffer
    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 N = texture(gNormal, TexCoord).rgb;
    vec3 albedo = texture(gAlbedo, TexCoord).rgb;
    vec2 MetallicRoughness = texture(gMetallicRoughness, TexCoord).rg;
    float metallic = MetallicRoughness.x;
    float roughness = MetallicRoughness.y;
    float ao = texture(gAO, TexCoord).r;



    // Input lighting data
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-V, N);

    // Calculate reflectance at normal incidence
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

        // Calculate lighting contribution
    vec3 Lo = vec3(0.0);
    
    // Point lights
    for(int i = 0; i < numLights; ++i) 
    {
        vec3 L = normalize(lightPositions[i] - FragPos);
        float distance = length(lightPositions[i] - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;
        
        Lo += calculatePBRContribution(L, radiance, N, V, albedo, metallic, roughness, F0);
    }
    
    // Spotlights
    for(int i = 0; i < numSpotLights; ++i) 
    {
        vec3 L = normalize(spotLightPositions[i] - FragPos);
        float distance = length(spotLightPositions[i] - FragPos);
        float attenuation = 1.0 / (distance * distance);
        
        // Calculate spotlight intensity based on angle
        float theta = dot(-L, spotLightDirections[i]);
        float epsilon = spotLightInnerCutoffs[i] - spotLightOuterCutoffs[i];
        float intensity = clamp((theta - spotLightOuterCutoffs[i]) / epsilon, 0.0, 1.0);
        
        vec3 radiance = spotLightColors[i] * attenuation * intensity;
        
        Lo += calculatePBRContribution(L, radiance, N, V, albedo, metallic, roughness, F0);
    }
    
    // Directional light
    if (hasDirLight) {
        vec3 L = normalize(-dirLightDirection); // Directional light points in the opposite direction
        vec3 radiance = dirLightColor; // No attenuation for directional lights
        
        Lo += calculatePBRContribution(L, radiance, N, V, albedo, metallic, roughness, F0);
    }
    
    // Ambient lighting (note that in PBR we typically use an HDR environment map)
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0/2.2)); 

    vec3 result = vec3(0.0);

    // Loop through all point lights
    for (int i = 0; i < numLights; i++) {
        vec3 lightVector = lightPositions[i] - FragPos;
        float distance = length(lightVector);
        
        // Calculate attenuation using the light's parameters
        float attenuation = 1.0;
        
        
        // Calculate the direction from fragment to light
        vec3 lightDir = normalize(lightVector);
        
        // Calculate diffuse lighting
        float diff = max(dot(N, lightDir), 0.0);
        vec3 diffuse = vec3(0.0);
        
        // Calculate ambient lighting
        vec3 ambient = 0.1 * lightColors[i];
        
        // Calculate Blinn-Phong specular lighting
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(N, halfwayDir), 0.0), 32);
        
        // Get specular intensity - either from texture or default value
        float specularIntensity;
        specularIntensity = 0.5; // Default specular intensity for vertex-colored objects

        vec3 specular = spec * specularIntensity * lightColors[i];
        
        // Add this light's contribution (only lighting, no base color)
        result += (ambient + diffuse + specular) * attenuation;
    }

    FragColor = vec4(color, 1.0);
}