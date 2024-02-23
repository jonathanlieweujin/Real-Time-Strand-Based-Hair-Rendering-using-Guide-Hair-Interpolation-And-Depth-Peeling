#include "GuiWindow.h"
#include "Application.h"
#include "Simulation.h"
#include <string>

GuiWindow::GuiWindow(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 440");
}

GuiWindow::~GuiWindow()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void GuiWindow::NewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GuiWindow::Update()
{
	float windowWidth = ImGui::GetIO().DisplaySize.x * 0.3; // Window Width
	float windowHeight = ImGui::GetIO().DisplaySize.y; // Window Height

	// Window Position
	float xPos = ImGui::GetIO().DisplaySize.x - windowWidth;
	float yPos = 0;

	ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight)); // Set the size of the window
	ImGui::SetNextWindowPos(ImVec2(xPos, yPos));
	ImGui::Begin("Render Hair Details", NULL, ImGuiWindowFlags_NoCollapse);
	//ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.6f);
	
	Hair* hair = mApp->GetHair();
	Light* light = mApp->GetLight();

	// FPS Stats
	double fps = (1.0 / mApp->GetDeltaTime());
	double rate = (mApp->GetDeltaTime() * 1000);
	ImGui::Text((std::to_string(fps) + " FPS").c_str());
	ImGui::Text((std::to_string(rate) + " ms").c_str());

	int numGuideHairs = hair->GetGuideHairs().size();
	int numGroupHairs = hair->GetNumGroupHairs();
	std::string stats = std::to_string(numGuideHairs) + " guide hairs \n" + std::to_string(numGuideHairs * numGroupHairs) + " total hairs \n";
	ImGui::Text(stats.c_str());

	if (ImGui::CollapsingHeader("Settings"))
	{
		ImGui::Dummy(ImVec2(0, 3));

		// Pause Button
		/*std::string pauseLabel = "Pause";
		if (mApp->IsPaused())
		{
			pauseLabel = "Unpause";
		}
		if (ImGui::Button(pauseLabel.c_str(), ImVec2(100, 0)))
		{
			mApp->TogglePause();
		}

		// Create a horizontal layout group
		ImGui::SameLine(); 

		// Reset
		if (ImGui::Button("Reset", ImVec2(100, 0)))
		{
			mApp->ResetSimulation();
		}
		
		ImGui::SameLine();
		*/

		// Reset
		if (ImGui::Button("Reset", ImVec2(100, 0)))
		{
			mApp->ResetSimulation();
		}

		ImGui::Checkbox("Render Sphere", &mApp->renderSphere);

		//ImGui::SameLine();

		//ImGui::PushItemWidth(150.0f);  // Adjust the width as needed

		ImGui::Dummy(ImVec2(0, 1));

		ImGui::Separator();

		ImGui::Dummy(ImVec2(0, 1));

		// Zoom Effect Value
		ImGui::SliderFloat("Zoom Sensitivity", &mApp->mZoomSens, 0.1f, 2.0f);

		// Restore the default width
		/*ImGui::PopItemWidth();*/

		ImGui::Dummy(ImVec2(0, 3));
	}

	if (ImGui::CollapsingHeader("Light Details")) // , ImGuiTreeNodeFlags_DefaultOpen
	{
		ImGui::Dummy(ImVec2(0, 3));

		ImGui::SliderFloat3("Light position", &mApp->lightPosition.x, -10, 10);
		ImGui::ColorEdit3("Light colour", &light->mColour.x);
		ImGui::SliderFloat("Intensity", &mApp->lightIntensity, 0.0f, 5.0f);
		ImGui::Checkbox("Shadows", &mApp->useShadows);

		ImGui::Dummy(ImVec2(0, 3));
	}

	/*if (ImGui::CollapsingHeader("Simulation"))
	{
		ImGui::Dummy(ImVec2(0, 3));

		ImGui::InputFloat3("Wind direction", &hair->mSimulation->windDirection.x);
		ImGui::SliderFloat("Wind strength", &hair->mSimulation->windStrength, 0.0f, 50.0f);
		ImGui::SliderFloat("Stiffness", &hair->mSimulation->stiffness, 0.0f, 1.0f);
		ImGui::SliderFloat("Dampening", &hair->mSimulation->dampening, 0.0f, 1.0f);

		ImGui::Dummy(ImVec2(0, 3));
	}*/

	if (ImGui::CollapsingHeader("Hair Attributes"))
	{
		ImGui::Dummy(ImVec2(0, 3));

		ImGui::Text("Shader");

		const char* items[] = { "Kajiya-Kay", "Marschner" }; // Labels for the dropdown items
		ImGui::Combo("##combo", &mApp->hairShaderType, items, IM_ARRAYSIZE(items));


		ImGui::Checkbox("Colour Gradient Shader", &mApp->useColourGradient);
		
		if (!mApp->useTradAlphaBlending) {
			ImGui::Checkbox("Depth Peel OIT", &mApp->useDepthPeeling);
		}

		if (!mApp->useDepthPeeling)
		{
			ImGui::Checkbox("Traditional Alpha Blending", &mApp->useTradAlphaBlending);
		}
		/*if (mApp->useTradAlphaBlending) {
			ImGui::Checkbox("Use Depth Mask", &mApp->useDepthMask);
		}
		else {
			mApp->useDepthMask = true;
		}*/

		//if (mApp->useDepthPeeling) {
			//ImGui::InputInt("Depth Peel Render Layers", &mApp->depthPeelLayers, 1, 1);
			//ImGui::SliderInt("Depth Peel Render Layers", &mApp->depthPeelLayers, 3, 1000);
		//}
		/*if (ImGui::SliderFloat("Density", &mApp->hairDensity, 0.0f, 1000.0f))
		{
			mApp->ResetSimulation();
		}*/
		//ImGui::SliderFloat("Spline Tension", &hair->mSplineTension, 0.1f, 1.0f);
		if (ImGui::InputInt("Total hair strands", &totalHairDisplayed, 500, 500)) {
			totalHairDisplayed = std::max(static_cast<int>(totalHairDisplayed), 0);
			mApp->numDispGuideHair = (int)(totalHairDisplayed / hair->mNumGroupHairs);
			int oldGroupHair = hair->mNumGroupHairs;
			int oldGuideHair = mApp->numDispGuideHair;
			mApp->ResetSimulation();
			hair->SetNumGroupHairs(oldGroupHair);
			mApp->numDispGuideHair = oldGuideHair;
			std::cout << oldGroupHair << ", " << hair->mNumGroupHairs << std::endl;
			totalHairDisplayed = (int)(hair->mNumGroupHairs * mApp->numDispGuideHair);
		}
		if (ImGui::InputInt("Total guide hairs", &mApp->numDispGuideHair, 50, 50))
		{
			mApp->numDispGuideHair = std::max(static_cast<int>(mApp->numDispGuideHair), 0);
			int oldGuideHair = mApp->numDispGuideHair;
			mApp->ResetSimulation();
			mApp->numDispGuideHair = oldGuideHair;
			totalHairDisplayed = (int)(hair->mNumGroupHairs * mApp->numDispGuideHair);
		}
		if (ImGui::SliderInt("Strands per guide hair", &hair->mNumGroupHairs, 0, 64)) {
			totalHairDisplayed = (int)(hair->mNumGroupHairs * mApp->numDispGuideHair);
		}
		ImGui::SliderInt("Vertices per strand", &hair->mNumSplineVertices, 2, 50);
		if (ImGui::SliderFloat("Max length", &mApp->maxLength, 0.01, 2))
		{
			mApp->ResetSimulation();
		}
		ImGui::SliderFloat("Hair radius", &hair->mHairRadius, 0.0f, 0.1f);
		ImGui::SliderFloat("Group spread", &hair->mGroupSpread, 0.0f, 1.0f);
		ImGui::ColorEdit3("Hair colour", &hair->mColour.x);
		ImGui::SliderFloat("Noise amplitude", &hair->mNoiseAmplitude, 0.0f, 1.0f);
		ImGui::SliderFloat("Noise frequency", &hair->mNoiseFrequency, 0.0f, 1.0f);
		ImGui::SliderFloat("Shadow intensity", &hair->mShadowIntensity, 0.0f, 50.0f);

		ImGui::Dummy(ImVec2(0, 3));

		ImGui::SliderFloat("Ambient intensity", &hair->mAmbientIntensity, 0.0f, 5.0f);
		ImGui::SliderFloat("Diffuse intensity", &hair->mDiffuseIntensity, 0.0f, 5.0f);
		ImGui::SliderFloat("Specular intensity", &hair->mSpecularIntensity, 0.0f, 5.0f);
		if (mApp->hairShaderType == 0) {
			ImGui::SliderFloat("Specular exponent", &mApp->kajiyaSpecularExponent, 0.0f, 30.0f);
		}

		ImGui::Dummy(ImVec2(0, 3));

		if (mApp->hairShaderType == 1) {
			if (ImGui::SliderFloat("Longitudinal shift", &hair->mAlphaR_, 5.0f, 10.0f)) {
				hair->mAlphaTRT = hair->mAlphaR_ / 2;
				hair->mAlphaTT = 3 * hair->mAlphaR_ / 2;
			};
			/*ImGui::SliderFloat("Alpha TRT", &hair->mAlphaTRT, 0.0f, 180.0f);
			ImGui::SliderFloat("Alpha TT", &hair->mAlphaTT, 0.0f, 180.0f);*/

			//ImGui::Dummy(ImVec2(0, 3));

			if (ImGui::SliderFloat("Longitudinal width", &hair->mExpR_, 0.0f, 30.0f)) {
				hair->mExpTRT = hair->mExpR_ / 2;
				hair->mExpTT = 2 * hair->mExpR_;
			};
			//ImGui::SliderFloat("Exp TRT", &hair->mExpTRT, 1.0f, 100.0f);
			//ImGui::SliderFloat("Exp TT", &hair->mExpTT, 1.0f, 100.0f);
		}

		ImGui::Dummy(ImVec2(0, 3));

		if (mApp->useDepthPeeling || mApp->useTradAlphaBlending)
		{
			ImGui::SliderFloat("Opacity", &hair->mOpacity, 0.0f, 1.0f);
		}

		if (mApp->useColourGradient) 
		{
			ImGui::SliderFloat("Colour variation", &hair->mColourChange, 0.0f, 5.0f);
		}

		ImGui::Dummy(ImVec2(0, 3));
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GuiWindow::SetApplication(Application* app)
{
	mApp = app;
}