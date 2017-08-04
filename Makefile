obj-m = cache-sim.o

all:
	g++ -w cache-sim.cpp -o cache-sim.o -std=c++11
run:
	./cache-sim.o config
clean:
	rm cache-sim.o
