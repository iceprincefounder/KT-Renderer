all: simple-render

simple-render: main.o
	g++ -o simple-render main.o

main.o: main.cpp
	g++ -c main.cpp -o main.o -O3 -Wall
run:
	./simple-render
	open ./output.ppm
clean:
	rm -f main.o simple-render output.ppm
