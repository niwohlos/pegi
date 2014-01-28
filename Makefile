CXX ?= g++
CXXFLAGS = -O3 -Wall -Wextra -Wshadow -std=c++11 -Iinclude -g2
RM = rm -f

OBJECTS = $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

.PHONY: all clean

all: pegi

pegi: $(OBJECTS)
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) pegi
