CXX = em++
FLAGS = -g -W -Wall -Wextra -Wpedantic -Werror -std=c++11
LIBRARIES = -lpthread
EXPORTED_FUNCTIONS=_main
ASYNCIFY_FLAGS=-s ASYNCIFY -s ASYNCIFY_IMPORTS=[jssleep]

SOURCES=$(wildcard feaux-s/*.cpp)
OBJECTS=$(patsubst feaux-s/%.cpp,feaux-s/objects/%.o,$(SOURCES))

.PHONY: default

default: public/main.wasm

feaux-s/objects/main.o: feaux-s/main.cpp feaux-s/*.h
	$(CXX) feaux-s/main.cpp -c -o $@

feaux-s/objects/%.o: feaux-s/%.cpp feaux-s/%.h
	$(CXX) $< -c -o $@

public/main.wasm: $(OBJECTS)
	$(CXX) $^ -o public/main.js -s EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS) $(ASYNCIFY_FLAGS) $(LIBS)

