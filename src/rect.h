class Rect {
  public:
  Rect(unsigned int width, unsigned int height):_width(width), _height(height){}

  unsigned int width() {return _width;}
  unsigned int height() {return _height;}

  float ratio() const {return (float)_width/_height;}
  bool landscape()  const{return ratio() >= 1.0; }
  private:
    const unsigned int _width;
    const unsigned int _height;
};

