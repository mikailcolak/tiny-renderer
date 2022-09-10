#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>

struct TGAColor {
  union {
    struct {
      unsigned char b, g, r, a;
    };
    unsigned char raw[4];
    unsigned int val;
  };
  int bytespp;

  TGAColor()
    : val(0), bytespp(1) {
  }

  TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
    : b(B), g(G), r(R), a(A), bytespp(4) {
  }

  TGAColor(const uint32_t &v, int bpp)
    : val(v), bytespp(bpp) {
  }

  TGAColor(const uint32_t rgba)
    : b((rgba & 0xFF000000) >> 24), g((rgba & 0x00FF0000) >> 16), r((rgba & 0x0000FF00) >> 8), a(rgba & 0x000000FF), bytespp(4) {
  }

  TGAColor(const TGAColor& c)
    : val(c.val), bytespp(c.bytespp) {
  }

  TGAColor(const unsigned char* p, int bpp)
    : val(0), bytespp(bpp) {
    for (int i = 0; i < bpp; i++) {
      raw[i] = p[i];
    }
  }

  TGAColor& operator=(const TGAColor& c) {
    if (this != &c) {
      bytespp = c.bytespp;
      val = c.val;
    }
    return *this;
  }

  TGAColor operator+(const TGAColor& c) {
	return {
        (uint8_t)std::min(b + c.b, 0xFF),
        (uint8_t)std::min(g + c.g, 0xFF),
        (uint8_t)std::min(r + c.r, 0xFF),
        (uint8_t)std::min(a + c.a, 0xFF),
    };
  }

  TGAColor operator*(const TGAColor& c) {
	return {
        (uint8_t)std::min(b * c.b, 0xFF),
        (uint8_t)std::min(g * c.g, 0xFF),
        (uint8_t)std::min(r * c.r, 0xFF),
        (uint8_t)std::min(a * c.a, 0xFF),
    };
  }

  TGAColor operator*(const float& s) {
	return {
        std::min(uint8_t(b * s), uint8_t(0xFF)),
        std::min(uint8_t(g * s), uint8_t(0xFF)),
        std::min(uint8_t(r * s), uint8_t(0xFF)),
        std::min(uint8_t(a * s), uint8_t(0xFF)),
    };
  }

  TGAColor& operator*=(const float& s) {
    this->b = std::min(uint8_t(b * s), uint8_t(0xFF));
    this->g = std::min(uint8_t(g * s), uint8_t(0xFF));
    this->r = std::min(uint8_t(r * s), uint8_t(0xFF));
    this->a = std::min(uint8_t(a * s), uint8_t(0xFF));
    return *this;
  }

  TGAColor& operator+=(const float& s) {
    this->b = std::min(uint8_t(b + s), uint8_t(0xFF));
    this->g = std::min(uint8_t(g + s), uint8_t(0xFF));
    this->r = std::min(uint8_t(r + s), uint8_t(0xFF));
    this->a = std::min(uint8_t(a + s), uint8_t(0xFF));
    return *this;
  }
};