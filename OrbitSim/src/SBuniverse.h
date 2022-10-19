#pragma once

//#include <GLCoreUtils.h> // Orbit.h includes this
//#include "Body.h" // Orbit.h Includes this
//#include <GLAbstraction.h> // Trail.h Includes this
#include "Orbit.h"
#include "Trail.h"
#include <GLCore.h>



class SBuniverse : public GLCore::Layer
{
public:
	SBuniverse();
	virtual ~SBuniverse();
	// Layer Functions
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
private:
	Body *Sun;
	Body *body;
	Orbit *orbit;
	Trail* trail;

	// SBuniverse Helper Functions
	void InitSBuniverse();
	void PhysicsLoop();
	void PauseSBuniverse();
	void ResetOrbits();
	void detectCollision(const float &orbitR, const float& body1R, const float& body2R, const float& scale);
	int scientificDivCount(float *num);
	
	// Time Variables
	float SBuniverseTime;
	float dt;
	uint16_t fastForward;
	// SBuniverse Rendering
	Renderer renderer;
	void RenderSBuniverse();

	////////// SBuniverse UI //////////
	ImGuiStyle *style;
	ImPlotStyle* plotStyle;
	const ImGuiViewport* viewport;
	void InitImGuiGlobalStyling();
	void TimeDisplay(const ImVec2 &work_pos, const ImVec2& work_size);
	void fastForwardDisplay(const ImVec2& work_pos, const ImVec2& work_size);
	void ButtonDisplay(const ImVec2& work_pos, const ImVec2& work_size);
	void StatsOverlay(const ImVec2& work_pos, const ImVec2& work_size);
	void InputsOverlay(const ImVec2& work_pos, const ImVec2& work_size);
	void PauseMenu(const ImVec2& work_pos, const ImVec2& work_size);
	void CrashMenu(const ImVec2& work_pos, const ImVec2& work_size);

	// Plots
	void EnergyPlot(const ImVec2& work_pos, const ImVec2& work_size, float& orbitKE, float& orbitPE);

	////////// SBuniverse Camera //////////
	GLCore::Utils::OrthographicCameraController m_CameraController;

};


// utility structure for realtime plot
struct RollingBuffer {
	float Span;
	ImVector<ImVec2> Data;
	RollingBuffer() {
		Span = 500.0f;
		Data.reserve(2000);
	}
	void AddPoint(float x, float y) {
		float xmod = fmodf(x, Span);
		if (!Data.empty() && xmod < Data.back().x)
			Data.shrink(0);
		Data.push_back(ImVec2(xmod, y));
	}
	void Reset() {
		Data.clear();
	}
};




