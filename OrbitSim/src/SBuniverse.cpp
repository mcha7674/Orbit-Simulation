#include "SBuniverse.h"
#include <algorithm>
using namespace GLCore;
using namespace GLCore::Utils;

static float mouseXpos = 0.0f;
static float mouseYpos = 0.0f;
static bool statOverlayFocused = false;
static bool showEnergyPlot = true;
// Universe State Booleans
static int fastForwardActive = 0;
bool pauseUniverse = false;
bool bodyCrashed = false;

SBuniverse::SBuniverse()
	:m_CameraController((float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), false, 1.0f) // init camera controller with the window aspect ratio
{
	SBuniverseTime = 0.0f;
	dt = 0.0001;

    // Init the ImGui Viewport
    viewport = ImGui::GetMainViewport();
    style = &ImGui::GetStyle();
    plotStyle = &ImPlot::GetStyle();
    // Universal ImGui UI Styles
    InitImGuiGlobalStyling();
    // Initiate The Universe (Objects Atrributes.. etc)
    InitSBuniverse();    
}

SBuniverse::~SBuniverse()
{
	delete body;
    delete Sun;
    delete trail;
    delete orbit;
}

// Universe's gl prelims 
void SBuniverse::OnAttach()
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

void SBuniverse::OnDetach()
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
void SBuniverse::OnEvent(Event& event)
{
	// orthographic camera Event Dispatching (seperate dispatching)
	m_CameraController.OnEvent(event);
	EventDispatcher dispatcher(event);
    // Pause Menu Space Bar Bind
    dispatcher.Dispatch<KeyPressedEvent>(
        [&](KeyPressedEvent &e){
        if ((e.GetKeyCode() == KEY_SPACE) && !bodyCrashed)
        {
            if (e.GetRepeatCount() >= 1) { pauseUniverse = false; }
            if (pauseUniverse) { pauseUniverse = false; }
            else if (!pauseUniverse) { pauseUniverse = true; }    


        }
        else if ((e.GetKeyCode() == KEY_SPACE) && bodyCrashed)
        {
            if (e.GetRepeatCount() >= 1) { 
                bodyCrashed = false; 
                pauseUniverse = false;
                ResetOrbits();
            }
            if (bodyCrashed) { 
                bodyCrashed = false;
                pauseUniverse = false;
                ResetOrbits();
            }
            else if (!bodyCrashed) { 
                bodyCrashed = true;
                pauseUniverse = true;
            }
        }
        return false;
    });
    
    
    // Mouse Click Body Positioning
    dispatcher.Dispatch<MouseMovedEvent>(
        [&](MouseMovedEvent& mme) {

            //std::cout << "(" << mme.GetX() << ", " << mme.GetY() << ")" << std::endl;
            mouseXpos = mme.GetX();
            mouseYpos = mme.GetY();

            return false;
        });
    dispatcher.Dispatch<MouseButtonPressedEvent>(
        [&](MouseButtonPressedEvent& mbe) {
            // Normalize and translate Mouse Positions to be within -1 and 1
            mouseXpos -= static_cast<float>(Application::Get().GetWindow().GetWidth()) / 2.0f;
            mouseYpos -= static_cast<float>(Application::Get().GetWindow().GetHeight()) / 2.0f;
            mouseXpos /= static_cast<float>(Application::Get().GetWindow().GetWidth()) / 2.0f;
            mouseYpos /= static_cast<float>(Application::Get().GetWindow().GetHeight()) / 2.0f;
            // Adjust for orthographic zoom
            mouseXpos*= m_CameraController.GetZoomLevel()*m_CameraController.GetAspectRatio();
            mouseYpos*= -1.0f * m_CameraController.GetZoomLevel();
            // Adjust for camera Movement
            mouseXpos += m_CameraController.GetCamPos().x;
            mouseYpos += m_CameraController.GetCamPos().y;
           
            orbit->x0 = mouseXpos;
            orbit->y0 = mouseYpos;
            ResetOrbits();
            return false;
        });

    // Arrow Key Bind With Fast Forward
    dispatcher.Dispatch<KeyPressedEvent>(
    [&](KeyPressedEvent& e) {
        if (!statOverlayFocused)
        {
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
            }
            else if (e.GetKeyCode() == KEY_RIGHT)
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
        }
        return true;
     });
	
}

