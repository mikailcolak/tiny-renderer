#pragma once

#include "glm/geometric.hpp"
#include "tga_color.hpp"
#include "tgaimage.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <type_traits>

template<class T>
inline T lerp(T a, T b, float_t t) {
  return a * (1.0 - t) + b * t;
}

template<class T>
inline T inverse_lerp(T a, T b, T v) {
  return (v - a) / (b - a);
}

inline void triangle(std::array<glm::vec2, 3> triangle, TGAImage& image, TGAColor color) {
  auto& [t0, t1, t2] = triangle;
  image
    .line(t0, t1, color)
    .line(t1, t2, color)
    .line(t2, t0, color);
}

template<size_t SIZE>
inline std::tuple<glm::vec2, glm::vec2> bbox(const std::array<glm::vec2, SIZE> points, glm::vec2 clamp_min, glm::vec2 clamp_max) {
  glm::vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
  glm::vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      min[j] = std::max(clamp_min[j], std::min(min[j], points[i][j]));
      max[j] = std::min(clamp_max[j], std::max(max[j], points[i][j]));
    }
  }
  return std::make_tuple(min, max);
}

template<size_t SIZE>
inline std::tuple<glm::vec2, glm::vec2> bbox(const std::array<glm::vec3, SIZE> points, glm::vec2 clamp_min, glm::vec2 clamp_max) {
  glm::vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
  glm::vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      min[j] = std::max(clamp_min[j], std::min(min[j], points[i][j]));
      max[j] = std::min(clamp_max[j], std::max(max[j], points[i][j]));
    }
  }
  return std::make_tuple(min, max);
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates

inline glm::vec2 barycentric_point_by_weights(std::array<glm::vec2, 3> triangle, float_t u, float_t v) {
  const auto& [A, B, C] = triangle;
  return A + u * (B - A) + v * (C - A);
}

inline glm::vec2 barycentric_point_by_weights(std::array<glm::vec2, 3> triangle, float_t u, float_t v, float_t w) {
  const auto& [A, B, C] = triangle;
  return u * A + v * B + w * C;
}

inline const glm::vec2 barycentric_weights_of_a_point(std::array<glm::vec2, 3> triangle, glm::vec2 P) {
  const auto& [A, B, C] = triangle;

  float_t u = float_t(A.x * (C.y - A.y) + (P.y - A.y) * (C.x - A.x) - P.x * (C.y - A.y)) / float_t((B.y - A.y) * (C.x - A.x) - (B.x - A.x) * (C.y - A.y));
  float_t v = float_t(P.y - A.y - u * (B.y - A.y)) / float_t(C.y - A.y);
  return std::move(glm::vec2{ u, v });
}

inline glm::vec3 barycentric(std::array<glm::vec2, 3> triangle, glm::vec2 P) {
  const auto& [A, B, C] = triangle;

  const auto cp = glm::cross(
    glm::vec3(
      C.x - A.x,
      B.x - A.x,
      A.x - P.x
    ),
    glm::vec3(
      C.y - A.y,
      B.y - A.y,
      A.y - P.y
    )
  );
  const auto u = cp.x;
  const auto v = cp.y;
  const auto w = cp.z;

  /* `pts` and `P` has integer value as coordinates
      so `abs(u[2])` < 1 means `u[2]` is 0, that means
      triangle is degenerate, in this case return something with negative coordinates */
  if (std::abs(w) < 1)
    return glm::vec3(-1, 1, 1);
  return glm::vec3(1.f - (u + v) / w, v / w, u / w);
}

inline glm::vec3 barycentric(std::array<glm::vec3, 3> triangle, glm::vec3 P) {
  const auto& [A, B, C] = triangle;

  glm::vec3 s[2];
  for (int i = 2; i--;) {
    s[i][0] = C[i] - A[i];
    s[i][1] = B[i] - A[i];
    s[i][2] = A[i] - P[i];
  }
  glm::vec3 u = glm::cross(s[0], s[1]);
  if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
    return glm::vec3(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
  return glm::vec3(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

inline void raster_triangle(std::array<glm::vec2, 3> triangle, TGAImage& image, const TGAColor& color) {
  std::sort(triangle.begin(), triangle.end(), [](glm::vec2& a, glm::vec2& b) { return a.y > b.y; });
  auto& [a, b, c] = triangle;

  const glm::vec2 clamp_min{ 0, 0 };
  const glm::vec2 clamp_max{ float_t(image.get_width()), float_t(image.get_height()) };
  auto [min, max] = bbox(triangle, clamp_min, clamp_max);

  for (int32_t y = min.y; y <= max.y; ++y) {
    for (int32_t x = min.x; x <= max.x; ++x) {
      // auto [u, v] = barycentric_weights_of_a_point(triangle, glm::vec2{x, y}).raw;
      // if (u + v > 1 || u < 0 || v < 0) continue;
      // image.set(x, y, color);

      auto bc = barycentric(triangle, glm::vec2{ x, y });
      const auto u = bc.x;
      const auto v = bc.y;
      const auto w = bc.z;

      if (u < 0 || v < 0 || w < 0)
        continue;
      image.set(x, y, color);
    }
  }
}

inline void raster_triangle_with_depth_buffer(
  std::array<glm::vec3, 3>& triangle,
  std::array<glm::vec2, 3>& texcoord,
  std::vector<float_t>& z_buffer,
  TGAImage& image,
  TGAImage& texture,
  float light_intensity
) {
  //std::sort(triangle.begin(), triangle.end(), [](glm::vec3& a, glm::vec3& b) { return a.y > b.y; });
  auto& [a, b, c] = triangle;
  auto& [tex_a, tex_b, tex_c] = texcoord;
  const float_t width = image.get_width();
  const float_t height = image.get_height();

  constexpr glm::vec2 clamp_min{ 0, 0 };
  const glm::vec2 clamp_max{ width, height };
  auto [min, max] = bbox(triangle, clamp_min, clamp_max);

  float_t z = 0;

  for (float_t y = min.y; y <= max.y; ++y) {
    for (float_t x = min.x; x <= max.x; ++x) {
      auto bc = barycentric(triangle, glm::vec3(x, y, z));

      // barycentric U/X screen coordinate
      const auto bcx = bc.x;
      // barycentric V/Y screen coordinate
      const auto bcy = bc.y;
      // barycentric W/Z screen coordinate
      const auto bcz = bc.z;

      if (bcx < 0 || bcy < 0 || bcz < 0)
        continue;

      z = a.z * bcx + b.z * bcy + c.z * bcz;

      int depth_index = size_t(x + y * width);
      if (z_buffer[depth_index] < z) {
        
        z_buffer[depth_index] = z;

        int u = ((tex_a.x * bcx) + (tex_b.x * bcy) + (tex_c.x * bcz)) * texture.get_width();
        int v = ((tex_a.y * bcx) + (tex_b.y * bcy) + (tex_c.y * bcz)) * texture.get_height();
        
        auto color = texture.get(u, v);
        color.r *= light_intensity;
        color.g *= light_intensity;
        color.b *= light_intensity;
        color.a = 255;

        image.set(x, y, color);
      }
    }
  }
}
