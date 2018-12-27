#include <iostream>
#include <cstring> 

#include "KRayTracer.h"
#include "KDriver.h"
#include "KLog.h"
#include "KGeoReader.h"

using namespace kt;
using namespace std;


//
// A common praser to get input args
//
static void usage(const char * const program) {
    fprintf(stderr, "usage: %s <command args ...>\n", "ktRender");
    fprintf(stderr, "\t\t -s     scene sources \n");
    fprintf(stderr, "\t\t -t     thread number \n");
    fprintf(stderr, "\t\t -o     output file(.ppm) \n");
    fprintf(stderr, "\t\t -wd    width of output file  (default 512) \n");
    fprintf(stderr, "\t\t -ht    height of output file (default 512) \n");
    fprintf(stderr, "\t\t -rd    ray depth    (default 2) \n");
    fprintf(stderr, "\t\t -ps    pixle sample (default 3) \n");
    fprintf(stderr, "\t\t -ls    light sample (default 1) \n");
    fprintf(stderr, "\t\t --help print help information! \n");
    fprintf(stderr, "\t kt-Renderer v0.20 by [Kevin Tsui] \n");
    exit(1);
}

int main(int argc, char *argv[]){
    const char *sources = NULL;
    const char *threads = "1";
    const char *width = "512";
    const char *height = "512";
    const char *outfile = "out/output.ppm";
    const char *rayDepth = "2";
    const char *pixleSample = "5";
    const char *lightSample = "3";

    // chasing arguments
    if (argc == 1) usage(argv[0]);
    for (int i = 1; i < argc; i++) {
        if (i > 14)
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
        else if (strcmp(argv[i], "-wd") == 0)
        {
            width = argv[i + 1];i++;
        }
        else if (strcmp(argv[i], "-ht") == 0)
        {
            height = argv[i + 1];i++;
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
        else if (strcmp(argv[i], "--help") == 0)
            usage(argv[0]); 
        else
            usage(argv[0]);
    }

    Log renderLog;
    // printf("[%s] %s\n", "kt-Renderer v0.10 by [Kevin Tsui]");
    renderLog.logging("-- Render Start ----");
    
    renderLog.logging("Genarating Scenes ...");

    renderLog.logging("\t\tcreate materials");
    // The Materials
    DiffuseMaterial basicLambert(Color(0.7f, 0.7f, 0.7f));
    DiffuseMaterial logoLambert(Color(0.9f, 0.5f, 0.0f));
    DiffuseMaterial redLambert(Color(0.7f, 0.1f, 0.1f));
    DiffuseMaterial greenLambert(Color(0.1f, 0.7f, 0.1f));
    DiffuseMaterial blueLambert(Color(0.1f, 0.1f, 0.7f));
    GlossyMaterial yellowGlossy(Color(0.9f, 0.5f, 0.1f), 0.1f);

    renderLog.logging("\t\tcreate shapes");
    // The 'scene'
    ShapeSet masterSet;
    
    Plane plane(Point(), Vector(0.0f, 1.0f, 0.0f), &basicLambert, true);
    plane.transform().translate(0.0f, Vector(0.0f, -2.0f, 0.0f));
    masterSet.addShape(&plane);
    
    
    Sphere sphere1(Point(), 1.0f, &redLambert);
    sphere1.transform().setTranslation(0.0f, Vector(2.0f, -1.0f, 0.0f));
    sphere1.transform().setTranslation(1.0f, Vector(3.0f, -1.0f, 0.0f));
    
    Sphere sphere2(Point(), 2.0f, &greenLambert);
    sphere2.transform().translate(0.0f, Vector(-3.0f, 0.0f, -2.0f));
    
    Sphere sphere3(Point(), 0.5f, &blueLambert);
    sphere3.transform().translate(0.0f, Vector(1.5f, -1.5f, 2.5f));

    if (sources !=NULL)
    {
        Polymesh* sourcesShape = readFromOBJFile(sources);
        sourcesShape->setMaterial(&basicLambert);
        sourcesShape->transform().translate(0.0f, Vector(0.0f, -2.0f, 0.0f));
        sourcesShape->transform().scale(0.0f, Vector(0.5f, 0.5f, 0.5f));
        // sourcesShape->transform().rotate(0.0f, Quaternion(Vector(0.0f, 1.0f, 0.0f).normalized(), M_PI * 0.5f));
        masterSet.addShape(sourcesShape);
    }
    else
    {
        // masterSet.addShape(&sphere1);
        // masterSet.addShape(&sphere2);
        // masterSet.addShape(&sphere3);
    }

    Polymesh* atangShape = readFromOBJFile("/home/xukai/Desktop/atang.obj");
    atangShape->setMaterial(&yellowGlossy);
    atangShape->transform().translate(0.0f, Vector(0.0f, -2.0f, 0.0f));
    atangShape->transform().scale(0.0f, Vector(0.5f, 0.5f, 0.5f));
    // sourcesShape->transform().rotate(0.0f, Quaternion(Vector(0.0f, 1.0f, 0.0f).normalized(), M_PI * 0.5f));
    masterSet.addShape(atangShape);


    renderLog.logging("\t\tcreate lights");
    // Add an top light
    RectangleLight rectangleTopLight(Point(),
                             Vector(5.0f, 0.0f, 0.0f),
                             Vector(0.0f, 0.0f, 5.0f),
                             Color(1.0f, 1.0f, 1.0f),
                             10.0f);
    rectangleTopLight.transform().setTranslation(0.0f, Vector(0.0f, 10.0f, 0.0f));
    masterSet.addShape(&rectangleTopLight);


    // Add an fill light
    RectangleLight rectangleFillLight(Point(),
                             Vector(0.0f, 5.0f, 0.0f),
                             Vector(0.0f, 0.0f, 5.0f),
                             Color(1.0f, 1.0f, 1.0f),
                             3.0f);
    rectangleFillLight.transform().setTranslation(0.0f, Vector(-10.0f, 0.0f, 0.0f));
    rectangleFillLight.transform().setRotation(0.0f, Quaternion(Vector(0.0f, 0.0f, 1.0f).normalized(), M_PI * 1.0f));
    masterSet.addShape(&rectangleFillLight);

    renderLog.logging("\t\tcreate camera");
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
    size_t imageWidth = atoi(width);
    size_t imageHeight = atoi(height);
    unsigned int pixelSamplesSpinBox = atoi(pixleSample);
    unsigned int lightSamplesSpinBox = atoi(lightSample);
    unsigned int rayDepthSpinBox = atoi(rayDepth);


    renderLog.logging("Ray Tracing ...");
    Image *pImage = rendering(
                        masterSet,
                        camera,
                        renderLog,
                        threadNumber,
                        imageWidth,
                        imageHeight,
                        pixelSamplesSpinBox,
                        lightSamplesSpinBox,
                        rayDepthSpinBox);

    renderLog.logging("Writing Output Image...");    
    // output images
    ppm_driver(pImage, outfile);
    
    // Clean up the scene and render
    delete pImage;
    renderLog.logging("-- Render Shut Down ----");    

}