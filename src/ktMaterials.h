#ifndef __KTMATERIALS_H__
#define __KTMATERIALS_H__

#include "ktCore.h"

namespace KT
{

// Lambertian diffuse material
class Lambert : public Shader
{
public:
    Lambert(const Color& color) : m_color(color) { }
    
    virtual ~Lambert() { }
    
    virtual Color shade(const Point& position,
                        const Vector& normal,
                        const Vector& incomingRayDirection,
                        const Vector& lightDirectionNorm);    
protected:
    Color m_color;
};

// Phong glossy material
class Phong : public Shader
{
public:
    Phong(const Color& color, float exponent) : m_color(color), m_exponent(exponent) { }
    
    virtual ~Phong() { }
    
    virtual Color shade(const Point& position,
                        const Vector& normal,
                        const Vector& incomingRayDirection,
                        const Vector& lightDirectionNorm);    
protected:
    Color m_color;
    float m_exponent;
};


// Emitter (light) material
class Emitter : public Shader
{
public:
    Emitter(const Color& color, float power) : m_color(color), m_power(power) { }
    
    virtual ~Emitter() { }
    
    virtual Color emittance();
    
    virtual Color shade(const Point& position,
                        const Vector& normal,
                        const Vector& incomingRayDirection,
                        const Vector& lightDirectionNorm);    
protected:
    Color m_color;
    float m_power;
};


} //ending namespace KT

#endif // __KTMATERIALS_H__