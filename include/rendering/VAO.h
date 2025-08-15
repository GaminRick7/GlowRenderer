#pragma once
#include <glad/glad.h>
#include "VBO.h"

class VAO {
public:
	VAO();

	GLuint id;

	void LinkAttrib(VBO& vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	void bind();
	void unbind();
	void destroy();
};
