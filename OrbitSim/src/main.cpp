#include "Universe.h"

// Universe Application //
class UniverseApp : public GLCore::Application
{
public:
	UniverseApp()
		: GLCore::Application("Star-Body Orbit Simulator")
	{
		// Add the Sun-Body Universe Layer to layer stack
		PushLayer(new Universe());		
	}
private:
		
};

int main()
{
	std::unique_ptr<UniverseApp> app = std::make_unique<UniverseApp>();
	app->Run();
}