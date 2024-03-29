#pragma once
#include <glew.h>
#include "Quad.h"
#include "shaderPrograms/ShaderProgram.h"

class Texture
{
public:
	Texture();
	virtual ~Texture();

	void Create(const char* filename, GLint magFilter, GLint minFilter);
	void Create(int width, int height, GLint magFilter, GLint minFilter);
	void CreateDepthTexture(int width, int height, GLint magFilter, GLint minFilter);
	void Resize(int width, int height);
	void Bind(GLenum textureUnit);
	void Unbind(GLenum textureUnit);
	void RenderFullScreen();
	GLuint GetID() const;
	GLuint GetWidth() const;
	GLuint GetHeight() const;

private:
	void Create(unsigned char* image, GLint internalFormat, int width, int height, GLenum format, GLenum type, GLint magFilter, GLint minFilter);
	Quad* mQuad;
	ShaderProgram* mProgram;
	GLuint mID;
	int mWidth;
	int mHeight;
	GLint mInternalFormat;
	GLenum mFormat;
	GLenum mType;

	const std::string DIRECTORY_PATH = "../../../images/";
};