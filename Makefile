
all: bin/lily.o tests

tests: t1

t1: bin/test/t1
	bin/test/t1 > test/t1.out
	git diff --exit-code test/t1.out

bin/lily.o: bin src/lily.hpp src/lilyConstruct.hpp src/lily.cpp
	$(CXX) -c -std=c++11 -Wall src/lily.cpp -o bin/lily.o

bin/test/t1: src/lilyConstruct.hpp bin/lily.o test/t1.cpp bin/test
	$(CXX) -std=c++11 -Wall -Isrc -lstdc++ -o bin/test/t1 bin/lily.o test/t1.cpp


bin:
	mkdir bin
bin/examples:
	mkdir bin/examples
bin/test:
	mkdir bin/test
clean:
	rm -rf bin
