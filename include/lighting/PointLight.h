#pragma once
#include "Light.h"

class PointLight : public Light {
private:
    float constant;
    float linear;
    float quadratic;

public:
    PointLight(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f, 
               float c = 1.0f, float l = 0.09f, float q = 0.032f, 
               const std::string& lightName = "PointLight");

    // Getters for attenuation parameters
    float getConstant() const { return constant; }
    float getLinear() const { return linear; }
    float getQuadratic() const { return quadratic; }

    // Setters for attenuation parameters
    void setConstant(float c) { constant = c; }
    void setLinear(float l) { linear = l; }
    void setQuadratic(float q) { quadratic = q; }

    // Override virtual methods
    void updateShaderUniforms(class Shader& shader, int lightIndex) const override;
    std::string getLightType() const override { return "PointLight"; }

    // Point light specific methods
    float calculateAttenuation(float distance) const;
}; 