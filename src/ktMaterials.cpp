#include "ktMaterials.h"

namespace KT
{

    
Color Lambert::shade(const Point& position,
                    const Vector& normal,
                    const Vector& incomingRayDirection,
                    const Vector& lightDirectionNorm)
{
    return std::max(0.0f, dot(normal, lightDirectionNorm)) * m_color;
}
    
Color Phong::shade(const Point& position,
                const Vector& normal,
                const Vector& incomingRayDirection,
                const Vector& lightDirectionNorm)
{
    Vector halfVec = (lightDirectionNorm - incomingRayDirection).normalized();
    return std::pow(std::max(0.0f, dot(halfVec, normal)), m_exponent) * m_color;
}
    

    
Color Emitter::emittance() 
{ 
    return m_color * m_power; 
}

Color Emitter::shade(const Point& position,
                    const Vector& normal,
                    const Vector& incomingRayDirection,
                    const Vector& lightDirectionNorm)
{
    // Let the emittance take care of return color
    return Color();
}

} //ending namespace KT