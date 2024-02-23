#pragma once
#include <glew.h>
#include <vector>
#include "shaderPrograms/ShaderProgram.h"
#include "Mesh.h"
#include "Strand.h"

class Texture;
class Simulation;

class Hair
{
public:
	friend class GuiWindow;
	Hair(int numGuides, Simulation* simulation);
	Hair(Mesh* mesh, int numDispHair, Simulation* simulation, Hair* oldHairAttributes = nullptr);
	Hair(Mesh* mesh, int numDispHair, const char* hairMap, double maxLength, Simulation* simulation, Hair* oldHairAttributes = nullptr);
	~Hair();

	void Update(float time);
	void Draw(ShaderProgram* program, glm::mat4 cameraProjection, bool useTradAlphaBlending);

	void SetAttributes(Hair* oldHairAttributes);
	void SetAttributes(glm::vec3 colour = glm::vec3(0.38f, 0.34f, 0.30f), int numGroupHairs = 64, float groupSpread = 0.3f, float hairRadius = 0.001f, float splineTension = 0.5f, float noiseAmplitude = 0.2f, float noiseFrequency =  0.5f, int numSplineVertices = 50, float shadowIntensity = 30.0f, 
		float ambientIntensity = 0.5f, float diffuseIntensity = 0.5f, float specularIntensity = 0.2f, 
		float alphaR_ = 5.0f, float alphaTRT = 2.5f, float alphaTT = 7.5f,
		float expR_ = 8.0f, float expTRT = 4.0f, float expTT = 16.0f,
		float opacity = 0.5, float colourChange = 0.5f);

	std::vector<Strand*> GetGuideHairs() const;
	int GetNumGroupHairs() const;
	void SetNumGroupHairs(const int num);
	void SetNumSplineVertices(const int num);
	void SetColour(const glm::vec3 colour);
	void SetShadowIntensity(const float intensity);
	void SetGroupSpread(const float spread);
	Texture* GetHairMap() const;
	glm::vec3 GetColour() const;
	float GetShadowIntensity() const;

	float GetAmbientIntensity() const;
	float GetDiffuseIntensity() const;
	float GetSpecularIntensity() const;

	float GetAlphaR() const;
	float GetAlphaTRT() const;
	float GetAlphaTT() const;

	float GetExpR() const;
	float GetExpTRT() const;
	float GetExpTT() const;

	float GetOpacity() const;
	float GetColourChange() const;

private:
	std::vector<Strand*> mGuideHairs;
	Simulation* mSimulation;
	int mNumGuideHairs;
	int mNumHairVertices;
	int mNumPatchHair;
	glm::vec3 mColour;
	int mNumGroupHairs = 64;
	float mGroupSpread;
	float mHairRadius;
	float mSplineTension = 0.5f;
	float mNoiseAmplitude;
	float mNoiseFrequency;
	int mNumSplineVertices;
	Texture* mHairMap;
	float mShadowIntensity;

	float mAmbientIntensity;
	float mDiffuseIntensity;
	float mSpecularIntensity;

	float mAlphaR_;
	float mAlphaTRT;
	float mAlphaTT;

	float mExpR_;
	float mExpTRT;
	float mExpTT;

	float mOpacity = 0.85f;
	float mColourChange;
};