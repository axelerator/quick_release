#ifndef INPUT_IMAGE_H
#define INPUT_IMAGE_H

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

enum InputImageState {NEW, LOADED, FAILED};

class InputImage {
  public:

  InputImage(const std::string& filename);
  ~InputImage();

  GLuint textureId();
  float ratio();

  private:

  void load();

  const std::string filename;
  int width;
  int height;
  InputImageState state;
  GLuint textureId_;
};

#endif

