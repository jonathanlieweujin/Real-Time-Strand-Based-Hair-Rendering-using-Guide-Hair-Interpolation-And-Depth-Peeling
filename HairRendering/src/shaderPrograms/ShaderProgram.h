#pragma once
#include <glew.h>
#include <vector>
#include <map>
#include <string>
#include <glm.hpp>
#include <gtc/type_ptr.hpp>

struct Uniforms
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	int numGroupHairs;
	int numHairVertices;
	int numSplineVertices;
	glm::vec3 vertexData[64];
	float lightIntensity;
	float groupSpread;
	float hairRadius;
	float splineTension;
	float tapering;
	float noiseAmplitude;
	float noiseFrequency;
	glm::vec3 triangleFace[2];
	glm::vec3 colour;
	glm::vec3 lightPosition;
	glm::vec3 camPosition;
	glm::vec3 lightColour;
	glm::mat4 dirToLight;
	int noiseTexture;
	int hairShadowMap;
	int meshShadowMap;
	int opacityMap;
	int depthPeelMap;
	//int depthBufferMap;
	int hairMap;
	bool useShadows;
	bool useColourGradient;
	float shadowIntensity;
	float occlusionLayerSize = 0.0005f;

	float ambientIntensity;
	float specularIntensity;
	float diffuseIntensity;

	float kajiyaSpecularExponent;

	float alphaR_;
	float alphaTRT;
	float alphaTT;

	float expR_;
	float expTRT;
	float expTT;

	float length;
	float opacity = 0.850f;
	float maxColourChange;
};

class ShaderProgram
{
public:
	ShaderProgram() = default;
	ShaderProgram(const char* vertex, const char* fragment, const char* geometry = "", const char* tessControl = "", const char* tessEval = "");

	GLuint GetID();
	virtual void SetGlobalUniforms();
	virtual void SetObjectUniforms();
	virtual void SetDrawUniforms();
	void Bind();
	void Unbind();

	Uniforms uniforms;

protected:
	GLint GetUniformLocation(const GLchar* name);
	GLuint CreateShader(GLenum type, const char* filename);

	GLuint mID;
	const char* mVertex;
	const char* mFragment;
	const char* mGeometry;
	const char* mTessControl;
	const char* mTessEval;

private:
	GLuint Load();
	GLuint CreateProgram(std::vector<GLuint>& shaders);

	std::map<std::string, int> mUniformLocations;

	const std::string DIRECTORY_PATH = "../../../HairRendering/src/shaders/";
};