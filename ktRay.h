#ifndef __KTRAY_H__
#define __KTRAY_H__

#include "ktConstants.h"

namespace KT
{

//
// Ray (directed line segment)
//

const float kRayTMin = 0.00001f;
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

// output our image
std::ostream& operator <<(std::ostream& stream, const Color& c)
{
    stream << '(' << c.m_r << ", " << c.m_g << ", " << c.m_b << ')';
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const Vector& v)
{
    stream << '[' << v.m_x << ", " << v.m_y << ", " << v.m_z << ']';
    return stream;
}


// Set up a camera ray given the look-at spec, FOV, and screen position to aim at.
Ray createCameraRay(float fieldOfViewInDegrees,
                  const Point& origin,
                  const Vector& target,
                  const Vector& targetUpDirection,
                  float xScreenPos0To1,
                  float yScreenPos0To1)
{
    Vector forward = (target - origin).normalized();
    Vector right = cross(forward, targetUpDirection).normalized();
    Vector up = cross(right, forward).normalized();
    
    // Convert to radians, as that is what the math calls expect
    float tanFov = std::tan(fieldOfViewInDegrees * KT_PI / 180.0f);
    
    Ray ray;

    // Set up ray info
    ray.m_origin = origin;
    ray.m_direction = forward +
                      right * ((xScreenPos0To1 - 0.5f) * tanFov) +
                      up * ((yScreenPos0To1 - 0.5f) * tanFov);
    ray.m_direction.normalize();
    
    return ray;
}


} //ending namespace KT

#endif // __KTRAY_H__