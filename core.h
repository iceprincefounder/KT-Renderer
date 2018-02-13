#ifndef __KTCORE_H__
#define __KTCORE_H__

#include <cmath>
#include <list>
#include <algorithm>

#include "ktColor.h"
#include "ktVector.h"


using namespace KT;

namespace KT
{

//
// Ray (directed line segment)
//

// Don't ever start a ray exactly where you previously hit; you must offset it
// a little bit so you don't accidentally 'self-intersect'.
const float kRayTMin = 0.00001f;
// Unless otherwise specified, rays are defined to be able to hit anything as
// far as the computer can see.  You can limit a ray's max if you need to though
// as is done often when calculating shadows, so you only check the range from
// the point on the surface to the point on the light.
const float kRayTMax = 1.0e30f;


struct Ray
{
    Point m_origin;
    Vector m_direction;
    float m_tMax;
    
    // Some sane defaults
    Ray(): m_origin(), m_direction(0.0f, 0.0f, 1.0f), m_tMax(kRayTMax)
    {

    }
    
    Ray(const Ray& r): m_origin(r.m_origin), m_direction(r.m_direction), m_tMax(r.m_tMax)
    {

    }
    
    Ray(const Point& origin, const Vector& direction, float tMax = kRayTMax): m_origin(origin), m_direction(direction), m_tMax(tMax)
    {

    }
    
    Ray& operator =(const Ray& r)
    {
        m_origin = r.m_origin;
        m_direction = r.m_direction;
        m_tMax = r.m_tMax;
        return *this;
    }
    
    // when intersected,get the hit point position.
    Point calculate(float t) const 
    { 
        return m_origin + t * m_direction; 
    }
};


//
// Intersection (results from casting a ray)
//

class Object;

struct Intersection
{
    Ray m_ray;
    float m_t;
    Object *m_pObject;
    Color m_color;
    Color m_emitted;
    Vector m_normal;
    
    
    Intersection()
        : m_ray(),
          m_t(kRayTMax),
          m_pObject(NULL),
          m_color(),
          m_emitted(),
          m_normal()
    {
        
    }
    
    Intersection(const Intersection& i)
        : m_ray(i.m_ray),
          m_t(i.m_t),
          m_pObject(i.m_pObject),
          m_color(i.m_color),
          m_emitted(i.m_emitted),
          m_normal(i.m_normal)
    {
        
    }
    
    Intersection(const Ray& ray)
         : m_ray(ray),
           m_t(ray.m_tMax),
           m_pObject(NULL),
           m_color(),
           m_emitted(),
           m_normal()
    {
        
    }
    
    Intersection& operator =(const Intersection& i)
    {
        m_ray = i.m_ray;
        m_t = i.m_t;
        m_pObject = i.m_pObject;
        m_color = i.m_color;
        m_emitted = i.m_emitted;
        m_normal = i.m_normal;
        return *this;
    }
    
    bool intersected() const { return (m_pObject == NULL) ? false : true; }
    
    Point position() const { return m_ray.calculate(m_t); }
};


//
// Objects (scene hierarchy)
//

class Object
{
public:
    virtual ~Object() { }
    
    // Subclasses must implement this; this is the meat of ray tracing
    virtual bool intersect(Intersection& intersection) = 0;
    
    // Usually for lights: given two random numbers between 0.0 and 1.0, find a
    // location + surface normal on the surface.  Return false if not a surface.
    virtual bool sampleSurface(float u1, float u2, const Point& referencePosition, Point& outPosition, Vector& outNormal)
    {
        return false;
    }
    
    // Find all lights in the scene starting with this Object
    virtual void findLights(std::list<Object*>& outLightList) { }
};


// List of Objects, so you can aggregate a pile of them
class ObjectSet : public Object
{
public:
    virtual ~ObjectSet() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        bool intersectedAny = false;
        for (std::list<Object*>::iterator iter = m_Objects.begin(); iter != m_Objects.end(); ++iter)
        {
            Object *pObject = *iter;
            bool intersected = pObject->intersect(intersection);
            if (intersected)
            {
                intersectedAny = true;
            }
        }
        return intersectedAny;
    }
    
    virtual void findLights(std::list<Object*>& outLightList)
    {
        for (std::list<Object*>::iterator iter = m_Objects.begin(); iter != m_Objects.end(); ++iter)
        {
            Object *pObject = *iter;
            pObject->findLights(outLightList);
        }
    }
    
    void addObject(Object *pObject) { m_Objects.push_back(pObject); }
    
    void clearObjects() { m_Objects.clear(); }
    
protected:
    std::list<Object*> m_Objects;
};


// Light base class, making it easy to find all the lights in the scene.
class Light : public Object
{
public:
    Light(const Color& c, float power) : m_color(c), m_power(power) { }
    
    virtual ~Light() { }
    
    // This *is* a light, so we put ourself on the list
    virtual void findLights(std::list<Object*>& outLightList) 
    { 
        outLightList.push_back(this); 
    }
    
    virtual Color emitted() const { return m_color * m_power; }
    
protected:
    Color m_color;
    float m_power;
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

