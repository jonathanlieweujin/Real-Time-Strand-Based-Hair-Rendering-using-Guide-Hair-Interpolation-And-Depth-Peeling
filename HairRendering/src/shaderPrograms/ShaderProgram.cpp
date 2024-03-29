#include "ShaderProgram.h"
#include <fstream>
#include <iostream>
#include <glfw3.h>
#include <string>

ShaderProgram::ShaderProgram(const char* vertex, const char* fragment, const char* geometry, const char* tessControl, const char* tessEval)
{
	mVertex = vertex;
	mFragment = fragment;
	mGeometry = geometry;
	mTessControl = tessControl;
	mTessEval = tessEval;

	mID = Load();
}

GLuint ShaderProgram::GetID()
{
	return mID;
}

void ShaderProgram::SetGlobalUniforms()
{
	glUniformMatrix4fv(GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(uniforms.view));
	glUniformMatrix4fv(GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(uniforms.projection));
	glUniformMatrix4fv(GetUniformLocation("dirToLight"), 1, GL_FALSE, glm::value_ptr(uniforms.dirToLight));
	glUniform3fv(GetUniformLocation("lightPosition"), 1, glm::value_ptr(uniforms.lightPosition));
	glUniform3fv(GetUniformLocation("camPosition"), 1, glm::value_ptr(uniforms.camPosition));
	glUniform3fv(GetUniformLocation("lightColour"), 1, glm::value_ptr(uniforms.lightColour));
	glUniform1i(GetUniformLocation("noiseTexture"), uniforms.noiseTexture);
	glUniform1i(GetUniformLocation("hairShadowMap"), uniforms.hairShadowMap);
	glUniform1i(GetUniformLocation("meshShadowMap"), uniforms.meshShadowMap);
	glUniform1i(GetUniformLocation("opacityMap"), uniforms.opacityMap);
	glUniform1i(GetUniformLocation("depthPeelMap"), uniforms.depthPeelMap);
	//glUniform1i(GetUniformLocation("depthBufferMap"), uniforms.depthBufferMap);
	glUniform1i(GetUniformLocation("useShadows"), uniforms.useShadows);
	glUniform1i(GetUniformLocation("useColourGradient"), uniforms.useColourGradient);
	glUniform1f(GetUniformLocation("shadowIntensity"), uniforms.shadowIntensity);
	glUniform1f(GetUniformLocation("layerSize"), uniforms.occlusionLayerSize);
}

void ShaderProgram::SetObjectUniforms()
{
	glUniform1f(GetUniformLocation("kajiyaSpecularExponent"), uniforms.kajiyaSpecularExponent);
	glUniform1f(GetUniformLocation("lightIntensity"), uniforms.lightIntensity);

	glUniformMatrix4fv(GetUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(uniforms.model));
	glUniform1i(GetUniformLocation("numGroupHairs"), uniforms.numGroupHairs);
	glUniform1i(GetUniformLocation("numSplineVertices"), uniforms.numSplineVertices);
	glUniform1f(GetUniformLocation("groupSpread"), uniforms.groupSpread);
	glUniform1f(GetUniformLocation("hairRadius"), uniforms.hairRadius);
	glUniform1f(GetUniformLocation("splineTension"), uniforms.splineTension);
	glUniform1f(GetUniformLocation("tapering"), uniforms.tapering);
	glUniform1f(GetUniformLocation("noiseAmplitude"), uniforms.noiseAmplitude);
	glUniform1f(GetUniformLocation("noiseFrequency"), uniforms.noiseFrequency);
	glUniform3fv(GetUniformLocation("colour"), 1, glm::value_ptr(uniforms.colour));

	glUniform1f(GetUniformLocation("ambientIntensity"), uniforms.ambientIntensity);
	glUniform1f(GetUniformLocation("specularIntensity"), uniforms.specularIntensity);
	glUniform1f(GetUniformLocation("diffuseIntensity"), uniforms.diffuseIntensity);

	glUniform1f(GetUniformLocation("alphaR_"), uniforms.alphaR_);
	glUniform1f(GetUniformLocation("alphaTRT"), uniforms.alphaTRT);
	glUniform1f(GetUniformLocation("alphaTT"), uniforms.alphaTT);

	glUniform1f(GetUniformLocation("expR_"), uniforms.expR_);
	glUniform1f(GetUniformLocation("expTRT"), uniforms.expTRT);
	glUniform1f(GetUniformLocation("expTT"), uniforms.expTT);

	glUniform1f(GetUniformLocation("opacity"), uniforms.opacity);
	glUniform1f(GetUniformLocation("maxColourChange"), uniforms.maxColourChange);
}

