#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// Vertex structure that matches the vertex shader attributes
struct Vertex {
    glm::vec3 position;    // aPos (location = 0)
    glm::vec3 color;       // aColor (location = 1)
    glm::vec2 texCoord;    // aTexCoord (location = 2)
    glm::vec3 normal;      // aNormal (location = 3)
    glm::vec3 tangent;     // aTangent (location = 4)
    glm::vec3 bitangent;   // aBitangent (location = 5)
    
    Vertex() = default;
    
    Vertex(const glm::vec3& pos, const glm::vec3& col, const glm::vec2& tex, const glm::vec3& norm)
        : position(pos), color(col), texCoord(tex), normal(norm), tangent(0.0f), bitangent(0.0f) {}
    
    Vertex(const glm::vec3& pos, const glm::vec3& col, const glm::vec2& tex, const glm::vec3& norm, const glm::vec3& tan, const glm::vec3& bitan)
        : position(pos), color(col), texCoord(tex), normal(norm), tangent(tan), bitangent(bitan) {}
};

//Holds the configurations (vertex attribute pointer, EBO) for the associated VBO
class VBO {
public:
	GLuint id;
	VBO(const std::vector<Vertex>& vertices);

	void bind();
	void unbind();
	void destroy();

};

