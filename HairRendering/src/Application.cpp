#include "Application.h"
#include <iostream>
#include "shaderPrograms/HairShaderProgram.h"
#include "shaderPrograms/MeshShaderProgram.h"
#include "shaderPrograms/HairOpacityShaderProgram.h"
#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtx/transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Simulation.h"
#include "Model.h"
#include "Texture.h"
#include "Framebuffer.h"
#include <SOIL2.h>

Application::Application(int width, int height)
{
	mPrevTime = glfwGetTime();
	mCurrentTime = glfwGetTime();
	mFrame = 0;

	// Window width and height
	mWidth = width;
	mHeight = height;

	mFirstMouse = true;
	mZoomSens = 0.2f;
	mLastX = width / 2.0;
	mLastY = width / 2.f;
	mIsPaused = true;
	/*mIsSpaceDown = false; // Space Down Entry Boolean*/

	useShadows = true;
	useColourGradient = true;
	useDepthPeeling = false;
	depthPeelLayers = 3;
	useTradAlphaBlending = false;
	useDepthMask = true;

	mHairMapName = "black.png";
	mModelName = "sphere_model.obj";
	mColliderName = "sphere_low_poly.obj";

	renderSphere = true;

	numDispGuideHair = 400;
	maxLength = 0.5;

	lightPosition = glm::vec3(-1.831f, 7.465f, 9.726f);

	mLightRotate = 0.0f;

	Initialise();
}

Application::~Application()
{
	for (auto& program : mPrograms)
	{
		delete program;
	}

	for (auto& fb : mFramebuffers)
	{
		delete fb;
	}

	delete mGui;
	delete mSphere;
	delete mCollider;
	delete mSimulation;
	delete mHair;
	delete mCamera;
	delete mLight;
	delete mNoiseTexture;
}

void Application::Run()
{
	//Loop until window closes
	while (!glfwWindowShouldClose(projectWindow))
	{
		Update();
	}
}

double Application::GetDeltaTime() const
{
	return mDeltaTime;
}

Hair* Application::GetHair() const
{
	return mHair;
}

Light* Application::GetLight() const
{
	return mLight;
}

void Application::TogglePause()
{
	mIsPaused = !mIsPaused;
}

void Application::ResetSimulation()
{
	InitRender();
}

bool Application::IsPaused() const
{
	return mIsPaused;
}

void Application::SetHairMap(const std::string filename)
{
	mHairMapName = filename;

	UpdateSettings();
}

void Application::SetModel(const std::string filename)
{
	mModelName = filename;

	//Get collider name
	std::string collider = "";
	for (int i = 0; i < mModelName.length() - 4; i++)
	{
		collider += mModelName[i];
	}
	collider += "Collider.obj";
	mColliderName = collider;

	//Set hair map
	if (mModelName == "sphere_model.obj")
	{
		SetHairMap("black.png");
	}

	UpdateSettings();
}

void Application::SetPreset(const std::string preset)
{
	if (preset == "Hair")
	{
		SetModel("sphere_model.obj");
		SetHairMap("black.png");

		maxLength = 0.5f;
	}
	UpdateSettings();
}


void Application::UpdateSettings()
{
	if (mModelName == "sphere_model.obj")
	{
		numDispGuideHair = 400;
	}
}

