#include "image_data.h"
#include <stdlib.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb/stb_image_write.h"

ImageData::ImageData(const Rect& size): _size(size) {
  data = new unsigned char [_size.width() * _size.height() * 3];
}

ImageData::~ImageData() {
  free(data);
}

void ImageData::writeToDisk(const std::string &filename) {
  stbi_write_png(filename.c_str(), _size.width(), _size.height(), 3, data, 0);
}

