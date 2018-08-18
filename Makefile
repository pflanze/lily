
all: bin/lily.o bin/examples/simple

bin/lily.o: bin src/lily.hpp src/lilyConstruct.hpp src/lily.cpp
	clang -c -std=c++11 -Wall src/lily.cpp -o bin/lily.o

bin/examples/simple: src/lilyConstruct.hpp bin/lily.o examples/simple.cpp bin/examples
	clang -std=c++11 -Wall -Isrc -lstdc++ -o bin/examples/simple bin/lily.o examples/simple.cpp


bin:
	mkdir bin
bin/examples:
	mkdir bin/examples
clean:
	rm -rf bin
