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
	void detectCollision(const float &orbitR, const float& body1R, const float& body2R, const float& scale);
	// Universe State Booleans
	bool pauseUniverse;
	bool bodyCrashed;

	// Time Variables
	float UniverseTime;
	float dt;
	uint16_t fastForward;
	// Universe Rendering
	Renderer renderer;
	void RenderUniverse();

	////////// Universe UI //////////
	ImGuiStyle *style;
	const ImGuiViewport* viewport;
	void InitImGuiGlobalStyling();
	void TimeDisplay(const ImVec2 &work_pos, const ImVec2& work_size);
	void fastForwardDisplay(const ImVec2& work_pos, const ImVec2& work_size);
	void ButtonDisplay(const ImVec2& work_pos, const ImVec2& work_size);
	void StatsOverlay(const ImVec2& work_pos, const ImVec2& work_size);
	void PauseMenu(const ImVec2& work_pos, const ImVec2& work_size);
	void CrashMenu(const ImVec2& work_pos, const ImVec2& work_size);

	// Plots
	void EnergyPlot();
	void ShowDemo_RealtimePlots();

	bool statOverlayFocused = false;

	////////// Universe Camera //////////
	GLCore::Utils::OrthographicCameraController m_CameraController;

};

// utility structure for realtime plot
struct ScrollingBuffer {
	int MaxSize;
	int Offset;
	ImVector<ImVec2> Data;
	ScrollingBuffer(int max_size = 2000) {
		MaxSize = max_size;
		Offset = 0;
		Data.reserve(MaxSize);
	}
	void AddPoint(float x, float y) {
		if (Data.size() < MaxSize)
			Data.push_back(ImVec2(x, y));
		else {
			Data[Offset] = ImVec2(x, y);
			Offset = (Offset + 1) % MaxSize;
		}
	}
	void Erase() {
		if (Data.size() > 0) {
			Data.shrink(0);
			Offset = 0;
		}
	}
};
// utility structure for realtime plot
struct RollingBuffer {
	float Span;
	ImVector<ImVec2> Data;
	RollingBuffer() {
		Span = 10.0f;
		Data.reserve(2000);
	}
	void AddPoint(float x, float y) {
		float xmod = fmodf(x, Span);
		if (!Data.empty() && xmod < Data.back().x)
			Data.shrink(0);
		Data.push_back(ImVec2(xmod, y));
	}
};


