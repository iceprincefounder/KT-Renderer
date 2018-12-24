#include <iostream>
#include <cstring> 

#include "KRayTracer.h"
#include "KDriver.h"
#include "KGeoReader.h"

using namespace KT;
using namespace std;

static void usage(const char * const program) {
    fprintf(stderr, "usage: %s -o <output.ppm>\n", "ktRender");
    fprintf(stderr, "\t\t -s   scene sources \n");
    fprintf(stderr, "\t\t -t   thread number \n");
    fprintf(stderr, "\t\t -o   output file(.ppm) \n");
    fprintf(stderr, "\t\t -rd  ray depth    (default 2) \n");
    fprintf(stderr, "\t\t -ps  pixle sample (default 3) \n");
    fprintf(stderr, "\t\t -ls  light sample (default 1) \n");
    fprintf(stderr, "\t KT-Renderer v0.10 by [Kevin Tsui] \n");
    exit(1);
}

int main(int argc, char *argv[]){
    const char *sources = NULL;
    const char *threads = "1";
    const char *outfile = "out/output.ppm";
    const char *rayDepth = "2";
    const char *pixleSample = "3";
    const char *lightSample = "1";
    // const char *resolution = "FLOAT";
    // chasing arguments
    // if (argc == 1) usage(argv[0]);
    for (int i = 1; i < argc; i++) {
        if (i > 6)
            printf("Too many arguments!");
        else if (strcmp(argv[i], "-s") == 0)
        {
            sources = argv[i + 1];i++;
        }
        else if (strcmp(argv[i], "-t") == 0)
        {
            threads = argv[i + 1];i++;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            outfile = argv[i + 1];i++;
        }
        else if (strcmp(argv[i], "-ld") == 0)
        {
            rayDepth = argv[i + 1];i++;
        }
        else if (strcmp(argv[i], "-ps") == 0)
        {
            pixleSample = argv[i + 1];i++;
        }
        else if (strcmp(argv[i], "-ls") == 0)
        {
            lightSample = argv[i + 1];i++;
        }
        else if (strcmp(argv[i], "-h") == 0)
            usage(argv[0]); 
        else
            // usage(argv[0]);
            printf("%s\n", "\t KT-Renderer v0.10 by [Kevin Tsui]");
    }
    
    // The Materials
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
    
    Polymesh* atangShape = readFromOBJFile("/home/xukai/Desktop/atang.obj");
    atangShape->setMaterial(&basicLambert);
    atangShape->transform().translate(0.0f, Vector(0.0f, -2.0f, 0.0f));
    atangShape->transform().scale(0.0f, Vector(0.3f, 0.3f, 0.3f));
    masterSet.addShape(atangShape);

    // Add an area light
    RectangleLight rectangleLight(Point(),
                             Vector(5.0f, 0.0f, 0.0f),
                             Vector(0.0f, 0.0f, 5.0f),
                             Color(1.0f, 1.0f, 1.0f),
                             10.0f);
    rectangleLight.transform().setTranslation(0.0f, Vector(-1.5f, 8.0f, -1.5f));
    masterSet.addShape(&rectangleLight);
    
    // Create the camera
    float FOV = 45;
    Point position = Point(-4.0f, 5.0f, 15.0f);
    Point target = Point(0.0f, 0.0f, 0.0f);
    Point targetUpDirection = Point(0.0f, 1.0f, 0.0f);
    float focalDistance = 12.0;
    float lensRadius = 0.0;
    float shutterOpen = 0.0;
    float shutterClose = 0.0;

    PerspectiveCamera camera( FOV, position, target,
                              targetUpDirection,focalDistance, lensRadius,
                              shutterOpen, shutterClose);    
    // Ray trace!
    size_t threadNumber = atoi(threads);
    size_t widthSpinBox = 512;
    size_t heightSpinBox = 512;
    unsigned int pixelSamplesSpinBox = atoi(pixleSample);
    unsigned int lightSamplesSpinBox = atoi(lightSample);
    unsigned int rayDepthSpinBox = atoi(rayDepth);


    Image *pImage = rendering(
                        masterSet,
                        camera,
                        threadNumber,
                        widthSpinBox,
                        heightSpinBox,
                        pixelSamplesSpinBox,
                        lightSamplesSpinBox,
                        rayDepthSpinBox);
    
    // output images
    ppm_driver(pImage,outfile);
    
    // Clean up the scene and render
    delete pImage;
}