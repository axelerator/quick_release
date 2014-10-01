#ifndef SOURCE_DIR_H
#define SOURCE_DIR_H

#include <string>
#include "../include/imgui/imgui.h"

#include "source_image.h"

class SourceDir {
  public:

    SourceDir(const std::string &path);

    void activate();
    void imgui(ImGuiWindowFlags layout_flags);
    void add(const char* filename);
    const std::string config_file_path();
    void load();
    void save();

    const std::string path;
    bool loaded;
    std::vector<SourceImage> sourceImages;
};

#endif

