
# KT-Renderer Core
![logo](https://user-images.githubusercontent.com/16664056/50462709-0d1fbd00-09c2-11e9-9fca-7b21d2fab006.png)
A full featured **Physically Based Path-tracing Renderer**

Currently, KT-Renderer is a standlone no dependence application, I only use standard c++ 11 library to implement it, so you could easily build and run in your computer.


## Features

 - Path Tracing
 - Bidirectional Reflective Distribution Function (BRDF)
 - Depth of Field (DOF)
 - Motion Blur
 - Multiple Importance Sampling (MIS)

## How to build
```
git clone https://github.com/iceprincefounder/KT-Renderer.git
cd KT-Renderer
make;make install
```

## How to use
KT-Renderer is a command line tool, you could compile and run in your terminal .
```
$ ktRender --help
```
```
usage: ktRender <command args ...>
         -s   scene sources 
         -t   thread number 
         -o   output file(.ppm) 
         -wd   width of output file  (default 512) 
         -ht   height of output file (default 512) 
         -rd  ray depth    (default 2) 
         -ps  pixle sample (default 3) 
         -ls  light sample (default 1)
         --help print help information! 
     KT-Renderer v0.20 by [Kevin Tsui]
```
