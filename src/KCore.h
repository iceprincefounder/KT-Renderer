#pragma once

#include "KMath.h"
#include "KRay.h"
#include "KMaterial.h"
#include "KShape.h"
#include "KLight.h"
#include "KCamera.h"

namespace KT
{


//
// Sampler container (for a given pixel, holds the samplers for all random features and bounces)
//

struct SamplerContainer
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
                Rng& rng,
                SamplerContainer& samplers,
                unsigned int pixelSampleIndex);

// Generate a ray-traced image of the scene, with the given camera, resolution,
// and sample settings
Image* raytrace(ShapeSet& scene,
                const Camera& cam,
                size_t width,
                size_t height,
                unsigned int pixelSamplesHint,
                unsigned int lightSamplesHint,
                unsigned int maxRayDepth);


Image* render(ShapeSet& scene,
                const Camera& camera,
                size_t width,
                size_t height,
                unsigned int pixelSamplesHint,
                unsigned int lightSamplesHint,
                unsigned int maxRayDepth);


} // namespace KT
