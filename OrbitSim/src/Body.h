#pragma once
//#include "Base_Models/Quad.h"
#include "Base_Models/Circle.h"


struct Body: public Circle
{
    // PARAMETERIZED CONSTRUCTOR
    Body(const unsigned int id, float m, float radius, unsigned int numSides = 100);
    glm::vec3 UpdateScaleResize(const float& aspectRatio);
    virtual void setColor(float r, float g, float b, float a) override;
    void setAlpha(const float alphaVal);

    glm::vec4 circleColor;
    Transform body_Transform;

    uint32_t ID;
    float mass; // in sun mass units
    float radius; // In Sun Radii

 
};


