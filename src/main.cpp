#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <string>
#include <ftw.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb/stb_image.h"                  // for .png loading
#include "../include/stb/stb_image_write.h"
#include "../include/imgui/imgui.h"
#include "../include/json11/json11.hpp"
#ifdef _MSC_VER
#pragma warning (disable: 4996)         // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#define debug(format, arg) printf(format, arg)

using namespace std;
static GLFWwindow* window;
static GLuint fontTex;

enum Errors {

};

enum InputImageState {NEW, LOADED, FAILED};



class Config {
  public:
  Config(const std::string path) : path(path) {};

  void load() {
    FILE *handler = fopen(path.c_str(),"r");
    if (handler) {
      char *configString = readFile(handler);
      string err;
      json11::Json configJson = json11::Json::parse(configString, err);
    } else {

    }
  }

  void save() {
    json11::Json::array jsonDirs;
    for (auto &dir : directories) {
      jsonDirs.push_back(dir);
    }

    json11::Json jsonConfig = json11::Json::object({
        { "directories", jsonDirs }
        });

    std::string jsonStr = jsonConfig.dump();
    std::ofstream outfile (path,std::ofstream::binary);
    outfile.write (jsonStr.c_str(), jsonStr.length());
    outfile.close();
  }
  private:

  char* readFile(FILE *handler) {
    char *buffer = NULL;
    int string_size,read_size;

    if (handler)
    {
      //seek the last byte of the file
      fseek(handler,0,SEEK_END);
      //offset from the first to the last byte, or in other words, filesize
      string_size = ftell (handler);
      //go back to the start of the file
      rewind(handler);

      //allocate a string that can hold it all
      buffer = (char*) malloc (sizeof(char) * (string_size + 1) );
      //read it all in one operation
      read_size = fread(buffer,sizeof(char),string_size,handler);
      //fread doesnt set it so put a \0 in the last position
      //and buffer is now officialy a string
      buffer[string_size] = '\0';

      if (string_size != read_size) {
        //something went wrong, throw away the memory and set
        //the buffer to NULL
        free(buffer);
        buffer = NULL;
      }
    }

    return buffer;
  }


  const std::string path;
  const std::vector<string> directories;
};

class InputImage {
  public:

  InputImage(const std::string& filename)
    :filename(filename), state(NEW) {
  }

  ~InputImage() {
  }

  GLuint textureId() {
    if (this->state == NEW) load();
    return textureId_;
  }

  float ratio() {
    if (this->state == NEW) load();
    return (float)width/height;
  }

  private:

   void load() {
    FILE *file = fopen(filename.c_str(), "rb");
    if (!file) return ;

    int comp;
    unsigned char *data = stbi_load_from_file(file, &width, &height, &comp, 0);
    fclose(file);

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

  const std::string filename;
  int width;
  int height;
  InputImageState state;
  GLuint textureId_;
};

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

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
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}


GLuint loadBmp(const char * imagepath){

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file)							    {printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0;}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	// Everything is in memory now, the file wan be closed
	fclose (file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}



// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
// - try adjusting ImGui::GetIO().PixelCenterOffset to 0.5f or 0.375f
static void ImImpl_RenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    if (cmd_lists_count == 0)
        return;

    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // A probable faster way to render would be to collate all vertices from all cmd_lists into a single vertex buffer.
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Setup texture
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glEnable(GL_TEXTURE_2D);

    // Setup orthographic projection matrix
    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render command lists
    for (int n = 0; n < cmd_lists_count; n++)
    {
        const ImDrawList* cmd_list = cmd_lists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)cmd_list->vtx_buffer.begin();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (void*)(vtx_buffer+8));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (void*)(vtx_buffer+16));

        int vtx_offset = 0;
        const ImDrawCmd* pcmd_end = cmd_list->commands.end();
        for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
        {
            glScissor((int)pcmd->clip_rect.x, (int)(height - pcmd->clip_rect.w), (int)(pcmd->clip_rect.z - pcmd->clip_rect.x), (int)(pcmd->clip_rect.w - pcmd->clip_rect.y));
            glDrawArrays(GL_TRIANGLES, vtx_offset, pcmd->vtx_count);
            vtx_offset += pcmd->vtx_count;
        }
    }
    glDisable(GL_SCISSOR_TEST);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

