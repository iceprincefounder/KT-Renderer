#ifndef __KTCORE_H__
#define __KTCORE_H__

#include <cmath>
#include <list>
#include <algorithm>

#include "ktColor.h"
#include "ktVector.h"
#include "ktRay.h"

using namespace KT;

namespace KT
{



//
// Intersection (results from casting a ray)
//

class Object;
class Shader;

struct Intersection
{
    Ray m_ray;
    float m_t;
    Object *m_pObject;
    Shader *m_pShader;
    Color m_color;
    Vector m_normal;
    
    
    Intersection()
        : m_ray(),
          m_t(kRayTMax),
          m_pObject(NULL),
          m_pShader(NULL),
          m_color(1.0f, 1.0f, 1.0f),
          m_normal()
    {
        
    }
    
    Intersection(const Intersection& i)
        : m_ray(i.m_ray),
          m_t(i.m_t),
          m_pObject(i.m_pObject),
          m_pShader(i.m_pShader),
          m_color(i.m_color),
          m_normal(i.m_normal)
    {
        
    }
    
    Intersection(const Ray& ray)
         : m_ray(ray),
           m_t(ray.m_tMax),
           m_pObject(NULL),
           m_pShader(NULL),
           m_color(1.0f, 1.0f, 1.0f),
           m_normal()
    {
        
    }
    
    Intersection& operator =(const Intersection& i)
    {
        m_ray = i.m_ray;
        m_t = i.m_t;
        m_pObject = i.m_pObject;
        m_pShader = i.m_pShader;
        m_color = i.m_color;
        m_normal = i.m_normal;
        return *this;
    }
    
    bool intersected() const { return (m_pObject == NULL) ? false : true; }
    
    Point position() const { return m_ray.calculate(m_t); }
};


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
        return std::max(0.0f, dot(lightDirectionNorm, normal)) * m_color;
    }
    
