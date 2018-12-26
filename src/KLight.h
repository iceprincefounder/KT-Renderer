#pragma once

#include "KMathCore.h"
#include "KMaterial.h"
#include "KShape.h"
#include "KPolymesh.h"


namespace kt{

// Light base class, making it easy to find all the lights in the scene.
class Light : public Shape
{
public:
    Light(const Color& c, float power) : Shape(), m_color(c), m_power(power), m_material(c, power) { }
    
    virtual ~Light() { }
    
    // This *is* a light, so we put ourself on the list
    virtual void findLights(std::vector<Shape*>& outLightList) { outLightList.push_back(this); }
    virtual bool isLight() const { return true; }
    
    virtual Color emitted() const { return m_color * m_power; }
    
    virtual float intersectPDF(const Intersection& intersection) = 0;

protected:
    Color m_color;
    float m_power;
    
    EmitterMaterial m_material;
};


// Area light with a corner and two sides to define a rectangular/parallelipiped shape
class RectangleLight : public Light
{
public:
    RectangleLight(const Point& position,
                   const Vector& side1,
                   const Vector& side2,
                   const Color& color,
                   float power)
        : Light(color, power), m_position(position), m_side1(side1), m_side2(side2)
    {
        
    }
    
    virtual ~RectangleLight() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        Ray localRay = intersection.m_ray.transformToLocal(m_transform);
        
        // This is much like a plane intersection, except we also range check it
        // to make sure it's within the rectangle.  Please see the plane shape
        // intersection method for a little more info.
        
        Vector normal = cross(m_side1, m_side2).normalized();
        float nDotD = dot(normal, localRay.m_direction);
        if (nDotD == 0.0f)
        {
            return false;
        }
        
        float t = (dot(m_position, normal) - dot(localRay.m_origin, normal)) / nDotD;
        
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
        
        Point point = localRay.calculate(t);
        Point relativePoint = point - m_position;
        Point localPoint = Point(dot(relativePoint, side1Norm),
                                 dot(relativePoint, side2Norm),
                                 0.0f);
        
        // Do the actual range check
        if (localPoint.x < 0.0f || localPoint.x > side1Length ||
            localPoint.y < 0.0f || localPoint.y > side2Length)
        {
            return false;
        }
        
        // This intersection is the closest so far, so record it.
        intersection.m_t = t;
        intersection.m_pShape = this;
        intersection.m_pMaterial = &m_material;
        intersection.m_colorModifier = Color(1.0f, 1.0f, 1.0f);
        intersection.m_normal = m_transform.fromLocalNormal(localRay.m_time, normal);
        // Hit the back side of the light?  We'll count it, so flip the normal
        // to effectively make a double-sided light.
        if (dot(intersection.m_normal, intersection.m_ray.m_direction) > 0.0f)
        {
            intersection.m_normal *= -1.0f;
        }
        
        return true;
    }
    
    virtual bool doesIntersect(const Ray& ray)
    {
        Ray localRay = ray.transformToLocal(m_transform);
        
        // This is much like a plane intersection, except we also range check it
        // to make sure it's within the rectangle.  Please see the plane shape
        // intersection method for a little more info.
        
        Vector normal = cross(m_side1, m_side2).normalized();
        float nDotD = dot(normal, localRay.m_direction);
        if (nDotD == 0.0f)
        {
            return false;
        }
        
        float t = (dot(m_position, normal) - dot(localRay.m_origin, normal)) / nDotD;
        
        // Make sure t is not behind the ray, and is closer than the current
        // closest intersection.
        if (t >= ray.m_tMax || t < kRayTMin)
        {
            return false;
        }
        
        // Take the intersection point on the plane and transform it to a local
        // space where we can really easily check if it's in range or not.
        Vector side1Norm = m_side1;
        Vector side2Norm = m_side2;
        float side1Length = side1Norm.normalize();
        float side2Length = side2Norm.normalize();
        
        Point point = localRay.calculate(t);
        Point relativePoint = point - m_position;
        Point localPoint = Point(dot(relativePoint, side1Norm),
                                 dot(relativePoint, side2Norm),
                                 0.0f);
        
        // Do the actual range check
        if (localPoint.x < 0.0f || localPoint.x > side1Length ||
            localPoint.y < 0.0f || localPoint.y > side2Length)
        {
            return false;
        }
        
        return true;
    }
    
