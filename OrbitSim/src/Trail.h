#pragma once

#include <GLAbstraction.h>
#include <GLCoreUtils.h>
#include <queue>


class Trail
{
public:
    Shader* Trail_shader;
    Trail();
    ~Trail();

    void setColor(const glm::vec4& color);
    void UpdateTrail(const float& x, const float& y,const bool &isPeriodComplete);
    void ResetTrail(const float x, const float y);
    bool addNewVertices = true;

    /* VAO VB and VB Layout */
    VertexArray va; // (GenVertexArray)
    VertexBuffer* vb; // (Gen, Bind, Data Implement)
    VertexBufferLayout layout;

    // Transform
    Transform trail_Transform;

    // Store Vertices
    std::vector <float> vertices;

private:
    // trail trimming Time function
    float T();
    

};

