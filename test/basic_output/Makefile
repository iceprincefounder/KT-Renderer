all: basic_output
OUT=out
SRC=src
basic_output: main.o
	g++ -o $(OUT)/render ${OUT}/main.o
main.o:
	g++ -c ${SRC}/main.cpp -o ${OUT}/main.o -O3 -Wall
run:
	cd ${OUT};./render
clean:
	rm -f ${OUT}/main.o ${OUT}/render ${OUT}/out.ppm ${OUT}/out.pfm
