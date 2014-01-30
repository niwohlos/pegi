CXX ?= g++
CXXFLAGS = -O3 -Wall -Wextra -Wshadow -Wno-unused-label -std=c++11 -Iinclude -g2
LIBS = -lm
RM = rm -f

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

.PHONY: all clean

all: pegi

pegi: $(OBJECTS)
	$(CXX) $^ -o $@ $(LIBS)

%.o: %.cpp src/parser-sv-handlers.cxx
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/parser-sv-handlers.cxx: src/syntax
	src/create-parser.rb

clean:
	$(RM) $(OBJECTS) pegi