    virtual BBox bbox()
    {
        // Calculate bbox in non-local space
        Point corners[] = { m_position,
                            m_position + m_side1,
                            m_position + m_side2,
                            m_position + m_side1 + m_side2 };
        BBox result;
        for (size_t ti = 0; ti < m_transform.numKeys(); ++ti)
        {
            float time = m_transform.keyTime(ti);
            for (int i = 0; i < 4; ++i)
            {
                result.expand(m_transform.fromLocalPoint(time, corners[i]));
            }
        }
        return result;
    }
    
    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(const Point& surfPosition,
                               const Vector& surfNormal,
                               float refTime,
                               float u1, float u2, float u3,
                               Point& outPosition,
                               Vector& outNormal,
                               float& outPDF)
    {
        // Take care which calculations must be done in local space, 
        // and which should be non-local
        outPosition = m_position + m_side1 * u1 + m_side2 * u2;
        outPosition = m_transform.fromLocalPoint(refTime, outPosition);
        Vector outgoing = surfPosition - outPosition;
        float dist = outgoing.normalize();
        outNormal = cross(m_side1, m_side2);
        // Transform out as a vector to preserve scale (we need it for the area calculation)
        outNormal = m_transform.fromLocalVector(refTime, outNormal);
        float area = outNormal.normalize();
        // Reference point out in back of the light?  That's okay, we'll flip
        // the normal to have a double-sided light.
        if (dot(outNormal, outgoing) < 0.0f)
        {
            outNormal *= -1.0f;
        }
        outPDF = dist * dist / (area * std::fabs(dot(outNormal, outgoing)));
        // Really big PDFs blow up power-heuristic MIS; detect it and don't
        // sample in that case
        if (outPDF > 1.0e10f)
        {
            outPDF = 0.0f;
            return false;
        }
        return true;
    }
    
    virtual float intersectPDF(const Intersection& intersection)
    {
        if (intersection.m_pShape == this)
        {
            // Take care which calculations must be done in local space,
            // and which should be non-local
            Vector side1 = m_transform.fromLocalVector(intersection.m_ray.m_time, m_side1);
            Vector side2 = m_transform.fromLocalVector(intersection.m_ray.m_time, m_side2);
            float pdf = intersection.m_t * intersection.m_t /
                        (std::fabs(dot(intersection.m_normal, -intersection.m_ray.m_direction)) *
                         cross(side1, side2).length());
            // Really big PDFs blow up power-heuristic MIS; detect it and don't
            // sample in that case
            if (pdf > 1.0e10f)
            {
                return 0.0f;
            }
            return pdf;
        }
        return 0.0f;
    }
    
protected:
    Point m_position;
    Vector m_side1, m_side2;
};


// Mesh light based on an arbitrary shape from the scene.  Note that transforming
// this actual light will have no effect; you should transform the shape that it
// is attached to.
//
// The MeshLight is not stable right now, we need to improve light sample algorithm.
class MeshLight : public Light
{
public:
    MeshLight(Polymesh *pShape,
               const Color& color,
               float power)
        : Light(color, power), m_pShape(pShape)
    {
        pShape->setMaterial(&m_material);
    }
    
    virtual ~MeshLight() { }
    
    virtual bool intersect(Intersection& intersection)
    {
        // Forward intersection test on to the shape, but patch in the light material
        if (m_pShape->intersect(intersection))
        {
            intersection.m_pMaterial = &m_material;
            intersection.m_pShape = this;
            return true;
        }
        return false;
    }
    
    virtual bool doesIntersect(const Ray& ray)
    {
        // Forward intersection test on to the shape
        return m_pShape->doesIntersect(ray);
    }
    
    virtual BBox bbox()
    {
        return m_pShape->bbox();
    }
    
    virtual void prepare()
    {
        m_pShape->prepare();
    }
    
    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(const Point& surfPosition,
                               const Point& surfNormal,
                               float refTime,
                               float u1, float u2, float u3,
                               Point& outPosition,
                               Vector& outNormal,
                               float& outPDF)
    {
        // Forward surface sampling on to the shape
        if (!m_pShape->sampleSurface(surfPosition, surfNormal, refTime,
                                     u1, u2, u3,
                                     outPosition, outNormal, outPDF))
        {
            outPDF = 0.0f;
            return false;
        }
        // Reference point out in back of the light or light on backside of sample?  Discard the sample.
        if (dot(outNormal, surfPosition - outPosition) < 0.0f)
        {
            return false;
        }
        return true;
    }
    
