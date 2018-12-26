#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace kt{

void driverImage(Image *image)
{

}

void ppm_driver(Image *image, const char *outfile)
{
    // set up the output file (TODO: the filename should probably be a commandline parameter)
    size_t kWidth = image->width();
    size_t kHeight = image->height();
    // Color* pixelColor = image->pixel();

    std::ostringstream headerStream;
    headerStream << "P6\n";
    headerStream << kWidth << ' ' << kHeight << '\n';
    headerStream << "255\n";
    std::ofstream fileStream(outfile, std::ios::out | std::ios::binary);
    fileStream << headerStream.str();

    // for each row...
    for (size_t y = 0; y < kHeight; ++y)
    {
        // for each pixel across the row...
        for (size_t x = 0; x < kWidth; ++x)
        {
            unsigned char r, g, b;
            Color pixelColor = image->pixel(x,y);
            r = static_cast<unsigned char>(pixelColor.r * 255.0f);
            g = static_cast<unsigned char>(pixelColor.g * 255.0f);
            b = static_cast<unsigned char>(pixelColor.b * 255.0f);
            fileStream << r << g << b;
        }
    }

    fileStream.flush();
    fileStream.close();
}

void png_driver(Image *image, char *outfile)
{
    // To Do...
}


} // ending namespace kt
