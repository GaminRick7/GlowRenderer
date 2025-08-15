#include "lighting/Light.h"

Light::Light(const glm::vec3& pos, const glm::vec3& col, float intens, const std::string& lightName)
    : position(pos), color(col), intensity(intens), name(lightName), isActive(true) {
} 