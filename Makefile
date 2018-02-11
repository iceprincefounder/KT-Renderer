all: simple-render

simple-render: main.o
	g++ -o simple-render main.o

main.o: main.cpp core.h
	g++ -c main.cpp -o main.o -O3 -Wall
run:
	./simple-render
	xdg-open ./out.ppm
clean:
	rm -f main.o simple-render out.ppm out.pfm
