#pragma once

namespace KT{

//
// RenderThread works on a small chunk of the image
//
class RenderThread : public QThread
{
public:
    RenderThread(size_t xstart, size_t xend, size_t ystart, size_t yend,
                 Image *pImage,
                 ShapeSet& masterSet,
                 const Camera& cam,
                 std::vector<Shape*>& lights,
                 unsigned int pixelSamplesHint,
                 unsigned int lightSamplesHint,
                 unsigned int maxRayDepth)
        : m_xstart(xstart), m_xend(xend), m_ystart(ystart), m_yend(yend),
          m_pImage(pImage), m_masterSet(masterSet), m_camera(cam), m_lights(lights),
          m_pixelSamplesHint(pixelSamplesHint), m_lightSamplesHint(lightSamplesHint),
          m_maxRayDepth(maxRayDepth) { }
    
protected:
    virtual void run()
    {
        // Random number generator (for random pixel positions, light positions, etc)
        // We seed the generator for this render thread based on something that
        // doesn't change, but gives us a good variable seed for each thread.
        Rng rng(static_cast<unsigned int>(((m_xstart << 16) | m_xend) ^ m_xstart),
                static_cast<unsigned int>(((m_ystart << 16) | m_yend) ^ m_ystart));
        
        // The aspect ratio is used to make the image only get more zoomed in when
        // the height changes (and not the width)
        float aspectRatioXToY = float(m_pImage->width()) / float(m_pImage->height());
        
        SamplerContainer samplers;
        samplers.m_numLightSamples = m_lights.empty() ? 0 : m_lightSamplesHint * m_lightSamplesHint;
        samplers.m_maxRayDepth = m_maxRayDepth;
        
        // Set up samplers for each of the ray bounces.  Each bounce will use
        // the same sampler for all pixel samples in the pixel to reduce noise.
        for (size_t i = 0; i < m_maxRayDepth; ++i)
        {
            samplers.m_bounceSamplers.push_back(new CorrelatedMultiJitterSampler(m_pixelSamplesHint,
                                                                                 m_pixelSamplesHint,
                                                                                 rng,
                                                                                 rng.nextUInt32()));
            samplers.m_lightSelectionSamplers.push_back(new CorrelatedMultiJitterSampler(m_pixelSamplesHint * m_lightSamplesHint *
                                                                                         m_pixelSamplesHint * m_lightSamplesHint,
                                                                                         rng,
                                                                                         rng.nextUInt32()));
            samplers.m_lightElementSamplers.push_back(new CorrelatedMultiJitterSampler(m_pixelSamplesHint * m_lightSamplesHint *
                                                                                       m_pixelSamplesHint * m_lightSamplesHint,
                                                                                       rng,
                                                                                       rng.nextUInt32()));
            samplers.m_lightSamplers.push_back(new CorrelatedMultiJitterSampler(m_pixelSamplesHint * m_lightSamplesHint,
                                                                                m_pixelSamplesHint * m_lightSamplesHint,
                                                                                rng,
                                                                                rng.nextUInt32()));
            samplers.m_brdfSamplers.push_back(new CorrelatedMultiJitterSampler(m_pixelSamplesHint * m_lightSamplesHint,
                                                                               m_pixelSamplesHint * m_lightSamplesHint,
                                                                               rng,
                                                                               rng.nextUInt32()));
        }
        // Set up samplers for each pixel sample
        samplers.m_timeSampler = new CorrelatedMultiJitterSampler(m_pixelSamplesHint * m_pixelSamplesHint, rng, rng.nextUInt32());
        samplers.m_lensSampler = new CorrelatedMultiJitterSampler(m_pixelSamplesHint, m_pixelSamplesHint, rng, rng.nextUInt32());
        samplers.m_subpixelSampler = new CorrelatedMultiJitterSampler(m_pixelSamplesHint, m_pixelSamplesHint, rng, rng.nextUInt32());
        unsigned int totalPixelSamples = samplers.m_subpixelSampler->total2DSamplesAvailable();

        // For each pixel row...
        for (size_t y = m_ystart; y < m_yend; ++y)
        {
            // For each pixel across the row...
            for (size_t x = m_xstart; x < m_xend; ++x)
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
                    float xu = (x + pu) / float(m_pImage->width());
                    // Flip pixel row to be in screen space (images are top-down)
                    float yu = 1.0f - (y + pv) / float(m_pImage->height());
                    
                    // Calculate a stratified random variation for depth-of-field
                    float lensU, lensV;
                    samplers.m_lensSampler->sample2D(psi, lensU, lensV);
                    
                    // Grab a time for motion blur
                    float timeU = samplers.m_timeSampler->sample1D(psi);
                    
                    // Find where this pixel sample hits in the scene
                    Ray ray = m_camera.makeRay((xu - 0.5f) * aspectRatioXToY + 0.5f,
                                               yu,
                                               lensU,
                                               lensV,
                                               timeU);
                    
                    // Trace a path out, gathering estimated radiance along the path
                    pixelColor += pathTrace(ray,
                                            m_masterSet,
                                            m_lights,
                                            rng,
                                            samplers,
                                            psi);
                }
                // Divide by the number of pixel samples (a box pixel filter, essentially)
                pixelColor /= totalPixelSamples;
                
                // Store off the computed pixel in a big buffer
                m_pImage->pixel(x, y) = pixelColor;
                
                // Reset samplers for the next pixel sample
                for (size_t i = 0; i < m_maxRayDepth; ++i)
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
        for (size_t i = 0; i < m_maxRayDepth; ++i)
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
    }
    
