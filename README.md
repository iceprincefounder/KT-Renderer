# simple-render

practise on creating ray-tracing based renderer


* Light shape subclass, with (double-sided) rectangle area light subclass
* Method to find the lights in a scene
* Shape sampling method to find a random point on a shape (used for lights currently)
* Emission added to intersection results
* Infinite plane has a procedural bullseye texture option
* Multiple samples per pixel supported (antialiasing)
* Fast random number stream added
* Shadow rays cast
* Simple Lambertian diffuse shading

* Math support: RGB colors, 3D vectors/points, ray definition
* Scene graph: abstract shape base class with ray intersectio method, infinite plane subclass
* Camera ray construction
* Single-sample-per-pixel ray-traced image