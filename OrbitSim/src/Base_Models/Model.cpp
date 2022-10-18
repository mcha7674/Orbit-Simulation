#include "Model.h"

Circle::Circle()
{
    // init Vertices and Indices   
    // Init Shader
    Circle_shader = Shader::FromGLSLTextFiles(
        "assets/shaders/Circle.vert.glsl",
        "assets/shaders/Circle.frag.glsl"
    );

    // INIT VBO and IBO
    vb = new VertexBuffer(NULL, (unsigned int)(vertices.size() * sizeof(vertices[0])), false);

    // Create layout
    layout.Push<float>(3); // 3 floats per vertex for position

    va.AddBuffer((*vb), layout);

}

void Circle::InitCircle(const unsigned int numberOfSides, const float r)
{
    NumberOfSides = numberOfSides;
    NumberOfVertices = NumberOfSides + 2;
    // Fill Vertex Buffer Array
    fillCircleVertices(r);
    // Update vb
    vb->UpdateBuffer(&vertices[0], (unsigned int)(vertices.size() * sizeof(vertices[0])), false);
}

// Triangle method
void Circle::fillCircleVertices(const float& r)
{
    float doublePi = 2.0f * PI;
    float increment = doublePi / NumberOfSides;
    // push back Starting Vertex
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    unsigned int i = 3;
    for (float angle = 0.0f; angle <= doublePi + 1.0f; angle += increment)
    {
        vertices.push_back(r * cos(angle));//x
        vertices.push_back(r * sin(angle));//y
        vertices.push_back(0.0f);//z
    }

}

Circle::~Circle()
{
    va.UnBind();
    vb->UnBind();
    Circle_shader->unuse();

    // delete mem allocation
    delete vb;
    delete Circle_shader;

}

//////////////////////////////////////////////////

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




