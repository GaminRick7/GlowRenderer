#include "rendering/PBRMaterial.h"
#include <iostream>

PBRMaterial::PBRMaterial() 
    : hasAlbedo(false), hasNormal(false), hasMetallic(false), hasRoughness(false), hasAO(false) {
    // Initialize all texture pointers to nullptr
    albedoTexture = nullptr;
    normalTexture = nullptr;
    metallicTexture = nullptr;
    roughnessTexture = nullptr;
    aoTexture = nullptr;
}

PBRMaterial::PBRMaterial(const std::string& albedoPath, 
                         const std::string& normalPath,
                         const std::string& metallicPath,
                         const std::string& roughnessPath,
                         const std::string& aoPath) {
    setAlbedo(albedoPath);
    setNormal(normalPath);
    setMetallic(metallicPath);
    setRoughness(roughnessPath);
    setAO(aoPath);
}

PBRMaterial::PBRMaterial(PBRMaterial&& other) noexcept
    : hasAlbedo(other.hasAlbedo), hasNormal(other.hasNormal), 
      hasMetallic(other.hasMetallic), hasRoughness(other.hasRoughness), hasAO(other.hasAO) {
    albedoTexture = std::move(other.albedoTexture);
    normalTexture = std::move(other.normalTexture);
    metallicTexture = std::move(other.metallicTexture);
    roughnessTexture = std::move(other.roughnessTexture);
    aoTexture = std::move(other.aoTexture);
    
    // Reset other's flags
    other.hasAlbedo = false;
    other.hasNormal = false;
    other.hasMetallic = false;
    other.hasRoughness = false;
    other.hasAO = false;
}

PBRMaterial& PBRMaterial::operator=(PBRMaterial&& other) noexcept {
    if (this != &other) {
        // Clean up existing resources
        destroy();
        
        // Move resources from other
        hasAlbedo = other.hasAlbedo;
        hasNormal = other.hasNormal;
        hasMetallic = other.hasMetallic;
        hasRoughness = other.hasRoughness;
        hasAO = other.hasAO;
        
        albedoTexture = std::move(other.albedoTexture);
        normalTexture = std::move(other.normalTexture);
        metallicTexture = std::move(other.metallicTexture);
        roughnessTexture = std::move(other.roughnessTexture);
        aoTexture = std::move(other.aoTexture);
        
        // Reset other's flags
        other.hasAlbedo = false;
        other.hasNormal = false;
        other.hasMetallic = false;
        other.hasRoughness = false;
        other.hasAO = false;
    }
    return *this;
}

void PBRMaterial::bindTextures() {
    if (hasAlbedo && albedoTexture) {
        albedoTexture->bind(GL_TEXTURE0);
    }
    if (hasNormal && normalTexture) {
        normalTexture->bind(GL_TEXTURE1);
    }
    if (hasMetallic && metallicTexture) {
        metallicTexture->bind(GL_TEXTURE2);
    }
    if (hasRoughness && roughnessTexture) {
        roughnessTexture->bind(GL_TEXTURE3);
    }
    if (hasAO && aoTexture) {
        aoTexture->bind(GL_TEXTURE4);
    }
}

void PBRMaterial::unbindTextures() {
    if (hasAlbedo && albedoTexture) {
        albedoTexture->unbind();
    }
    if (hasNormal && normalTexture) {
        normalTexture->unbind();
    }
    if (hasMetallic && metallicTexture) {
        metallicTexture->unbind();
    }
    if (hasRoughness && roughnessTexture) {
        roughnessTexture->unbind();
    }
    if (hasAO && aoTexture) {
        aoTexture->unbind();
    }
}

void PBRMaterial::setAlbedo(const std::string& path) {
    try {
        albedoTexture = std::make_unique<Texture>(path.c_str(), TextureType::ALBEDO);
        hasAlbedo = true;
    } catch (...) {
        std::cerr << "Failed to load albedo texture: " << path << std::endl;
        hasAlbedo = false;
    }
}

void PBRMaterial::setNormal(const std::string& path) {
    try {
        normalTexture = std::make_unique<Texture>(path.c_str(), TextureType::NORMAL);
        hasNormal = true;
    } catch (...) {
        std::cerr << "Failed to load normal texture: " << path << std::endl;
        hasNormal = false;
    }
}

void PBRMaterial::setMetallic(const std::string& path) {
    try {
        metallicTexture = std::make_unique<Texture>(path.c_str(), TextureType::METALLIC);
        hasMetallic = true;
    } catch (...) {
        std::cerr << "Failed to load metallic texture: " << path << std::endl;
        hasMetallic = false;
    }
}

void PBRMaterial::setRoughness(const std::string& path) {
    try {
        roughnessTexture = std::make_unique<Texture>(path.c_str(), TextureType::ROUGHNESS);
        hasRoughness = true;
    } catch (...) {
        std::cerr << "Failed to load roughness texture: " << path << std::endl;
        hasRoughness = false;
    }
}

void PBRMaterial::setAO(const std::string& path) {
    try {
        aoTexture = std::make_unique<Texture>(path.c_str(), TextureType::AO);
        hasAO = true;
    } catch (...) {
        std::cerr << "Failed to load AO texture: " << path << std::endl;
        hasAO = false;
    }
}

bool PBRMaterial::isValid() const {
    return hasAlbedo && hasNormal && hasMetallic && hasRoughness && hasAO;
}

void PBRMaterial::destroy() {
    // unique_ptr will automatically clean up the textures
    albedoTexture.reset();
    normalTexture.reset();
    metallicTexture.reset();
    roughnessTexture.reset();
    aoTexture.reset();
    
    hasAlbedo = false;
    hasNormal = false;
    hasMetallic = false;
    hasRoughness = false;
    hasAO = false;
} 