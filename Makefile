CXX = em++
FLAGS = -g -W -Wall -Wextra -Wpedantic -Werror -std=c++11
LIBRARIES = -lpthread
EXPORTED_FUNCTIONS=_main
ASYNCIFY_FLAGS=-s ASYNCIFY -s ASYNCIFY_IMPORTS=[jssleep]

.PHONY: default

default: public/main.wasm

feaux-s/objects/%.o: feaux-s/%.cpp
	$(CXX) $^ -c -o $@

public/main.wasm: feaux-s/objects/main.o feaux-s/objects/process.o feaux-s/objects/utils.o
	$(CXX) $^ -o public/main.js -s EXPORTED_FUNCTIONS=$(EXPORTED_FUNCTIONS) $(ASYNCIFY_FLAGS) $(LIBS)

