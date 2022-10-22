#include "Quad.h"

Quad::Quad()
{
    // init Vertices and Indices   
    // Init Shader
    quad_shader = Shader::FromGLSLTextFiles(
        "assets/shaders/quad.vert.glsl",
        "assets/shaders/quad.frag.glsl"
    );

    // INIT VBO and IBO
    vb = new VertexBuffer(vertices, sizeof(vertices)); // init vertex buffer (Gen, Bind, Data Implement)
    ib = new IndexBuffer(indices, 6);
    // Create layout
    layout.Push<float>(3); // 3 floats per vertex for position
    layout.Push<float>(2); // 2 floats poer texture coordinate
    // Binds Vertex Array and Vartex Buffer
    // grabs each vertex buffer layout element 
    // Enables each Vertex Attribute Array and and create Attribute pointers
    va.AddBuffer((*vb), layout);

    texture = nullptr;
}

Quad::~Quad()
{
    va.UnBind();
    vb->UnBind();
    ib->UnBind();
    quad_shader->unuse();

    // delete mem allocation
    delete vb;
    delete ib;
    delete quad_shader;
    delete texture;

}

void Quad::setTexture(const char* texturePath)
{
    quad_shader->use();
    quad_shader->SetUniform1i("texture1", 0);
    texture = new Texture(texturePath);
    texture->Bind(0);

}
void Quad::setColor(float r, float g, float b, float a)
{
    quad_shader->use();
    quad_shader->SetUniformVec4fv("u_Color", glm::vec4(r, g, b, a));
}
