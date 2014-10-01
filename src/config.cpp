#include "config.h"

#include <fstream>
#include <iostream>


#include "../include/json11/json11.hpp"
#include "../include/imgui/imgui.h"

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

Config::Config(const std::string path) : path(path) {};

void Config::load() {
  FILE *handler = fopen(path.c_str(),"r");
  if (handler) {
    char *configString = readFile(handler);
    std::string err;
    json11::Json configJson = json11::Json::parse(configString, err);
    std::cout << configString << std::endl;
    for (auto &k : configJson["directories"].array_items()) {
      directories.push_back(k.string_value());
    }
  } else {

  }
  for (auto &dir_name : directories) {
    sourceDirs.push_back(SourceDir(dir_name));
  }
}

void Config::save() {
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

void Config::addDirectory(const std::string &path) {
  directories.push_back(path);
}

void Config::activate(SourceDir &newDir) {
  newDir.activate();
  current = &newDir;
}

