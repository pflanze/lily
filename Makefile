

bin/lily.o: bin src/lily.hpp src/lily.cpp
	g++ -c -std=c++11 -Wall src/lily.cpp -o bin/lily.o

bin:
	mkdir bin
clean:
	rm -f bin/*
