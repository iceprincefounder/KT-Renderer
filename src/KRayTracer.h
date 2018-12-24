#pragma once

#include "KMathCore.h"
#include "KMaterial.h"
#include "KShape.h"
#include "KLight.h"
#include "KCamera.h"

namespace KT
{

//
// Sampler container (for a given pixel, holds the samplers for all random features and bounces)
//
struct SamplerSet
{
    // These are sampled once per pixel sample to start a path
    Sampler* m_lensSampler;
    Sampler* m_subpixelSampler;
    Sampler* m_timeSampler;
    // This is sampled once per bounce to determine the next leg of the path
    std::vector<Sampler*> m_bounceSamplers;
    // These are sampled N times per bounce (once for each light sample)
    std::vector<Sampler*> m_lightSelectionSamplers;
    std::vector<Sampler*> m_lightElementSamplers;
    std::vector<Sampler*> m_lightSamplers;
    std::vector<Sampler*> m_brdfSamplers;
    
    unsigned int m_numLightSamples;
    unsigned int m_maxRayDepth;
};


//
// Ray tracing
//
// Path trace through the scene, starting with an initial ray.
// Pass along scene information and various samplers so that we can reduce noise
// along the way.
Color pathTracer(const Ray& ray,
                ShapeSet& scene,
                std::vector<Shape*>& lights,
                RNG& rng,
                SamplerSet& samplers,
                unsigned int pixelSampleIndex);

Image* rendering(ShapeSet& scene,
                 const Camera& cam,
                 size_t theads,
                 size_t width,
                 size_t height,
                 unsigned int pixelSamplesHint,
                 unsigned int lightSamplesHint,
                 unsigned int maxRayDepth);

//
// RenderTask works on a small chunk of the image
// But currently KT-Renderer is not runing on muti-threads
//
class RenderTask
{
public:
    RenderTask(size_t xstart, size_t xend, size_t ystart, size_t yend,
               Image *pImage,
               ShapeSet& masterSet,
               const Camera& cam,
               std::vector<Shape*>& lights,
               unsigned int pixelSamplesHint,
               unsigned int lightSamplesHint,
               unsigned int maxRayDepth):
          m_xstart(xstart), m_xend(xend), m_ystart(ystart), m_yend(yend),
          m_pImage(pImage), m_masterSet(masterSet), m_camera(cam), m_lights(lights),
          m_pixelSamplesHint(pixelSamplesHint), m_lightSamplesHint(lightSamplesHint),
          m_maxRayDepth(maxRayDepth) { }

    virtual void raytracing();

private:
    size_t m_xstart, m_xend, m_ystart, m_yend;
    Image *m_pImage;
    ShapeSet& m_masterSet;
    const Camera& m_camera;
    std::vector<Shape*>& m_lights;
    unsigned int m_pixelSamplesHint, m_lightSamplesHint;
    unsigned int m_maxRayDepth;
};

} // namespace KT

