#include "SBuniverse.h"

// Universe Application //
class UniverseApp : public GLCore::Application
{
public:
	UniverseApp()
		: GLCore::Application("Orbit Simulator")
	{
		// Add the Universe Layer to layer stack
		PushLayer(new SBuniverse());
	}
};

int main()
{
	std::unique_ptr<UniverseApp> app = std::make_unique<UniverseApp>();
	app->Run();
}