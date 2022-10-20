#include "Universe.h"

using namespace GLCore;
using namespace GLCore::Utils;

static float mouseXpos = 0.0f;
static float mouseYpos = 0.0f;
static bool isPauseUniverse = false;
static bool bodyCrashed = false;
static bool showEnergyPlot = true;
static int fastForwardActive = 0;
static uint16_t fastForward = 1;

Universe::Universe()
	:Layer("SBsim"), m_CameraController((float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), false, 1.0f) // init camera controller with the window aspect ratio
{
	// Initiate the Sun-Body Universe
    InitUniverse();    
    // Initiate Sun-Body Universe UI
    universeUI = new UI(&UniverseTime, &dt, &m_CameraController, orbit);
}

Universe::~Universe()
{
	delete body;
    delete Sun;
    delete trail;
    delete orbit;
}

// Universe's gl prelims 
void Universe::OnAttach()
{
	EnableGLDebugging();

	// gl enable prelims //
    // Depth
    glDisable(GL_DEPTH_TEST);
    // Blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
    // AntiAliasing //
    // polygon Antialiasing
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Line Antialiasing
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

}

void Universe::OnDetach()
{
    // Unbind and delete buffers
	body->va.UnBind();
	body->vb->UnBind();
	Sun->va.UnBind();
	Sun->vb->UnBind();
	trail->va.UnBind();
	trail->vb->UnBind();
}

// Event Handling layer
void Universe::OnEvent(Event& event)
{
	// orthographic camera Event Dispatching (seperate dispatching)
	m_CameraController.OnEvent(event);
	EventDispatcher dispatcher(event);
    // Pause Menu Space Bar Bind
    dispatcher.Dispatch<KeyPressedEvent>(
        [&](KeyPressedEvent &e){
        // Pause Event
        if ((e.GetKeyCode() == KEY_SPACE) && !bodyCrashed)
        {
            if (e.GetRepeatCount() >= 1) { isPauseUniverse = false; }
            if (isPauseUniverse) { isPauseUniverse = false; }
            else if (!isPauseUniverse) { isPauseUniverse = true; }    
        } // Body Crashed Event
        else if ((e.GetKeyCode() == KEY_SPACE) && bodyCrashed)
        {
            if (e.GetRepeatCount() >= 1) { 
                bodyCrashed = false; 
                isPauseUniverse = false;
                ResetOrbits();
            }
            if (bodyCrashed) { 
                bodyCrashed = false;
                isPauseUniverse = false;
                ResetOrbits();
            }
            else if (!bodyCrashed) { 
                bodyCrashed = true;
                isPauseUniverse = true;
            }
        }
        return false;
    });
    
    // Only Process events If ImGui UI's are not currently processing events of their own
    if (!(universeUI->GetIO().WantCaptureMouse) || !(universeUI->GetIO().WantCaptureKeyboard) )
    {
        // Grab Raw Mouse Positions //
        dispatcher.Dispatch<MouseMovedEvent>(
            [&](MouseMovedEvent& mme) {
                mouseXpos = mme.GetX();
                mouseYpos = mme.GetY();
                return false;
            });
        // Mouse Click Body Positioning //
        dispatcher.Dispatch<MouseButtonReleasedEvent>(
            [&](MouseButtonReleasedEvent& mbe) {
                // Normalize and translate Mouse Positions to be within -1 and 1
                mouseXpos -= static_cast<float>(Application::Get().GetWindow().GetWidth()) / 2.0f;
                mouseYpos -= static_cast<float>(Application::Get().GetWindow().GetHeight()) / 2.0f;
                mouseXpos /= static_cast<float>(Application::Get().GetWindow().GetWidth()) / 2.0f;
                mouseYpos /= static_cast<float>(Application::Get().GetWindow().GetHeight()) / 2.0f;
                // Adjust for orthographic zoom
                mouseXpos *= m_CameraController.GetZoomLevel() * m_CameraController.GetAspectRatio();
                mouseYpos *= -1.0f * m_CameraController.GetZoomLevel();
                // Adjust for camera Movement
                mouseXpos += m_CameraController.GetCamPos().x;
                mouseYpos += m_CameraController.GetCamPos().y;
                // Set New Orbit Initial Positions to Mouse Position
                orbit->x0 = mouseXpos;
                orbit->y0 = mouseYpos;
                ResetOrbits();
                return false;
            });

        // Arrow Key Bind With Fast Forward
        dispatcher.Dispatch<KeyPressedEvent>(
            [&](KeyPressedEvent& e) {
                
                if (e.GetKeyCode() == KEY_LEFT)
                {
                    switch (fastForward)
                    {
                    case 1:
                        fastForward = 1;
                        fastForwardActive = 0;
                        break;
                    case 10:
                        fastForward = 1;
                        fastForwardActive = 0;
                        break;
                    case 50:
                        fastForward = 10;
                        fastForwardActive = 1;
                        break;
                    case 100:
                        fastForward = 50;
                        fastForwardActive = 2;
                        break;
                    case 500:
                        fastForward = 100;
                        fastForwardActive = 3;
                        break;

                    }
                } else if (e.GetKeyCode() == KEY_RIGHT)
                {
                    switch (fastForward)
                    {
                    case 1:
                        fastForward = 10;
                        fastForwardActive = 1;
                        break;
                    case 10:
                        fastForward = 50;
                        fastForwardActive = 2;
                        break;
                    case 50:
                        fastForward = 100;
                        fastForwardActive = 3;
                        break;
                    case 100:
                        fastForward = 500;
                        fastForwardActive = 4;
                        break;
                    case 500:
                        fastForward = 500;
                        fastForwardActive = 4;
                        break;
                    }
                }
                
                return true;
            });
    }
    
	
}

