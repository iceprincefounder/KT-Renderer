#include "ktCore.h"


#ifndef __KTSHADERS_H__
#define __KTSHADERS_H__

namespace KT
{

//
// Shaders (scene hierarchy)
//

class Shader
{
public:
    virtual ~Shader() { }
    
    // If the material is emitting, override this
    virtual Color emittance() { return Color(); }
    
    virtual Color shade(const Point& position,
                        const Vector& normal,
                        const Vector& incomingRayDirection,
                        const Vector& lightDirection) = 0;
};

// Lambertian diffuse material
class Lambert : public Shader
{
public:
    Lambert(const Color& color) : m_color(color) { }
    
    virtual ~Lambert() { }
    
    virtual Color shade(const Point& position,
                        const Vector& normal,
                        const Vector& incomingRayDirection,
                        const Vector& lightDirectionNorm)
    {
        return std::max(0.0f, dot(normal, lightDirectionNorm)) * m_color;
    }
    
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
                        const Vector& lightDirectionNorm)
    {
        Vector halfVec = (lightDirectionNorm - incomingRayDirection).normalized();
        return std::pow(std::max(0.0f, dot(halfVec, normal)), m_exponent) * m_color;
    }
    
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
    
    virtual Color emittance() { return m_color * m_power; }
    
    virtual Color shade(const Point& position,
                        const Vector& normal,
                        const Vector& incomingRayDirection,
                        const Vector& lightDirectionNorm)
    {
        // Let the emittance take care of return color
        return Color();
    }
    
protected:
    Color m_color;
    float m_power;
};


} //ending namespace KT


#endif // __KTSHADERS_H__