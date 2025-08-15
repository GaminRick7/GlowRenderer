#pragma once
#include "rendering/Mesh.h"
#include "rendering/PBRMaterial.h"
#include "rendering/shader.h"

class PBRMesh : public Mesh {
public:
    // Constructor with PBR material
    PBRMesh(const std::vector<Vertex>& vertices, 
             const std::vector<GLuint>& indices, 
             PBRMaterial&& material);
    
    // Destructor
    ~PBRMesh();
    
    // Render the mesh with PBR shader
    void drawPBR(Shader& pbrShader);
    
    // Set PBR material
    void setMaterial(PBRMaterial&& material);
    
    // Get PBR material
    const PBRMaterial& getMaterial() const { return pbrMaterial; }
    
    // Cleanup resources
    void destroy();

private:
    PBRMaterial pbrMaterial;
    
    // Setup PBR-specific vertex attributes
    void setupPBRVertexAttributes();
}; 