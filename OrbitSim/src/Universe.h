#pragma once

//#include <GLCoreUtils.h> // Orbit.h includes this
//#include "Body.h" // Orbit.h Includes this
//#include <GLAbstraction.h> // Trail.h Includes this
#include <GLCore.h>
#include "Orbit.h"
#include "Trail.h"



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
	// Moving Body Objects
	Body *body;
	Orbit *orbit;
	Trail *trail;

	// Body object Helper Functions
	void ResetOrbits();

	// Time Variables
	float UniverseTime;
	float dt;
	uint16_t fastForward;

	// Universe Rendering
	Renderer renderer;
	void TimeDisplay();
	void fastForwardDisplay();
	void ButtonDisplay();
	void StatsOverlay();

	// Universe Camera
	GLCore::Utils::OrthographicCameraController m_CameraController;
	
};


