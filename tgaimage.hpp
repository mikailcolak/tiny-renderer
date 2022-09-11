#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "tga_color.hpp"
#include "tga_header.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <fstream>

class TGAImage {
protected:
  unsigned char* data;
  int width;
  int height;
  int bytespp;

  bool load_rle_data(std::ifstream& in);
  bool unload_rle_data(std::ofstream& out);

public:
  enum Format {
    GRAYSCALE = 1,
    RGB = 3,
    RGBA = 4
  };

  TGAImage();
  TGAImage(int w, int h, int bpp);
  TGAImage(const TGAImage& img);
  bool read_tga_file(const char* filename);
  bool write_tga_file(const char* filename, bool rle = true);
  bool flip_horizontally();
  bool flip_vertically();
  bool scale(int w, int h);
  TGAColor get(int x, int y);
  bool set(int x, int y, TGAColor c);
  ~TGAImage();
  TGAImage& operator=(const TGAImage& img);
  int get_width();
  int get_height();
  int get_bytespp();
  unsigned char* buffer();
  void clear();

  TGAImage& line(int x0, int y0, int x1, int y1, const TGAColor color);
  TGAImage& line(glm::vec2 a, glm::vec2 b, const TGAColor color);
};

#endif //__IMAGE_H__