#include <iostream>
#include "KCore.h"
#include "KMesh.h"

using namespace KT;
using namespace std;

int main(int argc, char *argv[]){
    // Make a picture...
    
    // Available materials
    DiffuseMaterial blueishLambert(Color(0.6f, 0.6f, 0.9f));
    DiffuseMaterial purplishLambert(Color(0.8f, 0.3f, 0.7f));
    DiffuseMaterial reddishLambert(Color(0.8f, 0.3f, 0.1f));
    GlossyMaterial bluishGlossy(Color(0.5f, 0.3f, 0.8f), 0.3);
    GlossyMaterial greenishGlossy(Color(0.3f, 0.9f, 0.3f), 0.1f);
    GlossyMaterial reddishGlossy(Color(0.8f, 0.1f, 0.1f), 0.3f);
    ReflectionMaterial reflective(Color(0.7f, 0.7f, 0.2f));
    
    // The 'scene'
    ShapeSet masterSet;
    
    // Put a ground plane in (with bullseye texture!)
    // Last parameter is whether to do the bullseye texture or not
    Plane plane(Point(), Vector(0.0f, 1.0f, 0.0f), &blueishLambert, true);
    plane.transform().translate(0.0f, Vector(0.0f, -2.0f, 0.0f));
    masterSet.addShape(&plane);
    
    // Add a pile-o-spheres with a few interesting materials
    
    Sphere sphere1(Point(), 1.0f, &purplishLambert);
    sphere1.transform().setTranslation(0.0f, Vector(2.0f, -1.0f, 0.0f));
    sphere1.transform().setTranslation(1.0f, Vector(3.0f, -1.0f, 0.0f));
    masterSet.addShape(&sphere1);
    
    Sphere sphere2(Point(), 2.0f, &greenishGlossy);
    sphere2.transform().translate(0.0f, Vector(-3.0f, 0.0f, -2.0f));
    masterSet.addShape(&sphere2);
    
    Sphere sphere3(Point(), 0.5f, &bluishGlossy);
    sphere3.transform().translate(0.0f, Vector(1.5f, -1.5f, 2.5f));
    masterSet.addShape(&sphere3);
    
    Sphere sphere4(Point(), 0.5f, &reflective);
    sphere4.transform().translate(0.0f, Vector(-2.0, -1.5f, 1.0f));
    masterSet.addShape(&sphere4);
    
    //     // Add a manually created mesh (a box), and read an OBJ file into a mesh
        
    //     Mesh *cubeMesh = makeCube();
    //     cubeMesh->setMaterial(&reddishLambert);
    //     cubeMesh->transform().translate(0.0f, Vector(0.0f, -2.0f, -2.0f));
    //     cubeMesh->transform().rotate(1.0f, Quaternion(Vector(0.0f, 1.0f, 0.0f), M_PI / 4.0f));
    //     masterSet.addShape(cubeMesh);

    //     Mesh* pOBJMesh = createFromOBJFile("../models/bumpy.obj");
    //     pOBJMesh->setMaterial(&reddishGlossy);
    //     pOBJMesh->transform().setTranslation(0.0f, Vector(0.2f, 0.0f, 0.0f));
    //     pOBJMesh->transform().rotate(0.5f, Quaternion(Vector(0.0f, 1.0f, 0.0f), M_PI / 4.0f));
    //     pOBJMesh->transform().rotate(1.0f, Quaternion(Vector(0.0f, 1.0f, 0.0f), M_PI / 2.0f));
    // #if MAKE_OBJ_A_MESH_LIGHT
    //     // For some fun, you can turn the OBJ mesh into a light (it's a bit noisy, though)
    //     ShapeLight meshLight(pOBJMesh, Color(1.0f, 1.0f, 1.0f), 10.0f);
    //     masterSet.addShape(&meshLight);
    // #else
    //     masterSet.addShape(pOBJMesh);
    // #endif

    // Add an area light
    RectangleLight areaLight(Point(),
                             Vector(3.0f, 0.0f, 0.0f),
                             Vector(0.0f, 0.0f, 3.0f),
                             Color(1.0f, 1.0f, 1.0f),
                             5.0f);
    areaLight.transform().setTranslation(0.0f, Vector(-1.5f, 4.0f, -1.5f));
    // Uncomment this to have the rect light hinge-swing downward
//    areaLight.transform().setRotation(1.0f, Quaternion(Vector(0.0f, 0.0f, 1.0f), -M_PI / 4.0f));
    masterSet.addShape(&areaLight);

    // Add an area light based on a shape (a sphere)
    Sphere sphereForLight(Point(), 0.1f, &blueishLambert);
    sphereForLight.transform().setTranslation(0.0f, Vector(0.0f, 0.5f, 4.0f));
    sphereForLight.transform().setTranslation(0.33f, Vector(0.0f, 1.5f, 4.0f));
    sphereForLight.transform().setTranslation(0.67f, Vector(1.0f, 1.5f, 4.0f));
    sphereForLight.transform().setTranslation(1.0f, Vector(1.0f, 0.5f, 4.0f));
    ShapeLight sphereLight(&sphereForLight, Color(1.0f, 1.0f, 0.3f), 100.0f);
    masterSet.addShape(&sphereLight);
    
    // // Create the camera based on user settings
    // PerspectiveCamera cam((float)ui->camFovSpinBox->value(),
    //                       Point(-4.0f, 5.0f, 15.0f),
    //                       Point(0.0f, 0.0f, 0.0f),
    //                       Point(0.0f, 1.0f, 0.0f),
    //                       (float)ui->focalDistanceSpinBox->value(),
    //                       (float)ui->lensRadiusSpinBox->value(),
    //                       (float)ui->shutterOpenSpinBox->value(),
    //                       (float)ui->shutterCloseSpinBox->value());
    
    // // Ray trace!
    // Image *pImage = raytrace(masterSet,
    //                          cam,
    //                          (size_t)ui->widthSpinBox->value(),
    //                          (size_t)ui->heightSpinBox->value(),
    //                          (unsigned int)ui->pixelSamplesSpinBox->value(),
    //                          (unsigned int)ui->lightSamplesSpinBox->value(),
    //                          (unsigned int)ui->rayDepthSpinBox->value());
    
    // displayImage(pImage);
    
    // Clean up the scene and render
    // delete pImage;
    // delete pOBJMesh;
    cout << "=== KT Renderer ===" << endl;
}