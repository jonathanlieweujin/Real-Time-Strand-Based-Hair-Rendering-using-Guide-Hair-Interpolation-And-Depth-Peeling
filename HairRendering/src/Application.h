#pragma once
#include "GuiWindow.h"
#include <glew.h>
#include <glfw3.h>
#include "shaderPrograms/ShaderProgram.h"
#include "Camera.h"
#include "Hair.h"
#include "Model.h"
#include <string>
#include "Light.h"

class Simulation;
class Texture;
class Framebuffer;

class Application
{
public:
	Application(int width, int height);
	~Application();

	void Run();

	// GUI
	double GetDeltaTime() const;
	Hair* GetHair() const;
	Light* GetLight() const;
	void TogglePause();
	void ResetSimulation();
	bool IsPaused() const;
	void SetHairMap(const std::string filename);
	void SetModel(const std::string filename);
	void SetPreset(const std::string preset);
	bool useColourGradient;
	bool renderSphere;
	bool useShadows;
	bool useDepthPeeling;
	bool useTradAlphaBlending;
	bool useDepthMask;
	int numDispGuideHair;
	float maxLength;
	float lightIntensity = 1.5f;
	float kajiyaSpecularExponent = 7.0f;

	int hairShaderType;
	
	float mZoomSens;
	glm::vec3 lightPosition;
	glm::vec3 camPosition;
	int depthPeelLayers;

private:
	void UpdateSettings();
	void Initialise();
	void InitRender();
	void Draw();
	void Update();
	void ProcessInput();
	static void FrameBufferCallback(GLFWwindow* window, int width, int height);
	void MouseCallback(GLFWwindow* window, double xPos, double yPos);
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void DrawMesh(ShaderProgram* program, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
	void DrawHair(ShaderProgram* program, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
	void DrawLight(glm::mat4 view, glm::mat4 projection);

	Model* mSphere;
	Model* mCollider;

	Hair* mHair;
	Simulation* mSimulation;

	std::string mHairMapName;
	std::string mModelName;
	std::string mColliderName;

	std::vector<ShaderProgram*> mPrograms;
	ShaderProgram* mMeshProgram;
	ShaderProgram* mKkHairProgram;
	ShaderProgram* mMarschnerHairProgram;
	ShaderProgram* mHairOpacityProgram;
	ShaderProgram* mWhiteHairProgram;
	ShaderProgram* mWhiteMeshProgram;
	ShaderProgram* mKkHairDepthPeelProgram;
	ShaderProgram* mMarschnerHairDepthPeelProgram;
	ShaderProgram* mMeshDepthPeelProgram;
	ShaderProgram* mLightShaderProgram;
	
	Texture* mNoiseTexture;

	std::vector<Framebuffer*> mFramebuffers;
	Framebuffer* mHairShadowFramebuffer;
	Framebuffer* mMeshShadowFramebuffer;
	Framebuffer* mOpacityMapFramebuffer;
	Framebuffer* mDepthPeelFramebuffer;
	Framebuffer* mDepthPeelFramebuffer1;

	Camera* mCamera;
	bool mFirstMouse;
	double mLastX;
	double mLastY;

	GLFWwindow* projectWindow;
	int mWidth;
	int mHeight;

	glm::mat4 mDirToLight;

	double mPrevTime;
	double mCurrentTime;
	double mDeltaTime;
	int mFrame;

	bool mIsPaused;
	/*bool mIsSpaceDown;*/

	GuiWindow* mGui;

	Light* mLight;
	float mLightRotate;
};