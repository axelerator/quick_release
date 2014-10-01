# This makefile currently only works for mac os
# You should install via homebrew:
#    brew install glew
#    brew install glfw3
#
INCLUDE=-I/usr/local/Cellar/glew/1.11.0/include -I/usr/local/Cellar/glfw3/3.0.4/include

LDFLAGS=-framework OpenGL -framework Cocoa -framework IOKit 
LDFLAGS+=-L/usr/local/Cellar/glew/1.11.0/lib -L/usr/local/Cellar/glfw3/3.0.4/lib
LDFLAGS+=-lglew -lglfw3

CXXFLAGS= -D__APPLE__ -std=c++11 -stdlib=libc++

EXECUTABLE=build/quickrelease

SOURCES = $(wildcard src/*.cpp)
OBJECTS1 = $(SOURCES:.cpp=.o)

OBJECTS = $(subst src,build,$(OBJECTS1))

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o build/$*.o src/$*.cpp  $(INCLUDE)

json11: include/json11/json11.cpp
	$(CXX) $(CXXFLAGS) -c -o build/json11.o $^

imgui: include/imgui/imgui.cpp
	$(CXX) $(CXXFLAGS) -c -o build/imgui.o $^


#src/main.cpp  include/json11/json11.cpp
$(EXECUTABLE): $(OBJECTS) imgui json11
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) build/imgui.o build/json11.o -o $(EXECUTABLE)

all: $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
