#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include "KCore.h"


using namespace KT;


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
                const Camera& camera,
                size_t width,
                size_t height,
                unsigned int pixelSamplesHint,
                unsigned int lightSamplesHint,
                unsigned int maxRayDepth)
{
    // Get light list from the scene
    std::vector<Shape*> lights;
    scene.findLights(lights);

    scene.prepare();
    
    // Set up the output image
    Image *pImage = new Image(width, height);


    // Random number generator (for random pixel positions, light positions, etc)
    // We seed the generator for this render thread based on something that
    // doesn't change, but gives us a good variable seed for each thread.
    Rng rng(static_cast<unsigned int>(((0 << 16) | width) ^ width),
            static_cast<unsigned int>(((0 << 16) | height) ^ height));
    
    // The aspect ratio is used to make the image only get more zoomed in when
    // the height changes (and not the width)
    float aspectRatioXToY = float(pImage->width()) / float(pImage->height());
    
    SamplerContainer samplers;
    samplers.m_numLightSamples = lights.empty() ? 0 : lightSamplesHint * lightSamplesHint;
    samplers.m_maxRayDepth = maxRayDepth;
    
    // Set up samplers for each of the ray bounces.  Each bounce will use
    // the same sampler for all pixel samples in the pixel to reduce noise.
    for (size_t i = 0; i < maxRayDepth; ++i)
    {
        samplers.m_bounceSamplers.push_back(new CorrelatedMultiJitterSampler(pixelSamplesHint,
                                                                             pixelSamplesHint,
                                                                             rng,
                                                                             rng.nextUInt32()));
        samplers.m_lightSelectionSamplers.push_back(new CorrelatedMultiJitterSampler(pixelSamplesHint * pixelSamplesHint *
                                                                                     pixelSamplesHint * pixelSamplesHint,
                                                                                     rng,
                                                                                     rng.nextUInt32()));
        samplers.m_lightElementSamplers.push_back(new CorrelatedMultiJitterSampler(pixelSamplesHint * pixelSamplesHint *
                                                                                   pixelSamplesHint * pixelSamplesHint,
                                                                                   rng,
                                                                                   rng.nextUInt32()));
        samplers.m_lightSamplers.push_back(new CorrelatedMultiJitterSampler(pixelSamplesHint * pixelSamplesHint,
                                                                            pixelSamplesHint * pixelSamplesHint,
                                                                            rng,
                                                                            rng.nextUInt32()));
        samplers.m_brdfSamplers.push_back(new CorrelatedMultiJitterSampler(pixelSamplesHint * pixelSamplesHint,
                                                                           pixelSamplesHint * pixelSamplesHint,
                                                                           rng,
                                                                           rng.nextUInt32()));
    }
    // Set up samplers for each pixel sample
    samplers.m_timeSampler = new CorrelatedMultiJitterSampler(pixelSamplesHint * pixelSamplesHint, rng, rng.nextUInt32());
    samplers.m_lensSampler = new CorrelatedMultiJitterSampler(pixelSamplesHint, pixelSamplesHint, rng, rng.nextUInt32());
    samplers.m_subpixelSampler = new CorrelatedMultiJitterSampler(pixelSamplesHint, pixelSamplesHint, rng, rng.nextUInt32());
    unsigned int totalPixelSamples = samplers.m_subpixelSampler->total2DSamplesAvailable();

    // For each pixel row...
    for (size_t y = 0; y < height; ++y)
    {
        // For each pixel across the row...
        for (size_t x = 0; x < width; ++x)
        {
            // Accumulate pixel color
            Color pixelColor(0.0f, 0.0f, 0.0f);
            // For each sample in the pixel...
            for (size_t psi = 0; psi < totalPixelSamples; ++psi)
            {
                // Calculate a stratified random position within the pixel
                // to hide aliasing
                float pu, pv;
                samplers.m_subpixelSampler->sample2D(psi, pu, pv);
                float xu = (x + pu) / float(pImage->width());
                // Flip pixel row to be in screen space (images are top-down)
                float yu = 1.0f - (y + pv) / float(pImage->height());
                
                // Calculate a stratified random variation for depth-of-field
                float lensU, lensV;
                samplers.m_lensSampler->sample2D(psi, lensU, lensV);
                
                // Grab a time for motion blur
                float timeU = samplers.m_timeSampler->sample1D(psi);
                
                // Find where this pixel sample hits in the scene
                Ray ray = camera.makeRay((xu - 0.5f) * aspectRatioXToY + 0.5f,
                                          yu,
                                          lensU,
                                          lensV,
                                          timeU);
                
                // Trace a path out, gathering estimated radiance along the path
                pixelColor += pathTracer(ray,
                                        scene,
                                        lights,
                                        rng,
                                        samplers,
                                        psi);
            }
            // Divide by the number of pixel samples (a box pixel filter, essentially)
            pixelColor /= totalPixelSamples;
            // printf("--result (%i,%i)-(%f,%f,%f)\n", 
                // (int)x,(int)y,(float)pixelColor.m_r,(float)pixelColor.m_g,(float)pixelColor.m_b);
            // Store off the computed pixel in a big buffer
            pImage->pixel(x, y) = pixelColor;
            
            // Reset samplers for the next pixel sample
            for (size_t i = 0; i < maxRayDepth; ++i)
            {
                samplers.m_bounceSamplers[i]->refill(rng.nextUInt32());
                samplers.m_lightSelectionSamplers[i]->refill(rng.nextUInt32());
                samplers.m_lightElementSamplers[i]->refill(rng.nextUInt32());
                samplers.m_lightSamplers[i]->refill(rng.nextUInt32());
                samplers.m_brdfSamplers[i]->refill(rng.nextUInt32());
            }
            samplers.m_lensSampler->refill(rng.nextUInt32());
            samplers.m_timeSampler->refill(rng.nextUInt32());
            samplers.m_subpixelSampler->refill(rng.nextUInt32());
        }
    }
    
    // Deallocate all samplers
    for (size_t i = 0; i < maxRayDepth; ++i)
    {
        delete samplers.m_bounceSamplers[i];
        delete samplers.m_lightSelectionSamplers[i];
        delete samplers.m_lightElementSamplers[i];
        delete samplers.m_lightSamplers[i];
        delete samplers.m_brdfSamplers[i];
    }
    delete samplers.m_lensSampler;
    delete samplers.m_timeSampler;
    delete samplers.m_subpixelSampler;

    return pImage;
}

} // namespace KT
