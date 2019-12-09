
ifndef INSIDE_EMACS
	CFLAGS_COLOR=-fdiagnostics-color=always
endif

CFLAGS += -std=c++11 -gdwarf-3 -g3 $(CFLAGS_COLOR)

ifdef ONE
CFLAGS += -fmax-errors=1
endif

all: dirs bin/examples/repl

test: dirs t1 t_parse t2 t3

t1: bin/test/t1
	bin/test/t1 > test/t1.actual.out
	sbin/tdiff test/t1.out

t_parse: bin/test/t_parse
	bin/test/t_parse > test/t_parse.actual.out
	sbin/tdiff test/t_parse.out

t2: bin/test/t2
	bin/test/t2 > test/t2.actual.out
	sbin/tdiff test/t2.out

t3: bin/test/t3
	bin/test/t3 > test/t3.actual.out
	sbin/tdiff test/t3.out

src/lily.hpp: src/safemath.hpp
	touch src/lily.hpp

SipHash/siphash.c:
	git submodule update --init SipHash

bin/lilyUtil.o: src/lilyUtil.cpp src/lilyUtil.hpp src/parse.hpp
	$(CXX) $(CFLAGS) -c -Wall src/lilyUtil.cpp -o bin/lilyUtil.o

bin/symboltable.o: src/symboltable.cpp src/symboltable.hpp SipHash/siphash.c src/lilyUtil.hpp
	$(CXX) $(CFLAGS) -c -Wall src/symboltable.cpp -o bin/symboltable.o

bin/lilyConstruct.o: src/lilyConstruct.cpp src/lilyConstruct.hpp src/lily.hpp src/lilyUtil.hpp
	$(CXX) $(CFLAGS) -c -Wall src/lilyConstruct.cpp -o bin/lilyConstruct.o

bin/lily.o: src/lily.cpp src/lily.hpp src/lilyConstruct.hpp src/lily.cpp src/lilyUtil.hpp src/lilyParse.hpp src/parse.hpp src/symboltable.hpp 
	$(CXX) $(CFLAGS) -c -Wall src/lily.cpp -o bin/lily.o

bin/parse.o: src/parse.cpp src/parse.hpp 
	$(CXX) $(CFLAGS) -c -Wall src/parse.cpp -o bin/parse.o

bin/lilyParse.o: src/lilyParse.hpp src/lily.hpp src/lilyConstruct.hpp src/lilyParse.cpp src/lilyUtil.hpp src/parse.hpp src/functional.hpp
	$(CXX) $(CFLAGS) -c -Wall src/lilyParse.cpp -o bin/lilyParse.o

bin/lilyDefaultEnvironment.o: src/lilyDefaultEnvironment.cpp src/lilyDefaultEnvironment.hpp src/lilyConstruct.hpp src/lily.hpp src/lilyUtil.hpp src/lilyHelper.hpp
	$(CXX) $(CFLAGS) -c -Wall src/lilyDefaultEnvironment.cpp -o bin/lilyDefaultEnvironment.o

# library for linking 
bin/lily.a: bin/lilyUtil.o bin/parse.o bin/symboltable.o bin/lilyDefaultEnvironment.o bin/lily.o bin/lilyParse.o bin/lilyConstruct.o
	ar rcs bin/lily.a  bin/lilyUtil.o bin/parse.o bin/symboltable.o bin/lilyDefaultEnvironment.o bin/lily.o bin/lilyParse.o bin/lilyConstruct.o

# t1 needs a symbol from lilyParse.o which needs parse.o
bin/test/t1: test/t1.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyConstruct.o bin/symboltable.o
	$(CXX) $(CFLAGS) -Wall -Isrc -lstdc++ -o bin/test/t1 bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyConstruct.o bin/symboltable.o test/t1.cpp

# lily.o needs lilyParse.o -- XX change so that it doesn't
bin/test/t_parse: test/t_parse.cpp bin/lilyUtil.o bin/parse.o bin/symboltable.o
	$(CXX) $(CFLAGS) -Wall -Isrc -lstdc++ -o bin/test/t_parse bin/lilyUtil.o bin/lilyParse.o bin/lily.o bin/parse.o bin/lilyConstruct.o bin/symboltable.o test/t_parse.cpp

bin/test/t2: test/t2.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyConstruct.o bin/symboltable.o
	$(CXX) $(CFLAGS) -Wall -Isrc -lstdc++ -o bin/test/t2 bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyConstruct.o bin/symboltable.o test/t2.cpp

bin/test/t3: test/t3.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o bin/lilyConstruct.o bin/symboltable.o
	$(CXX) $(CFLAGS) -Wall -Isrc -lstdc++ -o bin/test/t3 bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o bin/lilyConstruct.o bin/symboltable.o test/t3.cpp

bin/examples/repl: examples/repl.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o bin/lilyConstruct.o bin/symboltable.o
	$(CXX) $(CFLAGS) -Wall -Isrc -lstdc++ -o bin/examples/repl bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o bin/lilyConstruct.o bin/symboltable.o examples/repl.cpp

bin/examples/qt: examples/qt.cpp bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o bin/lilyConstruct.o bin/symboltable.o
	$(CXX) $(CFLAGS) -Wall -Isrc -lstdc++ -o bin/examples/qt bin/lilyUtil.o bin/lily.o bin/parse.o bin/lilyParse.o bin/lilyDefaultEnvironment.o bin/lilyConstruct.o bin/symboltable.o examples/qt.cpp



dirs:
	mkdir -p bin bin/examples bin/test

clean:
	rm -rf bin
	mkdir -p bin bin/examples bin/test
