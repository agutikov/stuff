
CXX = g++

LIB_V ?= 1
APP_V ?= 1

CXXFLAGS = -O2 -std=c++20 -Wall -Werror -DLIB_VERSION=${LIB_V} -DAPP_VERSION=${APP_V}
LDFLAGS = -lfoo -L. -Wl,-rpath=.

LIB = libfoo.so
LIB_SRC = foo.cpp
LIB_OBJ = $(LIB_SRC:%.cpp=%.o)

APP = test
APP_SRC = main.cpp

all: $(APP) $(LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

$(APP): $(APP_SRC) | $(LIB)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(LIB): $(LIB_OBJ)
	$(CXX) -shared -o $(LIB) $^

clean:
	rm -fv *.o $(APP) *.so
