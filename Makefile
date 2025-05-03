SUITE = 1
CXX = em++
FLAGS = -g -W -Wall -Wextra -Wpedantic -Werror -std=c++11
LIBRARIES = -lpthread
EXPORTED_FUNCTIONS=_main
EXPORTED_RUNTIME_METHODS=wasmExports
ASYNCIFY_FLAGS=-s ASYNCIFY -s ASYNCIFY_IMPORTS=[jssleep]

SOURCES=$(wildcard feaux-s/*.cpp)
OBJECTS=$(patsubst feaux-s/%.cpp,feaux-s/objects/%.o,$(SOURCES))

.PHONY: default clean

benchmarks: CXX = g++
benchmarks: FLAGS += -DFEAUX_S_BENCHMARKING=$(SUITE)
benchmarks: PERMISSIVE_FLAGS = -DFEAUX_S_BENCHMARKING=$(SUITE) -fpermissive -Wno-int-to-pointer-cast -g

default: public/main.wasm
benchmarks: feaux-s/bin/bench
run-bench: benchmarks
	./feaux-s/bin/bench

feaux-s/objects/main.o: feaux-s/main.cpp feaux-s/*.h
	$(CXX) feaux-s/main.cpp -c -o $@ $(PERMISSIVE_FLAGS)

feaux-s/objects/browser-api.o: feaux-s/browser-api.cpp feaux-s/browser-api.h
	$(CXX) $< -c -o $@ $(PERMISSIVE_FLAGS)

feaux-s/objects/machine.o: feaux-s/machine.cpp feaux-s/machine.h
	$(CXX) $< -c -o $@ $(PERMISSIVE_FLAGS)

feaux-s/objects/%.o: feaux-s/%.cpp feaux-s/%.h
	$(CXX) $< -c -o $@ $(FLAGS)

public/main.wasm: $(OBJECTS)
	$(CXX) $^ -o public/main.js -s EXPORTED_RUNTIME_METHODS=$(EXPORTED_RUNTIME_METHODS) -s EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS) $(ASYNCIFY_FLAGS) $(LIBS)

feaux-s/bin/bench: $(OBJECTS)
	$(CXX) $^ -o $@ $(FLAGS) $(LIBS)

clean:
	@- rm feaux-s/objects/*.o
	@- rm public/main.wasm public/main.js