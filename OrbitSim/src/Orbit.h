#pragma once

#include <math.h>
#include <GLCoreUtils.h>
#include "Body.h"

//#include <vector>

// UNIVERSAL CONSTANTS
#define PI 3.14159265
const float G_M = 4 * PI * PI; // G * sunMass constant
const long float sunMass = 1.989e30;
#define FCONST(m,r,B) (G_M * m) / pow(r, B + 1.0f)

class Orbit
{
public:
    // Body
    Body* body;
    // Position vars (AU units)
    float x0;
    float y0;
    float x;
    float y;
    float r;
    // Velocity vars (AU/yr units)
    float vx;
    float vy;
    float v;
    float v0;
    float vx0;
    float vy0;

    float ax = 0;
    float ay = 0;
    // Force vars (MAGNITUDES)
    float fx;
    float fy;
    float f;
    float B; // beta constant for inverse law
    
    // ENERGY vars
    float KE;
    float PE;
    float E;
    // time related vars (yr units)
    float timeBuffer;
    float t;
    float dt;
    // Statistics
    float aphelion;   // max radius
    float perihelion; // min radius
    bool aphelionReached;
    bool perihelionReached;

    float period;     // period of the orbit
    bool finishedPeriod;
    bool finishedHalfPeriod;
    
    unsigned int iterations = 0;

public:
    // PARAMETERIZED CONSTRUCTOR
    Orbit(Body* body, float initx, float inity, float initvx, 
        float initvy, float beta, float t, float dt);
    // DATA CREATION
    void Update(float t, float dt, bool rkIntegration = false);
    void Reset();

    bool isAphelion(float v0, float v);
    bool isPerihelion(float v0, float v);
    inline bool isPeriodComplete() { return finishedPeriod; };

private:
    

    float PartialStep(float& f, float df, float scale);
    
};