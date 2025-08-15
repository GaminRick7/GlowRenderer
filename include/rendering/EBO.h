#pragma once
#include <glad/glad.h>
#include <vector>

class EBO {
public:
	GLuint id;
	EBO(GLuint* indices, GLsizeiptr size);
	EBO(const std::vector<GLuint>& indices);

	void bind();
	void unbind();
	void destroy();

};

