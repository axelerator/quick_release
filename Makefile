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
OBJECTS = $(SOURCES:.cpp=.o)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o build/$*.o src/$*.cpp  $(INCLUDE)


#src/main.cpp include/imgui/imgui.cpp include/json11/json11.cpp
all: $(OBJECTS) 
	echo $(OBJECTS)

clean:
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE)  $^
	rm $(EXECUTABLE)
