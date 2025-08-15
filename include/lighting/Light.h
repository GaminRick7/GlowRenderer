#pragma once
#include <glm/glm.hpp>
#include <string>

class Light {
protected:
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    std::string name;
    bool isActive;

public:
    Light(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f, const std::string& lightName = "Light");
    virtual ~Light() = default;

    // Getters
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getColor() const { return color; }
    float getIntensity() const { return intensity; }
    std::string getName() const { return name; }
    bool getIsActive() const { return isActive; }

    // Setters
    void setPosition(const glm::vec3& pos) { position = pos; }
    void setColor(const glm::vec3& col) { color = col; }
    void setIntensity(float intens) { intensity = intens; }
    void setName(const std::string& lightName) { name = lightName; }
    void setIsActive(bool active) { isActive = active; }

    // Virtual methods that derived classes must implement
    virtual void updateShaderUniforms(class Shader& shader, int lightIndex) const = 0;
    virtual std::string getLightType() const = 0;
    
    // Common utility methods
    glm::vec3 getEffectiveColor() const { return color * intensity; }
}; 