#pragma once

#include "glm/geometric.hpp"
#include "tgaimage.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <tuple>

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
  constexpr float_t minf = std::numeric_limits<float_t>::min();
  constexpr float_t maxf = std::numeric_limits<float_t>::max();
  glm::vec2 min{ maxf, maxf };
  glm::vec2 max{ minf, minf };

  for (size_t i = 0; i < SIZE; ++i) {
    min.x = std::min(clamp_max.x, std::min(min.x, points[i].x));
    min.y = std::min(clamp_max.y, std::min(min.y, points[i].y));
    max.x = std::max(clamp_min.x, std::max(max.x, points[i].x));
    max.y = std::max(clamp_min.y, std::max(max.y, points[i].y));
  }

  return std::make_tuple(min, max);
}

template<size_t SIZE>
inline std::tuple<glm::vec2, glm::vec2> bbox(const std::array<glm::vec3, SIZE> points, glm::vec2 clamp_min, glm::vec2 clamp_max) {
  constexpr float_t minf = std::numeric_limits<float_t>::min();
  constexpr float_t maxf = std::numeric_limits<float_t>::max();
  glm::vec2 min{ maxf, maxf };
  glm::vec2 max{ minf, minf };

  for (size_t i = 0; i < SIZE; ++i) {
    min.x = std::min(clamp_max.x, std::min(min.x, points[i].x));
    min.y = std::min(clamp_max.y, std::min(min.y, points[i].y));
    max.x = std::max(clamp_min.x, std::max(max.x, points[i].x));
    max.y = std::max(clamp_min.y, std::max(max.y, points[i].y));
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
  glm::vec3 u = cross(s[0], s[1]);
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

inline void raster_triangle_with_depth_buffer(std::array<glm::vec3, 3> triangle, std::vector<int32_t>& depth_buffer, TGAImage& image, const TGAColor& color) {
  auto& [a, b, c] = triangle;

  const glm::vec2 clamp_min{ 0, 0 };
  const glm::vec2 clamp_max{ float_t(image.get_width()), float_t(image.get_height()) };
  auto [min, max] = bbox(triangle, clamp_min, clamp_max);

  float_t z = 0;
  for (float_t y = min.y; y <= max.y; ++y) {
    for (float_t x = min.x; x <= max.x; ++x) {
      auto bc = barycentric(triangle, glm::vec3{ x, y, z });
      
      // barycentric U/X screen coordinate
      const auto usc = bc.x;
      // barycentric V/Y screen coordinate
      const auto vsc = bc.y;
      // barycentric W/Z screen coordinate
      const auto wsc = bc.z;

      if (usc < 0 || vsc < 0 || wsc < 0)
        continue;

      z = a.z * usc + b.z * vsc + c.z * wsc;
      size_t depth_index = size_t(x * y / image.get_width());
      if (depth_buffer[depth_index] < z) {
        depth_buffer[depth_index] = z;
        image.set(x, y, color);
      }
    }
  }
}

inline void ssloy_triangle_with_depth_buffer(std::array<glm::vec3, 3> pts, std::vector<int32_t>& zbuffer, TGAImage& image, const TGAColor& color) {
  const glm::vec2 clamp_min{ 0, 0 };
  const glm::vec2 clamp_max{ float_t(image.get_width()), float_t(image.get_height()) };
  auto [bboxmin, bboxmax] = bbox(pts, clamp_min, clamp_max);

  glm::vec3 P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
      glm::vec3 bc_screen = barycentric(pts, P);

      if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

      P.z = 0;

      for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
      if (zbuffer[int(P.x + P.y * image.get_width())] < P.z) {
        zbuffer[int(P.x + P.y * image.get_width())] = P.z;
        image.set(P.x, P.y, color);
      }
    }
  }
}