#include "rendering/DeferredRenderer.h"

#include <iostream>

DeferredRenderer::DeferredRenderer(int width, int height):
    width(width), height(height), gBuffer(0), gPosition(0), gNormal(0), gAlbedo(0), gAO(0), gMetallicRoughness(0), depthBuffer(0) {
}

DeferredRenderer::~DeferredRenderer(){
    cleanup();
}

bool DeferredRenderer::initialize(){
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER,gBuffer);
    if (gBuffer == 0){
        return false;
    }
    
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    

    // Create albedo texture
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    glGenTextures(1, &gMetallicRoughness);
    glBindTexture(GL_TEXTURE_2D, gMetallicRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gMetallicRoughness, 0);

    glGenTextures(1, &gAO);
    glBindTexture(GL_TEXTURE_2D, gAO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gAO, 0);

    // Create depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    GLuint attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(5, attachments);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "G-Buffer framebuffer is not complete!" << std::endl;
        return false;
    }

    std::cout << "G-Buffer initialized successfully with 4 attachments and depth buffer!" << std::endl;
    
    // Create full-screen quad for lighting pass
    createScreenQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true; 
}

void DeferredRenderer::renderGeometryPass(const std::vector<PBRMesh*>& meshes, 
                                         const std::vector<glm::mat4>& modelMatrices,
                                         Shader& geometryShader, 
                                         const glm::mat4& viewMatrix, 
                                         const glm::mat4& projectionMatrix){
        std::cout << "Starting geometry pass with " << meshes.size() << " meshes" << std::endl;
        bindGBuffer();

        geometryShader.use();
        geometryShader.setMat4("view", viewMatrix);
        geometryShader.setMat4("projection", projectionMatrix);

        for (size_t i = 0; i < meshes.size(); ++i){
            glm::mat4 modelMatrix = (i < modelMatrices.size()) ? modelMatrices[i] : glm::mat4(1.0f);
            geometryShader.setMat4("model", modelMatrix);
            std::cout << "Rendering mesh " << i << " to G-Buffer" << std::endl;
            meshes[i]->drawPBR(geometryShader);
        }

        std::cout << "Geometry pass completed" << std::endl;

        // Unbind all textures to prevent conflicts with lighting pass
        for (int i = 0; i < 4; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        unbindGBuffer();
}

void DeferredRenderer::renderLightingPass(Shader& lightingShader, const glm::vec3& viewPos) {
    std::cout << "Rendering lighting pass!" << std::endl;
    
    // Shader is already bound in main.cpp, so we don't need to call use() again
    lightingShader.setVec3("viewPos", viewPos);
    
    // Bind G-Buffer textures to texture units 5-8 to match the uniform values
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    lightingShader.setInt("gPosition", 5);
    
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    lightingShader.setInt("gNormal", 6);
    
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    lightingShader.setInt("gAlbedo", 7);
    
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, gMetallicRoughness);
    lightingShader.setInt("gMetallicRoughness", 8);
    
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, gAO);
    lightingShader.setInt("gAO", 9);
    
    // Debug: Check if textures are bound
    std::cout << "G-Buffer textures bound - Position: " << gPosition << ", Normal: " << gNormal 
              << ", Albedo: " << gAlbedo << ", MetallicRoughness: " << gMetallicRoughness << ", AO: " << gAO << std::endl;
    
    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error before rendering: " << err << std::endl;
    }
            
    // Render full-screen quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    // Check for OpenGL errors after rendering
    err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error after rendering: " << err << std::endl;
    }
    
    std::cout << "Screen quad rendered with VAO: " << quadVAO << std::endl;
}

void DeferredRenderer::bindGBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DeferredRenderer::unbindGBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
}

void DeferredRenderer::debugGBuffer() {
    // Bind G-Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    
    // Read a pixel from position texture
    unsigned char pixel[4];
    glReadPixels(400, 300, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    std::cout << "G-Buffer pixel (400,300): R=" << (int)pixel[0] << " G=" << (int)pixel[1] << " B=" << (int)pixel[2] << std::endl;
    
    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredRenderer::createScreenQuad() {
    // Full-screen quad vertices (position, texture coordinates)
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    std::cout << "Creating screen quad with vertices:" << std::endl;
    for (int i = 0; i < 6; i++) {
        std::cout << "Vertex " << i << ": pos(" << quadVertices[i*4] << ", " << quadVertices[i*4+1] 
                  << ") tex(" << quadVertices[i*4+2] << ", " << quadVertices[i*4+3] << ")" << std::endl;
    }
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    
    if (quadVAO == 0 || quadVBO == 0) {
        std::cerr << "Failed to create screen quad VAO or VBO!" << std::endl;
        return;
    }
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    std::cout << "Screen quad created successfully - VAO: " << quadVAO << ", VBO: " << quadVBO << std::endl;
}

void DeferredRenderer::cleanup(){
    if (gBuffer != 0){
        glDeleteFramebuffers(1, &gBuffer);
        gBuffer = 0;

        glDeleteTextures(1, &gPosition);
        gPosition = 0;

        glDeleteTextures(1, &gNormal);
        gNormal = 0;

        glDeleteTextures(1, &gAlbedo);
        gAlbedo = 0;

        glDeleteTextures(1, &gMetallicRoughness);
        gMetallicRoughness = 0;

        glDeleteTextures(1, &gAO);
        gAO = 0;
    }
    
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }

    if (depthBuffer) {
        glDeleteRenderbuffers(1, &depthBuffer);
        depthBuffer = 0;
    }
}