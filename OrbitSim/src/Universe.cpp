#include "Universe.h"

using namespace GLCore;
using namespace GLCore::Utils;

static float mouseXpos = 0.0f;
static float mouseYpos = 0.0f;
static float mouseXpos_Save = 0.0f;
static float mouseYpos_Save = 0.0f;
static float velocityDragWeight = 20.0f;
static float bodyScaling = 0.4f;
static bool isPauseUniverse = false;
static bool bodyCrashed = false;
static bool showEnergyPlot = true;
static int fastForwardActive = 0;
static uint16_t fastForward = 1;

Universe::Universe()
	:Layer("SBsim"), m_CameraController((float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), false, 1.0f) // init camera controller with the window aspect ratio
{
	// Initiate the star-Body Universe
    InitUniverse();    
    // Initiate star-Body Universe UI
    universeUI = new UI(&UniverseTime, &dt, &m_CameraController, bodyOrbit);
}

Universe::~Universe()
{
    delete star;
    delete bodyOrbit;
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
                // Get Mouse Positions
                mouseXpos = mme.GetX();
                mouseYpos = mme.GetY();
                // Mouse Coordinate Transformation (update mouseXpos and mouseYpos to NDC)
                transformMousePos();
                return true;
            });
        // Save Mouse Positions Once any mouse button is pressed
        dispatcher.Dispatch<MouseButtonPressedEvent>(
            [&](MouseButtonPressedEvent& mbe) {
                mouseXpos_Save = mouseXpos;
                mouseYpos_Save = mouseYpos;
                
                return true;
            });
        dispatcher.Dispatch<MouseButtonReleasedEvent>(
            [&](MouseButtonReleasedEvent& mbe) {
                if (mbe.GetMouseButton() == MOUSE_BUTTON_RIGHT)
                {
                    // Only Make Calculations if initial mouse click is on the body
                    if (isMouseOnBody(bodyOrbit->x, bodyOrbit->y, bodyOrbit->body->radius, bodyScaling)){
                        //std::cout << "mouse on Body" << std::endl;
                        // Drag Magnitude calculation for velocity
                        float vxMag = (mouseXpos - mouseXpos_Save) * velocityDragWeight;
                        float vyMag = (mouseYpos - mouseYpos_Save) * velocityDragWeight;
                        /*float newVx = vxMag + bodyOrbit->vx;
                        float newVy = vyMag + bodyOrbit->vy;*/
                        float newVx = vxMag;
                        float newVy = vyMag;
                        // Update Velocities
                        bodyOrbit->vx0 = newVx;
                        bodyOrbit->vy0 = newVy;
                        bodyOrbit->v0 = sqrt((newVx * newVx) + (newVy * newVy));
                        bodyOrbit->vx = bodyOrbit->vx0;
                        bodyOrbit->vy = bodyOrbit->vy0;
                        bodyOrbit->v = bodyOrbit->v0;
                    }
                    
                }
                return true;
            });
        // Mouse Click Body Positioning //
        dispatcher.Dispatch<MouseButtonPressedEvent>(
            [&](MouseButtonPressedEvent& mbe) {
                if (mbe.GetMouseButton() == MOUSE_BUTTON_LEFT)
                {
                    // Set New Orbit Initial Positions to Mouse Position
                    bodyOrbit->x0 = mouseXpos_Save;
                    bodyOrbit->y0 = mouseYpos_Save;
                    ResetOrbits();
                    return true;
                }
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

void Universe::transformMousePos()
{
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
}

bool Universe::isMouseOnBody(const float& orbitRX, const float& orbitRY, const float& bodyR, const float& scale)
{    
    // Approximate Body area to be a square to ease of calculation
    if ( (mouseXpos_Save <= orbitRX+(bodyR*scale))  && (mouseXpos_Save >= orbitRX - (bodyR * scale)) 
        && (mouseYpos_Save <= orbitRY + (bodyR * scale)) && (mouseYpos_Save >= orbitRY - (bodyR * scale)) )
    {
        return true;
    }
    return false;
}

void Universe::OnUpdate(Timestep ts)
{     
	// Window Clearing and pause functions 
    if (!isPauseUniverse || bodyCrashed) { 
        Application::Get().GetWindow().Clear();
        star->setAlpha(1.0f);
        bodyOrbit->body->setAlpha(1.0f);
        bodyOrbit->bodyTrail->setAlpha(0.7f);
        //background->setColor(1.0f,1.0f,1.0f,1.0f);
    } else { PauseUniverse(); }

	// Key/scroll Handling (cam controller update)
	m_CameraController.OnUpdate(ts);
	// set view matrix and orthographic matrix product Uniforms for all Bodies and Trails
    bodyOrbit->body->Circle_shader->use();
    bodyOrbit->body->Circle_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
	bodyOrbit->bodyTrail->Trail_shader->use();
    bodyOrbit->bodyTrail->Trail_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
	star->Circle_shader->use();
	star->Circle_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
    
	// Physics Loop //
    if (!isPauseUniverse){ PhysicsLoop(); }

	// Render Universe //
    RenderUniverse();
}

void Universe::OnImGuiRender()
{
    static bool isOrbitReset = false;
    static bool isTrailReset = false;
    universeUI->UpdateWorkSize(); // must include first to update the working viewport size and position
    // UI ELEMENTS //
    universeUI->TimeOverlay();
    universeUI->fastForwardOverlay(fastForward, fastForwardActive);
    universeUI->ButtonOverlay(isOrbitReset, showEnergyPlot, isTrailReset);
    universeUI->StatsOverlay();
    universeUI->InputsOverlay(isOrbitReset);
    //if (isPauseUniverse && !bodyCrashed) { universeUI->PauseMenu(); }
    if (isPauseUniverse && bodyCrashed) { universeUI->CrashMenu(); }
    if (showEnergyPlot) { universeUI->EnergyPlot(isPauseUniverse); }
    // END OF UI ELEMENTS //
    if (isOrbitReset) { ResetOrbits(); isOrbitReset = false; } 
    if (isTrailReset) { bodyOrbit->ResetTrail(); isTrailReset = false; }
}

//////////////////////////////////////////////////////////////////////////////////

// UNIVERSE FUNCTIONS //
void Universe::InitUniverse()
{
    // Init Universe
    UniverseTime = 0.0f;
    dt = 0.0001;
    // Objects
    star = new Body(1.0f,1.0f);
    // Orbit for Earth initially
    bodyOrbit = new Orbit(star->mass, 0.1f,earthMass, 1.0f, 0.0f, 0.0f, 2*PI, 2.0f, UniverseTime, dt);

    // Set Object Colors
    bodyOrbit->body->setColor(0.1f, 0.1f, 0.6f, 1.0f);
    bodyOrbit->bodyTrail->setColor(glm::vec4{ 0.5f,0.4f,0.4f,0.7f});
    star->setColor(1.0f, 1.0f, 0.0f, 1.0f);
}

void Universe::PhysicsLoop()
{
    // Transform Body Positions	
    glm::vec3 newpos = glm::vec3(bodyOrbit->x, bodyOrbit->y, 1.0f);
    bodyOrbit->body->body_Transform.setPosition(newpos);

    // Scale Bodies To see them better.
    glm::vec3 newScale = glm::vec3(bodyScaling, bodyScaling, 1.0f);
    star->body_Transform.setScale(newScale);
    bodyOrbit->body->body_Transform.setScale(newScale);

    // Update Orbit
    for (uint16_t i = 0; i < fastForward; i++)
    {
        bodyOrbit->Update(UniverseTime, dt);
        // Update Orbit Trail
        newpos.x = bodyOrbit->x;
        newpos.y = bodyOrbit->y;
        // Update Time Variable
        UniverseTime += dt;
        // update Trail in intervals
        if (i % 500 == 0)
            bodyOrbit->bodyTrail->UpdateTrail(newpos.x, newpos.y, !bodyOrbit->finishedPeriod);
        // Collision Detection //
        detectCollision(bodyOrbit->r, bodyOrbit->body->radius, star->radius, bodyScaling);
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
    bodyOrbit->Reset();
    bodyOrbit->bodyTrail->ResetVertices();
}

void  Universe::PauseUniverse()
    { Application::Get().GetWindow().Clear(5.0f / 255.0f, 5.0f / 255.0f, 5.0f / 255.0f, 1.0f); }

void Universe::RenderUniverse()
{
    // Trail																 
    renderer.DrawLineStrip(bodyOrbit->bodyTrail->va, *(bodyOrbit->bodyTrail->Trail_shader), bodyOrbit->bodyTrail->trail_Transform, static_cast<unsigned int>(bodyOrbit->bodyTrail->vertices.size() / 3)); //- divide trail by size/3 since GL_LINE_STRIP count is num of vertices
    // Bodies
    renderer.DrawCircle(bodyOrbit->body->va, *(bodyOrbit->body->Circle_shader), bodyOrbit->body->body_Transform, bodyOrbit->body->NumberOfVertices);
    renderer.DrawCircle(star->va, *(star->Circle_shader), star->body_Transform, star->NumberOfVertices);
}