    virtual float intersectPDF(const Intersection& intersection)
    {
        if (intersection.m_pShape == this)
        {
            return m_pShape->pdfSA(intersection.m_ray.m_origin,
                                   intersection.m_ray.m_direction, // TODO: this isn't quite correct, but it's unused ATM
                                   intersection.m_ray.m_time,
                                   intersection.position(),
                                   intersection.m_normal);
        }
        return 0.0f;
    }
    
protected:
    Shape *m_pShape;
};

// Directional light, distant light like sun light , is this physical?
class DistantLight : public Light
{
public:
    DistantLight(const Point& position,
                 const Vector& direction,
                 const Color& color,
                 float power): 
                    Light(color, power), 
                    m_position(position), 
                    m_direction(direction)
    {
        
    }
    
    virtual ~DistantLight() { }
    
    virtual bool intersect(Intersection& intersection)
    {   
        return false;
    }
    
    virtual bool doesIntersect(const Ray& ray)
    {        
        return false;
    }
    
    virtual BBox bbox()
    {
        // Calculate bbox in non-local space

        Point minCorners(-kRayTMax, -kRayTMax, -kRayTMax);
        Point maxCorners(kRayTMax, kRayTMax, kRayTMax);
        return BBox(minCorners, maxCorners);
    }
    
    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(const Point& surfPosition,
                               const Vector& surfNormal,
                               float refTime,
                               float u1, float u2, float u3,
                               Point& outPosition,
                               Vector& outNormal,
                               float& outPDF)
    {
        // outPosition = kRayTMax * m_direction + surfPosition;
        // outPosition = outPosition.normalized();
        outPosition = m_direction + surfPosition;
        outPosition = outPosition*(u1+u2)*u3;
        m_transform.setTranslation(0.0f, Vector(1000.0f, 1000.0f, 1000.0f));
        outPosition = m_transform.fromLocalPoint(refTime, outPosition);
        Vector outgoing = m_direction;
        float dist = outgoing.normalize();
        outNormal = m_direction;
        float area = outNormal.normalize();
        outPDF = dist * dist / (area * std::fabs(dot(outNormal, outgoing)));
        // Really big PDFs blow up power-heuristic MIS; detect it and don't
        // sample in that case
        if (outPDF > 1.0e10f)
        {
            outPDF = 0.0f;
            return false;
        }
        return true;
        // // Take care which calculations must be done in local space, 
        // // and which should be non-local
        // outPosition = m_position + m_side1 * u1 + m_side2 * u2;
        // outPosition = m_transform.fromLocalPoint(refTime, outPosition);
        // Vector outgoing = surfPosition - outPosition;
        // float dist = outgoing.normalize();
        // outNormal = cross(m_side1, m_side2);
        // // Transform out as a vector to preserve scale (we need it for the area calculation)
        // outNormal = m_transform.fromLocalVector(refTime, outNormal);
        // float area = outNormal.normalize();
        // // Reference point out in back of the light?  That's okay, we'll flip
        // // the normal to have a double-sided light.
        // if (dot(outNormal, outgoing) < 0.0f)
        // {
        //     outNormal *= -1.0f;
        // }
        // outPDF = dist * dist / (area * std::fabs(dot(outNormal, outgoing)));
        // // Really big PDFs blow up power-heuristic MIS; detect it and don't
        // // sample in that case
        // if (outPDF > 1.0e10f)
        // {
        //     outPDF = 0.0f;
        //     return false;
        // }
        // return true;
    }
    
    virtual float intersectPDF(const Intersection& intersection)
    {
        if (intersection.m_pShape == this)
        {
            // Take care which calculations must be done in local space,
            // and which should be non-local
            Vector direction_local = m_transform.fromLocalVector(intersection.m_ray.m_time, m_direction);
            float pdf = intersection.m_t * intersection.m_t /
                        (std::fabs(dot(intersection.m_normal, -intersection.m_ray.m_direction)) *
                         direction_local.length());
            // Really big PDFs blow up power-heuristic MIS; detect it and don't
            // sample in that case
            if (pdf > 1.0e10f)
            {
                return 0.0f;
            }
            return pdf;
        }
        return 0.0f;
    }
    
protected:
    Point m_position;
    Vector m_direction;
};
} // namespace kt
