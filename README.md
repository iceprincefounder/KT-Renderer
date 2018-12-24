# KT-Renderer Core

A full featured Physically Based Path-tracing Renderer


#How to build

```
git clone https://github.com/iceprincefounder/KT-Renderer.git
cd KT-Renderer
make;make install
```

#How to use

```
usage: ktRender -o <output.ppm>
         -s   scene sources 
         -t   thread number 
         -w   width of output file  (default 512) 
         -h   height of output file (default 512) 
         -o   output file(.ppm) 
         -rd  ray depth    (default 2) 
         -ps  pixle sample (default 3) 
         -ls  light sample (default 1) 
     KT-Renderer v0.20 by [Kevin Tsui]
```