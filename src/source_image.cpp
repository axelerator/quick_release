#include "source_image.h"

#include "source_dir.h"

SourceImage::SourceImage(const SourceDir *parent, const std::string &filename) : parent(parent), filename(filename), imageData(0) {
}

SourceImage::SourceImage(const json11::Json &jsonConfig) {
  filename = jsonConfig["filename"].string_value();
}

json11::Json SourceImage::to_json() {
  json11::Json jsonConfig = json11::Json::object({
      { "filename", filename }
      });
  return jsonConfig;
}

const InputImage &SourceImage::data() {
  if (!imageData) {
    imageData = new InputImage(fullPath());
  }
  return *imageData;
}

const std::string SourceImage::fullPath() {
  return parent->path + filename;
}