void Universe::OnUpdate(Timestep ts)
{     
	// Window Clearing and pause functions 
    if (!isPauseUniverse || bodyCrashed) { 
        Application::Get().GetWindow().Clear();
        Sun->setAlpha(1.0f);
        body->setAlpha(1.0f);
        trail->setAlpha(0.7f);
        //background->setColor(1.0f,1.0f,1.0f,1.0f);
    } else { PauseUniverse(); }

	// Key Handling (cam controller update)
	m_CameraController.OnUpdate(ts);
	// set view matrix and orthographic matrix product Uniforms for all Bodies and Trails
	body->Circle_shader->use();
	body->Circle_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
	trail->Trail_shader->use();
	trail->Trail_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
	Sun->Circle_shader->use();
	Sun->Circle_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
    
	// Physics Loop //
    if (!isPauseUniverse){ PhysicsLoop(); }

	// Render Universe //
    RenderUniverse();
}

void Universe::OnImGuiRender()
{
    static bool isOrbitReset = false;
    universeUI->UpdateWorkSize(); // must include first to update the working viewport size and position
    // UI ELEMENTS //
    universeUI->TimeOverlay();
    universeUI->fastForwardOverlay(fastForward, fastForwardActive);
    universeUI->ButtonOverlay(isOrbitReset, showEnergyPlot);
    universeUI->StatsOverlay();
    universeUI->InputsOverlay(isOrbitReset);
    if (isPauseUniverse && !bodyCrashed) { universeUI->PauseMenu(); }
    else if (isPauseUniverse && bodyCrashed) { universeUI->CrashMenu(); }
    if (showEnergyPlot) { universeUI->EnergyPlot(isPauseUniverse); }
    // END OF UI ELEMENTS //
    if (isOrbitReset) { ResetOrbits(); isOrbitReset = false; } 
}
//////////////////////////////////////////////////////////////////////////////////

// UNIVERSE FUNCTIONS //
void Universe::InitUniverse()
{
    // Init Universe
    UniverseTime = 0.0f;
    dt = 0.0001;
    // Objects
    Sun = new Body(0, 1.0f, 1.0f);
    body = new Body(1, 3e-6f, 0.1f);
    trail = new Trail;
    orbit = new Orbit(body,1.0f, 0.0f, 0.0f, 2*PI, 2.0f, UniverseTime, dt);

    // Set Object Colors
    body->setColor(0.1f, 0.1f, 0.6f, 1.0f);
    trail->setColor(glm::vec4{ 0.5f,0.4f,0.4f,0.7f});
    Sun->setColor(1.0f, 1.0f, 0.0f, 1.0f);
}

void Universe::PhysicsLoop()
{
    // Transform Body Positions	
    glm::vec3 newpos = glm::vec3(1.0f);
    newpos.x = orbit->x;
    newpos.y = orbit->y; // Correct for screen aspect ratio
    body->body_Transform.setPosition(newpos);

    // Scale Bodies To see them better.
    glm::vec3 newScale = glm::vec3(1.0f);
    newScale.x = 0.4f;
    newScale.y = newScale.x;
    Sun->body_Transform.setScale(newScale);
    body->body_Transform.setScale(newScale);

    // Update Orbit
    for (uint16_t i = 0; i < fastForward; i++)
    {
        orbit->Update(UniverseTime, dt, false);
        // Update Orbit Trail
        newpos.x = orbit->x;
        newpos.y = orbit->y;
        // Update Time Variable
        UniverseTime += dt;
        // update Trail in intervals
        if (i % 500 == 0 || i % 500 == 1)
            trail->UpdateTrail(newpos.x, newpos.y, !orbit->finishedPeriod);
        
        // Collision Detection //
        detectCollision(orbit->r, body->radius, Sun->radius, newScale.x);
    }
}

void Universe::detectCollision(const float& orbitR, const float& body1R, const float& body2R, const float& scale)
{
    // orbit from edge of body instead of center
    if ((orbitR - (body1R * scale)) <= body2R * scale)
    {
        bodyCrashed = true;
        isPauseUniverse = true;
    }
}

void Universe::ResetOrbits()
{
    UniverseTime = 0.0f;
    orbit->Reset();
    trail->ResetVertices();
}

void  Universe::PauseUniverse()
{
    Application::Get().GetWindow().Clear(1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f);
    Sun->setAlpha(0.5f);
    body->setAlpha(0.5f);
    trail->setAlpha(0.5f);
}

void Universe::RenderUniverse()
{
    // Trail																 
    renderer.DrawLineStrip(trail->va, *(trail->Trail_shader), trail->trail_Transform, (unsigned int)trail->vertices.size() / 3); //- divide trail by size/3 since GL_LINE_STRIP count is num of vertices
    // Bodies
    renderer.DrawCircle(body->va, *(body->Circle_shader), body->body_Transform, body->NumberOfVertices);
    renderer.DrawCircle(Sun->va, *(Sun->Circle_shader), Sun->body_Transform, Sun->NumberOfVertices);
}




