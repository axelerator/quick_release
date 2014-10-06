#ifndef SOURCE_DIR_H
#define SOURCE_DIR_H

#include <string>
#include "../include/imgui/imgui.h"

#include "source_image.h"

class Config;

class SourceDir {
  public:

    SourceDir(const std::string &path, Config *config);

    void activate();
    void imgui(ImGuiWindowFlags layout_flags, SourceImage **currentImage);
    void add(const char* filename);
    const std::string config_file_path();
    void load();
    void save();
    SourceImage *currentImage();
    void generatePreviews();

    const std::string path;
    bool loaded;
    std::vector<SourceImage> sourceImages;
    Config *config;

  private:
    char* readFile(FILE *handler);
};

#endif

