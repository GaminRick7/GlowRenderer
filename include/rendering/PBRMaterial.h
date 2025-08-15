#pragma once
#include <vector>
#include <memory>
#include "rendering/Texture.h"

class PBRMaterial {
public:
    PBRMaterial();
    PBRMaterial(const std::string& albedoPath, 
                const std::string& normalPath,
                const std::string& metallicPath,
                const std::string& roughnessPath,
                const std::string& aoPath);
    
    // Move constructor
    PBRMaterial(PBRMaterial&& other) noexcept;
    
    // Move assignment operator
    PBRMaterial& operator=(PBRMaterial&& other) noexcept;
    
    // Delete copy constructor and assignment operator
    PBRMaterial(const PBRMaterial&) = delete;
    PBRMaterial& operator=(const PBRMaterial&) = delete;
    
    // Bind all textures to their respective texture units
    void bindTextures();
    
    // Unbind all textures
    void unbindTextures();
    
    // Set material properties
    void setAlbedo(const std::string& path);
    void setNormal(const std::string& path);
    void setMetallic(const std::string& path);
    void setRoughness(const std::string& path);
    void setAO(const std::string& path);
    
    // Get texture references
    const Texture& getAlbedoTexture() const { return *albedoTexture; }
    const Texture& getNormalTexture() const { return *normalTexture; }
    const Texture& getMetallicTexture() const { return *metallicTexture; }
    const Texture& getRoughnessTexture() const { return *roughnessTexture; }
    const Texture& getAOTexture() const { return *aoTexture; }
    
    // Check if material has all required textures
    bool isValid() const;
    
    // Cleanup resources
    void destroy();

private:
    std::unique_ptr<Texture> albedoTexture;
    std::unique_ptr<Texture> normalTexture;
    std::unique_ptr<Texture> metallicTexture;
    std::unique_ptr<Texture> roughnessTexture;
    std::unique_ptr<Texture> aoTexture;
    
    bool hasAlbedo;
    bool hasNormal;
    bool hasMetallic;
    bool hasRoughness;
    bool hasAO;
}; 