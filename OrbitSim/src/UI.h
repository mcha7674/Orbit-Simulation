#pragma once
#include "Orbit.h"
#include "Trail.h"
#include <GLCore.h>
#include <functional>
using namespace GLCore;

int scientificDivCount(float* og_num);

class UI
{
public:
	UI(float* UniverseTime, float* TimeStep, GLCore::Utils::OrthographicCameraController* m_CameraController, Orbit* bodyOrbit);
	~UI() {}
	void UpdateWorkSize();
	void InitImGuiGlobalStyling();
	void TimeOverlay();
	void fastForwardOverlay(uint16_t& fastForward, int& fastForwardActive);
	void ButtonOverlay(bool& orbitReset, bool& showEnergyPlot);
	void StatsOverlay();
	void InputsOverlay(bool& orbitReset);
	void PauseMenu();
	void CrashMenu();
	void EnergyPlot(bool& pauseUniverse);

	// Getters
	inline ImGuiIO& GetIO() { return *io; }
private:
	ImGuiIO* io;
	ImGuiStyle* style;
	ImPlotStyle* plotStyle;
	const ImGuiViewport* viewport;
	ImVec2 work_pos;
	ImVec2 work_size;

	// Universe Objects/variables as pointers so we I can modify at original memory //
	float* UniverseTime;
	float* TimeStep;
	GLCore::Utils::OrthographicCameraController *m_CameraController;
	Orbit* bodyOrbit;
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



