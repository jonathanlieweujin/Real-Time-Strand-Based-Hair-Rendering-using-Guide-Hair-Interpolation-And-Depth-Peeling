#include "Hair.h"
#include "Simulation.h"
#include <SOIL2.h>
#include <iostream>
#include "Texture.h"

#define SIMULATE_PHYSICS true

Hair::Hair(int numGuides, Simulation* simulation)
{
	for (int i = 0; i < numGuides; i++)
	{
		mGuideHairs.push_back(new Strand(20, 1.0, glm::vec3(i + 0.25f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	}

	SetAttributes();

	mSimulation = simulation;
}

Hair::Hair(Mesh* mesh, int numDispHair, Simulation* simulation, Hair* oldHairAttributes)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(-0.03f, 0.03f);
	for (auto& vertex : mesh->GetVertices())
	{
		glm::vec3 random = glm::vec3(distribution(generator), distribution(generator), distribution(generator));
		glm::vec3 position = vertex.position + random;
		glm::vec3 normal = vertex.normal + random;
		mGuideHairs.push_back(new Strand(20, 0.4, position, normal));
	}

	SetAttributes(oldHairAttributes);
	mSimulation = simulation;
}

Hair::Hair(Mesh* mesh, int numDispHair, const char* hairMap, double maxLength, Simulation* simulation, Hair* oldHairAttributes)
{
	if (hairMap == "")
	{
		Hair(mesh, numDispHair, simulation, oldHairAttributes);
		return;
	}

	//Get hair map data
	std::string path = std::string("../../../images/") + hairMap;
	int width, height, channels;
	unsigned char* image = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_RGBA);
	
	if (image == NULL || width == 0 || height == 0)
	{
		std::cout << "ERROR: Failed to load image" << std::endl;
	}

	mHairMap = new Texture();
	mHairMap->Create(hairMap, GL_LINEAR, GL_LINEAR);

	std::random_device rd;
	std::mt19937 generator = std::mt19937(rd());

	std::vector<Triangle*> availableTriangles;
	for (auto& triangle : mesh->triangles)
	{
		availableTriangles.push_back(&triangle);
	}

	for (int i = 0; i < numDispHair && !availableTriangles.empty(); i++) {
		std::shuffle(availableTriangles.begin(), availableTriangles.end(), generator);
		Triangle* triangle = availableTriangles.back();
		Vertex randomPoint;
		triangle -> RandomPoint(randomPoint);
		randomPoint.texCoords = glm::vec2(glm::min(randomPoint.texCoords.x, 0.999f), glm::min(randomPoint.texCoords.y, 0.999f));

		int x = std::min(std::max(static_cast<int>(randomPoint.texCoords.x * width), 0), width);	
		int y = std::min(std::max(static_cast<int>(randomPoint.texCoords.y * height), 0), height);

		//Use hair map's alpha value to scale hair length
		unsigned char* pixel = image + y * width * channels + x * channels;
		unsigned char alpha = pixel[3];
		double alphaVal = (double)alpha / 255;

		if (alphaVal < 0.05)
		{
			i--;
			continue;
		}

		mGuideHairs.push_back(new Strand(20, maxLength * alphaVal, randomPoint.position, randomPoint.normal));
	}

	/*//Place hairs at random points in triangles
	for (auto& triangle : mesh->triangles)
	{

		int numHairs = (int)(hairDensity * triangle.Area() + rand() / (float)RAND_MAX);
		//std:: cout << hairDensity << std::endl;
		for (int i = 0; i < numHairs; i++)
		{
			Vertex randomPoint;
			triangle.RandomPoint(randomPoint);
			randomPoint.texCoords = glm::vec2(glm::min(randomPoint.texCoords.x, 0.999f), glm::min(randomPoint.texCoords.y, 0.999f));

			int x = randomPoint.texCoords.x * width;
			int y = randomPoint.texCoords.y * height;

			if (x < 0 || x > width || y < 0 || y > width)
			{
				continue;
			}

			//Use hair map's alpha value to scale hair length
			unsigned char* pixel = image + y * width * channels + x * channels;
			unsigned char alpha = pixel[3];
			double alphaVal = (double)alpha / 255;
			
			if (alphaVal < 0.05)
			{
				continue;
			}

			mGuideHairs.push_back(new Strand(20, maxLength * alphaVal, randomPoint.position, randomPoint.normal));
		}
	}*/

	SOIL_free_image_data(image);
	SetAttributes(oldHairAttributes);
	mSimulation = simulation;
}

Hair::~Hair()
{
	for (auto& guide : mGuideHairs)
	{
		delete guide;
	}

	if (mHairMap)
	{
		delete mHairMap;
	}
}

void Hair::Update(float time)
{
	if (SIMULATE_PHYSICS && mSimulation)
	{
		mSimulation->Simulate(this);
	}

	for (auto& guide : mGuideHairs)
	{
		guide->Update(time);
	}
}

void Hair::Draw(ShaderProgram* program, glm::mat4 cameraProjection, bool useTradAlphaBlending)
{
	program->uniforms.colour = mColour;
	program->uniforms.numGroupHairs = mNumGroupHairs;
	program->uniforms.groupSpread = mGroupSpread;
	program->uniforms.hairRadius = mHairRadius;
	program->uniforms.splineTension = mSplineTension;
	program->uniforms.noiseAmplitude = mNoiseAmplitude;
	program->uniforms.noiseFrequency = mNoiseFrequency;
	program->uniforms.numSplineVertices = mNumSplineVertices;
	program->SetObjectUniforms();

	if (useTradAlphaBlending) {
		std::sort(mGuideHairs.begin(), mGuideHairs.end(), [&](const Strand* a, const Strand* b) {

			// Compare based on the distance from the camera
			glm::vec4 clipPositionA = cameraProjection * glm::vec4(a->vertices[0]->position, 1.0f);
			glm::vec4 clipPositionB = cameraProjection * glm::vec4(b->vertices[0]->position, 1.0f);

			// Sort in descending order
			return clipPositionA.z / clipPositionA.w > clipPositionB.z / clipPositionB.w;
			});
	}
	
	for (auto& guide : mGuideHairs)
	{
		//std::cout << "First vertex z-coordinate: " << guide->vertices[0]->position.z << std::endl;
		guide->Draw(program);
	}
}

void Hair::SetAttributes(Hair* oldHairAttributes)
{
	if (!oldHairAttributes)
	{
		SetAttributes();
	}
	else
	{
		SetAttributes(oldHairAttributes->mColour, oldHairAttributes->mNumGroupHairs, oldHairAttributes->mGroupSpread, oldHairAttributes->mHairRadius, oldHairAttributes->mSplineTension, oldHairAttributes->mNoiseAmplitude, oldHairAttributes->mNumSplineVertices);
	}
}

void Hair::SetAttributes(glm::vec3 colour, int numGroupHairs, float groupSpread, float hairRadius, float splineTension, float noiseAmplitude, float noiseFrequency, int numSplineVertices, float shadowIntensity, 
	float ambientIntensity, float diffuseIntensity, float specularIntensity, 
	float alphaR_, float alphaTRT, float alphaTT,
	float expR_, float expTRT, float expTT,
	float opacity, float colourChange)
{
	mColour = colour;
	mNumGroupHairs = numGroupHairs;
	mGroupSpread = groupSpread;
	mHairRadius = hairRadius;
	mSplineTension = splineTension;
	mNoiseAmplitude = noiseAmplitude;
	mNoiseFrequency = noiseFrequency;
	mNumSplineVertices = numSplineVertices;
	mShadowIntensity = shadowIntensity;

	mAlphaR_ = alphaR_;
	mAlphaTRT = alphaTRT;
	mAlphaTT = alphaTT;

	mExpR_ = expR_; 
	mExpTRT = expTRT; 
	mExpTT = expTT;

	mAmbientIntensity = ambientIntensity;
	mDiffuseIntensity = diffuseIntensity;
	mSpecularIntensity = specularIntensity;

	mOpacity = opacity;
	mColourChange = colourChange;
}

std::vector<Strand*> Hair::GetGuideHairs() const
{
	return mGuideHairs;
}

int Hair::GetNumGroupHairs() const
{
	return mNumGroupHairs;
}

void Hair::SetNumGroupHairs(const int num)
{
	mNumGroupHairs = num;
}

void Hair::SetNumSplineVertices(const int num)
{
	mNumSplineVertices = num;
}

void Hair::SetColour(const glm::vec3 colour)
{
	mColour = colour;
}

void Hair::SetShadowIntensity(const float intensity)
{
	mShadowIntensity = intensity;
}

void Hair::SetGroupSpread(const float spread)
{
	mGroupSpread = spread;
}

Texture* Hair::GetHairMap() const
{
	return mHairMap;
}

glm::vec3 Hair::GetColour() const
{
	return mColour;
}

float Hair::GetShadowIntensity() const
{
	return mShadowIntensity;
}

float Hair::GetAmbientIntensity() const
{
	return mAmbientIntensity;
}

float Hair::GetDiffuseIntensity() const
{
	return mDiffuseIntensity;
}

float Hair::GetSpecularIntensity() const
{
	return mSpecularIntensity;
}

float Hair::GetAlphaR() const
{
	return mAlphaR_;
}

float Hair::GetAlphaTRT() const
{
	return mAlphaTRT;
}

float Hair::GetAlphaTT() const
{
	return mAlphaTT;
}

float Hair::GetExpR() const
{
	return mExpR_;
}

float Hair::GetExpTRT() const
{
	return mExpTRT;
}

float Hair::GetExpTT() const
{
	return mExpTT;
}

float Hair::GetOpacity() const
{
	return mOpacity;
}

float Hair::GetColourChange() const
{
	return mColourChange;
}
