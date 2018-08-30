#include <iostream>
#include <cstring> 
#include "KCore.h"
#include "KMesh.h"
#include "KDriver.h"

using namespace KT;
using namespace std;

static void usage(const char * const program) {
    fprintf(stderr, "usage: %s -o <output.ppm>\n", "ktRender");
    fprintf(stderr, "\t\t -o  output file \n");
    fprintf(stderr, "\t\t -h  print help information \n");
    fprintf(stderr, "\t Resize OpenEXR file, for proxy usage! \n");
    exit(1);
}

int main(int argc, char *argv[]){

    // chasing arguments
    if (argc == 1) ;
    for (int i = 1; i < argc; i++) {
        if (i > 4)
        {

        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            usage(argv[0]);
        }
        else
        {

        }
    }

    // Make a picture...
    
    // Available materials
    DiffuseMaterial basicLambert(Color(0.7f, 0.7f, 0.7f));

    DiffuseMaterial redLambert(Color(0.7f, 0.1f, 0.1f));
    DiffuseMaterial greenLambert(Color(0.1f, 0.7f, 0.1f));
    DiffuseMaterial blueLambert(Color(0.1f, 0.1f, 0.7f));
    
    // The 'scene'
    ShapeSet masterSet;
    
    Plane plane(Point(), Vector(0.0f, 1.0f, 0.0f), &basicLambert, false);
    plane.transform().translate(0.0f, Vector(0.0f, -2.0f, 0.0f));
    masterSet.addShape(&plane);
    
    
    Sphere sphere1(Point(), 1.0f, &redLambert);
    sphere1.transform().setTranslation(0.0f, Vector(2.0f, -1.0f, 0.0f));
    sphere1.transform().setTranslation(1.0f, Vector(3.0f, -1.0f, 0.0f));
    masterSet.addShape(&sphere1);
    
    Sphere sphere2(Point(), 2.0f, &greenLambert);
    sphere2.transform().translate(0.0f, Vector(-3.0f, 0.0f, -2.0f));
    masterSet.addShape(&sphere2);
    
    Sphere sphere3(Point(), 0.5f, &blueLambert);
    sphere3.transform().translate(0.0f, Vector(1.5f, -1.5f, 2.5f));
    masterSet.addShape(&sphere3);
    
    // Add an area light
    RectangleLight rectangleLight(Point(),
                             Vector(3.0f, 0.0f, 0.0f),
                             Vector(0.0f, 0.0f, 3.0f),
                             Color(1.0f, 1.0f, 1.0f),
                             10.0f);
    rectangleLight.transform().setTranslation(0.0f, Vector(-1.5f, 5.0f, -1.5f));
    masterSet.addShape(&rectangleLight);
    
    // Create the camera
    float FOV = 45;
    Point position = Point(-4.0f, 5.0f, 15.0f);
    Point target = Point(0.0f, 0.0f, 0.0f);
    Point targetUpDirection = Point(0.0f, 1.0f, 0.0f);
    float focalDistance = 12.0;
    float lensRadius = 5.0;
    float shutterOpen = 0.0;
    float shutterClose = 0.0;

    PerspectiveCamera camera( FOV, position, target,
                              targetUpDirection,focalDistance, lensRadius,
                              shutterOpen, shutterClose);    
    // Ray trace!
    size_t widthSpinBox = 512;
    size_t heightSpinBox = 512;
    unsigned int pixelSamplesSpinBox = 1;
    unsigned int lightSamplesSpinBox = 1;
    unsigned int rayDepthSpinBox = 2;


    Image *pImage = render(masterSet,
                             camera,widthSpinBox,heightSpinBox,
                             pixelSamplesSpinBox,lightSamplesSpinBox,
                             rayDepthSpinBox);
    
    // output images
    char outfile[] = "out/output.ppm";
    ppm_driver(pImage,outfile);
    
    // Clean up the scene and render
    delete pImage;
}