#ifndef RECT_H
#define RECT_H
class Rect {
  public:
  Rect(unsigned int width, unsigned int height):_width(width), _height(height){}

  unsigned int width() const {return _width;}
  unsigned int height() const {return _height;}

  float ratio() const {return (float)_width/_height;}
  bool landscape()  const{return ratio() >= 1.0; }
  Rect operator*(float r) const { return Rect(_width * r, _height * r); }
  Rect scaleToMax(int pixels) {
    float r = (float)pixels / ( landscape() ? _width : _height );
    return *this * r;
  };
  private:
    const unsigned int _width;
    const unsigned int _height;
};
#endif

