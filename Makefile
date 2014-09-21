# This makefile currently only works for mac os
# You should install via homebrew:
#    brew install glew
#    brew install glfw3
#
CXXFLAGS=-framework OpenGL -framework Cocoa -framework IOKit 
CXXFLAGS+=-I/usr/local/Cellar/glew/1.11.0/include -I/usr/local/Cellar/glfw3/3.0.4/include
CXXFLAGS+=-L/usr/local/Cellar/glew/1.11.0/lib -L/usr/local/Cellar/glfw3/3.0.4/lib
CXXFLAGS+=-lglew -lglfw3
CXXFLAGS+=-I../../
CXXFLAGS+= -D__APPLE__ -std=c++11 -stdlib=libc++

EXECUTABLE=build/quickrelease

all: src/main.cpp include/imgui/imgui.cpp include/json11/json11.cpp
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE)  $^

clean:
	rm $(EXECUTABLE)
