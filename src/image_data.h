#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <string>
#include "rect.h"


class ImageData {
  public:
    ImageData(const Rect& size);
    ~ImageData();
    void writeToDisk(const std::string &filename);

    unsigned char *data;
    const Rect size;

};
#endif
