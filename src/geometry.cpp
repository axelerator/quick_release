#include "geometry.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>

#include "../include/imgui/imgui.h"

#include "image_data.h"

GLuint Geometry::shaderProgramId = -1;
GLuint Geometry::zoomUniform;
GLuint Geometry::posUniform;
GLuint Geometry::aspectUniform;
GLuint Geometry::exposureUniform;
GLuint Geometry::shadowsUniform;
GLuint Geometry::highlightsUniform;
GLuint Geometry::contrastUniform;
GLuint Geometry::textureUniform;
GLuint Geometry::vertexPosition_modelspaceID ;
GLuint Geometry::vertexUVID;
GLuint Geometry::vertexbuffer;
GLuint Geometry::uvbuffer;

GLuint Geometry::loadShaders(const char * vertex_file_path,const char * fragment_file_path){

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( InfoLogLength < 1 ? 1 : InfoLogLength );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

Geometry::Geometry(InputImage &image, const Rect &screen): inputImage(image), screen(screen) {
  if (shaderProgramId == -1) {
    shaderProgramId = loadShaders("vertex.vertexshader", "fragment.fragmentshader");
    zoomUniform = glGetUniformLocation(shaderProgramId,"zoom");
    posUniform = glGetUniformLocation(shaderProgramId,"pos");
    aspectUniform = glGetUniformLocation(shaderProgramId,"aspect");
    exposureUniform = glGetUniformLocation(shaderProgramId,"exposure");
    shadowsUniform = glGetUniformLocation(shaderProgramId,"shadows");
    highlightsUniform = glGetUniformLocation(shaderProgramId,"highlights");
    contrastUniform = glGetUniformLocation(shaderProgramId,"contrast");
  }

  // Get a handle for our buffers
  vertexPosition_modelspaceID = glGetAttribLocation(shaderProgramId, "vertexPosition_modelspace");
  vertexUVID = glGetAttribLocation(shaderProgramId, "vertexUV");

  // An array of 3 vectors which represents 3 vertices
  static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, 1.0f , 0.0f,
   -1.0f,-1.0f , 0.0f,
    1.0f,-1.0f , 0.0f,
    1.0f, 1.0f , 0.0f
  };
  // Two UV coordinatesfor each vertex. They were created with Blender. You'll learn shortly how to do this yourself.
  static const GLfloat g_uv_buffer_data[] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f
  };

  // Generate 1 buffer, put the resulting identifier in vertexbuffer
  glGenBuffers(1, &vertexbuffer);
  // The following commands will talk about our 'vertexbuffer' buffer
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  // Give our vertices to OpenGL.
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get a handle for our "myTextureSampler" uniform
  textureUniform  = glGetUniformLocation(shaderProgramId, "myTextureSampler");
}

void Geometry::draw(Parameters *parameters) {
  // Use our shader
  glUseProgram(shaderProgramId);

  glUniform1f(exposureUniform, parameters->exposure);
  glUniform1f(shadowsUniform, parameters->shadows);
  glUniform1f(highlightsUniform, parameters->highlights);
  glUniform1f(zoomUniform, parameters->zoom);
  glUniform2f(posUniform, parameters->pos[0], parameters->pos[1]);
  float r = screen.ratio() / inputImage.ratio(); //(1.0 / image.ratio()) *0.5;
  glUniform1f(aspectUniform,r );
  glUniform1f(contrastUniform, parameters->contrast);
  // Set our "myTextureSampler" sampler to user Texture Unit 0
  glUniform1i(textureUniform, 0);

  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, inputImage.textureId());

  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(vertexPosition_modelspaceID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glVertexAttribPointer(
      vertexPosition_modelspaceID,  // The attribute we want to configure
      3,                            // size
      GL_FLOAT,                     // type
      GL_FALSE,                     // normalized?
      0,                            // stride
      (void*)0                      // array buffer offset
      );

  // 2nd attribute buffer : UVs
  glEnableVertexAttribArray(vertexUVID);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glVertexAttribPointer(
      vertexUVID,                   // The attribute we want to configure
      2,                            // size : U+V => 2
      GL_FLOAT,                     // type
      GL_FALSE,                     // normalized?
      0,                            // stride
      (void*)0                      // array buffer offset
      );

  // Draw the triangles !
  glDrawArrays(GL_QUADS, 0, 4*3); // 12*3 indices starting at 0 -> 12 triangles

  glDisableVertexAttribArray(vertexPosition_modelspaceID);
  glDisableVertexAttribArray(vertexUVID);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
}

void Geometry::writeToDisk(std::string &path, Parameters *parameters, unsigned int width, unsigned int height) {
  // PREP for render2text ------------------------------
  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
  GLuint FramebufferName = 0;
  glGenFramebuffers(1, &FramebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  // The depth buffer
  if ( !GLEW_ARB_framebuffer_object ){ // OpenGL 2.1 doesn't require this, 3.1+ does
    printf("Your GPU does not provide framebuffer objects. Use a texture instead.");
    return;
  }
  GLuint depthrenderbuffer;
  glGenRenderbuffers(1, &depthrenderbuffer);

  // The texture we're going to render to
  GLuint renderedTexture;
  glGenTextures(1, &renderedTexture);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, renderedTexture);

  // Give an empty image to OpenGL ( the last "0" means "empty" )
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

  // Set "renderedTexture" as our colour attachement #0
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return ;
  // PREP for render2text END ------------------------------
  //
  // Render to our framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  glViewport(0,0,width,height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
  // Rendering
  glViewport(0, 0, width, height);
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  //
  this->draw(parameters);

  ImageData imageData(Rect(width, height));
  glReadPixels(0,0,width,height, GL_RGB, GL_UNSIGNED_BYTE, imageData.data);

  // Render to display frame buffer again and free resources
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  GLuint deleteBuffers[] = {FramebufferName};
  glDeleteFramebuffers(1, deleteBuffers);
  GLuint deleteTexture[] = {renderedTexture};
  glDeleteTextures(1, deleteTexture);
  GLuint deleteBuffer[] = {depthrenderbuffer};
  glDeleteRenderbuffers(1, deleteBuffer);
}

