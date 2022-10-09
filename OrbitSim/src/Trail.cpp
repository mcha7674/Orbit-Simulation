#include "Trail.h"

Trail::Trail()
{
    // Init Shader
    Trail_shader = Shader::FromGLSLTextFiles(
        "assets/shaders/trail.vert.glsl",
        "assets/shaders/trail.frag.glsl"
    );
    // INIT VBO and IBO
    // Vertex Budder initiated with Dynamic Draw (Indicated by the true final arg boolean)
    vb = new VertexBuffer(NULL, (unsigned int)(vertices.size() * sizeof(vertices[0])), true); // init vertex buffer (Gen, Bind, Data Implement)
    // Create layout
    layout.Push<float>(3); // 3 floats per vertex for position
    va.AddBuffer((*vb), layout);

    //set default color
    setColor(glm::vec4(1.0f, 0.0f, 0.0f,0.5f));
}

Trail::~Trail()
{
    va.UnBind();
    vb->UnBind();
    delete vb;
    delete Trail_shader;
}


void Trail::setColor(const glm::vec4 &color)
{
    Trail_shader->use();
    Trail_shader->SetUniformVec4fv("trailColor", color);
}


void Trail::UpdateTrail(const float &x,const float &y)
{

    // Update vertices array
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(0.0f);
    vb->UpdateBuffer(&vertices[0], (unsigned int)(vertices.size() * sizeof(vertices[0])), true);
}

void Trail::SetBuffers()
{
    vb->UpdateBuffer(&vertices[0], (unsigned int)(vertices.size() * sizeof(vertices[0])), true);    
}

//void Trail::SetTrailAlpha(const float &a)
//{
//    Trail_shader->use();
//    Trail_shader->SetUniform1f("trailAlpha", a);
//}