    size_t m_xstart, m_xend, m_ystart, m_yend;
    Image *m_pImage;
    ShapeSet& m_masterSet;
    const Camera& m_camera;
    std::vector<Shape*>& m_lights;
    unsigned int m_pixelSamplesHint, m_lightSamplesHint;
    unsigned int m_maxRayDepth;
};

Image* raytrace(ShapeSet& scene,
                const Camera& cam,
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
    
    // Set up render threads; we make as much as 16 chunks of the image that
    // can render in parallel.
    const size_t kChunkDim = 4;
    
    // Chunk size is the number of pixels per image chunk (we have to take care
    // to deal with tiny images)
    size_t xChunkSize = width >= kChunkDim ? width / kChunkDim : 1;
    size_t yChunkSize = height >= kChunkDim ? height / kChunkDim : 1;
    // Chunks are the number of chunks in each dimension we can chop the image
    // into (again, taking care to deal with tiny images, and also images that
    // don't divide clealy into 4 chunks)
    size_t xChunks = width > kChunkDim ? width / xChunkSize : 1;
    size_t yChunks = height > kChunkDim ? height / yChunkSize : 1;
    if (xChunks * xChunkSize < width) xChunks++;
    if (yChunks * yChunkSize < height) yChunks++;
    
    // Set up render threads
    size_t numRenderThreads = xChunks * yChunks;
    RenderThread **renderThreads = new RenderThread*[numRenderThreads];
    
    // Launch render threads
    for (size_t yc = 0; yc < yChunks; ++yc)
    {
        // Get the row start/end (making sure the last chunk doesn't go off the end)
        size_t yStart = yc * yChunkSize;
        size_t yEnd = std::min((yc + 1) * yChunkSize, height);
        for (size_t xc = 0; xc < xChunks; ++xc)
        {
            // Get the column start/end (making sure the last chunk doesn't go off the end)
            size_t xStart = xc * xChunkSize;
            size_t xEnd = std::min((xc + 1) * xChunkSize, width);
            // Render the chunk!
            renderThreads[yc * xChunks + xc] = new RenderThread(xStart,
                                                                xEnd,
                                                                yStart,
                                                                yEnd,
                                                                pImage,
                                                                scene,
                                                                cam,
                                                                lights,
                                                                pixelSamplesHint,
                                                                lightSamplesHint,
                                                                maxRayDepth);
            renderThreads[yc * xChunks + xc]->start();
        }
    }
    
    // Wait until the render finishes
    bool stillRunning;
    do
    {
        // See if any render thread is still going...
        stillRunning = false;
        for (size_t i = 0; i < numRenderThreads; ++i)
        {
            if (renderThreads[i]->isRunning())
            {
                stillRunning = true;
                break;
            }
        }
        if (stillRunning)
        {
            // Give up the CPU so the render threads can do their thing
            QThread::yieldCurrentThread();
        }
    } while (stillRunning);
    
    // Clean up render thread objects
    for (size_t i = 0; i < numRenderThreads; ++i)
    {
        delete renderThreads[i];
    }
    delete[] renderThreads;
    
    // We made a picture!
    return pImage;
}

} // ending namespace KT