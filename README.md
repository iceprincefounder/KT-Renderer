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
         -o   output file(.ppm) 
         -rd  ray depth    (default 1) 
         -ps  pixle sample (default 1) 
         -ls  light sample (default 1) 
     KT-Renderer vx.xx by [Kevin Tsui]
```