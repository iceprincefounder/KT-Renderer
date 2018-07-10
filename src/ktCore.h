#ifndef __KTCORE_H__
#define __KTCORE_H__

#include <cmath>
#include <list>
#include <algorithm>

namespace KT
{

//
// Declare data type Color
//

struct Color
{
    float m_r, m_g, m_b;
    
    Color()                          : m_r(0.0f), m_g(0.0f), m_b(0.0f)    { }
    Color(const Color& c)            : m_r(c.m_r), m_g(c.m_g), m_b(c.m_b) { }
    Color(float r, float g, float b) : m_r(r), m_g(g), m_b(b)             { }
    explicit Color(float f)          : m_r(f), m_g(f), m_b(f)             { }
    
    
    void clamp(float min = 0.0f, float max = 1.0f)
    {
        m_r = std::max(min, std::min(max, m_r));
        m_g = std::max(min, std::min(max, m_g));
        m_b = std::max(min, std::min(max, m_b));
    }
    
    
    Color& operator =(const Color& c)
    {
        m_r = c.m_r;
        m_g = c.m_g;
        m_b = c.m_b;
        return *this;
    }
    
    Color& operator +=(const Color& c)
    {
        m_r += c.m_r;
        m_g += c.m_g;
        m_b += c.m_b;
        return *this;
    }
    
    Color& operator -=(const Color& c)
    {
        m_r -= c.m_r;
        m_g -= c.m_g;
        m_b -= c.m_b;
        return *this;
    }
    
    Color& operator *=(const Color& c)
    {
        m_r *= c.m_r;
        m_g *= c.m_g;
        m_b *= c.m_b;
        return *this;
    }
    
    Color& operator /=(const Color& c)
    {
        m_r /= c.m_r;
        m_g /= c.m_g;
        m_b /= c.m_b;
        return *this;
    }
    
    Color& operator *=(float f)
    {
        m_r *= f;
        m_g *= f;
        m_b *= f;
        return *this;
    }
    
    Color& operator /=(float f)
    {
        m_r /= f;
        m_g /= f;
        m_b /= f;
        return *this;
    }
};


inline Color operator +(const Color& c1, const Color& c2)
{
    return Color(c1.m_r + c2.m_r,
                 c1.m_g + c2.m_g,
                 c1.m_b + c2.m_b);
}


inline Color operator -(const Color& c1, const Color& c2)
{
    return Color(c1.m_r - c2.m_r,
                 c1.m_g - c2.m_g,
                 c1.m_b - c2.m_b);
}


inline Color operator *(const Color& c1, const Color& c2)
{
    return Color(c1.m_r * c2.m_r,
                 c1.m_g * c2.m_g,
                 c1.m_b * c2.m_b);
}


inline Color operator /(const Color& c1, const Color& c2)
{
    return Color(c1.m_r / c2.m_r,
                 c1.m_g / c2.m_g,
                 c1.m_b / c2.m_b);
}


inline Color operator *(const Color& c, float f)
{
    return Color(f * c.m_r,
                 f * c.m_g,
                 f * c.m_b);
}


inline Color operator *(float f, const Color& c)
{
    return Color(f * c.m_r,
                 f * c.m_g,
                 f * c.m_b);
}


inline Color operator /(const Color& c, float f)
{
    return Color(c.m_r / f,
                 c.m_g / f,
                 c.m_b / f);
}


//
// Declare data type Vector
//

struct Vector
{
    float m_x, m_y, m_z;
    
    Vector()                          : m_x(0.0f), m_y(0.0f), m_z(0.0f)    { }
    Vector(const Vector& v)           : m_x(v.m_x), m_y(v.m_y), m_z(v.m_z) { }
    Vector(float x, float y, float z) : m_x(x), m_y(y), m_z(z)             { }
    explicit Vector(float f)          : m_x(f), m_y(f), m_z(f)             { }
    
    
    float length2() const { return m_x * m_x + m_y * m_y + m_z * m_z; }
    float length()  const { return std::sqrt(length2()); }
    
    float  normalize()        { float len = length(); *this /= len; return len; }
    Vector normalized() const { Vector r(*this); r.normalize(); return r; }
    
    
    Vector& operator =(const Vector& v)
    {
        m_x = v.m_x;
        m_y = v.m_y;
        m_z = v.m_z;
        return *this;
    }
    
    Vector& operator +=(const Vector& v)
    {
        m_x += v.m_x;
        m_y += v.m_y;
        m_z += v.m_z;
        return *this;
    }
    
    Vector& operator -=(const Vector& v)
    {
        m_x -= v.m_x;
        m_y -= v.m_y;
        m_z -= v.m_z;
        return *this;
    }
    
    Vector& operator *=(float f)
    {
        m_x *= f;
        m_y *= f;
        m_z *= f;
        return *this;
    }
    
    Vector& operator /=(float f)
    {
        m_x /= f;
        m_y /= f;
        m_z /= f;
        return *this;
    }
};


inline Vector operator +(const Vector& v1, const Vector& v2)
{
    return Vector(v1.m_x + v2.m_x,
                  v1.m_y + v2.m_y,
                  v1.m_z + v2.m_z);
}


inline Vector operator -(const Vector& v1, const Vector& v2)
{
    return Vector(v1.m_x - v2.m_x,
                  v1.m_y - v2.m_y,
                  v1.m_z - v2.m_z);
}


inline Vector operator *(const Vector& v, float f)
{
    return Vector(f * v.m_x,
                  f * v.m_y,
                  f * v.m_z);
}


inline Vector operator *(float f, const Vector& v)
{
    return Vector(f * v.m_x,
                  f * v.m_y,
                  f * v.m_z);
}


inline float dot(const Vector& v1, const Vector& v2)
{
    return v1.m_x * v2.m_x + v1.m_y * v2.m_y + v1.m_z * v2.m_z;
}


inline Vector cross(const Vector& v1, const Vector& v2)
{
    return Vector(v1.m_y * v2.m_z - v1.m_z * v2.m_y,
                  v1.m_z * v2.m_x - v1.m_x * v2.m_z,
                  v1.m_x * v2.m_y - v1.m_y * v2.m_x);
}




//
// Declare data type Point
//

typedef Vector Point;


//
// Declare data type Ray
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

#ifndef KT_PI
    #define KT_PI 3.14159265358979
#endif

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
    Color m_emitted;
    Vector m_normal;
    
    
    Intersection()
        : m_ray(),
          m_t(kRayTMax),
          m_pObject(NULL),
          m_pShader(NULL),
          m_color(),
          m_emitted(),
          m_normal()
    {
        
    }
    
    Intersection(const Intersection& i)
        : m_ray(i.m_ray),
          m_t(i.m_t),
          m_pObject(i.m_pObject),
          m_pShader(i.m_pShader),
          m_color(i.m_color),
          m_emitted(i.m_emitted),
          m_normal(i.m_normal)
    {
        
    }
    
    Intersection(const Ray& ray)
         : m_ray(ray),
           m_t(ray.m_tMax),
           m_pObject(NULL),
           m_pShader(NULL),
           m_color(),
           m_emitted(),
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
        m_emitted = i.m_emitted;
        m_normal = i.m_normal;
        return *this;
    }
    
    bool intersected() const { return (m_pObject == NULL) ? false : true; }
    
    Point position() const { return m_ray.calculate(m_t); }
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

} // namespace KT

#endif // __KTCORE_H__