#include "Universe.h"

using namespace GLCore;
using namespace GLCore::Utils;

Universe::Universe()
	:m_CameraController((float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight()) // init camera controller with the window aspect ratio
{
	UniverseTime = 0.0f;
	dt = 0.001;
}

Universe::~Universe()
{
	delete body;
}

void Universe::OnAttach()
{
	EnableGLDebugging();

	// gl enable prelims
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	body = new Body(1, 1.0f, 1.0f, 1.0f);
	fastForward = 1;
	Sun = new Body(0, 10.0f, 0.0f,1.0f);
	body = new Body(1, 3e-6f, 1.0f,0.5f);
	trail = new Trail;
	orbit = new Orbit(body, body->a, 0.0f, 0.0f, 2*PI, 2.0f, UniverseTime, dt);

	// Set Object Colors
	body->setColor(0.2f, 0.4f, 0.2f, 1.0f);
	trail->setColor(glm::vec4{1.0f,0.0f,0.0f,1.0f});
	Sun->setColor(1.0f, 1.0f, 0.0f, 1.0f);

}

void Universe::OnDetach()
{
	// Delete All buffers that were attached
	/*glDeleteVertexArrays(1, &m_QuadVA);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);*/
	body->va.UnBind();
	body->vb->UnBind();
	body->ib->UnBind();
	Sun->va.UnBind();
	Sun->vb->UnBind();
	Sun->ib->UnBind();
	trail->va.UnBind();
	trail->vb->UnBind();
}

// Event Handling layer
void Universe::OnEvent(Event& event)
{
	// orthographic camera Event Dispatching (seperate dispatching)
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);
	
	
	
}

void Universe::OnUpdate(Timestep ts)
{
	// Window Clearing
	Application::Get().GetWindow().Clear();
	// Key Handling (cam controller update)
	m_CameraController.OnUpdate(ts);
	// set view matrix Uniforms for all objects
	body->quad_shader->use();
	body->quad_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
	trail->Trail_shader->use();
	trail->Trail_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());
	Sun->quad_shader->use();
	Sun->quad_shader->SetUniformMatrix4fv("viewProjection", m_CameraController.GetCamera().GetViewProjectionMatrix());

	// Transform Body Positions
	body->body_Transform.setPosition(glm::vec3{cos(UniverseTime),sin(UniverseTime),0.0f});
	
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
	}
	trail->UpdateTrail(newpos.x, newpos.y);

	// Scale Bodies Based on Radius
	glm::vec3 newScale = glm::vec3(1.0f);
	newScale.x = 0.5f;
	newScale.y = 0.5f;
	Sun->body_Transform.setScale(newScale);
	newScale.x = 0.1f;
	newScale.y = 0.1f;
	body->body_Transform.setScale(newScale);
	

	// Render Objects
																 //- divide trail by size/3 since GL_LINE_STRIP count is num of vertices
	renderer.DrawLineStrip(trail->va, *(trail->Trail_shader), trail->trail_Transform, (unsigned int)trail->vertices.size() / 3); // trail
	renderer.DrawRect(body->va, *(body->ib), *(body->quad_shader), body->body_Transform);
	renderer.DrawRect(Sun->va, *(Sun->ib), *(Sun->quad_shader), Sun->body_Transform);
	
	UniverseTime += dt;
}

void Universe::OnImGuiRender()
{
	ImGui::Begin("Controls");
	ImGui::Text("Start");
	ImGui::End();
}
