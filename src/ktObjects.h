#ifndef __KTOBJECTS_H__
#define __KTOBJECTS_H__

// I think that the lights and the shapes should be the same thing in renderer,
// they both fire rays and get advanced, I just create it as the same class.

#include "ktCore.h"

namespace KAI{

class Object
{
public:
    virtual ~Object() { }
    virtual bool intersect(Intersection& intersection) = 0;
    virtual bool isIntersected(const Ray& ray) = 0;
    virtual bool sampleSurface(const Point& refPosition,
                               const Vector& refNormal,
                               float u1,
                               float u2,
                               Point& outPosition,
                               Vector& outNormal,
                               float& outPdf);
    virtual float pdfSA(const Point& refPosition,
                        const Vector& refNormal,
                        const Point& surfPosition,
                        const Vector& surfNormal);
    virtual float surfaceAreaPdf() const;    
    // Find all lights in the scene starting with this shape
    virtual void findLights(std::list<Shape*>& outLightList);
    // Is this shape itself actually a light?
    virtual bool isLight();
};

class InfinitePlane : public Object
{

};

class Sphere : public Object
{

};

class Light : public Object
{

};

class RectangleLight : public Light
{

};

class PolyMesh : public Object;

} //ending namespace KAI
#endif // __KTOBJECTS_H__