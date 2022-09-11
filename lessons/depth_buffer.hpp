#pragma once

#include "../tgaimage.hpp"
#include "../model.hpp"
#include "../rasterization.hpp"
#include "glm/geometric.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <vector>

typedef std::array<glm::vec2, 2> line2d_i32;

inline void rasterize_1d(line2d_i32 line, TGAImage& image, TGAColor color, std::vector<int32_t>& y_buffer) {
  if (line[0].x > line[1].x) {
      std::reverse(line.begin(), line.end());
  }
  
  auto [a, b] = line;

  for (int32_t x = a.x; x <= b.x; ++x) {
    float_t t = inverse_lerp(float(a.x), float(b.x), float(x));
    int32_t y = lerp(float(a.y), float(b.y), t);

    if (y_buffer[x] < y) {
      y_buffer[x] = y;
      
      uint8_t yc = t * y / 4;
      // Resterized y depth buffer
      image.set(x, 9, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, 8, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, 7, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, 6, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, 5, TGAColor(yc, yc, yc, 0xFF));

      // Rasterized image
      image.set(x, 4, color);
      image.set(x, 3, color);
      image.set(x, 2, color);
      image.set(x, 1, color);
      image.set(x, 0, color);
    }
  }
}

inline void depth_buffer_1(TGAImage& image) {

  // Scaling ratio x
  const float_t srx = image.get_width() < 800 ? image.get_width() / 800.0f : 1.0f;
  // Scaling ratio y
  const float_t sry = image.get_height() < 600 ? image.get_height() / 600.0f : 1.0f;
  // Scaling ratio uniform
  const float_t sru = std::min(srx, sry);

  const line2d_i32 lineA{ glm::vec2( 20,  34) * sru, glm::vec2(744, 400) * sru };
  const line2d_i32 lineB{ glm::vec2(120, 434) * sru, glm::vec2(444, 400) * sru };
  const line2d_i32 lineC{ glm::vec2(330, 463) * sru, glm::vec2(594, 200) * sru };

  // scene "2d mesh"
  image
    .line(lineA[0], lineA[1], RED)
    .line(lineB[0], lineB[1], GREEN)
    .line(lineC[0], lineC[1], BLUE);

  // screen line
  image.line(glm::vec2(0, 10), glm::vec2(image.get_width(), 10), WHITE);

  // screen depth buffer line
  std::vector<int32_t> y_buffer(image.get_width(), std::numeric_limits<int32_t>::min());

  int i = 0;
  std::for_each(y_buffer.begin(), y_buffer.end(), [&image, &i](auto val){
    if (val == std::numeric_limits<int32_t>::min()) {
      image.set(i, 9, MAGENTA);
      image.set(i, 8, MAGENTA);
      image.set(i, 7, MAGENTA);
      image.set(i, 6, MAGENTA);
      image.set(i, 5, MAGENTA);
      ++i;
    };
  });

  rasterize_1d(lineA, image, RED, y_buffer);
  rasterize_1d(lineB, image, GREEN, y_buffer);
  rasterize_1d(lineC, image, BLUE, y_buffer);
}

inline glm::vec3 world2screen(glm::vec3 v, size_t width, size_t height) {
  const size_t width_half = width / 2;
  const size_t height_half = height / 2;
  const float_t aspect_ratio = width < height ? height / float(width) : width / float(height);
  return glm::vec3(int((v.x * aspect_ratio +1.)*width_half+.5), int((v.y+1.)*width_half+.5), v.z);
}

inline void depth_buffer_2(TGAImage& image) {
  const size_t width = image.get_width();
  const size_t height = image.get_height();
  const TGAColor model_color = WHITE;

  Model model{ "./assets/african_head.obj" };

  glm::vec3 light_dir{ 0, 0, -1 };

  for (int i = 0; i < model.nfaces(); i++) {
    std::vector<int> face = model.face(i);
    std::array<glm::vec3, 3> vertices = { model.vert(face[0]), model.vert(face[1]), model.vert(face[2]) };

    auto& [a, b, c] = vertices;

    auto u = c - a;
    auto v = b - a;

    // normalized normal = cross product of u and v (u x v)
    auto face_normal = glm::normalize(glm::vec3{
      u.y * v.z - u.z * v.y,
      u.z * v.x - u.x * v.z,
      u.x * v.y - u.y * v.x
    });
    // auto face_normal = (u ^ v).normalized();

    const float_t light_intensity = glm::dot(face_normal, light_dir);

    if (light_intensity <= 0)
      continue;

    TGAColor color{ model_color * light_intensity };
    color.a = 255;

    std::array<glm::vec3, 3> screen_vertices;
    std::transform(vertices.begin(), vertices.end(), screen_vertices.begin(), [&](auto& x) {
      return world2screen(x, width, height);
    });

    // screen depth buffer line
    std::vector<int32_t> z_buffer(image.get_width() * image.get_height(), -std::numeric_limits<float>::max());

    raster_triangle_with_depth_buffer(screen_vertices, z_buffer, image, color);
    //ssloy_triangle_with_depth_buffer(screen_vertices, z_buffer, image, color);
  }
}