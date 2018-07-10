#ifndef __SAMPLERS_H__
#define __SAMPLERS_H__

#include "ktCore.h"

namespace KT
{

// genarate multiply-with-carry psuedo random number
// see http://groups.google.com/group/sci.crypt/browse_thread/thread/ca8682a4658a124d
struct RNG
{
    unsigned int m_z, m_w;
    
    RNG(unsigned int z = 362436069, unsigned int w = 521288629) : m_z(z), m_w(w) { }
    
    
    // Returns a 'canonical' float from [0,1)
    float nextFloat()
    {
        unsigned int i = nextUInt32();
        return i * 2.328306e-10f;
    }
 
    // Returns an int with random bits set
    unsigned int nextUInt32()
    {
        m_z = 36969 * (m_z & 65535) + (m_z >> 16);
        m_w = 18000 * (m_w & 65535) + (m_w >> 16);
        return (m_z << 16) + m_w;  /* 32-bit result */
    }
};


} //ending namespace KT


#endif // __SAMPLERS_H__