void SBuniverse::OnUpdate(Timestep ts)
{     
	// Window Clearing and pause functions 
    if (!pauseUniverse || bodyCrashed) { 
        Application::Get().GetWindow().Clear();
        Sun->setAlpha(1.0f);
        body->setAlpha(1.0f);
        trail->setAlpha(0.7f);
        //background->setColor(1.0f,1.0f,1.0f,1.0f);
    } else { PauseSBuniverse(); }

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
    if (!pauseUniverse)
    {
        PhysicsLoop();
    }

	// Render Universe //
    RenderSBuniverse();
}

void SBuniverse::OnImGuiRender()
{
    // ViewPort Positions
    ImVec2 work_pos = viewport->Pos;
    ImVec2 work_size = viewport->Size;
    // ENERGY PLOT
    if (showEnergyPlot) { EnergyPlot(work_pos, work_size, orbit->KE, orbit->PE); }
    // Overlays
    TimeDisplay(work_pos, work_size);
    StatsOverlay(work_pos, work_size);
    InputsOverlay(work_pos, work_size);
    fastForwardDisplay(work_pos, work_size);
    ButtonDisplay(work_pos, work_size);
    // Pause And Crash Menus
    if (pauseUniverse && !bodyCrashed) { PauseMenu(work_pos, work_size); }
    else if (pauseUniverse && bodyCrashed) { CrashMenu(work_pos, work_size); }
    
}
//////////////////////////////////////////////////////////////////////////////////

// UNIVERSE FUNCTIONS //
void SBuniverse::InitSBuniverse()
{
    // Init Universe
    fastForward = 1;
    
    // Objects
    Sun = new Body(0, 1.0f, 1.0f);
    body = new Body(1, 3e-6f, 0.1f);
    trail = new Trail;
    orbit = new Orbit(body,1.0f, 0.0f, 0.0f, 2*PI, 2.0f, SBuniverseTime, dt);

    // Set Object Colors
    body->setColor(0.1f, 0.1f, 0.6f, 1.0f);
    trail->setColor(glm::vec4{ 0.5f,0.4f,0.4f,0.7f});
    Sun->setColor(1.0f, 1.0f, 0.0f, 1.0f);
}

void SBuniverse::PhysicsLoop()
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
        orbit->Update(SBuniverseTime, dt, false);
        // Update Orbit Trail
        newpos.x = orbit->x;
        newpos.y = orbit->y;
        // Update Time Variable
        SBuniverseTime += dt;
        // update Trail in intervals
        if (i % 500 == 0 || i % 500 == 1)
            trail->UpdateTrail(newpos.x, newpos.y, !orbit->finishedPeriod);
        
        // Collision Detection //
        detectCollision(orbit->r, body->radius, Sun->radius, newScale.x);
    }
}

void SBuniverse::detectCollision(const float& orbitR, const float& body1R, const float& body2R, const float& scale)
{
    // orbit from edge of body instead of center
    if ((orbitR - (body1R * scale)) <= body2R * scale)
    {
        bodyCrashed = true;
        pauseUniverse = true;
    }
}

void SBuniverse::ResetOrbits()
{
    SBuniverseTime = 0.0f;
    orbit->Reset();
    trail->ResetVertices();
}

void  SBuniverse::PauseSBuniverse()
{
    Application::Get().GetWindow().Clear(1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 1.0f);
    Sun->setAlpha(0.5f);
    body->setAlpha(0.5f);
    trail->setAlpha(0.5f);
}

