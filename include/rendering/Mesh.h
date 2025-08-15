#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "rendering/VBO.h"
#include "rendering/EBO.h"
#include "rendering/VAO.h"
#include "rendering/Texture.h"
#include "rendering/shader.h"
#include "core/Camera.h"
#include "utils/FrustumCulling.h"

class Mesh {
public:
    // Constructor that takes vertex and index data (no textures)
    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
    
    // Constructor that takes vertex, index, and texture data
    Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures);
    
    // Destructor
    ~Mesh();
    
    // Render the mesh
    void draw(Shader& shader);
    
    // Render the mesh with texture type handling
    void Draw(Shader& shader);
    
    // Bind/unbind the mesh
    void bind();
    void unbind();
    
    // Cleanup resources
    void destroy();
    
    // Getter for vertex count
    GLsizei getVertexCount() const { return vertexCount; }
    
    // Getter for index count
    GLsizei getIndexCount() const { return indexCount; }
    
    // Getter for texture count
    size_t getTextureCount() const { return textures.size(); }
    
    // Get the bounding box of this mesh
    BoundingBox getBoundingBox() const;
    
    // Check if this mesh is visible in the frustum
    bool isVisibleInFrustum(const Frustum& frustum, const glm::mat4& modelMatrix) const;

private:
    std::unique_ptr<VAO> vao;
    std::unique_ptr<VBO> vbo;
    std::unique_ptr<EBO> ebo;
    std::vector<Texture> textures;
    
    GLsizei vertexCount;
    GLsizei indexCount;
    
    // Bounding box for frustum culling
    BoundingBox boundingBox;
    
    // Setup vertex attributes
    void setupVertexAttributes();
}; 