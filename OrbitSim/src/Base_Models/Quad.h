#pragma once

#include "GLAbstraction.h"

static float vertices[] = { // INIT VERTICES
    // positions          
    0.5f, 0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f
};
static unsigned int indices[] = {
    0, 1, 3, // first triangle that makes up the square
    1, 2, 3 // second triangle
};

struct Quad
{
    Shader* quad_shader;
    Quad();
    ~Quad();

    virtual void setColor(float r, float g, float b, float a)=0;
    
    /* VAO VB and VB Layout */
    VertexArray va; // (GenVertexArray)
    VertexBuffer *vb; // (Gen, Bind, Data Implement)
    IndexBuffer *ib; 
    VertexBufferLayout layout; 

    
};

