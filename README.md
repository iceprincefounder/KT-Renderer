# KT-Renderer Core
![enter image description here](https://user-images.githubusercontent.com/16664056/50396197-678f0280-07a3-11e9-9bda-a335aeb2321b.png)
A full featured Physically Based Path-tracing Renderer

#How to build

```
git clone https://github.com/iceprincefounder/KT-Renderer.git
cd KT-Renderer
make;make install
```

#How to use

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
     KT-Renderer v0.20 by [Kevin Tsui]
```
