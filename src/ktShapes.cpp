#include "ktShapes.h"

namespace KT
{

bool InfinitePlane::intersect(Intersection& intersection)
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
    intersection.m_normal = m_normal;
    intersection.m_pObject = this;
    intersection.m_pShader = m_shader;
    intersection.m_color = m_color;
    intersection.m_emitted = Color();
    return true;
}

bool Sphere::intersect(Intersection& intersection)
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

bool Sphere::sampleSurface(float u1, float u2, const Point& referencePosition, Point& outPosition, Vector& outNormal)
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

Vector Sphere::uniformToSphere(float u1, float u2)
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

} //ending namespace KT