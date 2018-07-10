#ifndef __KTLIGHTS_H__
#define __KTLIGHTS_H__

#include "ktCore.h"
#include "ktMaterials.h"

namespace KT
{

//
// Lights (scene hierarchy)
//

// Light base class, making it easy to find all the lights in the scene.
class Light : public Object
{
public:
    Light(const Color& c, float power ) : m_color(c), m_power(power), m_shader(c,power) { }
    
    virtual ~Light() { }
    
    // This *is* a light, so we put ourself on the list
    virtual void findLights(std::list<Object*>& outLightList);
    
    virtual Color emitted() const;
    
protected:
    Color m_color;
    float m_power;
    Emitter m_shader;
};

// Point light
class PointLight : public Light
{
public:
    PointLight(const Point& pos,
                   const Color& color,
                   float power)
        : Light(color, power), m_position(pos)
    {

    }
    virtual ~PointLight() { }

    virtual bool intersect(Intersection& intersection);

    virtual bool sampleSurface(float u1, float u2, 
        const Point& referencePosition, Point& outPosition, Vector& outNormal);

protected:
    Point m_position;

};

// Area light with a corner and two sides to define a rectangular/parallelipiped Object
class RectangleLight : public Light
{
public:
    RectangleLight(const Point& pos,
                   const Vector& side1,
                   const Vector& side2,
                   const Color& color,
                   float power)
        : Light(color, power), m_position(pos), m_side1(side1), m_side2(side2)
    {
        
    }

    virtual ~RectangleLight() { }

    virtual bool intersect(Intersection& intersection);

    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(float u1, float u2, 
        const Point& referencePosition, Point& outPosition, Vector& outNormal);
    
protected:
    Point m_position;
    Vector m_side1, m_side2;
};


} //ending namespace KT


#endif // __KTLIGHTS_H__