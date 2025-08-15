#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "rendering/shader.h"
#include "rendering/PBRMesh.h"

class DeferredRenderer {
    public:
        DeferredRenderer(int width, int height);
        ~DeferredRenderer();

        bool initialize();
        void cleanup();

        void renderGeometryPass(const std::vector<PBRMesh*>& meshes, 
                               const std::vector<glm::mat4>& modelMatrices,
                               Shader& geometryShader, 
                               const glm::mat4& viewMatrix, 
                               const glm::mat4& projectionMatrix);

        // Render lighting pass (calculate lighting using G-Buffer)
        void renderLightingPass(Shader& lightingShader, const glm::vec3& viewPos);
        
        // Debug G-Buffer contents
        void debugGBuffer();
        
        // Bind G-Buffer for geometry pass
        void bindGBuffer();

        // Unbind G-Buffer and return to default framebuffer
        void unbindGBuffer();

    private:
        int width, height;
        GLuint gBuffer;
        GLuint gPosition;
        GLuint gNormal;
        GLuint gAlbedo;
        GLuint gMetallicRoughness;
        GLuint gAO;
        GLuint depthBuffer;
        GLuint quadVAO;
        GLuint quadVBO;
        
        // Create full-screen quad for lighting pass
        void createScreenQuad();
};
