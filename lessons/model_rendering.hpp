#pragma once

#include "../model.hpp"
#include "../tga_color.hpp"
#include "../tgaimage.hpp"
#include "../rasterization.hpp"
#include "glm/geometric.hpp"

#include <random>

inline void model_rendering(TGAImage& image) {
  const size_t width = image.get_width();
  const size_t height = image.get_height();
  const size_t width_half = width / 2;
  const size_t height_half = height / 2;
  const float_t aspect_ratio = width < height ? height / float(width) : width / float(height);

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  // TGAColor modelColor = TGAColor(dist(mt));
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

    std::array<glm::vec2, 3> screen_vertices;
    std::transform(vertices.begin(), vertices.end(), screen_vertices.begin(), [&](auto& x) {
      return glm::vec2{
        (x.x / aspect_ratio + 1.0f) * width_half,
        (x.y + 1.0f) * height_half
      };
    });

    raster_triangle(screen_vertices, image, color);
    //triangle(screen_vertices, image, WHITE);
  }
}