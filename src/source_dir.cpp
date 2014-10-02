#include "source_dir.h"

#include <fstream>
#include <iostream>
#include <ftw.h>
#include <sys/stat.h>
#include <limits.h>     /* for PATH_MAX */
#include <unistd.h>	/* for getdtablesize(), getcwd() declarations */

#include "../include/imgui/imgui.h"
#include "../include/json11/json11.hpp"

#include "config.h"
SourceDir *currentSourceDir = 0;

SourceDir::SourceDir(const std::string &path, Config *config) :
  path(path),
  loaded(false),
  config(config) {
}

void SourceDir::activate() {
  if (!loaded) {
    load();
  }
}

void SourceDir::imgui(ImGuiWindowFlags layout_flags) {
    bool t = true;
    ImGui::Begin(path.c_str(), &t, ImVec2(200,500), 1.0, layout_flags);
    for( auto &i : sourceImages) {
        ImGui::Text(i.filename.c_str());
    }
    ImGui::End();
}

void SourceDir::add(const char* filename) {
  sourceImages.push_back(SourceImage(this, std::string(filename)));
}

const std::string SourceDir::config_file_path() {
  return path + ".quick_release/config";
}

int ftwSrcDirCallback(const char *file, const struct stat *sb, int flag, struct FTW *s) {
  int retval = 0;
  const char *name = file + s->base;

  if (flag == FTW_F) {
    currentSourceDir->add(name);
  }
  return 0;
}


char* SourceDir::readFile(FILE *handler) {
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

void SourceDir::load() {
  FILE *handler = fopen(config_file_path().c_str(), "r");
  if (handler) {
    char *configString = readFile(handler);
    std::string err;
    json11::Json configJson = json11::Json::parse(configString, err);

    for (auto &k : configJson["images"].array_items()) {
      sourceImages.push_back(SourceImage(this, k));
    }
  } else {
    int flags = FTW_PHYS;
    currentSourceDir = this;
    int res = nftw(path.c_str(), ftwSrcDirCallback, 1, flags);
    currentSourceDir = 0;
    mkdir((path + ".quick_release").c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    save();
  }
  loaded = true;
}

void SourceDir::save() {
  json11::Json::array jsonImages;
  for (auto &image : sourceImages) {
    jsonImages.push_back(image.to_json());
  }

  json11::Json jsonConfig = json11::Json::object({
      { "images", jsonImages }
      });

  std::string jsonStr = jsonConfig.dump();
  std::ofstream outfile (config_file_path() ,std::ofstream::binary);
  outfile.write (jsonStr.c_str(), jsonStr.length());
  outfile.close();
}

SourceImage *SourceDir::currentImage() {
  if (!loaded) load();
  return &(sourceImages.front());
}

