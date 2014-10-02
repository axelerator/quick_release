#ifndef SOURCE_IMAGE_H
#define SOURCE_IMAGE_H

#include "../include/json11/json11.hpp"

#include <string>
#include "input_image.h"
#include "geometry.h"

class SourceDir;

class SourceImage {
  public:
    SourceImage(const SourceDir *parent, const std::string &filename);

    SourceImage(const SourceDir *parent, const json11::Json &jsonConfig);
    json11::Json to_json();
    const InputImage &data();
    Geometry *geometry();
    const std::string fullPath();

    const SourceDir *parent;
    std::string filename;
    InputImage *imageData;
    Geometry *_geometry;
};

#endif
