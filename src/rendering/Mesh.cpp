#include "rendering/Mesh.h"


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) 
    : vertexCount(static_cast<GLsizei>(vertices.size())), 
      indexCount(static_cast<GLsizei>(indices.size())) {
    
    // Calculate bounding box from vertices
    std::vector<glm::vec3> positions;
    positions.reserve(vertices.size());
    for (const auto& vertex : vertices) {
        positions.push_back(vertex.position);
    }
    boundingBox = BoundingBox::fromVertices(positions);
    
    // Create VAO, VBO, and EBO
    vao = std::make_unique<VAO>();
    vbo = std::make_unique<VBO>(vertices);
    ebo = std::make_unique<EBO>(indices);
    
    // Bind VAO and setup vertex attributes
    vao->bind();
    vbo->bind();
    ebo->bind();
    
    setupVertexAttributes();
    
    // Unbind everything
    vao->unbind();
    vbo->unbind();
    ebo->unbind();
}

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, const std::vector<Texture>& textures) 
    : vertexCount(static_cast<GLsizei>(vertices.size())), 
      indexCount(static_cast<GLsizei>(indices.size())),
      textures(textures) {
    
    // Calculate bounding box from vertices
    std::vector<glm::vec3> positions;
    positions.reserve(vertices.size());
    for (const auto& vertex : vertices) {
        positions.push_back(vertex.position);
    }
    boundingBox = BoundingBox::fromVertices(positions);
    
    // Create VAO, VBO, and EBO
    vao = std::make_unique<VAO>();
    vbo = std::make_unique<VBO>(vertices);
    ebo = std::make_unique<EBO>(indices);
    
    // Bind VAO and setup vertex attributes
    vao->bind();
    vbo->bind();
    ebo->bind();
    
    setupVertexAttributes();
    
    // Unbind everything
    vao->unbind();
    vbo->unbind();
    ebo->unbind();
}

Mesh::~Mesh() {
    destroy();
}

void Mesh::draw(Shader& shader) {
    // Bind textures to texture units
    for (size_t i = 0; i < textures.size(); ++i) {
        textures[i].bind(GL_TEXTURE0 + i);
    }
    
    // Draw the mesh
    vao->bind();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    vao->unbind();
    
    // Unbind textures
    for (size_t i = 0; i < textures.size(); ++i) {
        textures[i].unbind();
    }
}

void Mesh::Draw(Shader& shader) {
    // Keep track of how many of each type of textures we have
    unsigned int numDiffuse = 0;
    unsigned int numSpecular = 0;
    unsigned int numNormal = 0;

    // Bind textures and upload to uniforms
    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string num;
        std::string type = textures[i].getTypeString();
        
        if (type == "diffuse") {
            num = std::to_string(numDiffuse++);
        }
        else if (type == "specular") {
            num = std::to_string(numSpecular++);
        }
        else if (type == "normal") {
            num = std::to_string(numNormal++);
        }
        
        // Create uniform name based on texture type and count
        std::string uniformName = type + num;

        // Bind texture to texture unit
        textures[i].bind(GL_TEXTURE0 + i);
        
        // Upload texture to uniform using the uploadToUniform method
        textures[i].uploadToUniform(shader.id, uniformName.c_str(), GL_TEXTURE0 + i);
    }
    
    // Draw the mesh
    vao->bind();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    vao->unbind();
    
    // Unbind textures
    for (size_t i = 0; i < textures.size(); ++i) {
        textures[i].unbind();
    }
}

void Mesh::bind() {
    vao->bind();
}

void Mesh::unbind() {
    vao->unbind();
}

void Mesh::destroy() {
    if (vao) vao->destroy();
    if (vbo) vbo->destroy();
    if (ebo) ebo->destroy();
}

void Mesh::setupVertexAttributes() {
    // Position attribute (location = 0)
    vao->LinkAttrib(*vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
    
    // Color attribute (location = 1)
    vao->LinkAttrib(*vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
    
    // Texture coordinate attribute (location = 2)
    vao->LinkAttrib(*vbo, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
    
    // Normal attribute (location = 3)
    vao->LinkAttrib(*vbo, 3, 3, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float)));
    
    // Tangent attribute (location = 4)
    vao->LinkAttrib(*vbo, 4, 3, GL_FLOAT, sizeof(Vertex), (void*)(11 * sizeof(float)));
    
    // Bitangent attribute (location = 5)
    vao->LinkAttrib(*vbo, 5, 3, GL_FLOAT, sizeof(Vertex), (void*)(14 * sizeof(float)));
}

BoundingBox Mesh::getBoundingBox() const {
    return boundingBox;
}

bool Mesh::isVisibleInFrustum(const Frustum& frustum, const glm::mat4& modelMatrix) const {
    // Transform the bounding box by the model matrix
    BoundingBox transformedBox = boundingBox.transform(modelMatrix);
    
    // Check if the transformed bounding box is inside the frustum
    return frustum.isBoundingBoxInside(transformedBox);
} 