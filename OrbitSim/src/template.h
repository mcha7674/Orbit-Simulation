#pragma once

// OpenGL Renderer
#include "OpenGL-Core\Renderer.h"
#include "Window.h"
#include "Camera/Camera2D.h"

// IMGUI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


class Template
{
public:
    Template(const char* windowTitle = "OpenGL Template", const int& width = 1280, const int& height = 720) : window(windowTitle, width, height)
    {
        // Window characteristics
        window.backgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    ~Template() {}

    // main program Run Method
    void Run()
    {
        // IIMGUI SETUP
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window.getGLFWwindow(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
        //==============================================================================
        OnStart();
        //==============================================================================
        while (!window.closed())
        {
            // Updat Aspect ratio on window resize
            window.clear(); // clear color and other buffer bits
            // process camera inputs
            camera.Update(window);
            // implement the following:
            renderer.SetViewMatrix(camera.GetViewMatrix());
            //renderer.SetOrthoMatrix(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));
            renderer.SetOrthoMatrix(glm::ortho(20.0f/camera.zoom, -20.0f / camera.zoom, -20.0f / camera.zoom, 20.0f / camera.zoom, -1.0f, 1.0f));

            //glEnable(GL_MULTISAMPLE);
            //==============================================================================
            OnUpdate();
            //==============================================================================
            OnRender();
            //==============================================================================
            
            // ImGui Loop
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            //==============================================================================
            OnImGuiRender();
            //==============================================================================
            ImGui::Render();
            // ImGui::UpdatePlatformWindows();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            //==============================================================================
            
            window.update();
        }

    }
protected:
    Window      window;
    Camera2D    camera;
    Renderer    renderer;

protected:
    virtual void OnStart() {};
    virtual void OnUpdate() {};
    virtual void OnRender() {};
    virtual void OnImGuiRender() {};

    

};