    virtual bool intersect(Intersection& intersection)
    {
        return false;
    }
    // virtual bool intersect(Intersection& intersection)
    // {
    //     // This is much like a plane intersection, except we also range check it
    //     // to make sure it's within the rectangle.  Please see the plane Object
    //     // intersection method for a little more info.
        
    //     Vector normal = cross(m_side1, m_side2).normalized();
    //     float nDotD = dot(normal, intersection.m_ray.m_direction);
    //     if (nDotD == 0.0f)
    //     {
    //         return false;
    //     }
        
    //     float t = (dot(m_position, normal) - dot(intersection.m_ray.m_origin, normal)) / dot(intersection.m_ray.m_direction, normal);
        
    //     // Make sure t is not behind the ray, and is closer than the current
    //     // closest intersection.
    //     if (t >= intersection.m_t || t < kRayTMin)
    //     {
    //         return false;
    //     }
        
    //     // Take the intersection point on the plane and transform it to a local
    //     // space where we can really easily check if it's in range or not.
    //     Vector side1Norm = m_side1;
    //     Vector side2Norm = m_side2;
    //     float side1Length = side1Norm.normalize();
    //     float side2Length = side2Norm.normalize();
        
    //     Point worldPoint = intersection.m_ray.calculate(t);
    //     Point worldRelativePoint = worldPoint - m_position;
    //     Point localPoint = Point(dot(worldRelativePoint, side1Norm),
    //                              dot(worldRelativePoint, side2Norm), 0.0f);

        
    //     // Do the actual range check
    //     if (localPoint.m_x < 0.0f || localPoint.m_x > side1Length ||
    //         localPoint.m_y < 0.0f || localPoint.m_y > side2Length)
    //     {
    //         return false;
    //     }
        
    //     // This intersection is the closest so far, so record it.
    //     intersection.m_t = t;
    //     intersection.m_pObject = this;
    //     intersection.m_color = Color();
    //     intersection.m_emitted = m_color * m_power;
    //     intersection.m_normal = normal;
    //     // Hit the back side of the light?  We'll count it, so flip the normal
    //     // to effectively make a double-sided light.
    //     if (dot(intersection.m_normal, intersection.m_ray.m_direction) > 0.0f)
    //     {
    //         intersection.m_normal *= -1.0f;
    //     }
        
    //     return true;
    // }
    
    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(float u1, float u2, const Point& referencePosition, Point& outPosition, Vector& outNormal)
    {
        outNormal = cross(m_side1, m_side2).normalized();
        outPosition = m_position + m_side1 * u1 + m_side2 * u2;
        // Reference point out in back of the light?  That's okay, we'll flip
        // the normal to have a double-sided light.
        if (dot(outNormal, outPosition - referencePosition) > 0.0f)
        {
            outNormal *= -1.0f;
        }
        return true;
    }
    
protected:
    Point m_position;
    Vector m_side1, m_side2;
};


// Infinite-extent plane, with option bullseye texturing to make it interesting.
class Plane : public Object
{
public:
    Plane(const Point& position, const Vector& normal, const Color& color, bool bullseye = false)
        : m_position(position),
          m_normal(normal.normalized()),
          m_color(color),
          m_bullseye(bullseye)
    {
        
    }
    
    virtual ~Plane() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        // Plane eqn: ax+by+cz+d=0; another way of writing it is: dot(n, p-p0)=0
        // where n=normal=(a,b,c), and p=(x,y,z), and p0 is position.  Now, p is
        // the ray equation (the intersection point is along the ray): p=origin+t*direction
        // So the plane-ray intersection eqn is dot(n, origin+t*direction-p0)=0.
        // Distribute, and you get:
        //     dot(n, origin) + t*dot(n, direction) - dot(n, p0) = 0
        // Solve for t, and you get:
        //    t = (dot(n, p0) - dot(n, origin)) / dot(n, direction)
        
        // Check if it's even possible to intersect
        float nDotD = dot(m_normal, intersection.m_ray.m_direction);
        if (nDotD >= 0.0f)
        {
            return false;
        }

        // Assume we have a ray R (or segment S) from P0 to P1, and a plane P through V0 with normal n. 
        float t = (dot(m_normal,m_position) - dot(m_normal, intersection.m_ray.m_origin)) / dot(m_normal, intersection.m_ray.m_direction);
        
        // Make sure t is not behind the ray, and is closer than the current
        // closest intersection.
        if (t >= intersection.m_t || t < kRayTMin)
        {
            return false;
        }
        
        // This intersection is closer, so record it.
        intersection.m_t = t;
        intersection.m_pObject = this;
        intersection.m_normal = m_normal;
        intersection.m_emitted = Color();
        intersection.m_color = m_color;

        // Hack bullseye pattern to get some variation
        if (m_bullseye && std::fmod((intersection.position() - m_position).length() * 0.25f, 1.0f) > 0.5f)
        {
            intersection.m_color *= 0.2f;
        }
        
        return true;
    }

protected:
    Point m_position;
    Vector m_normal;
    Color m_color;
    bool m_bullseye;
};


} // namespace KT

#endif // __KTCORE_H__