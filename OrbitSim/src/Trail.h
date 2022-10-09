#pragma once

#include <GLAbstraction.h>
#include <GLCoreUtils.h>
#include <vector>


struct Trail
{

    Shader* Trail_shader;
    Trail();
    ~Trail();

    void setColor(const glm::vec4 &color);

    /* VAO VB and VB Layout */
    VertexArray va; // (GenVertexArray)
    VertexBuffer* vb; // (Gen, Bind, Data Implement)
    VertexBufferLayout layout;

    // Transform
    Transform trail_Transform;

    // Store Vertices
    std::vector <float> vertices;
    void UpdateTrail(const float &x, const float& y);
    void SetBuffers();
    //void SetTrailAlpha(const float &a = 1.0f);
};

