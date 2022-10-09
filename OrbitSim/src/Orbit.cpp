#include "Orbit.h"


// Constructor Initializes all attribute vars
Orbit::Orbit(Body *body, float initx, float inity, float initvx, float initvy, float beta, float t, float dt)
    : body(body), x{ initx }, y{ inity }, vx{ initvx }, vy{ initvy }, B{ beta }, t(t), dt(dt)
{
    x0 = x;
    y0 = y;
    vx0 = vx;
    vy0 = vy;
    // initialize magnitudes:
    r = sqrt(x * x + y * y);     // magnitude of initial position
    v = sqrt(vx * vx + vy * vy); // magnitude of initial velocity
    v0 = v; // set initial var variable
    // Initialize the force (magnitude)
    fx = FCONST(body->mass, r, B) * x;
    fy = FCONST(body->mass, r, B) * y;
    f = sqrt(fx * fx + fy * fy);

    // calculate Theoretical Statistics (period, aphelion, perihelion)
    aphelion = NULL;
    perihelion = NULL;
    period = NULL;

    std::cout << "ORBIT INITIALIZED" << std::endl;
}



//Update Orbit Positions using EULER CROMER METHOD
void Orbit::Update(float universeTime, float deltaTime, bool rkIntegration)
{
    // Time variables
    t = universeTime;
    dt = deltaTime;
    // Distance from the sun
    r = sqrt((x * x) + (y * y));
    // Force Constant
    
    // Euler Cromer Method //
    if (!rkIntegration)
    { 
        // updating the force
        fx = FCONST(body->mass,r,B) * x;
        fy = FCONST(body->mass, r, B) * y;
        f = sqrt(fx * fx + fy * fy);
        // updating velocity
        vx = vx - (fx / body->mass) * dt;
        vy = vy - (fy / body->mass) * dt;
        v = sqrt(vx * vx + vy * vy);
        // updating orbit position
        x = x + (vx * dt);
        y = y + (vy * dt);
        r = sqrt(x * x + y * y);
    }
    else { // Runga Kutta 4th Order Method //

        // 2nd order
        // partial step: f + (df * dt * scale)
        /*float k1y = -FCONST(1.0, r, B) * y;
        float k1x = -FCONST(1.0, r, B) * x;

        float vy_u = vy + (k1y*dt*0.5);
        float y_u = y + (vy_u * dt * 0.5);
        float k2y = -FCONST(1.0, r, B) * y_u;

        float vx_u = vx + (k1x * dt * 0.5);
        float x_u = x + (vx_u * dt * 0.5);
        float k2x = -FCONST(1.0, r, B) * x_u;

        ax = (k1x + (k2x) ) / 2.0f;
        ay = (k1y + (k2y) ) / 2.0f;*/

        
        //k1 - acceleration at current location
        float k1x = -fx;
        float k1y = -fy;
        ////k2 - acceleration 0.5 timesteps in the future based on k1 acceleration value
        float vx_u = PartialStep(vx, k1x, 0.5f);
        float x_u = PartialStep(x, vx_u, 0.5f);
        float k2x = -FCONST(body->mass, r, B) * x_u;

        float vy_u = PartialStep(vy, k1y, 0.5f);
        float y_u = PartialStep(y, vy_u, 0.5f);
        float k2y = -FCONST(body->mass, r, B) * y_u;
        ////k3 acceleration 0.5 timesteps in the future using k2 acceleration
        vx_u = PartialStep(vx, k2x, 0.5f);
        x_u = PartialStep(x, vx_u, 0.5f);
        float k3x = -FCONST(body->mass, r, B) * x_u;

        vy_u = PartialStep(vy, k2y, 0.5f);
        y_u = PartialStep(y, vy_u, 0.5f);
        float k3y = -FCONST(body->mass, r, B) * y_u;
        ////k4 - location 1 timestep in the future using k3 acceleration
        vx_u = PartialStep(vx, k3x, 1.0f);
        x_u = PartialStep(x, vx_u, 1.0f);
        float k4x = -FCONST(body->mass, r, B) * x_u;

        vy_u = PartialStep(vy, k3y, 1.0f);
        y_u = PartialStep(y, vy_u, 1.0f);
        float k4y = -FCONST(body->mass, r, B) * y_u;


        // Final Accelerations, Velocities and Postions
        float ax = (1.0f/body->mass)*(k1x + (2.0f * k2x) + (2.0f * k3x) + k4x) / 6.0f;
        float ay = (1.0f / body->mass) * (k1y + (2.0f * k2y) + (2.0f * k3y) + k4y) / 6.0f;

        vx += ax * dt;
        vy += ay * dt;
        v = sqrt(vx * vx + vy * vy);
        x += vx * dt;
        y += vy * dt;
        r = sqrt(x * x + y * y);
        fx = FCONST(body->mass, r, B) * x;
        fy = FCONST(body->mass, r, B) * y;
        f = sqrt(fx * fx + fy * fy);
    }
    

   
    iterations++;
}

float Orbit::PartialStep(float& f, float df, float scale)
{
    return f + (df * dt * scale);
}


void Orbit::Reset()
{
    // reset position, velocities, and energies to their previous values
    x = x0;
    y = y0;
    vx = vx0;
    vy = vy0;
    t = 0.0f;
    
    // initialize magnitudes:
    r = sqrt(x * x + y * y);     // magnitude of initial position
    v = sqrt(vx * vx + vy * vy); // magnitude of initial velocity
    v0 = v; // set initial var variable
    // Initialize the force (magnitude)
    fx = (G_M * body->mass * x) / pow(r, B + 1.0f);
    fy = (G_M * body->mass * y) / pow(r, B + 1.0f);
    f = sqrt(fx * fx + fy * fy);

    // calculate Theoretical Statistics (period, aphelion, perihelion)

    std::cout << "ORBIT RESET" << std::endl;
}


bool Orbit::isAphelion(float v0, float v)
{
    if ((v - v0) > 0)
    {
        return true;
    }
    return false;
}

bool Orbit::isPerihelion(float v0, float v)
{
    if ((v - v0) < 0)
    {
        return true;
    }
    return false;
}







   

