#pragma once

#include <list>
#include <vector>
#include <algorithm>

#include "KMathCore.h"
#include "KMaterial.h"
#include "KRay.h"
#include "KSampler.h"
#include "KAccelerator.h"
#include "KShape.h"


namespace KT
{


// Polygon Face
struct Face
{
    // Both of these must be the same size (or else m_normalIndices must be empty)
    // These indices point into the vertices and normals in the owner mesh
    std::vector<unsigned int> m_vertexIndices;
    std::vector<unsigned int> m_normalIndices;
};


// Polygon mesh.  Faces may have 3 or more sides, but each face must be convex
// (no holes or edges going back inside the hull at all).  Faces are triangulated
// by making a triangle fan out from the first vertex.
class Polymesh : public Shape
{
public:
    Polymesh(const std::vector<Point>& verts,
         const std::vector<Vector>& normals,
         const std::vector<Face>& faces,
         Material* pMaterial): 
         m_vertices(verts),
         m_normals(normals),
         m_faces(faces),
         m_pMaterial(pMaterial),
         m_bbox(),
         m_bvh(*this),
         m_faceAreaCDF(),
         m_totalArea(0.0f)
    {
        
    }
    
    virtual ~Polymesh() { }
    
    void setMaterial(Material* pMaterial) { m_pMaterial = pMaterial; }
    
    virtual bool intersect(Intersection& intersection);
    
    virtual bool doesIntersect(const Ray& ray);
    
    virtual BBox bbox();
    
    virtual void prepare();
    
    // Given two random numbers between 0.0 and 1.0, find a location + surface
    // normal on the surface of the *light*.
    virtual bool sampleSurface(const Point& refPosition,
                               const Vector& refNormal,
                               float refTime,
                               float u1,
                               float u2,
                               float u3,
                               Point& outPosition,
                               Vector& outNormal,
                               float& outPdf);
    
    virtual float pdfSA(const Point &refPosition,
                        const Vector &refNormal,
                        float refTime,
                        const Point &surfPosition,
                        const Vector &surfNormal) const;
    
    virtual float surfaceAreaPdf() const
    {
        // TODO: this does not account for scaling
        return 1.0f / m_totalArea;
    }
    
    // Methods for BVH build
    
    virtual unsigned int numElements() const { return m_faces.size(); }
    
    virtual BBox elementBBox(unsigned int index) const;
    
    virtual float elementArea(unsigned int index) const
    {
        return m_faceAreaCDF[index + 1] - m_faceAreaCDF[index];
    }
    
    // Methods for BVH intersection
    
    virtual bool intersect(Intersection& intersection, unsigned int index);
    
    virtual bool doesIntersect(const Ray& ray, unsigned int index);

protected:
    std::vector<Point> m_vertices;
    std::vector<Vector> m_normals;
    std::vector<Face> m_faces;
    Material *m_pMaterial;
    BBox m_bbox;
    Bvh<Polymesh> m_bvh;
    std::vector<float> m_faceAreaCDF;
    float m_totalArea;
    
    bool intersectTri(unsigned int faceIndex, unsigned int tri, Intersection& intersection);
    
    bool doesIntersectTri(unsigned int faceIndex, unsigned int tri, const Ray& ray);

};


} // namespace KT

