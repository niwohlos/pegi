CXX ?= g++
CXXFLAGS = -O3 -Wall -Wextra -Wshadow -Wno-unused-label -std=c++11 -Iinclude -g2
LIBS = -lm
CXXSPECFLAGS = -Wall -Wextra -std=c++11 -Iinclude -Ispec/include
RM = rm -f

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

.PHONY: all clean specs

all: pegi

pegi: $(OBJECTS)
	$(CXX) $^ -o $@ $(LIBS)

%.o: %.cpp src/parser-sv-handlers.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/parser-sv-handlers.cxx: src/syntax
	src/create-parser.rb

specs:
	$(CXX) $(CXXSPECFLAGS) spec/specs.cpp -o spec/specs

clean:
	$(RM) $(OBJECTS) pegi