protected:
    Color m_color;
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
    virtual bool intersect(Intersection& intersection)
    {
        return false;
    }
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

    virtual bool intersect(Intersection& intersection)
    {
        // This is much like a plane intersection, except we also range check it
        // to make sure it's within the rectangle.  Please see the plane Object
        // intersection method for a little more info.
        
        Vector normal = cross(m_side1, m_side2).normalized();
        float nDotD = dot(normal, intersection.m_ray.m_direction);
        if (nDotD == 0.0f)
        {
            return false;
        }
        
        float t = (dot(m_position, normal) - dot(intersection.m_ray.m_origin, normal)) / dot(intersection.m_ray.m_direction, normal);
        
        // Make sure t is not behind the ray, and is closer than the current
        // closest intersection.
        if (t >= intersection.m_t || t < kRayTMin)
        {
            return false;
        }
        
        // Take the intersection point on the plane and transform it to a local
        // space where we can really easily check if it's in range or not.
        Vector side1Norm = m_side1;
        Vector side2Norm = m_side2;
        float side1Length = side1Norm.normalize();
        float side2Length = side2Norm.normalize();
        
        Point worldPoint = intersection.m_ray.calculate(t);
        Point worldRelativePoint = worldPoint - m_position;
        Point localPoint = Point(dot(worldRelativePoint, side1Norm),
                                 dot(worldRelativePoint, side2Norm), 0.0f);

        
        // Do the actual range check
        if (localPoint.m_x < 0.0f || localPoint.m_x > side1Length ||
            localPoint.m_y < 0.0f || localPoint.m_y > side2Length)
        {
            return false;
        }
        
        // This intersection is the closest so far, so record it.
        intersection.m_t = t;
        intersection.m_pObject = this;
        intersection.m_color = Color();
        intersection.m_normal = normal;
        // Hit the back side of the light?  We'll count it, so flip the normal
        // to effectively make a double-sided light.
        if (dot(intersection.m_normal, intersection.m_ray.m_direction) > 0.0f)
        {
            intersection.m_normal *= -1.0f;
        }
        
        return true;
    }
    
    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(float u1, float u2, const Point& referencePosition, Point& outPosition, Vector& outNormal)
    {
        outNormal = cross(m_side1, m_side2).normalized();
        outPosition = m_position + m_side1 * u1 + m_side2 * u2;
        // outPosition = m_position + m_side1  + m_side2 ;
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
class InfinitePlane : public Object
{
public:
    InfinitePlane(const Point& position, const Vector& normal, const Color& color)
        : m_position(position),
          m_normal(normal.normalized()),
          m_color(color)
    {
        
    }
    
    virtual ~InfinitePlane() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        // More info,check on wiki reference

        // Check if it's even possible to intersect
        float nDotD = dot(m_normal, intersection.m_ray.m_direction);
        if (nDotD >= 0.0f)
        {
            return false;
        }

        // Assume we have a ray R (or segment S) from P0 to P1, and a plane P through V0 with normal n. 
        float t = (dot(m_normal,m_position) - dot(m_normal, intersection.m_ray.m_origin)) / dot(m_normal, intersection.m_ray.m_direction);
        
        if (t >= intersection.m_t || t < kRayTMin)
        {
            return false;
        }
        
        // This intersection is closer, so record it.
        intersection.m_t = t;
        intersection.m_pObject = this;
        intersection.m_normal = m_normal;
        intersection.m_color = m_color;
        
        return true;
    }

protected:
    Point m_position;
    Vector m_normal;
    Color m_color;
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
    
    virtual bool intersect(Intersection& intersection)
    {
        Ray localRay = intersection.m_ray;
        localRay.m_origin -= m_position;
        
        // Ray-sphere intersection can result in either zero, one or two points
        // of intersection.  It turns into a quadratic equation, so we just find
        // the solution using the quadratic formula.  Note that there is a
        // slightly more stable form of it when computing it on a computer, and
        // we use that method to keep everything accurate.
        
        // Calculate quadratic coeffs
        float a = localRay.m_direction.length2();
        float b = 2.0f * dot(localRay.m_direction, localRay.m_origin);
        float c = localRay.m_origin.length2() - m_radius * m_radius;
        
        float t0, t1, discriminant;
        discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f)
        {
            // Discriminant less than zero?  No solution => no intersection.
            return false;
        }
        discriminant = std::sqrt(discriminant);
        
        // Compute a more stable form of our param t (t0 = q/a, t1 = c/q)
        // q = -0.5 * (b - sqrt(b * b - 4.0 * a * c)) if b < 0, or
        // q = -0.5 * (b + sqrt(b * b - 4.0 * a * c)) if b >= 0
        float q;
        if (b < 0.0f)
        {
            q = -0.5f * (b - discriminant);
        }
        else
        {
            q = -0.5f * (b + discriminant);
        }
        
        // Get our final parametric values
        t0 = q / a;
        if (q != 0.0f)
        {
            t1 = c / q;
        }
        else
        {
            t1 = intersection.m_t;
        }
        
        // Swap them so they are ordered right
        if (t0 > t1)
        {
            float temp = t1;
            t1 = t0;
            t0 = temp;
        }
        
        // Check our intersection for validity against this ray's extents
        if (t0 >= intersection.m_t || t1 < kRayTMin)
        {
            return false;
        }
        
        if (t0 >= kRayTMin)
        {
            intersection.m_t = t0;
        }
        else if (t1 < intersection.m_t)
        {
            intersection.m_t = t1;
        }
        else
        {
            return false;
        }
        
        // Create our intersection data
        Point localPos = localRay.calculate(intersection.m_t);
        Vector worldNorm = localPos.normalized();
        
        intersection.m_pObject = this;
        intersection.m_pShader = m_pShader;
        intersection.m_normal = worldNorm;
        intersection.m_color = Color(1.0f, 1.0f, 1.0f);
        
        return true;
    }
    
    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(float u1, float u2, const Point& referencePosition, Point& outPosition, Vector& outNormal)
    {
        // Pick a random point on the whole sphere surface
        outNormal = uniformToSphere(u1, u2);
        outPosition = outNormal * m_radius + m_position;
        if (dot(outNormal, referencePosition - outPosition) < 0.0f)
        {
            // Point was on the opposite side?  Flip around to make this
            // more reasonable.
            outNormal *= -1.0f;
            outPosition = outNormal * m_radius + m_position;
        }
        return true;
    }

protected:
    Point m_position;
    float m_radius;
    Shader *m_pShader;
    
    // Helper method for finding a random point on the sphere
    Vector uniformToSphere(float u1, float u2)
    {
        // Find a height uniformly distributed on the sphere
        float z = 1.0f - 2.0f * u1;
        // Find the radius based on that height that sits on the sphere surface
        float radius = std::sqrt(std::max(0.0f, 1.0f - z * z));
        // Find a random angle around the sphere's equator
        float phi = KT_PI * 2.0f * u2;
        // And put it all together...
        return Vector(radius * std::cos(phi),
                      radius * std::sin(phi),
                      z);
    }
};


// Marsaglia multiply-with-carry psuedo random number generator.  It's very fast
// and has good distribution properties.  Has a period of 2^60. See
// http://groups.google.com/group/sci.crypt/browse_thread/thread/ca8682a4658a124d/
struct RNG
{
    unsigned int m_z, m_w;
    
    RNG(unsigned int z = 362436069, unsigned int w = 521288629) : m_z(z), m_w(w) { }
    
    
    // Returns a 'canonical' float from [0,1)
    float nextFloat()
    {
        unsigned int i = nextUInt32();
        return i * 2.328306e-10f;
    }
 
    // Returns an int with random bits set
    unsigned int nextUInt32()
    {
        m_z = 36969 * (m_z & 65535) + (m_z >> 16);
        m_w = 18000 * (m_w & 65535) + (m_w >> 16);
        return (m_z << 16) + m_w;  /* 32-bit result */
    }
};

} // namespace KT

#endif // __KTCORE_H__