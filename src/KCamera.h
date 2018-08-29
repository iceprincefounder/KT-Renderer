#pragma once

#include "KMath.h"
#include "KRay.h"
#include "KMaterial.h"
#include "KScene.h"
#include "KLight.h"


namespace KT
{

//
// Cameras
//

class Camera
{
public:
    Camera(float shutterOpen = 0.0f, float shutterClose = 0.0f)
        : m_shutterOpen(shutterOpen), m_shutterClose(shutterClose) { }
    
    virtual ~Camera() { }
    
    // Generate a ray origin+direction for the camera, possibly with depth-of-field
    virtual Ray makeRay(float xScreen, float yScreen, float lensU, float lensV, float timeU) const = 0;
    
protected:
    float m_shutterOpen;
    float m_shutterClose;
    
    float time(float timeU) const { return m_shutterOpen + (m_shutterClose - m_shutterOpen) * timeU; }
};


class PerspectiveCamera : public Camera
{
public:
    // Create a perspective camera, with a given field-of-view in degrees,
    // look-at parameters, and depth-of-field parameters (lensRadius=0 to disable DOF)
    PerspectiveCamera(float fieldOfViewInDegrees,
                      const Point& origin,
                      const Vector& target,
                      const Vector& targetUpDirection,
                      float focalDistance,
                      float lensRadius,
                      float shutterOpen,
                      float shutterClose);
    
    virtual ~PerspectiveCamera() { }
    
    virtual Ray makeRay(float xScreen, float yScreen, float lensU, float lensV, float timeU) const;
    
protected:
    Point m_origin;
    Vector m_forward;
    Vector m_right;
    Vector m_up;
    float m_tanFov;
    float m_focalDistance;
    float m_lensRadius;
};

} // namespace KT