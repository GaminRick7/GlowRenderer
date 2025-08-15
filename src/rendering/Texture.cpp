#include "rendering/Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

Texture::Texture(const char* filePath) : type(TextureType::DIFFUSE), path(filePath) {
    // Generate texture ID
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
    // Set default texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image data
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
            
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
    }
    
    stbi_image_free(data);
}

Texture::Texture(const char* filePath, TextureType textureType) : type(textureType), path(filePath) {
    // Generate texture ID
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
    // Set default texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image data
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
            
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
    }
    
    stbi_image_free(data);
}

void Texture::bind(GLenum textureUnit) {
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::setWrapMode(GLenum sWrap, GLenum tWrap) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);
}

void Texture::setFilterMode(GLenum minFilter, GLenum magFilter) {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Texture::generateMipmaps() {
    glBindTexture(GL_TEXTURE_2D, id);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::uploadToUniform(GLuint shaderProgram, const char* uniformName, GLenum textureUnit) {
    // Bind the texture to the specified texture unit
    bind(textureUnit);
    
    // Get the uniform location
    GLint uniformLocation = glGetUniformLocation(shaderProgram, uniformName);
    if (uniformLocation == -1) {
        std::cerr << "Warning: Uniform '" << uniformName << "' not found in shader program" << std::endl;
        return;
    }
    
    // Calculate the texture unit number (GL_TEXTURE0 = 0, GL_TEXTURE1 = 1, etc.)
    GLint textureUnitNumber = textureUnit - GL_TEXTURE0;
    
    // Set the uniform to the texture unit
    glUniform1i(uniformLocation, textureUnitNumber);
}

void Texture::destroy() {
    glDeleteTextures(1, &id);
}

std::string Texture::getTypeString() const {
    switch (type) {
        case TextureType::DIFFUSE:
            return "diffuse";
        case TextureType::SPECULAR:
            return "specular";
        case TextureType::NORMAL:
            return "normal";
        default:
            return "unknown";
    }
}