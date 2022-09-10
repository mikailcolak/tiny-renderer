#include "geometry.hpp"
#include "model.hpp"
#include "tgaimage.hpp"

// std
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <sched.h>
#include <tuple>
#include <vector>
#include <random>

const uint32_t white   {0xFF'FF'FF'FF};
const uint32_t red     {0xFF'00'00'FF};
const uint32_t green   {0x00'FF'00'FF};
const uint32_t blue    {0x00'00'FF'FF};
const uint32_t grey    {0x7F'7F'7F'FF};
const uint32_t darkgrey{0x40'40'40'FF};
const uint32_t black   {0x00'00'00'FF};

constexpr size_t width = 800;
constexpr size_t height = 600;
constexpr size_t width_half = width / 2;
constexpr size_t height_half = height / 2;
constexpr float_t aspect_ratio = width < height ? height / float(width) : width / float(height);

typedef std::array<Vec2i, 3> triangle_i32;

void triangle(triangle_i32 triangle, TGAImage& image, TGAColor color) {
  auto& [t0, t1, t2] = triangle;
  image
    .line(t0, t1, color)
    .line(t1, t2, color)
    .line(t2, t0, color);
}

std::tuple<Vec2i, Vec2i> triangle_boundary(const Vec2i& t0, const Vec2i& t1, const Vec2i& t2) {
  constexpr size_t w = width;
  constexpr size_t h = height;

  Vec2i min{ w, h };
  min.x = min.x < t0.x ? min.x : t0.x;
  min.x = min.x < t1.x ? min.x : t1.x;
  min.x = min.x < t2.x ? min.x : t2.x;
  min.x = min.x <    0 ?     0 : min.x;

  min.y = min.y < t0.y ? min.y : t0.y;
  min.y = min.y < t1.y ? min.y : t1.y;
  min.y = min.y < t2.y ? min.y : t2.y;
  min.y = min.y <    0 ?     0 : min.y;

  Vec2i max{ 0, 0 };
  max.x = max.x > t0.x ? max.x : t0.x;
  max.x = max.x > t1.x ? max.x : t1.x;
  max.x = max.x > t2.x ? max.x : t2.x;
  max.x = max.x >    w ?     w : max.x;

  max.y = max.y > t0.y ? max.y : t0.y;
  max.y = max.y > t1.y ? max.y : t1.y;
  max.y = max.y > t2.y ? max.y : t2.y;
  max.y = max.y >    h ?     h : max.y;

  return std::make_tuple(min, max);
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates

Vec2i barycentric_point_by_weights(triangle_i32 triangle, float_t u, float_t v) {
  const auto& [A, B, C] = triangle;
  return A + u * (B - A) + v * (C - A);
}

Vec2i barycentric_point_by_weights(triangle_i32 triangle, float_t u, float_t v, float_t w) {
  const auto& [A, B, C] = triangle;
  return u * A + v * B + w * C;
}

const Vec2f barycentric_weights_of_a_point(triangle_i32 triangle, Vec2i P) {
  const auto& [A, B, C] = triangle;

  float_t u = float_t(A.x * (C.y - A.y) + (P.y - A.y) * (C.x - A.x) - P.x * (C.y - A.y))
            / float_t((B.y - A.y) * (C.x - A.x) - (B.x - A.x) * (C.y - A.y));
  float_t v = float_t(P.y - A.y - u * (B.y - A.y))
            / float_t(C.y - A.y);
  return std::move(Vec2f{u, v});
}

Vec3f barycentric(triangle_i32 triangle, Vec2i P) { 
  const Vec2i A = triangle[0];
  const Vec2i B = triangle[1];
  const Vec2i C = triangle[2];
  
  const auto [u, v, w] = (Vec3f(
    C.x-A.x,
    B.x-A.x,
    A.x-P.x
  ) ^ Vec3f(
    C.y-A.y,
    B.y-A.y,
    A.y-P[1]
  )).raw;
  /* `pts` and `P` has integer value as coordinates
      so `abs(u[2])` < 1 means `u[2]` is 0, that means
      triangle is degenerate, in this case return something with negative coordinates */
  if (std::abs(w)<1) return Vec3f(-1,1,1);
  return Vec3f(1.f-(u+v)/w, v/w, u/w); 
} 

void raster_triangle(triangle_i32 triangle, TGAImage& image, const TGAColor &color) {
  std::sort(triangle.begin(), triangle.end(), [](Vec2i& a, Vec2i& b) { return a.y > b.y; });
  auto& [a, b, c] = triangle;

  auto [min, max] = triangle_boundary(triangle[0], triangle[1], triangle[2]);

  for (int y = min.y; y <= max.y; ++y) {
    for (int x = min.x; x <= max.x; ++x) {
      // auto [u, v] = barycentric_weights_of_a_point(triangle, Vec2i{x, y}).raw;
      // if (u + v > 1 || u < 0 || v < 0) continue;
      // image.set(x, y, color);

      auto [u, v, w] = barycentric(triangle, Vec2i{x, y}).raw;
      if (u < 0 || v < 0 || w < 0) continue;
      image.set(x, y, color);
    }
  }
}

void model_rendering(TGAImage& image) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);

  // TGAColor modelColor = TGAColor(dist(mt));
  const TGAColor model_color = white;

  Model model{"./assets/african_head.obj"};

  Vec3f light_dir{0, 0, -1};

  for (int i = 0; i < model.nfaces(); i++) {
    std::vector<int> face = model.face(i);
    std::array<Vec3f, 3> vertices = { model.vert(face[0]), model.vert(face[1]), model.vert(face[2]) };

    auto& [a, b, c] = vertices;

    auto u = c - a;
    auto v = b - a;

    //normalized normal = cross product of u and v (u x v)
    auto face_normal = Vec3f{
      u.y * v.z - u.z * v.y,
      u.z * v.x - u.x * v.z,
      u.x * v.y - u.y * v.x
    }.normalize();
    // auto face_normal = (u ^ v).normalized();

    const float_t light_intensity = face_normal * light_dir;

    if (light_intensity <= 0) continue;

    TGAColor color { model_color * light_intensity };
    color.a = 255;
    
    triangle_i32 screen_vertices;
    std::transform(vertices.begin(), vertices.end(), screen_vertices.begin(), [](auto& x){
        return Vec2i{
          int32_t((x.x / aspect_ratio + 1.0f) * width_half),
          int32_t((x.y + 1.0f) * height_half)
        };
    });

    raster_triangle(screen_vertices, image, color);
    triangle(screen_vertices, image, white);
  }
}

void triangle_rendering(TGAImage& image) {
  const triangle_i32 t0{ Vec2i(0, 0), Vec2i(0, height), Vec2i(width/2, height/2) };
  const triangle_i32 t1{ Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180) };
  const triangle_i32 t2{ Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
  raster_triangle(t0, image, red);
  raster_triangle(t1, image, green);
  raster_triangle(t2, image, blue);

  const triangle_i32 test{Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)};
  raster_triangle(test, image, white);
}

int main(int, char**) {

  TGAImage image(width, height, TGAImage::RGB);
  
  triangle_rendering(image);
  model_rendering(image);

  image.flip_vertically(); // i want to have the origin at the left bottom
  image.write_tga_file("wireframe.tga");

  return EXIT_SUCCESS;
}