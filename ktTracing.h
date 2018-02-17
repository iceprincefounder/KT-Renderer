#ifndef __KTTRACING_H__
#define __KTTRACING_H__

#include "ktDataTypes.h"

using namespace KT;

namespace KT
{

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

// Color trace(const Ray& ray, ObjectSet& sceneSet, std::list<Object*>& lights, RNG& rng, size_t lightSamplesHint)
inline Color basic_tracing(const Ray& ray, ObjectSet& sceneSet, std::list<Object*>& lights, RNG& rng)
{
    Color result = Color();
    Intersection hitPoint(ray);
    // Test if this camera ray hit aything
    if (sceneSet.intersect(hitPoint))
    {
        // Add in emission at intersection
        result += hitPoint.m_pShader->emittance();


        // Find out what lights the intersected point can see
        Point position = hitPoint.position();
        for (std::list<Object*>::iterator iter = lights.begin(); iter != lights.end(); ++iter)
        {
            // Ask the light for a random position/normal we can use for lighting
            Point lightPoint;
            Vector lightNormal;
            Light *pLightObject = dynamic_cast<Light*>(*iter);
            pLightObject->sampleSurface(rng.nextFloat(), rng.nextFloat(), position, lightPoint, lightNormal);


            // Fire a shadow ray to make sure we can actually see that light position
            Vector toLight = lightPoint - position;
            float lightDistance = toLight.normalize();
            Ray shadowRay(position, toLight, lightDistance);
            Intersection shadowIntersection(shadowRay);
            bool intersected = sceneSet.intersect(shadowIntersection);


            if (!intersected || shadowIntersection.m_pObject == pLightObject)                        
            {
                // The light point is visible, so let's add that lighting contribution
                Color shading = hitPoint.m_pShader->shade(position, hitPoint.m_normal, ray.m_direction, toLight);

                result += hitPoint.m_color * pLightObject->emitted() * shading;
            }

        } // ending light loop
    }
    return result;
}


} // ending namespace KT

#endif // __KTTRACING_H__