void SBuniverse::RenderSBuniverse()
{
   
    
    // Trail																 
    renderer.DrawLineStrip(trail->va, *(trail->Trail_shader), trail->trail_Transform, (unsigned int)trail->vertices.size() / 3); //- divide trail by size/3 since GL_LINE_STRIP count is num of vertices
    // Bodies
    renderer.DrawCircle(body->va, *(body->Circle_shader), body->body_Transform, body->NumberOfVertices);
    renderer.DrawCircle(Sun->va, *(Sun->Circle_shader), Sun->body_Transform, Sun->NumberOfVertices);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//// CUSTOM IMGUI DISPLAYS ////
void SBuniverse::InitImGuiGlobalStyling()
{
    style->Colors[ImGuiCol_Text] = ImColor(153, 178, 242);
    style->Colors[ImGuiCol_Border] = ImColor(0, 0, 0);
    style->Colors[ImGuiCol_Button] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_ButtonHovered] = ImColor(153.0f, 178.0f, 242.0f, 0.2f);
    style->Colors[ImGuiCol_ButtonActive] = ImColor(153.0f, 178.0f, 242.0f, 0.2f);

    //bg coloring
    style->Colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0);
    //style->Colors[ImGuiCol_FrameBgHovered] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_ButtonActive] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_FrameBg] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Title bar for window is transparent
    style->Colors[ImGuiCol_TitleBg] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);

    style->WindowBorderSize = 0.0f;
}

void SBuniverse::TimeDisplay(const ImVec2& work_pos, const ImVec2& work_size)
{
    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    // Universe Time //
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
	ImGui::SetNextWindowPos(ImVec2(work_pos.x+work_size.x*0.5f, work_pos.y ), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
	//ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    if (ImGui::Begin("Time", p_open, window_flags)) {

        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("Time: %f Years", SBuniverseTime);
    }
    ImGui::End();
    // FPS
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - 15.0f, work_pos.y + 1.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    if (ImGui::Begin("FPS", p_open, window_flags)) {
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("FPS : %d", (uint8_t)(1.0f / Application::Get().GetDeltaTime()));
    }
    ImGui::End();
    
}

void SBuniverse::fastForwardDisplay(const ImVec2& work_pos, const ImVec2& work_size)
{
    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGuiIO io = ImGui::GetIO(); // for keyboard capture
	ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
	ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x * 0.5f, work_pos.y + 25.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(20, 20, 100, 1));
    if (ImGui::Begin("FF", p_open, window_flags)) {
        
        ImGui::SetWindowFontScale(1.2f);
        if (io.WantCaptureKeyboard) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        // Fast Forward
        
        if (ImGui::RadioButton("x1", &fastForwardActive, 0)) { fastForward = 1; }
        ImGui::SameLine();
        if (ImGui::RadioButton("x10", &fastForwardActive, 1)) { fastForward = 10; }
        ImGui::SameLine();
        if (ImGui::RadioButton("x50", &fastForwardActive, 2)) { fastForward = 50; }
        ImGui::SameLine();
        if (ImGui::RadioButton("x100", &fastForwardActive, 3)) { fastForward = 100; }
        ImGui::SameLine();
        if (ImGui::RadioButton("x500", &fastForwardActive, 4)) { fastForward = 500; }
    }
    ImGui::End();
    ImGui::PopStyleColor();

}



void SBuniverse::ButtonDisplay(const ImVec2& work_pos, const ImVec2& work_size)
{

    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGuiIO io = ImGui::GetIO(); // for event capture

    // Reset Orbit Button
    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - 15.0f, work_pos.y + 30.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    if (ImGui::Begin("BDReset", p_open, window_flags)) {
        ImGui::SetWindowFontScale(1.5f);
        if (io.WantCaptureKeyboard) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        // Reset Sim Button //
        if (ImGui::Button("Reset Orbit")) { ResetOrbits(); }
    }
    ImGui::End();

    // Center Cam Button
    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - 25.0f, work_pos.y +70.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    if (ImGui::Begin("CenterCam", p_open, window_flags))
    {
        ImGui::SetWindowFontScale(1.5f);
        if (io.WantCaptureKeyboard) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        if (ImGui::Button("Center Cam")) { m_CameraController.ResetCamera(); }
    }
    ImGui::End();

    // Show Plot CheckBox Button
    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - 7.0f, work_pos.y  + 115.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 0.15));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(255.0f, 255.0f, 255.0f, 0.5));
    
    if (ImGui::Begin("ShowPlot", p_open, window_flags))
    {
        ImGui::SetWindowFontScale(1.5f);
        if (io.WantCaptureKeyboard) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        ImGui::Text("Show Plot");
        ImGui::SameLine();
        if (ImGui::Checkbox("##ShowEnergy", &showEnergyPlot)) { }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

}

