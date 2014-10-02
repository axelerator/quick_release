#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include "source_dir.h"
#include "rect.h"

class Config {
  public:
  Config(const std::string path, const Rect &screen);
  void load();
  void save();

  void addDirectory(const std::string &path);

  void activate(SourceDir &newDir);

  const std::string path;
  const Rect &screen;
  std::vector<std::string> directories;
  std::vector<SourceDir> sourceDirs;
  SourceDir *current;
};
#endif
