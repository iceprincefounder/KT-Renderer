#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ktCore.h"
#include "ktTracing.h"

using namespace KT;


// render samples
const size_t kWidth = 512;
const size_t kHeight = 512;
const size_t kNumPixelSamples = 64;


int main(int argc, char **argv)
{
    // size_t pixelSamplesHint = 4;
    // size_t lightSamplesHint = 4;

    // The 'scene',push all the objects in
    ObjectSet sceneSet;
    Lambert defaultLambert(Color(0.7f, 0.7f, 0.7f));
    
    // genarate the whole scene
    InfinitePlane plane(Point(0.0f, 0.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Color(0.7f, 0.7f, 0.7f), &defaultLambert);
    PointLight frontLight(Point(2.0f, 2.0f, 2.0f), Color(1.0f, 1.0f, 1.0f), 1.0f);
    PointLight sideLight(Point(-2.0f, 2.0f, 0.0f), Color(1.0f, 1.0f, 1.0f), 0.5f);
    PointLight backtLight(Point(0.0f, 2.0f, -4.0f), Color(1.0f, 1.0f, 1.0f), 0.5f);
    PointLight topLight(Point(0.0f, 4.0f, 0.0f), Color(1.0f, 1.0f, 1.0f), 1.0f);
    RectangleLight areaLight(Point(-2.5f, 4.0f, -2.5f), Vector(5.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 5.0f), Color(1.0f, 1.0f, 1.0f), 2.0f);
    RectangleLight smallAreaLight(Point(-2.0f, 2.0f, -2.0f), Vector(4.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 4.0f), Color(1.0f, 1.0f, 0.5f), 0.75f);
    Sphere sphere(Point(0.0f, 1.5f, 0.0f), 1.5f, &defaultLambert);


    sceneSet.addObject(&plane);
    // sceneSet.addObject(&frontLight);
    // sceneSet.addObject(&sideLight);
    // sceneSet.addObject(&backtLight);

    sceneSet.addObject(&sphere);
    sceneSet.addObject(&areaLight);
    // sceneSet.addObject(&topLight); 
    // sceneSet.addObject(&smallAreaLight);

    
    // Get light list from the scene
    std::list<Object*> lights;
    sceneSet.findLights(lights);
    
    // Random number generator (for random pixel positions, light positions, etc)
    RNG rng;
    
    
    // Set up the output file (TODO: the filename should probably be a commandline parameter)
    std::ostringstream headerStream;
    headerStream << "P6\n";
    headerStream << kWidth << ' ' << kHeight << '\n';
    headerStream << "255\n";
    std::ofstream fileStream("output.ppm", std::ios::out | std::ios::binary);
    fileStream << headerStream.str();


    // For each row...
    for (size_t y = 0; y < kHeight; ++y)
    {
        // float yu = 1.0f - (float(y) / float(kHeight - 1));
        
        // For each pixel across the row...
        for (size_t x = 0; x < kWidth; ++x)
        {
            // float xu = float(x) / float(kWidth - 1);
            
            // For each sample in the pixel...
            Color pixelColor(0.0f, 0.0f, 0.0f);

            for (size_t si = 0; si < kNumPixelSamples; ++si)
            {
                // Calculate a random position within the pixel to hide aliasing,I would usr pbrt functions to re-implanmtat it
                // PPMs are top-down, and we're bottom up.  Flip pixel row to be in screen space.
                float yu = 1.0f - ((y + rng.nextFloat()) / float(kHeight - 1));
                float xu = (x + rng.nextFloat()) / float(kWidth - 1);
                
                // Find where this pixel sample hits in the scene, create a camera ray
                Ray ray = createCameraRay(45.0f, Point(0.0f, 8.0f, 30.0f), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f), xu, yu);
                pixelColor += trace(ray, sceneSet, lights, rng);

            }
            // Divide by the number of pixel samples (a box filter, essentially)
            pixelColor /= kNumPixelSamples;

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
    
    fileStream.flush();
    fileStream.close();
    
    return 0;
}