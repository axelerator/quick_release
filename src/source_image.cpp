#include "source_image.h"

#include <cassert>

#include "source_dir.h"
#include "config.h"

SourceImage::SourceImage(const SourceDir *parent, const std::string &filename) : parent(parent), filename(filename), imageData(0) {
  assert(parent);
}

SourceImage::SourceImage(const SourceDir *parent, const json11::Json &jsonConfig) : parent(parent) {
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

Geometry *SourceImage::geometry() {
  if (!_geometry) {
    data();
    _geometry = new Geometry(*imageData, parent->config->screen);
  }
  return _geometry;
}

const std::string SourceImage::fullPath() {
  return parent->path + filename;
}

