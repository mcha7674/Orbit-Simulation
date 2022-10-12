#include "Universe.h"

using namespace GLCore;
using namespace GLCore::Utils;

static int fastForwardActive = 0;

Universe::Universe()
	:m_CameraController((float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight(), false, 1.0f) // init camera controller with the window aspect ratio
{
	UniverseTime = 0.0f;
	dt = 0.0001;

    // Init the ImGui Viewport
    viewport = ImGui::GetMainViewport();
    style = &ImGui::GetStyle();
    // Universal ImGui UI Styles
    InitImGuiGlobalStyling();
    // Initiate The Universe (Objects Atrributes.. etc)
    InitUniverse();
}
Universe::~Universe()
{
	delete body;
    delete Sun;
    delete trail;
    delete orbit;
}

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
        if (e.GetKeyCode() == KEY_SPACE)
        {
            if (e.GetRepeatCount() >= 1) { pauseUniverse = false; }
            if (pauseUniverse) { pauseUniverse = false; }
            else if (!pauseUniverse) { pauseUniverse = true; }
            
        }
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

void Universe::OnUpdate(Timestep ts)
{     
	// Window Clearing and pause functions
    if (!pauseUniverse) { 
        Application::Get().GetWindow().Clear();
        Sun->setAlpha(1.0f);
        body->setAlpha(1.0f);
        trail->setAlpha(0.7f);
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
    if (!pauseUniverse)
    {
        PhysicsLoop();
        UniverseTime += dt;
    }
	// Render Universe //
    RenderUniverse();
}

void Universe::OnImGuiRender()
{
    ImVec2 work_pos = viewport->Pos;
    ImVec2 work_size = viewport->Size;
    TimeDisplay(work_pos, work_size);
    StatsOverlay(work_pos, work_size);
    fastForwardDisplay(work_pos, work_size);
    ButtonDisplay(work_pos, work_size);
    if (pauseUniverse) { PauseMenu(work_pos, work_size); }
    //ImGui::ShowDemoWindow();
}
//////////////////////////////////////////////////////////////////////////////////

// HELPER UNIVERSE FUNCTIONS
void Universe::InitUniverse()
{
    // Init Universe
    fastForward = 1;
    Sun = new Body(0, 10.0f, 0.0f, 1.0f);
    body = new Body(1, 3e-6f, 1.0f, 0.2f);
    trail = new Trail;
    orbit = new Orbit(body, body->a, 0.0f, 0.0f, 7.0f, 2.0f, UniverseTime, dt);

    // Set Object Colors
    body->setColor(0.1f, 0.1f, 0.6f, 1.0f);
    trail->setColor(glm::vec4{ 0.5f,0.4f,0.4f,0.7f});
    Sun->setColor(1.0f, 1.0f, 0.0f, 1.0f);

    pauseUniverse = false;
}

void Universe::PhysicsLoop()
{
    // Transform Body Positions	
    glm::vec3 newpos = glm::vec3(1.0f);
    newpos.x = orbit->x;
    newpos.y = orbit->y; // Correct for screen aspect ratio
    body->body_Transform.setPosition(newpos);

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
        
    }

    // Scale Bodies To see them better.
    glm::vec3 newScale = glm::vec3(1.0f);
    newScale.x = 0.2f;
    newScale.y = 0.2f;
    Sun->body_Transform.setScale(newScale);
    body->body_Transform.setScale(newScale);
}

void Universe::ResetOrbits()
{
    UniverseTime = 0.0f;
    orbit->Reset();
    trail->ResetVertices();
}

void  Universe::PauseUniverse()
{
    Application::Get().GetWindow().Clear(10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 1.0f);
    Sun->setAlpha(0.2f);
    body->setAlpha(0.2f);
    trail->setAlpha(0.1f);
}

void Universe::RenderUniverse()
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
void Universe::InitImGuiGlobalStyling()
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
    style->Colors[ImGuiCol_ButtonActive] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_TitleBg] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(0.0f, 0.0f, 0.0f, 0.0f);

    style->WindowBorderSize = 0.0f;
}

void Universe::TimeDisplay(const ImVec2& work_pos, const ImVec2& work_size)
{
    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    // Universe Time //
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
	ImGui::SetNextWindowPos(ImVec2(work_pos.x+work_size.x*0.5f, work_pos.y ), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
	//ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    if (ImGui::Begin("Time", p_open, window_flags)) {

        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("Time: %f Years", UniverseTime);
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

void Universe::fastForwardDisplay(const ImVec2& work_pos, const ImVec2& work_size)
{
    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	// Universe Time //
	ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
	ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x * 0.5f, work_pos.y + 25.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(20, 20, 100, 1));
    if (ImGui::Begin("FF", p_open, window_flags)) {
        
        ImGui::SetWindowFontScale(1.2f);
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



void Universe::ButtonDisplay(const ImVec2& work_pos, const ImVec2& work_size)
{

    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    // For Top Right Corners - pivot (3rd parameter) allow for center and corner positioning (1.0,0.0) is top right for example
    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - 15.0f, work_pos.y + 60.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    if (ImGui::Begin("BDReset", p_open, window_flags)) {
        ImGui::SetWindowFontScale(1.5f);
        // Reset Sim Button //
        if (ImGui::Button("Reset Orbit")) { ResetOrbits(); }
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - 15.0f, work_pos.y + work_size.y-30.0f), ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    if (ImGui::Begin("CenterCam", p_open, window_flags))
    {
        ImGui::SetWindowFontScale(1.5f);
        if (ImGui::Button("Center Cam")) { m_CameraController.ResetCamera(); }
    }
    ImGui::End();

}

void Universe::StatsOverlay(const ImVec2& work_pos, const ImVec2& work_size)
{
    static int corner = 0;
    static bool* p_open;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
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

    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(189, 204, 242, 1));
    if (ImGui::Begin("Body Stats", p_open, window_flags)) {
        // Set This Condition to prevent key interactiong with sim
        if (io.WantCaptureKeyboard) {
            statOverlayFocused = true;
        }
        else { statOverlayFocused = false; }
        // Window Settings
        ImGui::SetWindowFontScale(1.5f);
        // Orbit Stats //
        ImGui::Text("r: %f AU", orbit->r);
        if (orbit->finishedPeriod) { ImGui::Text("period: %f yrs", orbit->period); }
        else { ImGui::Text("period: UnDetermined"); }
        // INPUTS //
        // Delta Toggle // - ERASE FOR FINAL RELEASE
        ImGui::Text("dt: ");
        ImGui::SameLine(0.0f, 20.0f);
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputFloat("(yrs)", &dt, 0.0f, 0.0f, "%.4f")) { ResetOrbits(); }
        // Delta constraints //
        if (dt > 0.01f) { dt = 0.01f; }
        if (dt <= 0.00001f) { dt = 0.00001f; }
        //ImGui::SliderFloat(" ", &dt, 0.001,0.1);
        // major axis toggle //
        ImGui::Text("x0: ");
        ImGui::SameLine(0.0f, 20.0f);
        ImGui::PushItemWidth(100.0f);
        if (ImGui::InputFloat("##a", &orbit->x0))
        {
            body->a = orbit->x0;
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

}

void Universe::PauseMenu(const ImVec2& work_pos, const ImVec2& work_size)
{
    static bool* p_open;
    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    // Universe Time //
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background
    ImGui::SetNextWindowPosCenter(ImGuiCond_Always);
    if (ImGui::Begin("Paused", p_open, window_flags)) {
        ImGui::SetWindowFontScale(4.0f);
        ImGui::Text("PAUSED", UniverseTime);
    }
    ImGui::End();

    
}