void SBuniverse::StatsOverlay(const ImVec2& work_pos, const ImVec2& work_size)
{
    static int corner = 0;
    static bool* p_open;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1) {
        const float PAD = 15.0f;
        ImVec2 window_pos, window_pos_pivot;
        // if corner is 1, then window_pos.x = work_pos.x + work_size.x - PAD else it gets work_pos.x + PAD
        window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGuiIO io = ImGui::GetIO(); // for keyboard capture

    ImGui::SetNextWindowBgAlpha(0.02f); // Transparent background
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(189, 204, 242, 1));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));
    ImGui::SetNextWindowSize(ImVec2(235, 160));
    if (ImGui::Begin("Body Stats", p_open, window_flags)) {
        // Set This Condition to prevent key interactiong with sim
        if (io.WantCaptureKeyboard) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        // Window Settings
        ImGui::SetWindowFontScale(1.5f);
        // Orbit Stats //
        ImGui::Text("   Body Statistics");
        ImGui::Separator();
        ImGui::Text("r: %f AU", orbit->r);
        ImGui::Text("v: %f AU/yr", orbit->v);
        
        static int E_expCount = scientificDivCount(&orbit->E);
        static int KE_expCount = scientificDivCount(&orbit->KE);
        static int PE_expCount = scientificDivCount(&orbit->PE);
        ImGui::Text("E: %.1f+e%i J",orbit->E/pow(10,E_expCount), E_expCount);
        ImGui::Text("KE: %.1f+e%i J",orbit->KE/pow(10,KE_expCount), KE_expCount);
        ImGui::Text("PE: %.1f+e%i J",orbit->PE/pow(10,PE_expCount), PE_expCount);
        ImGui::NewLine();
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

}

void SBuniverse::InputsOverlay(const ImVec2& work_pos, const ImVec2& work_size)
{

    static bool* p_open;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGuiIO io = ImGui::GetIO(); // for keyboard capture
    ImGui::SetNextWindowBgAlpha(0.02f); // Transparent background
    ImGui::SetNextWindowPos(ImVec2((work_pos.x + work_size.x) * 0.0f + 15.0f, work_pos.y + 185.0f), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(189, 204, 242, 1));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 1, 1, 1));
    if (ImGui::Begin("Body Inputs", p_open, window_flags)) {
        // Set This Condition to prevent key interactiong with sim
        if (io.WantCaptureKeyboard) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        ImGui::SetWindowFontScale(1.5f);
        // INPUTS //
        ImGui::Text("       Inputs");
        ImGui::Separator();
        // Delta Toggle // - ERASE FOR FINAL RELEASE
        ImGui::Text("dt: ");
        ImGui::SameLine(0.0f, 20.0f);
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputFloat("(yrs)", &dt, 0.0f, 0.0f, "%.5f")) { ResetOrbits(); }
        // Delta constraints //
        if (dt > 0.01f) { dt = 0.01f; }
        if (dt <= 0.00001f) { dt = 0.00001f; }
        //ImGui::SliderFloat(" ", &dt, 0.001,0.1);
        // X position Toggle //
        ImGui::Text("x0: ");
        ImGui::SameLine(0.0f, 20.0f);
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputFloat("##x0", &orbit->x0))
        {
            ResetOrbits();
        }
        // Y position toggle //
        ImGui::Text("y0: ");
        ImGui::SameLine(0.0f, 20.0f);
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputFloat("##y0", &orbit->y0))
        {
        ResetOrbits();
        }
        // Velocity and major axis toggle //
        ImGui::Text("vx0: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputFloat("##vx", &orbit->vx0))
        {
            ResetOrbits();
        }
        ImGui::Text("vy0: ");
        ImGui::SameLine();
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputFloat("##vy", &orbit->vy0))
        {
            ResetOrbits();
        }

    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
}

