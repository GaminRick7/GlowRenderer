#pragma once
#include "Light.h"

class DirectionalLight : public Light {
private:
    glm::vec3 direction;

public:
    DirectionalLight(const glm::vec3& dir, const glm::vec3& col, float intens = 1.0f, 
                     const std::string& lightName = "DirectionalLight");

    // Getters and setters for direction
    glm::vec3 getDirection() const { return direction; }
    void setDirection(const glm::vec3& dir) { direction = glm::normalize(dir); }

    // Override virtual methods
    void updateShaderUniforms(class Shader& shader, int lightIndex) const override;
    std::string getLightType() const override { return "DirectionalLight"; }

    // Directional light specific methods
    glm::vec3 getNormalizedDirection() const { return glm::normalize(direction); }
}; 