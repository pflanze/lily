# -O2
CFLAGS=-gdwarf-4 -g3


all: dirs bin/lily.o tests

tests: t1 t2

t1: bin/test/t1
	bin/test/t1 > test/t1.out
	git diff --exit-code test/t1.out

t2: bin/test/t2
	bin/test/t2 > test/t2.out
	git diff --exit-code test/t2.out

bin/lily.o: src/lily.hpp src/lilyConstruct.hpp src/lily.cpp src/lilyUtil.hpp
	$(CXX) $(CFLAGS) -c -std=c++11 -Wall src/lily.cpp -o bin/lily.o

bin/lilyParse.o: src/lilyParse.hpp src/lily.hpp src/lilyConstruct.hpp src/lilyParse.cpp src/lilyUtil.hpp
	$(CXX) $(CFLAGS) -c -std=c++11 -Wall src/lilyParse.cpp -o bin/lilyParse.o

bin/test/t1: bin/lily.o test/t1.cpp
	$(CXX) $(CFLAGS) -std=c++11 -Wall -Isrc -lstdc++ -o bin/test/t1 bin/lily.o test/t1.cpp

bin/test/t2: bin/lily.o bin/lilyParse.o test/t2.cpp
	$(CXX) $(CFLAGS) -std=c++11 -Wall -Isrc -lstdc++ -o bin/test/t2 bin/lily.o bin/lilyParse.o test/t2.cpp


dirs:
	mkdir -p bin bin/examples bin/test

clean:
	rm -rf bin
	mkdir -p bin bin/examples bin/test
