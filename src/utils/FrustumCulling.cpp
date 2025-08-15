#include "utils/FrustumCulling.h"
#include <algorithm>
#include <limits>

// Frustum implementation
void Frustum::extractPlanes(const glm::mat4& viewProjection) {
    // Extract the combined view-projection matrix
    const glm::mat4& m = viewProjection;
    
    // Left plane: row 3 + row 0
    planes[LEFT].x = m[0][3] + m[0][0];
    planes[LEFT].y = m[1][3] + m[1][0];
    planes[LEFT].z = m[2][3] + m[2][0];
    planes[LEFT].w = m[3][3] + m[3][0];
    
    // Right plane: row 3 - row 0
    planes[RIGHT].x = m[0][3] - m[0][0];
    planes[RIGHT].y = m[1][3] - m[1][0];
    planes[RIGHT].z = m[2][3] - m[2][0];
    planes[RIGHT].w = m[3][3] - m[3][0];
    
    // Bottom plane: row 3 + row 1
    planes[BOTTOM].x = m[0][3] + m[0][1];
    planes[BOTTOM].y = m[1][3] + m[1][1];
    planes[BOTTOM].z = m[2][3] + m[2][1];
    planes[BOTTOM].w = m[3][3] + m[3][1];
    
    // Top plane: row 3 - row 1
    planes[TOP].x = m[0][3] - m[0][1];
    planes[TOP].y = m[1][3] - m[1][1];
    planes[TOP].z = m[2][3] - m[2][1];
    planes[TOP].w = m[3][3] - m[3][1];
    
    // Near plane: row 3 + row 2
    planes[NEAR].x = m[0][3] + m[0][2];
    planes[NEAR].y = m[1][3] + m[1][2];
    planes[NEAR].z = m[2][3] + m[2][2];
    planes[NEAR].w = m[3][3] + m[3][2];
    
    // Far plane: row 3 - row 2
    planes[FAR].x = m[0][3] - m[0][2];
    planes[FAR].y = m[1][3] - m[1][2];
    planes[FAR].z = m[2][3] - m[2][2];
    planes[FAR].w = m[3][3] - m[3][2];
    
    // Normalize all planes
    for (auto& plane : planes) {
        normalizePlane(plane);
    }
}

void Frustum::normalizePlane(glm::vec4& plane) {
    float length = glm::length(glm::vec3(plane.x, plane.y, plane.z));
    if (length > 0.0f) {
        plane /= length;
    }
}

bool Frustum::isPointInside(const glm::vec3& point) const {
    for (const auto& plane : planes) {
        if (glm::dot(glm::vec3(plane.x, plane.y, plane.z), point) + plane.w < 0.0f) {
            return false;
        }
    }
    return true;
}

bool Frustum::isBoundingBoxInside(const BoundingBox& bbox) const {
    // Get the 8 corners of the bounding box
    auto corners = bbox.getCorners();
    
    // Check if any corner is inside the frustum
    for (const auto& corner : corners) {
        if (isPointInside(corner)) {
            return true;
        }
    }
    
    // Check if the bounding box is completely outside any plane
    for (const auto& plane : planes) {
        bool allOutside = true;
        for (const auto& corner : corners) {
            if (glm::dot(glm::vec3(plane.x, plane.y, plane.z), corner) + plane.w >= 0.0f) {
                allOutside = false;
                break;
            }
        }
        if (allOutside) {
            return false;
        }
    }
    
    return true;
}

bool Frustum::isSphereInside(const glm::vec3& center, float radius) const {
    for (const auto& plane : planes) {
        float distance = glm::dot(glm::vec3(plane.x, plane.y, plane.z), center) + plane.w;
        if (distance < -radius) {
            return false;
        }
    }
    return true;
}

float Frustum::getDistanceToPlane(const glm::vec3& point, Plane plane) const {
    if (plane >= 0 && plane < 6) {
        const auto& p = planes[plane];
        return glm::dot(glm::vec3(p.x, p.y, p.z), point) + p.w;
    }
    return 0.0f;
}

// BoundingBox implementation
BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) 
    : min(min), max(max), valid(true) {}

BoundingBox BoundingBox::fromVertices(const std::vector<glm::vec3>& vertices) {
    if (vertices.empty()) {
        return BoundingBox();
    }
    
    glm::vec3 min = vertices[0];
    glm::vec3 max = vertices[0];
    
    for (const auto& vertex : vertices) {
        min = glm::min(min, vertex);
        max = glm::max(max, vertex);
    }
    
    return BoundingBox(min, max);
}

std::array<glm::vec3, 8> BoundingBox::getCorners() const {
    if (!valid) {
        return {};
    }
    
    return {
        glm::vec3(min.x, min.y, min.z), // 0: min
        glm::vec3(max.x, min.y, min.z), // 1: min.x, min.y, min.z
        glm::vec3(max.x, max.y, min.z), // 2: min.x, max.y, min.z
        glm::vec3(min.x, max.y, min.z), // 3: max.x, max.y, min.z
        glm::vec3(min.x, min.y, max.z), // 4: min.x, min.y, max.z
        glm::vec3(max.x, min.y, max.z), // 5: max.x, min.y, max.z
        glm::vec3(max.x, max.y, max.z), // 6: max.x, max.y, max.z
        glm::vec3(min.x, max.y, max.z)  // 7: min.x, max.y, max.z
    };
}

BoundingBox BoundingBox::transform(const glm::mat4& matrix) const {
    if (!valid) {
        return BoundingBox();
    }
    
    auto corners = getCorners();
    BoundingBox result;
    
    for (const auto& corner : corners) {
        glm::vec4 transformed = matrix * glm::vec4(corner, 1.0f);
        result.expand(glm::vec3(transformed) / transformed.w);
    }
    
    return result;
}

bool BoundingBox::intersects(const BoundingBox& other) const {
    if (!valid || !other.valid) {
        return false;
    }
    
    return (min.x <= other.max.x && max.x >= other.min.x) &&
           (min.y <= other.max.y && max.y >= other.min.y) &&
           (min.z <= other.max.z && max.z >= other.min.z);
}

glm::vec3 BoundingBox::getCenter() const {
    if (!valid) {
        return glm::vec3(0.0f);
    }
    return (min + max) * 0.5f;
}

glm::vec3 BoundingBox::getSize() const {
    if (!valid) {
        return glm::vec3(0.0f);
    }
    return max - min;
}

float BoundingBox::getBoundingSphereRadius() const {
    if (!valid) {
        return 0.0f;
    }
    return glm::length(getSize()) * 0.5f;
}

void BoundingBox::expand(const glm::vec3& point) {
    if (!valid) {
        min = max = point;
        valid = true;
    } else {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }
}

void BoundingBox::expand(const BoundingBox& other) {
    if (!other.valid) {
        return;
    }
    
    if (!valid) {
        min = other.min;
        max = other.max;
        valid = true;
    } else {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }
} 