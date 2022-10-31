#pragma once

//#include <GLCoreUtils.h> // Orbit.h includes this
//#include "Body.h" // Orbit.h Includes this
//#include <GLAbstraction.h> // Trail.h Includes this in orbit.h
#include <GLCore.h>
#include "Orbit.h"
#include "UI.h"


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
	////////// Universe OBJECTS //////////
	Body *star;
	Orbit *bodyOrbit;
	////////// Universe Helper Functions //////////
	void InitUniverse();
	void PhysicsLoop();
	void PauseUniverse();
	void ResetOrbits();
	void detectCollision(const float &orbitR, const float& body1R, const float& body2R, const float& scale);	
	bool isMouseOnBody(const float& orbitRX, const float& orbitRY, const float& bodyR, const float& scale);
	void transformMousePos();
	////////// Universe Time Variables //////////
	float UniverseTime;
	float dt;
	////////// Universe Rendering //////////
	Renderer renderer;
	void RenderUniverse();
	////////// Universe UI //////////
	UI *universeUI;
	////////// Universe Camera //////////
	GLCore::Utils::OrthographicCameraController m_CameraController;

};



