#include "ktLights.h"


namespace KT
{

void Light::findLights(std::list<Object*>& outLightList) 
{ 
    outLightList.push_back(this); 
}

Color Light::emitted() const 
{ 
    return m_color * m_power; 
}

bool PointLight::intersect(Intersection& intersection)
{
    return false;
}

bool PointLight::sampleSurface(float u1, float u2, const Point& referencePosition, Point& outPosition, Vector& outNormal)
{
    outNormal = Vector(0.5f, 0.5f, 1.0f);
    outPosition = m_position + m_position*(u1+u2)*0.5;
    return true;
}



bool RectangleLight::intersect(Intersection& intersection)
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
    // this shader would draw light emison shape
    intersection.m_pShader = &m_shader;
    intersection.m_color = Color();
    intersection.m_emitted = m_color * m_power;
    intersection.m_normal = normal;
    // Hit the back side of the light?  We'll count it, so flip the normal
    // to effectively make a double-sided light.
    if (dot(intersection.m_normal, intersection.m_ray.m_direction) > 0.0f)
    {
        intersection.m_normal *= -1.0f;
    }
    
    return true;
}
    
bool RectangleLight::sampleSurface(float u1, float u2, const Point& referencePosition, Point& outPosition, Vector& outNormal)
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


} //ending namespace KT