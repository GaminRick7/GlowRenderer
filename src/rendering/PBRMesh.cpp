#include "rendering/PBRMesh.h"

PBRMesh::PBRMesh(const std::vector<Vertex>& vertices, 
                   const std::vector<GLuint>& indices, 
                   PBRMaterial&& material)
    : Mesh(vertices, indices), pbrMaterial(std::move(material)) {
    setupPBRVertexAttributes();
}

PBRMesh::~PBRMesh() {
    destroy();
}

void PBRMesh::drawPBR(Shader& pbrShader) {
    // Bind PBR material textures
    pbrMaterial.bindTextures();
    
    // Set texture uniforms
    pbrShader.setInt("albedoMap", 0);
    pbrShader.setInt("normalMap", 1);
    pbrShader.setInt("metallicMap", 2);
    pbrShader.setInt("roughnessMap", 3);
    pbrShader.setInt("aoMap", 4);
    
    // Draw the mesh
    draw(pbrShader);
    
    // Unbind textures
    pbrMaterial.unbindTextures();
}

void PBRMesh::setMaterial(PBRMaterial&& material) {
    pbrMaterial = std::move(material);
}

void PBRMesh::destroy() {
    pbrMaterial.destroy();
    Mesh::destroy();
}

void PBRMesh::setupPBRVertexAttributes() {
    // PBR mesh uses the same vertex attributes as the base Mesh class
    // The PBR-specific functionality is handled in the shader and material
} 