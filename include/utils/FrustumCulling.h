#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <vector>

// Forward declaration
class BoundingBox;

class Frustum {
public:
    // Frustum planes: LEFT, RIGHT, BOTTOM, TOP, NEAR, FAR
    enum Plane {
        LEFT = 0,
        RIGHT = 1,
        BOTTOM = 2,
        TOP = 3,
        NEAR = 4,
        FAR = 5
    };

    Frustum() = default;
    
    // Extract frustum planes from view-projection matrix
    void extractPlanes(const glm::mat4& viewProjection);
    
    // Test if a point is inside the frustum
    bool isPointInside(const glm::vec3& point) const;
    
    // Test if a bounding box is inside the frustum
    bool isBoundingBoxInside(const BoundingBox& bbox) const;
    
    // Test if a sphere is inside the frustum
    bool isSphereInside(const glm::vec3& center, float radius) const;
    
    // Get the distance from a point to a specific plane
    float getDistanceToPlane(const glm::vec3& point, Plane plane) const;

private:
    // Frustum planes in the form: ax + by + cz + d = 0
    // Each plane is stored as (a, b, c, d)
    std::array<glm::vec4, 6> planes;
    
    // Normalize plane coefficients
    void normalizePlane(glm::vec4& plane);
};

class BoundingBox {
public:
    BoundingBox() = default;
    BoundingBox(const glm::vec3& min, const glm::vec3& max);
    
    // Create bounding box from mesh vertices
    static BoundingBox fromVertices(const std::vector<glm::vec3>& vertices);
    
    // Get the 8 corners of the bounding box
    std::array<glm::vec3, 8> getCorners() const;
    
    // Transform bounding box by a matrix
    BoundingBox transform(const glm::mat4& matrix) const;
    
    // Check if this bounding box intersects with another
    bool intersects(const BoundingBox& other) const;
    
    // Get the center of the bounding box
    glm::vec3 getCenter() const;
    
    // Get the size of the bounding box
    glm::vec3 getSize() const;
    
    // Get the radius of the bounding sphere that contains this box
    float getBoundingSphereRadius() const;
    
    // Expand the bounding box to include a point
    void expand(const glm::vec3& point);
    
    // Expand the bounding box to include another bounding box
    void expand(const BoundingBox& other);

private:
    glm::vec3 min;
    glm::vec3 max;
    bool valid = false;
}; 