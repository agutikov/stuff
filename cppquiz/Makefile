


CXX=g++


CXXFLAGS=-std=c++11 -g -O2
CXXFLAGS+= -Wall

SRCS=$(wildcard *.cpp)
ELFS=$(SRCS:.cpp=)

all: $(ELFS)

%: $.c
	$(CXX) $(CXXFLAGS) $@ $^

clean:
	rm -f $(ELFS)

.PHONY: all clean















