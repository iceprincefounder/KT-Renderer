#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "core.h"
#include "pbrt.h"

using namespace KT;



const size_t kWidth = 512;
const size_t kHeight = 512;
const size_t kNumPixelSamples = 64;


int main(int argc, char **argv)
{
    // The 'scene'
    ObjectSet masterSet;
    
    // Put a ground plane in (with bullseye texture!)
    Plane plane(Point(0.0f, -2.0f, 0.0f), Vector(0.0f, 1.0f, 0.0f), Color(1.0f, 1.0f, 1.0f), false);
    masterSet.addObject(&plane);
    
    // Add an area light
    RectangleLight areaLight(Point(-2.5f, 2.0f, -2.5f), Vector(5.0f, 0.0f, 0.0f), Vector(0.0f, 0.0f, 5.0f), Color(1.0f, 1.0f, 1.0f), 1.0f);
    
    masterSet.addObject(&areaLight);
    
    // Add another area light below it, darker, that will make a shadow too.
    RectangleLight smallAreaLight(Point(-2.0f, -1.0f, -2.0f),
                                  Vector(4.0f, 0.0f, 0.0f),
                                  Vector(0.0f, 0.0f, 4.0f),
                                  Color(1.0f, 1.0f, 0.5f),
                                  0.75f);
    masterSet.addObject(&smallAreaLight);
    
    // Get light list from the scene
    std::list<Object*> lights;
    masterSet.findLights(lights);
    
    // Random number generator (for random pixel positions, light positions, etc)
    RNG rng;
    
    
    // Set up the output file (TODO: the filename should probably be a commandline parameter)
    std::ostringstream headerStream;
    headerStream << "P6\n";
    headerStream << kWidth << ' ' << kHeight << '\n';
    headerStream << "255\n";
    std::ofstream fileStream("out.ppm", std::ios::out | std::ios::binary);
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
                // Calculate a random position within the pixel to hide aliasing.
                // PPMs are top-down, and we're bottom up.  Flip pixel row to be in screen space.
                float yu = 1.0f - ((y + rng.nextFloat()) / float(kHeight - 1));
                float xu = (x + rng.nextFloat()) / float(kWidth - 1);
                
                // Find where this pixel sample hits in the scene, create a camera ray
                Ray ray = createCameraRay(45.0f, Point(0.0f, 5.0f, 30.0f), Point(0.0f, 0.0f, 0.0f), Point(0.0f, 1.0f, 0.0f), xu, yu);

                Intersection intersection(ray);
                // Test if this camera ray hit aything
                if (masterSet.intersect(intersection))
                {
                    // Add in emission at intersection
                    pixelColor += intersection.m_emitted;
                    
                    // Find out what lights the intersected point can see
                    Point position = intersection.position();
                    for (std::list<Object*>::iterator iter = lights.begin(); iter != lights.end(); ++iter)
                    {
                        // Ask the light for a random position/normal we can use
                        // for lighting
                        Point lightPoint;
                        Vector lightNormal;
                        Light *pLightObject = dynamic_cast<Light*>(*iter);
                        pLightObject->sampleSurface(rng.nextFloat(), rng.nextFloat(), position, lightPoint, lightNormal);
                        

                        // Fire a shadow ray to make sure we can actually see that light position
                        Vector toLight = lightPoint - position;
                        float lightDistance = toLight.normalize();
                        Ray shadowRay(position, toLight, lightDistance);
                        Intersection shadowIntersection(shadowRay);
                        bool intersected = masterSet.intersect(shadowIntersection);
                        
                        if (!intersected || shadowIntersection.m_pObject == pLightObject)
                        {
                            // The light point is visible, so let's add that
                            // lighting contribution
                            float lightAttenuation = std::max(0.0f, dot(intersection.m_normal, toLight));
                            pixelColor += intersection.m_color * pLightObject->emitted() * lightAttenuation;
                        }
                    }
                }
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
    
    // Tidy up (probably unnecessary)
    fileStream.flush();
    fileStream.close();
    
    return 0;
}