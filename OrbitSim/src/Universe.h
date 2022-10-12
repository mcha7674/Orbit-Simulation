#pragma once

//#include <GLCoreUtils.h> // Orbit.h includes this
//#include "Body.h" // Orbit.h Includes this
//#include <GLAbstraction.h> // Trail.h Includes this
#include "Orbit.h"
#include "Trail.h"
#include <GLCore.h>

class Universe : public GLCore::Layer
{
public:
	Universe();
	virtual ~Universe();
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

	// Universe Helper Functions
	void InitUniverse();
	void PhysicsLoop();
	void PauseUniverse();
	void ResetOrbits();

	// Pause Universe Activation Bool
	bool pauseUniverse;


	// Time Variables
	float UniverseTime;
	float dt;
	uint16_t fastForward;
	// Universe Rendering
	Renderer renderer;
	void RenderUniverse();
	// Universe UI
	ImGuiStyle *style;
	const ImGuiViewport* viewport;
	void InitImGuiGlobalStyling();
	void TimeDisplay(const ImVec2 &work_pos, const ImVec2& work_size);
	void fastForwardDisplay(const ImVec2& work_pos, const ImVec2& work_size);
	void ButtonDisplay(const ImVec2& work_pos, const ImVec2& work_size);
	void StatsOverlay(const ImVec2& work_pos, const ImVec2& work_size);
	void PauseMenu(const ImVec2& work_pos, const ImVec2& work_size);
	bool statOverlayFocused = false;
	// Universe Camera
	GLCore::Utils::OrthographicCameraController m_CameraController;

	
	
};


