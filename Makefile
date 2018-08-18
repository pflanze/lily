
all: bin/lily.o bin/examples/simple

bin/lily.o: bin src/lily.hpp src/lily.cpp
	g++ -c -std=c++11 -Wall src/lily.cpp -o bin/lily.o

bin/examples/simple: bin/lily.o examples/simple.cpp bin/examples
	g++ -std=c++11 -Wall -Isrc examples/simple.cpp bin/lily.o -o bin/examples/simple


bin:
	mkdir bin
bin/examples:
	mkdir bin/examples
clean:
	rm -rf bin
