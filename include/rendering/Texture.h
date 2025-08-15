#pragma once
#include <glad/glad.h>
#include <string>

enum class TextureType {
    DIFFUSE,
    SPECULAR,
    NORMAL,
    ALBEDO,
    METALLIC,
    ROUGHNESS,
    AO
};

class Texture {
public:
    GLuint id;
    int width, height, nrChannels;
    TextureType type;
    std::string path;
    
    // Constructor - loads texture from file
    Texture(const char* filePath);
    
    // Constructor - loads texture from file with type
    Texture(const char* filePath, TextureType textureType);
    
    // Bind/unbind texture to texture unit
    void bind(GLenum textureUnit = GL_TEXTURE0);
    void unbind();
    
    // Set texture parameters
    void setWrapMode(GLenum sWrap, GLenum tWrap);
    void setFilterMode(GLenum minFilter, GLenum magFilter);
    
    // Generate mipmaps
    void generateMipmaps();
    
    // Upload texture to uniform
    void uploadToUniform(GLuint shaderProgram, const char* uniformName, GLenum textureUnit = GL_TEXTURE0);
    
    // Get texture type as string
    std::string getTypeString() const;
    
    // Cleanup
    void destroy();
};