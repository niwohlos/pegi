CXX ?= g++
CXXFLAGS = -O3 -Wall -Wextra -Wshadow -Wno-unused-label -std=c++11 -Iinclude -g2
LIBS = -lm
CXXSPECFLAGS = -Wall -Wextra -std=c++11 -Iinclude -Ispec/include
RM = rm -f

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard src/*.cpp) src/parser-enum-names.cpp)
NMOBJECTS = $(subst src/main.o,,$(OBJECTS))
GENERATED = $(subst %,src/%,parser-sv-handlers.cxx parser-sv-prototypes.cxx parser-enum-names.cpp) include/parser-enum-content.hpp

.PHONY: all clean generate specs

all: pegi

pegi: $(OBJECTS)
	$(CXX) $^ -o $@ $(LIBS)

%.o: %.cpp include/parser-enum-content.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/parser-sv-handlers.cxx: include/parser-enum-content.hpp
src/parser-sv-prototypes.cxx: include/parser-enum-content.hpp
src/parser-enum-names.cpp: include/parser-enum-content.hpp

include/parser-enum-content.hpp: src/syntax
	src/create-parser.rb

specs: spec/specs

specs/specs: $(NMOBJECTS)
	$(CXX) $(CXXSPECFLAGS) spec/specs.cpp $^ -o spec/specs

clean:
	$(RM) $(OBJECTS) $(GENERATED) pegi
