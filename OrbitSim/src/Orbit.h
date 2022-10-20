#pragma once

#include <math.h>
#include <GLCoreUtils.h>
#include "Body.h"

//#include <vector>

// UNIVERSAL CONSTANTS
#define PI static_cast<float>(3.14159265)
const float G_M = 4 * PI * PI; // G * sunMass constant
const float sunMass = 1.989e30;
// Force Constant predicted by General Relativity
#define ALPHA_FACTOR static_cast<float>(1.1e-8) // AU^2 - correction factor as predected by general relativity
#define FCONST(m,r,B) static_cast<float>(( (G_M * m) / pow(r, B + 1.0f) ) * (1+ (ALPHA_FACTOR/pow(r,2))))

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
    float v_minus1;
    
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
    float t;
    float dt;

    // Orbit Statistics
    float aphelion;   // max radius
    float perihelion; // min radius
    bool aphelionReached;
    bool perihelionReached;

    float period;     // period of the orbit
    bool finishedPeriod;
    bool finishedHalfPeriod;
    
    unsigned int periodCycles = 0;
    
    unsigned int iterations = 0;
    unsigned int itersTo1Period = MAXUINT;

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