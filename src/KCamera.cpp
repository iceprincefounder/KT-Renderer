#include <string>
#include <iostream>
#include <sstream>

#include "KRayTracer.h"

// #include <QThread>


using namespace kt;

namespace kt{


// Construct a perspective camera, precomputing a few things to ray trace faster
PerspectiveCamera::PerspectiveCamera(float fieldOfViewInDegrees,
                                     const Point& origin,
                                     const Vector& target,
                                     const Vector& targetUpDirection,
                                     float focalDistance,
                                     float lensRadius,
                                     float shutterOpen,
                                     float shutterClose): 
                                    Camera(shutterOpen, shutterClose),
                                    m_origin(origin),
                                    m_forward((target - origin).normalized()),
                                    m_tanFov(std::tan(fieldOfViewInDegrees * M_PI / 180.0f)),
                                    m_focalDistance(focalDistance),
                                    m_lensRadius(lensRadius)
{
    m_right = cross(m_forward, targetUpDirection);
    m_up = cross(m_right, m_forward); // no need to normalize, it already is
}

Ray PerspectiveCamera::makeRay(float xScreen, float yScreen, float lensU, float lensV, float timeU) const
{
    // Set up a camera ray given the look-at spec, FOV, and screen position to aim at
    
    // Set up ray info
    Ray ray;
    ray.m_origin = m_origin;
    ray.m_direction = m_forward +
                      m_right * ((xScreen - 0.5f) * m_tanFov) +
                      m_up * ((yScreen - 0.5f) * m_tanFov);
    ray.m_direction.normalize();
    ray.m_time = time(timeU);
    
    if (m_lensRadius > 0)
    {
        // Modify it for DOF if necessary
        float horizontalShift = 0.0f, verticalShift = 0.0f;
        float nearDistance = 0.0f; // TODO: be able to set a custom near distance?

        // Compute shifts on the near plane due to DOF
        uniformToUniformDisk(lensU, lensV, horizontalShift, verticalShift);
        horizontalShift *= m_lensRadius;
        verticalShift *= m_lensRadius;

        // Compute local direction rays for computing focal parameters
        Vector localRayDirection((xScreen - 0.5f) * m_tanFov,
                                 (yScreen - 0.5f) * m_tanFov,
                                 1.0f);
        localRayDirection.normalize();

        // Compute primary ray focal plane intersection
        float focusT = (m_focalDistance - nearDistance) / localRayDirection.z;
        Point focusPoint = m_origin + ray.m_direction * focusT;

        // Modify primary ray origin
        ray.m_origin += m_right * horizontalShift + m_up * verticalShift;

        // Compute primary ray direction
        ray.m_direction = focusPoint - ray.m_origin;
        ray.m_direction.normalize();
    }
    
    return ray;
}

} // namespace kt
