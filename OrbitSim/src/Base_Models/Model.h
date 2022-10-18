#pragma once

#include "GLAbstraction.h"

static float PI = 3.141592653589f;
// Quad vertices
static float vertices[] = {
    // positions    // texture coords - origin is bottom left
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f    // top left 
};
static unsigned int indices[] = {
    0, 1, 3, // first triangle that makes up the square
    1, 2, 3 // second triangle
};


struct Circle
{
    Shader* Circle_shader;
    Circle();
    ~Circle();

    virtual void setColor(float r, float g, float b, float a) = 0;
    void InitCircle(const unsigned int numberOfSides, const float radius);
    void fillCircleVertices(const float& r);

    /* VAO VB and VB Layout */
    VertexArray va; // (GenVertexArray)
    VertexBuffer* vb; // (Gen, Bind, Data Implement)
    //IndexBuffer* ib;
    VertexBufferLayout layout;

    std::vector <float> vertices;

    unsigned int NumberOfSides;
    unsigned int NumberOfVertices;
    //float radius;
};



struct Quad
{
    Shader* quad_shader;
    Quad();
    ~Quad();

    Transform trans;
    Texture* texture;
    void setTexture(const char* texturePath);

    void setColor(float r, float g, float b, float a);

    /* VAO VB and VB Layout */
    VertexArray va; // (GenVertexArray)
    VertexBuffer* vb; // (Gen, Bind, Data Implement)
    IndexBuffer* ib;
    VertexBufferLayout layout;


};