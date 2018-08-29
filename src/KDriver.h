#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "KCore.h"

namespace KT{

void driverImage(Image *image)
{

}

void ppm_driver(Image *image)
{
    // set up the output file (TODO: the filename should probably be a commandline parameter)
    std::ostringstream headerStream;
    headerStream << "P6\n";
    headerStream << kWidth << ' ' << kHeight << '\n';
    headerStream << "255\n";
    std::ofstream fileStream(outfile, std::ios::out | std::ios::binary);
    fileStream << headerStream.str();

    // for each row...
    for (size_t y = 0; y < image.height(); ++y)
    {
        
        // for each pixel across the row...
        for (size_t x = 0; x < image.width(); ++x)
        {
            unsigned char r, g, b;
            r = static_cast<unsigned char>(pixelColor.m_r * 255.0f);
            g = static_cast<unsigned char>(pixelColor.m_g * 255.0f);
            b = static_cast<unsigned char>(pixelColor.m_b * 255.0f);
            fileStream << r << g << b;

        }
    }

    fileStream.flush();
    fileStream.close();
}

void png_driver(Image *image)
{

}


} // ending namespace KT