void SBuniverse::PauseMenu(const ImVec2& work_pos, const ImVec2& work_size)
{
    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    // center the window
    ImGui::SetNextWindowPos(ImVec2((work_pos.x + work_size.x) * 0.5f, (work_pos.y + work_size.y) * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("Paused", p_open, window_flags)) {
        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("PAUSED");
    }
    ImGui::End();
}

void SBuniverse::CrashMenu(const ImVec2& work_pos, const ImVec2& work_size)
{
    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    // center the window
    ImGui::SetNextWindowPos(ImVec2((work_pos.x + work_size.x) * 0.5f, (work_pos.y + work_size.y) * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("Crashed", p_open, window_flags)) {
        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("     Crashed");
        ImGui::SetWindowFontScale(2.0f);
        ImGui::NewLine();
        ImGui::Text("Press 'Space Bar' to Reset");
    }
    ImGui::End();


}


void SBuniverse::EnergyPlot(const ImVec2& work_pos, const ImVec2& work_size, float &orbitKE, float& orbitPE) {

    static bool* p_open;
    static float* SBTime = &SBuniverseTime; // point to address of the updating universe time
    static RollingBuffer   rdata1, rdata2;

    // Add points
    if (!pauseUniverse)
    {
        rdata1.AddPoint(*SBTime, orbitKE);
        rdata2.AddPoint(*SBTime, orbitPE);
    }
    
   

    ImGuiIO io = ImGui::GetIO();
    // Set Styles
    plotStyle->MarkerSize = 2.0f; 
    plotStyle->Marker = ImPlotMarker_Asterisk;
    plotStyle->FillAlpha = 0.0f;
    plotStyle->Colors[ImPlotCol_AxisBg] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    plotStyle->Colors[ImPlotCol_FrameBg] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    plotStyle->Colors[ImPlotCol_PlotBg] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);

    static ImPlotAxisFlags flags = ImPlotAxisFlags_NoMenus;
    //static ImPlotFlags plotFlags = ImPlotFlags_NoTitle |ImPlotFlags_NoFrame;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowPos(ImVec2((work_pos.x + work_size.x) * 0.5f, (work_pos.y + work_size.y)), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::Begin("Plot Window", p_open, window_flags);
    if (ImPlot::BeginPlot("##RollingPlot", ImVec2(1000, 200))) {
        if (io.WantCaptureMouse) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        static float y_min = std::min(rdata1.Data[0].y, rdata2.Data[0].y);
        static float y_max = std::max(rdata1.Data[0].y, rdata2.Data[0].y);
        //Plot Setup
        ImPlot::SetupAxes("Time (Years)", "Energy (Joules)", flags, flags);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, *SBTime, ImGuiCond_Always);
        //std::cout << y_min << ", "<<y_max << std::endl;
        // Determine realtime y limits
        if (abs(y_min) < y_max)
        { // Make Bounderies Symmetrical
            y_min = y_max;
        }else if (abs(y_min) > y_max)
        { // Make Bounderies Symmetrical
            y_max = abs(y_min);
        }
        ImPlot::SetupAxisLimits(ImAxis_Y1, y_min * 2, y_max * 2);
        ImPlot::SetupLegend(ImPlotLocation_SouthEast);

        ImPlot::PlotScatter("Kinetic Energy", &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size());
        ImPlot::PlotScatter("Potential Energy", &rdata2.Data[0].x, &rdata2.Data[0].y, rdata2.Data.size());
        
        ImPlot::EndPlot();
    }
    ImGui::End();
}



// Utility Function to help convert Large Number to Scientific Notation for use in imgui stats
int SBuniverse::scientificDivCount(float *og_num)
{
    float num = *og_num;
    int count = 0;
    while (((int)num)/100 != 0 )
    {
        num /= 10;
        count++;
    }
    return count;
}



