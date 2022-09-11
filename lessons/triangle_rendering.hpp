#pragma once

#include "../tga_color.hpp"
#include "../tgaimage.hpp"
#include "../rasterization.hpp"

inline void triangle_rendering(TGAImage& image) {
  const size_t width = image.get_width();
  const size_t height = image.get_height();

  const std::array<glm::vec2, 3> t0{ glm::vec2(0, 0), glm::vec2(0, height), glm::vec2(width/2, height/2) };
  const std::array<glm::vec2, 3> t1{ glm::vec2(180, 50), glm::vec2(150, 1), glm::vec2(70, 180) };
  const std::array<glm::vec2, 3> t2{ glm::vec2(180, 150), glm::vec2(120, 160), glm::vec2(130, 180) };
  raster_triangle(t0, image, RED);
  raster_triangle(t1, image, GREEN);
  raster_triangle(t2, image, BLUE);

  const std::array<glm::vec2, 3> test{glm::vec2(10,10), glm::vec2(100, 30), glm::vec2(190, 160)};
  raster_triangle(test, image, WHITE);
}
