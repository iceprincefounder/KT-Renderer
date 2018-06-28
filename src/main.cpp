#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ktShapes.h"
#include "ktShaders.h"
#include "ktTracing.h"

using namespace KT;


// render samples
const size_t kWidth = 512;
const size_t kHeight = 512;
const size_t kNumPixelSamples = 64;


int main(int argc, char **argv)
{

    const char *outfile = NULL;

    if (argc == 2 )
        outfile = argv[1];
    else
        outfile = "output.ppm";

    // size_t pixelSamplesHint = 4;
    // size_t lightSamplesHint = 4;

    // the 'scene',push all the objects in
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


    // push objects into a set
    // sceneSet.addObject(&frontLight);
    // sceneSet.addObject(&sideLight);
    // sceneSet.addObject(&backtLight);
    // sceneSet.addObject(&topLight); 
    // sceneSet.addObject(&smallAreaLight);
    
    sceneSet.addObject(&plane);
    sceneSet.addObject(&sphere);
    sceneSet.addObject(&areaLight);

    
    // get light list from the scene
    std::list<Object*> lights;
    sceneSet.findLights(lights);
    
    // random number generator (for random pixel positions, light positions, etc)
    RNG rng;
    
    
    // set up the output file (TODO: the filename should probably be a commandline parameter)
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
            
            Color pixelColor(0.0f, 0.0f, 0.0f);

            // for each sample in the pixel...
            for (size_t si = 0; si < kNumPixelSamples; ++si)
            {
                // calculate a random position within the pixel to hide aliasing,I would usr pbrt functions to re implement it
                // PPMs are top-down, and we're bottom up.  Flip pixel row to be in screen space.
                float yu = 1.0f - ((y + rng.nextFloat()) / float(kHeight - 1));
                float xu = (x + rng.nextFloat()) / float(kWidth - 1);
                
                // find out where this pixel sample hits in the scene, create a camera ray
                Ray ray = createCameraRay(45.0f, Point(0.0f, 8.0f, 30.0f), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f), xu, yu);
                pixelColor += basic_tracing(ray, sceneSet, lights, rng);

            }
            // divide by the number of pixel samples (a box filter, essentially)
            pixelColor /= kNumPixelSamples;

            // writing LDR pixel values, so clamp to 0..1 range first
            pixelColor.clamp();
            // get 24-bit pixel sample and write it out
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