#pragma once
#include "Base_Models/Quad.h"


struct Body: public Quad
{
    Body();
    // PARAMETERIZED CONSTRUCTOR
    Body(const unsigned int id, float m, float a, float radius);
    glm::vec3 UpdateScaleResize(const float& aspectRatio);
    virtual void setColor(float r, float g, float b, float a) override;

    Transform body_Transform;

    uint32_t ID;
    float mass; // in sun mass units
    float a;    // semi-major axis in AU
    float radius;

 
};