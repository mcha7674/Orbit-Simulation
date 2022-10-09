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
    //layout.Push<float>(3); // 3 floats per vertex for Color values
    //layout.Push<float>(2); // 2 floats poer texture coordinate
    // Binds Vertex Array and Vartex Buffer
    // grabs each vertex buffer layout element 
    // Enables each Vertex Attribute Array and and create Attribute pointers
    va.AddBuffer((*vb), layout);
  
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
   
}




