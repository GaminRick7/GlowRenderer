#include "rendering/OBJLoader.h"
#include <iostream>

// Function to calculate normals from geometry when they're missing
void calculateNormalsFromGeometry(std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
    // Initialize all normals to zero
    for (auto& vertex : vertices) {
        vertex.normal = glm::vec3(0.0f);
    }
    
    // Calculate normals for each triangle
    for (size_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) break;
        
        GLuint idx0 = indices[i];
        GLuint idx1 = indices[i + 1];
        GLuint idx2 = indices[i + 2];
        
        if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) continue;
        
        // Get the three vertices of the triangle
        glm::vec3 v0 = vertices[idx0].position;
        glm::vec3 v1 = vertices[idx1].position;
        glm::vec3 v2 = vertices[idx2].position;
        
        // Calculate two edge vectors
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        
        // Calculate the normal using cross product
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
        
        // Add this normal to all three vertices (for smooth shading)
        vertices[idx0].normal += normal;
        vertices[idx1].normal += normal;
        vertices[idx2].normal += normal;
    }
    
    // Normalize all accumulated normals
    for (auto& vertex : vertices) {
        if (glm::length(vertex.normal) > 0.0f) {
            vertex.normal = glm::normalize(vertex.normal);
        } else {
            // Fallback for vertices with no normal contribution
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}

std::pair<std::vector<Vertex>, std::vector<GLuint>> loadOBJFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + filepath);
    }

    // Temporary storage for OBJ data
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    
    // Final output
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    
    // For vertex deduplication
    std::unordered_map<OBJKey, GLuint, OBJKeyHash> vertexMap;
    vertexMap.reserve(1024);  // Reserve space for efficiency

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip empty lines and comments
        
        std::istringstream lineStream(line);
        std::string token;
        lineStream >> token;
        
        if (token == "v") {
            // Vertex position: v x y z
            glm::vec3 position;
            lineStream >> position.x >> position.y >> position.z;
            positions.push_back(position);
            
        } else if (token == "vt") {
            // Texture coordinate: vt u v
            glm::vec2 texCoord;
            lineStream >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
            
        } else if (token == "vn") {
            // Normal: vn x y z
            glm::vec3 normal;
            lineStream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
            
        } else if (token == "f") {
            // Face: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            std::vector<std::string> faceTokens;
            std::string faceToken;
            
            // Read all face tokens
            while (lineStream >> faceToken) {
                faceTokens.push_back(faceToken);
            }
            
            // Process triangles (OBJ files can have quads or n-gons)
            for (size_t i = 1; i + 1 < faceTokens.size(); ++i) {
                std::string triangleVerts[3] = { faceTokens[0], faceTokens[i], faceTokens[i + 1] };
                
                // Process each vertex of the triangle
                for (int k = 0; k < 3; ++k) {
                    std::string& faceVert = triangleVerts[k];
                    
                    // Parse vertex format: v/vt/vn, v//vn, v/vt, or v
                    int posIndex = 0, texIndex = -1, normIndex = -1;
                    
                    size_t firstSlash = faceVert.find('/');
                    if (firstSlash == std::string::npos) {
                        // Just vertex: "1"
                        posIndex = parseIndex(faceVert);
                    } else {
                        // Has slashes: parse format
                        size_t secondSlash = faceVert.find('/', firstSlash + 1);
                        posIndex = parseIndex(faceVert.substr(0, firstSlash));
                        
                        if (secondSlash == std::string::npos) {
                            // v/vt format: "1/1"
                            std::string texStr = faceVert.substr(firstSlash + 1);
                            if (!texStr.empty()) texIndex = parseIndex(texStr);
                        } else {
                            // v//vn or v/vt/vn format
                            std::string mid = faceVert.substr(firstSlash + 1, secondSlash - firstSlash - 1);
                            if (!mid.empty()) texIndex = parseIndex(mid);
                            
                            std::string normStr = faceVert.substr(secondSlash + 1);
                            if (!normStr.empty()) normIndex = parseIndex(normStr);
                        }
                    }
                    
                    // Create key for deduplication
                    OBJKey key{posIndex, texIndex, normIndex};
                    
                    // Check if we've seen this vertex combination before
                    auto it = vertexMap.find(key);
                    if (it != vertexMap.end()) {
                        // Reuse existing vertex
                        indices.push_back(it->second);
                    } else {
                        // Create new vertex
                        Vertex vertex;
                        
                        // Set position
                        vertex.position = positions[posIndex];
                        
                        // Set texture coordinate (with default if missing)
                        if (texIndex >= 0 && texIndex < (int)texCoords.size()) {
                            vertex.texCoord = texCoords[texIndex];
                        } else {
                            vertex.texCoord = glm::vec2(0.0f, 0.0f);  // Default
                        }
                        
                        // Set normal (with default if missing)
                        if (normIndex >= 0 && normIndex < (int)normals.size()) {
                            vertex.normal = normals[normIndex];
                        } else {
                            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);  // Default up
                        }
                        
                        // Set color (OBJ files don't have colors, so use default)
                        vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);  // White
                        
                        // Add vertex and track index
                        GLuint newIndex = (GLuint)vertices.size();
                        vertices.push_back(vertex);
                        indices.push_back(newIndex);
                        vertexMap.emplace(key, newIndex);
                    }
                }
            }
        }
    }
    
    // If no normals were loaded from the file, calculate them from geometry
    if (normals.empty()) {
        calculateNormalsFromGeometry(vertices, indices);
    }
    
    return {vertices, indices};
} 