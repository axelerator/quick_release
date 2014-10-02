#include "input_image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"                  // for .png loading

InputImage::InputImage(const std::string& filename)
  :filename(filename), state(NEW) {
}

InputImage::~InputImage() {
   if (state != NEW) {
     GLuint t[] = {this->textureId()};
     glDeleteTextures(1, t);
     delete _size;
   }
}

GLuint InputImage::textureId() {
  if (state == NEW) load();
  return textureId_;
}

float InputImage::ratio() {
  if (this->state == NEW) load();
  return _size->ratio();
}

const Rect& InputImage::size() {
  if (this->state == NEW) load();
  return *_size;
}

void InputImage::load() {
  FILE *file = fopen(filename.c_str(), "rb");
  if (!file) return ;

  int comp, width, height;
  unsigned char *data = stbi_load_from_file(file, &width, &height, &comp, 0);
  fclose(file);
  _size = new Rect(width, height);

  // Create one OpenGL texture
  GLuint textureID;
  glGenTextures(1, &textureID);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Give the image to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

  // OpenGL has now copied the data. Free our own version
  delete [] data;
  // ... nice trilinear filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Return the ID of the texture we just created
  textureId_ = textureID;
  this->state = LOADED;
}


