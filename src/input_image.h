#ifndef INPUT_IMAGE_H
#define INPUT_IMAGE_H

#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "rect.h"

enum InputImageState {NEW, LOADED, FAILED};

class InputImage {
  public:

  InputImage(const std::string& filename);
  ~InputImage();

  GLuint textureId();
  float ratio();
  const Rect& size();

  private:

  void load();

  const std::string filename;
  Rect *_size;
  InputImageState state;
  GLuint textureId_;
};

#endif

