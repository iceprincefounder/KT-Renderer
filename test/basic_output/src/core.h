
#ifndef __RAYITO_H__
#define __RAYITO_H__


#include <cmath>
#include <list>
#include <algorithm>

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


Color operator +(const Color& c1, const Color& c2)
{
    return Color(c1.m_r + c2.m_r,
                 c1.m_g + c2.m_g,
                 c1.m_b + c2.m_b);
}


Color operator -(const Color& c1, const Color& c2)
{
    return Color(c1.m_r - c2.m_r,
                 c1.m_g - c2.m_g,
                 c1.m_b - c2.m_b);
}


Color operator *(const Color& c1, const Color& c2)
{
    return Color(c1.m_r * c2.m_r,
                 c1.m_g * c2.m_g,
                 c1.m_b * c2.m_b);
}


Color operator /(const Color& c1, const Color& c2)
{
    return Color(c1.m_r / c2.m_r,
                 c1.m_g / c2.m_g,
                 c1.m_b / c2.m_b);
}


Color operator *(const Color& c, float f)
{
    return Color(f * c.m_r,
                 f * c.m_g,
                 f * c.m_b);
}


Color operator *(float f, const Color& c)
{
    return Color(f * c.m_r,
                 f * c.m_g,
                 f * c.m_b);
}


Color operator /(const Color& c, float f)
{
    return Color(c.m_r / f,
                 c.m_g / f,
                 c.m_b / f);
}

#endif // __RAYITO_H__