#include "lighting/PointLight.h"
#include "rendering/shader.h"

PointLight::PointLight(const glm::vec3& pos, const glm::vec3& col, float intens, 
                       float c, float l, float q, const std::string& lightName)
    : Light(pos, col, intens, lightName), constant(c), linear(l), quadratic(q) {
}

void PointLight::updateShaderUniforms(Shader& shader, int lightIndex) const {
    if (!isActive) return;
    
    shader.setVec3("pointLights_positions[" + std::to_string(lightIndex) + "]", position);
    shader.setVec3("pointLights_colors[" + std::to_string(lightIndex) + "]", getEffectiveColor());
    shader.setFloat("pointLights_constants[" + std::to_string(lightIndex) + "]", constant);
    shader.setFloat("pointLights_linears[" + std::to_string(lightIndex) + "]", linear);
    shader.setFloat("pointLights_quadratics[" + std::to_string(lightIndex) + "]", quadratic);
}

float PointLight::calculateAttenuation(float distance) const {
    return 1.0f / (constant + linear * distance + quadratic * distance * distance);
} 