static const char* ImImpl_GetClipboardTextFn()
{
    return glfwGetClipboardString(window);
}

static void ImImpl_SetClipboardTextFn(const char* text, const char* text_end)
{
    if (!text_end)
        text_end = text + strlen(text);

    if (*text_end == 0)
    {
        // Already got a zero-terminator at 'text_end', we don't need to add one
        glfwSetClipboardString(window, text);
    }
    else
    {
        // Add a zero-terminator because glfw function doesn't take a size
        char* buf = (char*)malloc(text_end - text + 1);
        memcpy(buf, text, text_end-text);
        buf[text_end-text] = '\0';
        glfwSetClipboardString(window, buf);
        free(buf);
    }
}


// GLFW callbacks to get events
static void glfw_error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = (yoffset != 0.0f) ? yoffset > 0.0f ? 1 : - 1 : 0;           // Mouse wheel: -1,0,+1
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;
    io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
    io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
    if (c > 0 && c <= 255)
        ImGui::GetIO().AddInputCharacter((char)c);
}

// OpenGL code based on http://open.gl tutorials
void InitGL()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        exit(1);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(1280, 720, "Quick Release", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetCharCallback(window, glfw_char_callback);

    glewInit();
}

void InitImGui()
{
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);        // Display size, in pixels. For clamping windows positions.
    io.DeltaTime = 1.0f/60.0f;                          // Time elapsed since last frame, in seconds (in this sample app we'll override this every frame because our timestep is variable)
    io.PixelCenterOffset = 0.0f;                        // Align OpenGL texels
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;             // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = ImImpl_RenderDrawLists;
    io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
    io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;

    // Load font texture
    glGenTextures(1, &fontTex);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    const void* png_data;
    unsigned int png_size;
    ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
    int tex_x, tex_y, tex_comp;
    void* tex_data = stbi_load_from_memory((const unsigned char*)png_data, (int)png_size, &tex_x, &tex_y, &tex_comp, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_x, tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);
    stbi_image_free(tex_data);
}

void UpdateImGui()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup timestep
    static double time = 0.0f;
    const double current_time =  glfwGetTime();
    io.DeltaTime = (float)(current_time - time);
    time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);                           // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    io.MouseDown[0] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != 0;
    io.MouseDown[1] = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != 0;

    // Start the frame
    ImGui::NewFrame();
}

class Rect {
  public:
  Rect(unsigned int width, unsigned int height):_width(width), _height(height){}

  unsigned int width() {return _width;}
  unsigned int height() {return _height;}

  float ratio() const {return (float)_width/_height;}
  bool landscape()  const{return ratio() >= 1.0; }
  private:
    const unsigned int _width;
    const unsigned int _height;

};

typedef struct {
	float exposure;
	float shadows;
  float highlights;
  float contrast;
} Parameters;

class Geometry {
  public:

