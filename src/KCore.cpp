#include <string>
#include <iostream>
#include <sstream>

#include "KCore.h"

// #include <QThread>


using namespace KT;


// A few debug print helpers, in case we need them.

std::ostream& operator <<(std::ostream& stream, const Color& c)
{
    stream << '(' << c.m_r << ", " << c.m_g << ", " << c.m_b << ')';
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const Vector& v)
{
    stream << '[' << v.m_x << ", " << v.m_y << ", " << v.m_z << ']';
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const Quaternion& q)
{
    stream << '[' << q.m_w << "; " << q.m_v.m_x << ", " << q.m_v.m_y << ", " << q.m_v.m_z << ']';
    return stream;
}

std::ostream& operator <<(std::ostream& stream, const Ray& r)
{
    stream << r.m_origin << ":" << r.m_direction << ":[" << kRayTMin << ' ' << r.m_tMax << "]@" << r.m_time;
    return stream;
}



namespace KT
{

Color pathTracer(const Ray& ray,
                ShapeSet& scene,
                std::vector<Shape*>& lights,
                Rng& rng,
                SamplerContainer& samplers,
                unsigned int pixelSampleIndex)
{
    // Accumulate total incoming radiance in 'result'
    Color result = Color(0.0f, 0.0f, 0.0f);
    // As we get through more and more bounces, we track how much the light is
    // diminished through each bounce
    Color throughput = Color(1.0f, 1.0f, 1.0f);
    
    // Start with the initial ray from the camera
    Ray currentRay = ray;
    
    // While we have bounces left we can still take...
    size_t numBounces = 0;
    size_t numDiracBounces = 0;
    bool lastBounceDiracDistribution = false;
    while (numBounces < samplers.m_maxRayDepth)
    {
        // Trace the ray to see if we hit anything
        Intersection intersection(currentRay);
        if (!scene.intersect(intersection))
        {
            // No hit, return black (background)
            break;
        }
        
        // Add in emission when directly visible or via perfect specular bounces
        // (Note that we stop including it through any non-Dirac bounce to
        // prevent caustic noise.)
        if (numBounces == 0 || numBounces == numDiracBounces)
        {
            result += throughput * intersection.m_pMaterial->emittance();
        }
        
        // Evaluate the material and intersection information at this bounce
        Point position = intersection.position();
        Vector normal = intersection.m_normal;
        Vector outgoing = -currentRay.m_direction;
        Brdf* pBrdf = NULL;
        float brdfWeight = 1.0f;
        Color matColor = intersection.m_pMaterial->evaluate(position,
                                                            normal,
                                                            outgoing,
                                                            pBrdf,
                                                            brdfWeight);
        // No BRDF?  We can't evaluate lighting, so bail.
        if (pBrdf == NULL)
        {
            return result;
        }
        
        // Was this a perfect specular bounce?
        lastBounceDiracDistribution = pBrdf->isDiracDistribution();
        if (lastBounceDiracDistribution)
            numDiracBounces++;
        
        // Evaluate direct lighting at this bounce
        
        if (!lastBounceDiracDistribution)
        {
            Color lightResult = Color(0.0f, 0.0f, 0.0f);
            float lightSelectionWeight = float(lights.size()) / samplers.m_numLightSamples;
            for (size_t lightSampleIndex = 0; lightSampleIndex < samplers.m_numLightSamples; ++lightSampleIndex)
            {
                // Sample lights using MIS between the light and the BRDF.
                // This means we ask the light for a direction, and the likelihood
                // of having sampled that direction (the PDF).  Then we ask the
                // BRDF what it thinks of that direction (its PDF), and weight
                // the light sample with MIS.
                //
                // Then, we ask the BRDF for a direction, and the likelihood of
                // having sampled that direction (the PDF).  Then we ask the
                // light what it thinks of that direction (its PDF, and whether
                // that direction even runs into the light at all), and weight
                // the BRDF sample with MIS.
                //
                // By doing both samples and asking both the BRDF and light for
                // their PDF for each one, we can combine the strengths of both
                // sampling methods and get the best of both worlds.  It does
                // cost an extra shadow ray and evaluation, though, but it is
                // generally such an improvement in quality that it is very much
                // worth the overhead.
                
                // Select a light randomly for this sample
                unsigned int finalLightSampleIndex = pixelSampleIndex * samplers.m_numLightSamples +
                                                     lightSampleIndex;
                float liu = samplers.m_lightSelectionSamplers[numBounces]->sample1D(finalLightSampleIndex);
                size_t lightIndex = (size_t)(liu * lights.size());
                if (lightIndex >= lights.size())
                    lightIndex = lights.size() - 1;
                Light *pLightShape = (Light*) lights[lightIndex];
                
                // Ask the light for a random position/normal we can use for lighting
                float lsu, lsv;
                samplers.m_lightSamplers[numBounces]->sample2D(finalLightSampleIndex, lsu, lsv);
                float leu = samplers.m_lightElementSamplers[numBounces]->sample1D(finalLightSampleIndex);
                Point lightPoint;
                Vector lightNormal;
                float lightPdf = 0.0f;
                pLightShape->sampleSurface(position,
                                           normal,
                                           ray.m_time,
                                           lsu, lsv, leu,
                                           lightPoint,
                                           lightNormal,
                                           lightPdf);
                
                if (lightPdf > 0.0f)
                {   
                    // Ask the BRDF what it thinks of this light position (for MIS)
                    Vector lightIncoming = position - lightPoint;
                    float lightDistance = lightIncoming.normalize();
                    float brdfPdf = 0.0f;
                    float brdfResult = pBrdf->evaluateSA(lightIncoming,
                                                         outgoing,
                                                         normal,
                                                         brdfPdf);
                    if (brdfResult > 0.0f && brdfPdf > 0.0f)
                    {
                        // Fire a shadow ray to make sure we can actually see the light position
                        Ray shadowRay(position, -lightIncoming, lightDistance - kRayTMin, ray.m_time);
                        if (!scene.doesIntersect(shadowRay))
                        {
                            // The light point is visible, so let's add that
                            // contribution (mixed by MIS)
                            float misWeightLight = powerHeuristic(1, lightPdf, 1, brdfPdf);
                            lightResult += pLightShape->emitted() *
                                           intersection.m_colorModifier * matColor *
                                           brdfResult *
                                           std::fabs(dot(-lightIncoming, normal)) *
                                           misWeightLight / (lightPdf * brdfWeight);
                        }
                    }
                }
                
                // Ask the BRDF for a sample direction
                float bsu, bsv;
                samplers.m_brdfSamplers[numBounces]->sample2D(finalLightSampleIndex, bsu, bsv);
                Vector brdfIncoming;
                float brdfPdf = 0.0f;
                float brdfResult = pBrdf->sampleSA(brdfIncoming,
                                                   outgoing,
                                                   normal,
                                                   bsu,
                                                   bsv,
                                                   brdfPdf);
                if (brdfPdf > 0.0f && brdfResult > 0.0f)
                {
                    Intersection shadowIntersection(Ray(position, -brdfIncoming, kRayTMax, ray.m_time));
                    bool intersected = scene.intersect(shadowIntersection);
                    if (intersected && shadowIntersection.m_pShape == pLightShape)
                    {
                        // Ask the light what it thinks of this direction (for MIS)
                        lightPdf = pLightShape->intersectPdf(shadowIntersection);
                        if (lightPdf > 0.0f)
                        {
                            // BRDF chose the light, so let's add that
                            // contribution (mixed by MIS)
                            float misWeightBrdf = powerHeuristic(1, brdfPdf, 1, lightPdf);
                            lightResult += pLightShape->emitted() * 
                                           intersection.m_colorModifier * matColor * brdfResult *
                                           std::fabs(dot(-brdfIncoming, normal)) * misWeightBrdf /
                                           (brdfPdf * brdfWeight);
                        }
                    }
                }
            }
            
            // Average light samples
            lightResult *= samplers.m_numLightSamples > 0 ? lightSelectionWeight : 0.0f;
            
            // Add direct lighting at this bounce (modified by how much the
            // previous bounces have dimmed it)
            result += throughput * lightResult;
        }
                
        // Sample the BRDF to find the direction the next leg of the path goes in
        float brdfSampleU, brdfSampleV;
        samplers.m_bounceSamplers[numBounces]->sample2D(pixelSampleIndex, brdfSampleU, brdfSampleV);
        Vector incoming;
        float incomingBrdfPdf = 0.0f;
        float incomingBrdfResult = pBrdf->sampleSA(incoming,
                                                   outgoing,
                                                   normal,
                                                   brdfSampleU,
                                                   brdfSampleV,
                                                   incomingBrdfPdf);

        if (incomingBrdfPdf > 0.0f)
        {
            currentRay.m_origin = position;
            currentRay.m_direction = -incoming;
            currentRay.m_tMax = kRayTMax;
            // Reduce lighting effect for the next bounce based on this bounce's BRDF
            throughput *= intersection.m_colorModifier * matColor * incomingBrdfResult *
                          (std::fabs(dot(-incoming, normal)) /
                          (incomingBrdfPdf * brdfWeight));
        }
        else
        {
            break; // BRDF is zero, stop bouncing
        }
        
        numBounces++;
    }
    
    // This represents an estimate of the total light coming in along the path
    return result;
}

Image* render(ShapeSet& scene,
                const Camera& cam,
                size_t width,
                size_t height,
                unsigned int pixelSamplesHint,
                unsigned int lightSamplesHint,
                unsigned int maxRayDepth)
{
    return NULL;
}
// Image* raytrace(ShapeSet& scene,
//                 const Camera& cam,
//                 size_t width,
//                 size_t height,
//                 unsigned int pixelSamplesHint,
//                 unsigned int lightSamplesHint,
//                 unsigned int maxRayDepth)
// {
//     // Get light list from the scene
//     std::vector<Shape*> lights;
//     scene.findLights(lights);
    
//     scene.prepare();
    
//     // Set up the output image
//     Image *pImage = new Image(width, height);
    
//     // Set up render threads; we make as much as 16 chunks of the image that
//     // can render in parallel.
//     const size_t kChunkDim = 4;
    
//     // Chunk size is the number of pixels per image chunk (we have to take care
//     // to deal with tiny images)
//     size_t xChunkSize = width >= kChunkDim ? width / kChunkDim : 1;
//     size_t yChunkSize = height >= kChunkDim ? height / kChunkDim : 1;
//     // Chunks are the number of chunks in each dimension we can chop the image
//     // into (again, taking care to deal with tiny images, and also images that
//     // don't divide clealy into 4 chunks)
//     size_t xChunks = width > kChunkDim ? width / xChunkSize : 1;
//     size_t yChunks = height > kChunkDim ? height / yChunkSize : 1;
//     if (xChunks * xChunkSize < width) xChunks++;
//     if (yChunks * yChunkSize < height) yChunks++;
    
//     // Set up render threads
//     size_t numRenderThreads = xChunks * yChunks;
//     RenderThread **renderThreads = new RenderThread*[numRenderThreads];
    
//     // Launch render threads
//     for (size_t yc = 0; yc < yChunks; ++yc)
//     {
//         // Get the row start/end (making sure the last chunk doesn't go off the end)
//         size_t yStart = yc * yChunkSize;
//         size_t yEnd = std::min((yc + 1) * yChunkSize, height);
//         for (size_t xc = 0; xc < xChunks; ++xc)
//         {
//             // Get the column start/end (making sure the last chunk doesn't go off the end)
//             size_t xStart = xc * xChunkSize;
//             size_t xEnd = std::min((xc + 1) * xChunkSize, width);
//             // Render the chunk!
//             renderThreads[yc * xChunks + xc] = new RenderThread(xStart,
//                                                                 xEnd,
//                                                                 yStart,
//                                                                 yEnd,
//                                                                 pImage,
//                                                                 scene,
//                                                                 cam,
//                                                                 lights,
//                                                                 pixelSamplesHint,
//                                                                 lightSamplesHint,
//                                                                 maxRayDepth);
//             renderThreads[yc * xChunks + xc]->start();
//         }
//     }
    
//     // Wait until the render finishes
//     bool stillRunning;
//     do
//     {
//         // See if any render thread is still going...
//         stillRunning = false;
//         for (size_t i = 0; i < numRenderThreads; ++i)
//         {
//             if (renderThreads[i]->isRunning())
//             {
//                 stillRunning = true;
//                 break;
//             }
//         }
//         if (stillRunning)
//         {
//             // Give up the CPU so the render threads can do their thing
//             QThread::yieldCurrentThread();
//         }
//     } while (stillRunning);
    
//     // Clean up render thread objects
//     for (size_t i = 0; i < numRenderThreads; ++i)
//     {
//         delete renderThreads[i];
//     }
//     delete[] renderThreads;
    
//     // We made a picture!
//     return pImage;
// }


} // namespace KT