void ShaderProgram::SetDrawUniforms()
{
	glUniform1i(GetUniformLocation("numHairSegments"), uniforms.numHairVertices - 1);
	glUniform3fv(GetUniformLocation("vertexData"), uniforms.numHairVertices, &uniforms.vertexData[0][0]);
	glUniform3fv(GetUniformLocation("triangleFace"), 2, &uniforms.triangleFace[0][0]);
	glUniform1f(GetUniformLocation("length"), uniforms.length);
}

void ShaderProgram::Bind()
{
	glUseProgram(mID);
}

void ShaderProgram::Unbind()
{
	glUseProgram(mID);
}

GLuint ShaderProgram::Load()
{
	//Create shaders
	std::vector<GLuint> shaders;
	shaders.push_back(CreateShader(GL_VERTEX_SHADER, mVertex));
	shaders.push_back(CreateShader(GL_FRAGMENT_SHADER, mFragment));
	if (mGeometry != "")
	{
		shaders.push_back(CreateShader(GL_GEOMETRY_SHADER, mGeometry));
	}
	if (mTessControl != "" && mTessEval != "")
	{
		shaders.push_back(CreateShader(GL_TESS_CONTROL_SHADER, mTessControl));
		shaders.push_back(CreateShader(GL_TESS_EVALUATION_SHADER, mTessEval));
	}

	//Create program
	GLuint programID = CreateProgram(shaders);

	//Clean up
	for (auto& shader : shaders)
	{
		glDeleteShader(shader);
	}

	return programID;
}

GLuint ShaderProgram::CreateShader(GLenum type, const char* filename)
{
	GLuint shaderID = glCreateShader(type);

	//Read from file
	std::string path = DIRECTORY_PATH + filename;
	std::ifstream file(path);
	std::string out;

	if (file.is_open())
	{
		std::string str;
		while (std::getline(file, str))
		{
			out += str + "\n";
		}
	}
	else
	{
		std::cout << "ERROR: Could not open file " << path << std::endl;
	}

	file.close();

	//Compile shader
	const char* code = out.c_str();
	glShaderSource(shaderID, 1, &code, NULL);
	glCompileShader(shaderID);

	GLint success;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
		std::string log(length, ' ');
		glGetShaderInfoLog(shaderID, length, &length, &log[0]);
		std::cout << "ERROR Failed to compile " << path << std::endl << log << std::endl;
		glDeleteShader(shaderID);
		glfwTerminate();
	}

	return shaderID;
}

GLuint ShaderProgram::CreateProgram(std::vector<GLuint>& shaders)
{
	GLuint programID = glCreateProgram();

	for (auto& shader : shaders)
	{
		glAttachShader(programID, shader);
	}

	glLinkProgram(programID);

	GLint result = GL_FALSE;
	int logLength;
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		std::vector<char> infoLog(logLength);
		glGetProgramInfoLog(programID, logLength, NULL, &infoLog[0]);
		std::cout << "ERROR: Failed to link program" << std::endl << &infoLog << std::endl;
	}

	return programID;
}

GLint ShaderProgram::GetUniformLocation(const GLchar* name)
{
	if (mUniformLocations.count(name) == 0)
	{
		mUniformLocations[name] = glGetUniformLocation(mID, name);
	}

	return mUniformLocations[name];
}
