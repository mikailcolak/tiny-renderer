#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <bit>
#include <iostream>

constexpr uint32_t RGBA2BGRA(const uint32_t color) {
  if constexpr (std::endian::native == std::endian::big){
    return color; // TODO..
  } else {
    return ((color & 0x00'00'00'FF) << 24) | ((color & 0xFF'FF'FF'00) >> 8);;
  }
} 

constexpr uint32_t WHITE     {RGBA2BGRA(0xFF'FF'FF'FF)};
constexpr uint32_t RED       {RGBA2BGRA(0xFF'00'00'FF)};
constexpr uint32_t GREEN     {RGBA2BGRA(0x00'FF'00'FF)};
constexpr uint32_t BLUE      {RGBA2BGRA(0x00'00'FF'FF)};
constexpr uint32_t YELLOW    {RGBA2BGRA(0xFF'FF'00'FF)};
constexpr uint32_t GREY      {RGBA2BGRA(0x7F'7F'7F'FF)};
constexpr uint32_t DARK_GREY {RGBA2BGRA(0x40'40'40'FF)};
constexpr uint32_t BLACK     {RGBA2BGRA(0x00'00'00'FF)};
constexpr uint32_t MAGENTA   {RGBA2BGRA(0xFF'00'FF'FF)};



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

  TGAColor(const uint32_t v, int bpp)
    : val(v), bytespp(bpp) {
  }

  TGAColor(const uint32_t bgra)
    : val(bgra), bytespp(4) {
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

  const TGAColor operator+(const TGAColor& c) const {
    return std::move(TGAColor{
      (uint8_t)std::min(b + c.b, 0xFF),
      (uint8_t)std::min(g + c.g, 0xFF),
      (uint8_t)std::min(r + c.r, 0xFF),
      (uint8_t)std::min(a + c.a, 0xFF),
    });
  }

  const TGAColor operator*(const TGAColor& c) const {
    return std::move(TGAColor{
      (uint8_t)std::min(b * c.b, 0xFF),
      (uint8_t)std::min(g * c.g, 0xFF),
      (uint8_t)std::min(r * c.r, 0xFF),
      (uint8_t)std::min(a * c.a, 0xFF),
    });
  }

  const TGAColor operator*(const float& s) const {
    return std::move(TGAColor{
      std::min(uint8_t(b * s), uint8_t(0xFF)),
      std::min(uint8_t(g * s), uint8_t(0xFF)),
      std::min(uint8_t(r * s), uint8_t(0xFF)),
      std::min(uint8_t(a * s), uint8_t(0xFF)),
    });
  }

  TGAColor operator*=(const float& s) {
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