#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <string>
#include <GL/glew.h>

class Renderer {
  Renderer(const std::string &vertex, const std::string &fragment);
  GLuint loadShaders(const char * vertex_file_path,const char * fragment_file_path);

  static GLuint shaderProgramId;
};
#endif
