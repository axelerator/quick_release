#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdio.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "input_image.h"
#include "rect.h"
#include "types.h"
#include "image_data.h"

class Geometry {
  public:

    Geometry(InputImage &image, const Rect &screen);
    void draw(Parameters *parameters);

    void writeToDisk(std::string &path, Parameters *parameters, unsigned int width, unsigned int height);
    void writeToMem(ImageData &imageData, Parameters *parameters);

  private:
    InputImage &inputImage;
    const Rect &screen;

    static GLuint zoomUniform;
    static GLuint posUniform;
    static GLuint aspectUniform;
    static GLuint exposureUniform;
    static GLuint shadowsUniform;
    static GLuint highlightsUniform;
    static GLuint contrastUniform;
    static GLuint textureUniform;
    static GLuint vertexPosition_modelspaceID ;
    static GLuint vertexUVID;
    static GLuint vertexbuffer;
    static GLuint uvbuffer;
    static GLuint shaderProgramId;

    GLuint loadShaders(const char * vertex_file_path,const char * fragment_file_path);
};
#endif