void Application::Initialise()
{
	//Init GLFW
	if (!glfwInit())
	{
		std::cout << "ERROR: Failed to init GLFW" << std::endl;
		glfwTerminate();
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwSwapInterval(1);

	//Create window
	projectWindow = glfwCreateWindow(mWidth, mHeight, "FYP BS20110240", NULL, NULL);
	if (!projectWindow)
	{
		std::cout << "ERROR: Failed to create window" << std::endl;
		glfwTerminate();
	}

	//Set window icon
	GLFWimage icon[1];
	icon[0].pixels = SOIL_load_image("../../../images/icon.png", &icon[0].width, &icon[0].height, 0, SOIL_LOAD_RGBA);
	glfwSetWindowIcon(projectWindow, 1, icon);
	SOIL_free_image_data(icon[0].pixels);

	glfwMakeContextCurrent(projectWindow);
	glfwSetInputMode(projectWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwMaximizeWindow(projectWindow);

	//Callback functions
	glfwSetFramebufferSizeCallback(projectWindow, FrameBufferCallback);

	glfwSetWindowUserPointer(projectWindow, this);
	auto CursorCallback = [](GLFWwindow* window, double xPos, double yPos)
	{
		Application* app = (Application*)glfwGetWindowUserPointer(window);
		app->MouseCallback(window, xPos, yPos);
	};

	glfwSetCursorPosCallback(projectWindow, CursorCallback);

	auto MouseCallback = [](GLFWwindow* window, int button, int action, int mods)
	{
		Application* app = (Application*)glfwGetWindowUserPointer(window);
		app->MouseButtonCallback(window, button, action, mods);
	};

	glfwSetMouseButtonCallback(projectWindow, MouseCallback);

	auto ScrollCallback = [](GLFWwindow* window, double xOffset, double yOffset)
	{
		Application* app = (Application*)glfwGetWindowUserPointer(window);
		app->MouseScrollCallback(window, xOffset, yOffset);
	};

	glfwSetScrollCallback(projectWindow, ScrollCallback);

	//Initialise GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "ERROR: Failed to initialise GLEW" << std::endl;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Background Colour

	//Initialise GUI
	mGui = new GuiWindow(projectWindow);
	mGui->SetApplication(this);

	// Load Camera
	mCamera = new Camera(3.0f, glm::perspective(0.8f, (float)mWidth / mHeight, 0.1f, 100.0f));
	camPosition = mCamera->GetPosition();
	mLight = new Light(lightPosition, glm::vec3(1.0f, 1.0f, 1.0f));

	// Load Shaders

	hairShaderType = 1;
	mPrograms = {
		mMeshProgram = new MeshShaderProgram(),
		mKkHairProgram = new HairShaderProgram("hair.vert", "kkHair.frag"),
		mMarschnerHairProgram = new HairShaderProgram("hair.vert", "marschnerHair.frag"),
		mHairOpacityProgram = new HairOpacityShaderProgram(),
		mWhiteHairProgram = new HairShaderProgram("hair.vert", "white.frag"),
		mWhiteMeshProgram = new MeshShaderProgram("mesh.vert", "white.frag"),
		mKkHairDepthPeelProgram = new HairShaderProgram("hair.vert", "kkHairDepthPeel.frag"),
		mMarschnerHairDepthPeelProgram = new HairShaderProgram("hair.vert", "marschnerHairDepthPeel.frag"),
		mMeshDepthPeelProgram = new MeshShaderProgram("mesh.vert", "meshDepthPeel.frag"),
		mLightShaderProgram = new ShaderProgram("mesh.vert", "light.frag"),
	};
	
	//Textures
	mNoiseTexture = new Texture();
	mNoiseTexture->Create("Noise.jpg", GL_LINEAR, GL_LINEAR);

	//Framebuffers
	int shadowMapSize = 4096;
	glm::vec2 size = glm::vec2(2 * mWidth, 2 * mHeight);
	mFramebuffers = {
		mHairShadowFramebuffer = new Framebuffer(),
		mMeshShadowFramebuffer = new Framebuffer(),
		mOpacityMapFramebuffer = new Framebuffer(),
		mDepthPeelFramebuffer = new Framebuffer(),
		mDepthPeelFramebuffer1 = new Framebuffer(),
	};

	for (auto& fb : mFramebuffers)
	{
		fb->Create();
	}

	mHairShadowFramebuffer->GenerateDepthTexture(shadowMapSize, shadowMapSize, GL_NEAREST, GL_NEAREST);
	mMeshShadowFramebuffer->GenerateDepthTexture(shadowMapSize, shadowMapSize, GL_LINEAR, GL_LINEAR);
	mOpacityMapFramebuffer->GenerateTexture(shadowMapSize, shadowMapSize, GL_NEAREST, GL_NEAREST);
	mOpacityMapFramebuffer->GenerateDepthBuffer(shadowMapSize, shadowMapSize);
	mDepthPeelFramebuffer->GenerateTexture(size.x, size.y, GL_LINEAR, GL_LINEAR);
	mDepthPeelFramebuffer->GenerateDepthTexture(size.x, size.y, GL_NEAREST, GL_NEAREST);
	mDepthPeelFramebuffer1->GenerateTexture(size.x, size.y, GL_LINEAR, GL_LINEAR);
	mDepthPeelFramebuffer1->GenerateDepthBuffer(size.x, size.y);

	InitRender();
}

void Application::InitRender()
{
	if (mSphere)
	{
		mSphere = nullptr;
	}

	if (mCollider)
	{
		mCollider = nullptr;
	}

	if (mSimulation)
	{
		mSimulation = nullptr;
	}

	//Sphere model
	mSphere = new Model(mModelName);

	//Collision model
	mCollider = new Model(mColliderName, 1.1f);

	mSimulation = new Simulation(mCollider->GetFirstMesh());

	mHair = new Hair(mSphere->GetFirstMesh(), numDispGuideHair, mHairMapName.c_str(), (double)maxLength, mSimulation);

}


void Application::Draw()
{
	//Resize depth peel framebuffers
	int width, height;
	/*glm::vec4 cameraPosition = mCamera->GetPosition();
	std::cout << cameraPosition.w << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << std::endl;*/
	{
		/*width = mWidth;
		height = mHeight;

		Texture* texture = mDepthPeelFramebuffer->GetColourTexture();
		if (texture->GetWidth() != width || texture->GetHeight() != height)
		{
			mDepthPeelFramebuffer->GetColourTexture()->Resize(width, height);
			mDepthPeelFramebuffer->GetDepthTexture()->Resize(width, height);
			mDepthPeelFramebuffer1->GetColourTexture()->Resize(width, height);
			mDepthPeelFramebuffer1->ResizeDepthBuffer(width, height);
		}*/
	}

	//Update hair
	if (!mIsPaused)
	{
		float time = mFrame++ / 60.0f;
		mSimulation->Update(time);
		mHair->Update(time);

		/* if (orbitLight) Rotate Light
		{
			lightPosition = glm::vec3(cos(mLightRotate) * 10.0f, lightPosition.y, sin(mLightRotate) * 10.0f);
			mLightRotate += 0.7f * mDeltaTime;
		} */
	}

	mGui->NewFrame();

	//Light
	glm::mat4 model = mSimulation->GetTransform();
	mLight->SetPosition(lightPosition);
	glm::mat4 lightProjection = glm::perspective(1.3f, 1.0f, 1.0f, 100.0f);
	glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mDirToLight = lightProjection * lightView * glm::inverse(mCamera->GetView());

	//Bind textures
	mNoiseTexture->Bind(GL_TEXTURE0);
	mHairShadowFramebuffer->GetDepthTexture()->Bind(GL_TEXTURE1);
	mOpacityMapFramebuffer->GetColourTexture()->Bind(GL_TEXTURE2);
	mMeshShadowFramebuffer->GetDepthTexture()->Bind(GL_TEXTURE3);
	mHair->GetHairMap()->Bind(GL_TEXTURE5);
	mDepthPeelFramebuffer->GetDepthTexture()->Bind(GL_TEXTURE6);
	mDepthPeelFramebuffer->GetColourTexture()->Bind(GL_TEXTURE7);
	mDepthPeelFramebuffer1->GetColourTexture()->Bind(GL_TEXTURE8);

	//Shadow map
	if (useShadows)
	{
		//Hair shadow map
		mHairShadowFramebuffer->Bind();
		glViewport(0, 0, mHairShadowFramebuffer->GetDepthTexture()->GetWidth(), mHairShadowFramebuffer->GetDepthTexture()->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawHair(mWhiteHairProgram, model, lightView, lightProjection);

		//Mesh shadow map
		mMeshShadowFramebuffer->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Render mesh shadow
		if (renderSphere) {
			DrawMesh(mWhiteMeshProgram, model, lightView, lightProjection);
		}
		
		//Opacity map - Additive blending
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		mOpacityMapFramebuffer->Bind();
		glViewport(0, 0, mHairShadowFramebuffer->GetDepthTexture()->GetWidth(), mHairShadowFramebuffer->GetDepthTexture()->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		DrawHair(mHairOpacityProgram, model, lightView, lightProjection);

		mOpacityMapFramebuffer->Unbind();
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}

	if (useDepthPeeling)
	{
		glViewport(0, 0, mDepthPeelFramebuffer->GetColourTexture()->GetWidth(), mDepthPeelFramebuffer->GetColourTexture()->GetHeight());

		//1st layer
		mDepthPeelFramebuffer->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (hairShaderType == 0) {
			DrawHair(mKkHairProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}
		else if (hairShaderType == 1) {
			DrawHair(mMarschnerHairProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}
		
		
		if (renderSphere) {
			DrawMesh(mMeshProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}
		DrawLight(mCamera->GetView(), mCamera->GetProjection());

		//2nd layer
		mDepthPeelFramebuffer1->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (hairShaderType == 0) {
			DrawHair(mKkHairDepthPeelProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}
		else if (hairShaderType == 1) {
			DrawHair(mMarschnerHairDepthPeelProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}
		
		if (renderSphere) {
			DrawMesh(mMeshDepthPeelProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}

		DrawLight(mCamera->GetView(), mCamera->GetProjection());

		//for (int i = 0; i < depthPeelLayers - 2; i++) {
		mDepthPeelFramebuffer1->Unbind();
		glViewport(0, 0, mWidth, mHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		mDepthPeelFramebuffer1->GetColourTexture()->RenderFullScreen();
		//}	

		//Blend top layers
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		mDepthPeelFramebuffer->GetColourTexture()->RenderFullScreen();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	else if (useTradAlphaBlending) {
		glViewport(0, 0, mWidth, mHeight);

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Enable blending
		glEnable(GL_BLEND);

		if (!useDepthMask) {
			glDepthMask(GL_FALSE);
		}

		// Set the blending function
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Draw the hair with alpha blending
		if (hairShaderType == 0) {
			DrawHair(mKkHairProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}
		else if (hairShaderType == 1) {
			DrawHair(mMarschnerHairProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		//Render mesh
		if (renderSphere) {
			DrawMesh(mMeshProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}

		//Render light
		DrawLight(mCamera->GetView(), mCamera->GetProjection());
	}

	else
	{
		{
			glViewport(0, 0, mWidth, mHeight);
		}

		//Render hair
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (hairShaderType == 0) {
			DrawHair(mKkHairProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}
		else if (hairShaderType == 1) {
			DrawHair(mMarschnerHairProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}

		// Clear the color and depth buffers
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Enable blending
		//glEnable(GL_BLEND);

		// Set the blending function
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Draw the hair with alpha blending
		//DrawHair(mKkHairProgram, model, mCamera->GetView(), mCamera->GetProjection());

		// Disable blending if it's not needed for subsequent rendering
		//glDisable(GL_BLEND);

		//Render mesh
		if (renderSphere) {
			DrawMesh(mMeshProgram, model, mCamera->GetView(), mCamera->GetProjection());
		}

		//Render light
		DrawLight(mCamera->GetView(), mCamera->GetProjection());
	}

	mNoiseTexture->Unbind(GL_TEXTURE0);
	mHairShadowFramebuffer->GetDepthTexture()->Unbind(GL_TEXTURE1);
	mOpacityMapFramebuffer->GetColourTexture()->Unbind(GL_TEXTURE2);
	mMeshShadowFramebuffer->GetDepthTexture()->Unbind(GL_TEXTURE3);
	mHair->GetHairMap()->Unbind(GL_TEXTURE5);
	mDepthPeelFramebuffer->GetDepthTexture()->Unbind(GL_TEXTURE6);
	mDepthPeelFramebuffer->GetColourTexture()->Unbind(GL_TEXTURE7);
	mDepthPeelFramebuffer1->GetColourTexture()->Unbind(GL_TEXTURE8);
}

void Application::Update()
{
	/*if (!mIsPaused)
	{*/
	mCurrentTime = glfwGetTime();
	mDeltaTime = mCurrentTime - mPrevTime;
	//}


	if (!ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard)
	{
		ProcessInput();
	}

	//Draw 60 times per second
	if (mDeltaTime >= 1.0f / 60.0f)
	{
		//std::cout << 1 / mDeltaTime << " FPS" << std::endl;
		Draw();

		mGui->Update();

		//Swap front and back buffers
		glfwSwapBuffers(projectWindow);

		mPrevTime = mCurrentTime;
	}

	//Poll events
	glfwPollEvents();
}

void Application::ProcessInput()
{
	//Escape to exit
	if (glfwGetKey(projectWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(projectWindow, true);
	}

	//Space to toggle pause
	/*if (glfwGetKey(projectWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		if (!mIsSpaceDown)
		{
			TogglePause();
		}

		mIsSpaceDown = true;
	}
	else
	{
		mIsSpaceDown = false;
	}*/

}

void Application::FrameBufferCallback(GLFWwindow* window, int width, int height)
{
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
}

void Application::MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	//Ignore mouse if over GUI
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	//Left mouse to move hair
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glm::mat4 view = mCamera->GetView();
		glm::vec3 up = glm::normalize(glm::vec3(view[2][1], view[2][2], view[2][3]));
		glm::vec3 forward = glm::normalize(glm::vec3(glm::inverse(view) * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f)));
		glm::vec3 right = glm::cross(up, forward);
		glm::vec2 delta = glm::vec2(xPos, yPos) - mCamera->GetPreviousPosition();
		glm::vec3 transform = glm::vec3();
		transform += delta.x * 0.005f * right;
		transform += -delta.y * 0.005f * up;

		mSimulation->UpdatePosition(mHair, transform);
		mCamera->SetPreviousPosition(glm::vec2(xPos, yPos));
		camPosition = mCamera->GetPosition();
	}

	//Right mouse to rotate camera
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		float x = 10 * (xPos - mCamera->GetPrevMousePosition().x) / (float)mWidth;
		float y = 10 * (yPos - mCamera->GetPrevMousePosition().y) / (float)mHeight;

		mCamera->SetAngles(x, y);
		mCamera->UpdateViewMatrix();
		mCamera->SetPrevMousePosition(glm::vec2(xPos, yPos));
		camPosition = mCamera->GetPosition();
	}

	if (mIsPaused)
	{
		Update();
	}
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			mCamera->SetPreviousPosition(glm::vec2(x, y));
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			mCamera->SetPrevMousePosition(glm::vec2(x, y));
		}

		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			double x, y;
			glfwGetCursorPos(window, &x, &y);
			mCamera->SetPreviousRotation(glm::vec2(x, y));
		}
	}
}

void Application::MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
	{
		return;
	}

	mCamera->SetZoom(mZoomSens * yOffset); // Zoom In EFfect
	mCamera->UpdateViewMatrix();

	if (mIsPaused)
	{
		Update();
	}
}

void Application::DrawMesh(ShaderProgram* program, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	program->Bind();
	program->uniforms.hairShadowMap = 1;
	program->uniforms.opacityMap = 2;
	program->uniforms.meshShadowMap = 3;
	program->uniforms.hairMap = 5;
	program->uniforms.projection = projection;
	program->uniforms.view = view;
	program->uniforms.model = model;
	program->uniforms.lightPosition = lightPosition;
	program->uniforms.camPosition = camPosition;
	program->uniforms.lightColour = mLight->GetColour();
	program->uniforms.dirToLight = mDirToLight;
	program->uniforms.shadowIntensity = mHair->GetShadowIntensity();
	program->uniforms.useShadows = useShadows;
	program->uniforms.colour = 2.0f * mHair->GetColour();

	program->uniforms.lightIntensity = lightIntensity;

	program->SetGlobalUniforms();
	program->SetObjectUniforms();
	mSphere->Draw();
}

void Application::DrawHair(ShaderProgram* program, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	program->Bind();
	program->uniforms.noiseTexture = 0;
	program->uniforms.hairShadowMap = 1;
	program->uniforms.opacityMap = 2;
	program->uniforms.meshShadowMap = 3;
	program->uniforms.depthPeelMap = 6;
	//program->uniforms.depthBufferMap = 6;
	program->uniforms.projection = projection;
	program->uniforms.view = view;
	program->uniforms.model = model;
	program->uniforms.dirToLight = mDirToLight;
	program->uniforms.lightPosition = lightPosition;
	program->uniforms.camPosition = camPosition;
	program->uniforms.lightColour = mLight->GetColour();
	program->uniforms.shadowIntensity = mHair->GetShadowIntensity();
	program->uniforms.useShadows = useShadows;
	program->uniforms.useColourGradient = useColourGradient;

	program->uniforms.ambientIntensity = mHair->GetAmbientIntensity();
	program->uniforms.specularIntensity = mHair->GetSpecularIntensity();
	program->uniforms.diffuseIntensity = mHair->GetDiffuseIntensity();

	program->uniforms.alphaR_ = mHair->GetAlphaR();
	program->uniforms.alphaTRT = mHair->GetAlphaTRT();
	program->uniforms.alphaTT = mHair->GetAlphaTT();

	program->uniforms.expR_ = mHair->GetExpR();
	program->uniforms.expTRT = mHair->GetExpTRT();
	program->uniforms.expTT = mHair->GetExpTT();

	program->uniforms.lightIntensity = lightIntensity;
	program->uniforms.kajiyaSpecularExponent = kajiyaSpecularExponent;

	program->uniforms.opacity = mHair->GetOpacity();
	program->uniforms.maxColourChange = mHair->GetColourChange();
	program->SetGlobalUniforms();
	mHair->Draw(program, mCamera->GetProjection(), useTradAlphaBlending);
}


void Application::DrawLight(glm::mat4 view, glm::mat4 projection)
{
	mLightShaderProgram->Bind();
	mLightShaderProgram->uniforms.projection = projection;
	mLightShaderProgram->uniforms.view = view;
	mLightShaderProgram->uniforms.model = glm::mat4(1.0f);
	mLightShaderProgram->uniforms.colour = mLight->GetColour();
	mLightShaderProgram->SetGlobalUniforms();
	mLightShaderProgram->SetObjectUniforms();
	mLight->Draw();
}
