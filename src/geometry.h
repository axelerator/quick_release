#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdio.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "input_image.h"
#include "rect.h"
#include "types.h"

class Geometry {
  public:

    Geometry(InputImage &image, const Rect &screen);
    void draw(Parameters *parameters);

    void writeToDisk(std::string &path, Parameters *parameters, unsigned int width, unsigned int height);

  private:
    InputImage &inputImage;
    const Rect &screen;

    GLuint shaderProgramId;
    GLuint exposureUniform;
    GLuint shadowsUniform;
    GLuint highlightsUniform;
    GLuint contrastUniform;
    GLuint textureUniform;
    GLuint vertexPosition_modelspaceID ;
    GLuint vertexUVID;
    GLuint vertexbuffer;
    GLuint uvbuffer;

};
#endif
