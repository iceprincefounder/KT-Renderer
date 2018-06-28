all: simple-render

simple-render: main.o
	g++ -o out/ktRender out/main.o

main.o: src/main.cpp
	g++ -c src/main.cpp -o out/main.o -O3 -Wall
install:
	out/ktRender out/output.ppm
	xdg-open out/output.ppm
	@# open ./output.ppm # MacOS setting
clean:
	rm -f out/main.o out/ktRender out/output.ppm