#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <glm/glm.hpp>
#include "rendering/VBO.h"  // For Vertex struct

// Key for vertex deduplication
struct OBJKey {
    int positionIndex, texCoordIndex, normalIndex;
    
    bool operator==(const OBJKey& key) const {
        return positionIndex == key.positionIndex && 
               texCoordIndex == key.texCoordIndex && 
               normalIndex == key.normalIndex;
    }
};

// Hash function for OBJKey
struct OBJKeyHash {
    size_t operator()(const OBJKey& key) const noexcept {
        return ((size_t)key.positionIndex * 73856093u) ^ 
               ((size_t)key.texCoordIndex * 19349663u) ^ 
               ((size_t)key.normalIndex * 83492791u);
    }
};

// Helper function to safely parse integers
static inline int parseIndex(const std::string& str) {
    return std::stoi(str) - 1;  // Convert to 0-based indexing
}

// Function to calculate normals from geometry when they're missing
void calculateNormalsFromGeometry(std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);

// Load OBJ file and return vertices and indices compatible with your Mesh class
std::pair<std::vector<Vertex>, std::vector<GLuint>> loadOBJFile(const std::string& filepath); 