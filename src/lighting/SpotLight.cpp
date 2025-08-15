#include "lighting/SpotLight.h"
#include "rendering/shader.h"
#include <glm/gtc/matrix_transform.hpp>

SpotLight::SpotLight(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& col, 
                     float innerCut, float outerCut, float intens, float c, float l, float q, const std::string& lightName)
    : Light(pos, col, intens, lightName), direction(glm::normalize(dir)), 
      innerCutoff(innerCut), outerCutoff(outerCut), constant(c), linear(l), quadratic(q) {
}

void SpotLight::updateShaderUniforms(Shader& shader, int lightIndex) const {
    if (!isActive) return;
    
    shader.setVec3("spotLights_positions[" + std::to_string(lightIndex) + "]", position);
    shader.setVec3("spotLights_directions[" + std::to_string(lightIndex) + "]", getNormalizedDirection());
    shader.setVec3("spotLights_colors[" + std::to_string(lightIndex) + "]", getEffectiveColor());
    shader.setFloat("spotLights_innerCutoffs[" + std::to_string(lightIndex) + "]", innerCutoff);
    shader.setFloat("spotLights_outerCutoffs[" + std::to_string(lightIndex) + "]", outerCutoff);
    shader.setFloat("spotLights_constants[" + std::to_string(lightIndex) + "]", constant);
    shader.setFloat("spotLights_linears[" + std::to_string(lightIndex) + "]", linear);
    shader.setFloat("spotLights_quadratics[" + std::to_string(lightIndex) + "]", quadratic);
}

float SpotLight::calculateSpotIntensity(const glm::vec3& lightDir) const {
    float theta = glm::dot(lightDir, -getNormalizedDirection());
    float epsilon = innerCutoff - outerCutoff;
    return glm::clamp((theta - outerCutoff) / epsilon, 0.0f, 1.0f);
}

float SpotLight::calculateAttenuation(float distance) const {
    return 1.0f / (constant + linear * distance + quadratic * distance * distance);
} 