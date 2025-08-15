#include "lighting/DirectionalLight.h"
#include "rendering/shader.h"

DirectionalLight::DirectionalLight(const glm::vec3& dir, const glm::vec3& col, float intens, const std::string& lightName)
    : Light(glm::vec3(0.0f), col, intens, lightName), direction(glm::normalize(dir)) {
}

void DirectionalLight::updateShaderUniforms(Shader& shader, int lightIndex) const {
    if (!isActive) return;
    
    std::string prefix = "directionalLights[" + std::to_string(lightIndex) + "].";
    shader.setVec3(prefix + "direction", getNormalizedDirection());
    shader.setVec3(prefix + "color", getEffectiveColor());
} 