    Geometry(InputImage &image, const Rect &screen): inputImage(image), screen(screen) {
      shaderProgramId = LoadShaders("vertex.vertexshader", "fragment.fragmentshader");
      exposureUniform = glGetUniformLocation(shaderProgramId,"exposure");
      shadowsUniform = glGetUniformLocation(shaderProgramId,"shadows");
      highlightsUniform = glGetUniformLocation(shaderProgramId,"highlights");
      contrastUniform = glGetUniformLocation(shaderProgramId,"contrast");

      // Get a handle for our buffers
      vertexPosition_modelspaceID = glGetAttribLocation(shaderProgramId, "vertexPosition_modelspace");
      vertexUVID = glGetAttribLocation(shaderProgramId, "vertexUV");

      // An array of 3 vectors which represents 3 vertices
      float r = screen.ratio() / image.ratio(); //(1.0 / image.ratio()) *0.5;
      static const GLfloat g_vertex_buffer_data[] = {
       -1.0f, 1.0f * r, 0.0f,
       -1.0f,-1.0f * r, 0.0f,
        1.0f,-1.0f * r, 0.0f,
        1.0f, 1.0f * r, 0.0f
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

    void draw(Parameters *parameters) {
      // Use our shader
      glUseProgram(shaderProgramId);

      glUniform1f(exposureUniform, parameters->exposure);
      glUniform1f(shadowsUniform, parameters->shadows);
      glUniform1f(highlightsUniform, parameters->highlights);
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

    void writeToDisk(string &path, Parameters *parameters, unsigned int width, unsigned int height) {
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

      unsigned char *pixel_data = new unsigned char [width * height * 3];
      glReadPixels(0,0,width,height, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);

      stbi_write_png(path.c_str(), width, height, 3, pixel_data, 0);
      free(pixel_data);
      // Render to display frame buffer again and free resources
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      GLuint deleteBuffers[] = {FramebufferName};
      glDeleteFramebuffers(1, deleteBuffers);
      GLuint deleteTexture[] = {renderedTexture};
      glDeleteTextures(1, deleteTexture);
      GLuint deleteBuffer[] = {depthrenderbuffer};
      glDeleteRenderbuffers(1, deleteBuffer);
    }

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

#include <limits.h>     /* for PATH_MAX */
#include <unistd.h>	/* for getdtablesize(), getcwd() declarations */

class DirTree;

DirTree *currentDirTree;
extern int ftwCallback(const char *file, const struct stat *sb, int flag, struct FTW *s);

class DirTreeNode {
  public:
  DirTreeNode(const std::string &name, DirTreeNode *parent, int level) : name(name), parent(parent), level(level) {}

  DirTreeNode *add(const char *name, int level) {
    std::cout << "adding " << name << " to " << this->name << std::endl;
    std::string fileName = name;
    DirTreeNode *child = new DirTreeNode(fileName, this, level);
    children.push_back(child);
    return child;
  }

  const std::string fullPath() {
    auto prefix = parent ? parent->fullPath() : "";
    return  + "/" + name;
  }

  void inspect() {
    printf("%*s", level , "");	/* indent over */
    std::cout << fullPath() << "(" << children.size() << ")" << std::endl;
    for (auto child : children) {
      child->inspect();
    }
  }

  void mkImgui() {
    if (children.size()) {
        ImGui::TreeNode(name.c_str());
        for (auto child : children) {
          child->mkImgui();
        }
        ImGui::TreePop();
    } else {
        ImGui::Text(name.c_str()); ImGui::NextColumn();
    }
  }

  std::string name;
  DirTreeNode *parent;
  int level;
  std::vector<DirTreeNode *> children;
};

class DirTree {
  public:
  DirTree(const std::string &rootName) {
    std::cout << "AAAAAAAAAA" << std::endl;
    currentDirTree = this;
    int flags = FTW_PHYS;
    currentLvl = 0;
    root = new DirTreeNode(rootName, 0, 0);
    currentParent = root;
    lastNode = root;
    int res = nftw(rootName.c_str(), ftwCallback,4, flags);
  }

  void inspect() {
    root->inspect();
  }

  void add(const char* name, int level) {
    if (level == 0) return;
    if (level == lastNode->level) {
      lastNode = lastNode->parent->add(name, level);
    } else if (level > lastNode->level) {
      lastNode = lastNode->add(name, level);
    } else if (level < lastNode->level) {
      int steps = lastNode->level - level + 1;
      DirTreeNode *p = lastNode;
      for (int i = 0; i < steps; ++i) {
        p = p->parent;
      }

      if (!p) {std::cout << "current parent nil" << std::endl; exit(0);}
      lastNode = p->add(name, level);
    }

    /*
    if (level > currentLvl) {
      currentParent = lastNode;
    } else if (level < currentLvl) {
      if (!currentParent) {std::cout << "current parent nil(pop)" << std::endl; exit(0);}
      currentParent = currentParent->parent;
    }
    if (!currentParent) {std::cout << "current parent nil" << std::endl; exit(0);}
    lastNode = currentParent->add(name);
    currentLvl = level;
    */
  };

  void imgui() {
    root->mkImgui();
  }

  private:
  int currentLvl;
  DirTreeNode *root;
  DirTreeNode *currentParent;
  DirTreeNode *lastNode;
};

int ftwCallback(const char *file, const struct stat *sb, int flag, struct FTW *s) {
	int retval = 0;
	const char *name = file + s->base;

	//printf("%*s", s->level , "");	/* indent over */

  if (flag == FTW_D) {
		printf("%s (directory)%i\n", name, s->level);
    currentDirTree->add(name, s->level);
  }
  return 0;
}


// Application code
int main(int argc, char** argv)
{
  Config config(".quickrelease.cfg");
  InitGL();
  Parameters parameters = { .exposure = 1.0,
                            .shadows = 0.0,
                            .highlights = 0.0,
                            .contrast = 0.0};
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  const Rect screen(w, h);


  //InputImage tesla("stone.jpg");
  //Geometry g(tesla, screen);
  InitImGui();
  bool save_now = false;
    DirTree d("./");

  while (!glfwWindowShouldClose(window))
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel = 0;
        glfwPollEvents();
        UpdateImGui();

        static bool show_test_window = true;
        static bool show_debug_window = false;
        static bool show_controls = false;
        static float f;
        d.imgui();
        if (show_debug_window) {
          // Create a simple window
          // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
          ImGui::Text("Hello, world!");
          ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
          show_test_window ^= ImGui::Button("Test Window");
          show_controls ^= ImGui::Button("Another Window");

          // Calculate and show framerate
          static float ms_per_frame[120] = { 0 };
          static int ms_per_frame_idx = 0;
          static float ms_per_frame_accum = 0.0f;
          ms_per_frame_accum -= ms_per_frame[ms_per_frame_idx];
          ms_per_frame[ms_per_frame_idx] = ImGui::GetIO().DeltaTime * 1000.0f;
          ms_per_frame_accum += ms_per_frame[ms_per_frame_idx];
          ms_per_frame_idx = (ms_per_frame_idx + 1) % 120;
          const float ms_per_frame_avg = ms_per_frame_accum / 120;
          ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms_per_frame_avg, 1000.0f / ms_per_frame_avg);
        }

        // Show the ImGui test window
        // Most of user example code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNewWindowDefaultPos(ImVec2(650, 20));        // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowTestWindow(&show_test_window);
        }
        // Show another simple window
        if (show_controls)
        {
            const ImGuiWindowFlags layout_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ;
            ImGui::SetNewWindowDefaultPos(ImVec2(0,0));
            ImGui::Begin("Adjustments", &show_controls, ImVec2(200,500), 1.0, layout_flags);
            ImGui::Text("Exposure");
            ImGui::SliderFloat("f0", &parameters.exposure, 0.0f,2.0f);
            ImGui::Text("Shadows");
            ImGui::SliderFloat("f1", &parameters.shadows, 0.0f,1.0f);
            ImGui::Text("Highlights");
            ImGui::SliderFloat("f2", &parameters.highlights, 0.0f,1.0f);
            ImGui::Text("Contrast");
            ImGui::SliderFloat("f3", &parameters.contrast, 0.0f,1.0f);
            std::string fout("out.png");
            //if (ImGui::Button("Save")) {g.writeToDisk(fout, &parameters, 1280,1024);}
            ImGui::End();
        }

        // Rendering
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //g.draw(&parameters);
        ImGui::Render();
        glfwSwapBuffers(window);
    }
    ImGui::Shutdown();
    glfwTerminate();
    config.save();
    d.inspect();
    return 0;
}
