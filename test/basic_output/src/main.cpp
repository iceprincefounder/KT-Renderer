#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "core.h"

std::ostream& operator <<(std::ostream& stream, const Color& c)
{
    stream << '(' << c.m_r << ", " << c.m_g << ", " << c.m_b << ')';
    return stream;
}

const size_t kWidth = 720;
const size_t kHeight = 512;

int main(int argc, char **argv)
{

	std::ostringstream headerStream;
    headerStream << "P6\n";
    headerStream << kWidth << ' ' << kHeight << '\n';
    headerStream << "255\n";
    std::ofstream fileStream("out.ppm", std::ios::out | std::ios::binary);
    fileStream << headerStream.str();

    // For each row...
    for (size_t y = 0; y < kHeight; ++y)
    {
      
        // For each pixel across the row...
        for (size_t x = 0; x < kWidth; ++x)
        {
            
            Color pixelColor(1.0f, 0.0f, 0.0f);

            // We're writing LDR pixel values, so clamp to 0..1 range first
            pixelColor.clamp();
            // Get 24-bit pixel sample and write it out
            unsigned char r, g, b;
            r = static_cast<unsigned char>(pixelColor.m_r * 255.0f);
            g = static_cast<unsigned char>(pixelColor.m_g * 255.0f);
            b = static_cast<unsigned char>(pixelColor.m_b * 255.0f);
            fileStream << r << g << b;
        }
    }
    return 0;
}