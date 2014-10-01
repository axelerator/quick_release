#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include "source_dir.h"

class Config {
  public:
  Config(const std::string path);
  void load();
  void save();

  void addDirectory(const std::string &path);

  void activate(SourceDir &newDir);

  const std::string path;
  std::vector<std::string> directories;
  std::vector<SourceDir> sourceDirs;
  SourceDir *current;
};
#endif
