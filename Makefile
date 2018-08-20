# -O2
CFLAGS=-gdwarf-4 -g3


all: dirs tests

tests: t1 t2 t3

t1: bin/test/t1
	bin/test/t1 > test/t1.out
	git diff --exit-code test/t1.out

t2: bin/test/t2
	bin/test/t2 > test/t2.out
	git diff --exit-code test/t2.out

t3: bin/test/t3
	bin/test/t3 > test/t3.out
	git diff --exit-code test/t3.out

bin/lilyUtil.o: src/lilyUtil.cpp src/lilyUtil.hpp
	$(CXX) $(CFLAGS) -c -std=c++11 -Wall src/lilyUtil.cpp -o bin/lilyUtil.o

bin/lily.o: src/lily.cpp src/lily.hpp src/lilyConstruct.hpp src/lily.cpp src/lilyUtil.hpp
	$(CXX) $(CFLAGS) -c -std=c++11 -Wall src/lily.cpp -o bin/lily.o

bin/parse.o: src/parse.cpp src/parse.hpp 
	$(CXX) $(CFLAGS) -c -std=c++11 -Wall src/parse.cpp -o bin/parse.o

bin/lilyParse.o: src/lilyParse.hpp src/lily.hpp src/lilyConstruct.hpp src/lilyParse.cpp src/lilyUtil.hpp
	$(CXX) $(CFLAGS) -c -std=c++11 -Wall src/lilyParse.cpp -o bin/lilyParse.o

bin/lilyDefaultEnvironment.o: src/lilyDefaultEnvironment.cpp src/lilyDefaultEnvironment.hpp src/lilyConstruct.hpp src/lily.hpp
	$(CXX) $(CFLAGS) -c -std=c++11 -Wall src/lilyDefaultEnvironment.cpp -o bin/lilyDefaultEnvironment.o


# t1 needs a symbol from lilyParse.o which needs parse.o
bin/test/t1: test/t1.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o src/lilyConstruct.hpp
	$(CXX) $(CFLAGS) -std=c++11 -Wall -Isrc -lstdc++ -o bin/test/t1 bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o test/t1.cpp

bin/test/t2: test/t2.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o src/lilyConstruct.hpp
	$(CXX) $(CFLAGS) -std=c++11 -Wall -Isrc -lstdc++ -o bin/test/t2 bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o test/t2.cpp

bin/test/t3: test/t3.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o src/lilyConstruct.hpp
	$(CXX) $(CFLAGS) -std=c++11 -Wall -Isrc -lstdc++ -o bin/test/t3 bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o test/t3.cpp


dirs:
	mkdir -p bin bin/examples bin/test

clean:
	rm -rf bin
	mkdir -p bin bin/examples bin/test
