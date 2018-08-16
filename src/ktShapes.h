#ifndef __KTSHAPES_H__
#define __KTSHAPES_H__

#include "ktCore.h"

namespace KT
{

//
// meshShapes (scene hierarchy)
//

class InfinitePlane : public Object
{
public:
    InfinitePlane(const Point& position, const Vector& normal, Color color, Shader *pShader)
        : m_position(position),
          m_normal(normal.normalized()),
          m_color(color),
          m_shader(pShader)
    {
        
    }
    
    virtual ~InfinitePlane() { }
    
    virtual bool intersect(Intersection& intersection);

protected:
    Point m_position;
    Vector m_normal;
    Color m_color;
    Shader *m_shader;
};

// Sphere (heh, what else?)
class Sphere : public Object
{
public:
    Sphere(const Point& position, float radius, Shader* pShader)
        : m_position(position),
          m_radius(radius),
          m_pShader(pShader)
    {
        
    }
    
    virtual ~Sphere() { }
    
    virtual bool intersect(Intersection& intersection);

    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(float u1, float u2, 
        const Point& referencePosition, 
        Point& outPosition, Vector& outNormal);
protected:
    Point m_position;
    float m_radius;
    Shader *m_pShader;
    
    // Helper method for finding a random point on the sphere
    Vector uniformToSphere(float u1, float u2);
};

} //ending namespace KT