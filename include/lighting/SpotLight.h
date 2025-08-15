#pragma once
#include "Light.h"

class SpotLight : public Light {
private:
    glm::vec3 direction;
    float innerCutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;

public:
    SpotLight(const glm::vec3& pos, const glm::vec3& dir, const glm::vec3& col, 
              float innerCut, float outerCut, float intens = 1.0f,
              float c = 1.0f, float l = 0.09f, float q = 0.032f,
              const std::string& lightName = "SpotLight");

    // Getters
    glm::vec3 getDirection() const { return direction; }
    float getInnerCutoff() const { return innerCutoff; }
    float getOuterCutoff() const { return outerCutoff; }
    float getConstant() const { return constant; }
    float getLinear() const { return linear; }
    float getQuadratic() const { return quadratic; }

    // Setters
    void setDirection(const glm::vec3& dir) { direction = glm::normalize(dir); }
    void setInnerCutoff(float innerCut) { innerCutoff = innerCut; }
    void setOuterCutoff(float outerCut) { outerCutoff = outerCut; }
    void setConstant(float c) { constant = c; }
    void setLinear(float l) { linear = l; }
    void setQuadratic(float q) { quadratic = q; }

    // Override virtual methods
    void updateShaderUniforms(class Shader& shader, int lightIndex) const override;
    std::string getLightType() const override { return "SpotLight"; }

    // Spotlight specific methods
    glm::vec3 getNormalizedDirection() const { return glm::normalize(direction); }
    float calculateSpotIntensity(const glm::vec3& lightDir) const;
    float calculateAttenuation(float distance) const;
}; 