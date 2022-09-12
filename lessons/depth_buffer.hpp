#pragma once
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc

#include "../tgaimage.hpp"
#include "../model.hpp"
#include "../rasterization.hpp"
#include "obj_loader_helper.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <vector>

using line2d_i32 = std::array<glm::vec2, 2>;

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
      image.set(x, image.get_height() - 4, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, image.get_height() - 3, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, image.get_height() - 2, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, image.get_height() - 1, TGAColor(yc, yc, yc, 0xFF));
      image.set(x, image.get_height() - 0, TGAColor(yc, yc, yc, 0xFF));

      // Rasterized image
      image.set(x, image.get_height() - 9, color);
      image.set(x, image.get_height() - 8, color);
      image.set(x, image.get_height() - 7, color);
      image.set(x, image.get_height() - 6, color);
      image.set(x, image.get_height() - 5, color);
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
  image.line(glm::vec2(0, image.get_height() - 10), glm::vec2(image.get_width(), image.get_height() - 10), WHITE);

  // screen depth buffer line
  std::vector<int32_t> y_buffer(image.get_width(), std::numeric_limits<int32_t>::min());

  int i = 0;
  std::for_each(y_buffer.begin(), y_buffer.end(), [&image, &i](auto val){
    if (val == std::numeric_limits<int32_t>::min()) {
      image.set(image.get_height() - i, image.get_height() - 9, MAGENTA);
      image.set(image.get_height() - i, image.get_height() - 8, MAGENTA);
      image.set(image.get_height() - i, image.get_height() - 7, MAGENTA);
      image.set(image.get_height() - i, image.get_height() - 6, MAGENTA);
      image.set(image.get_height() - i, image.get_height() - 5, MAGENTA);
      ++i;
    };
  });

  rasterize_1d(lineA, image, RED, y_buffer);
  rasterize_1d(lineB, image, GREEN, y_buffer);
  rasterize_1d(lineC, image, BLUE, y_buffer);
}

inline void depth_buffer_2(TGAImage& image) {
  const size_t width = image.get_width();
  const size_t height = image.get_height();
  
  constexpr glm::vec3 light_dir{ 0.3, 0, -1 };
  constexpr float_t ambient_light_contribution = 0.4;

  std::vector<float_t> z_buffer(width * height, -std::numeric_limits<float_t>::max());

  TGAImage texture{};
  texture.read_tga_file("./assets/african_head_diffuse.tga");
  texture.flip_vertically();


  load_model_and_for_each_face("./assets/african_head.obj", [&](auto face_vertices, auto face_normals, auto face_texcoords) -> void {
    // Render it
    std::array<glm::vec3, 3> world_coords = face_vertices;
    std::array<glm::vec3, 3> screen_coords;
    
    auto& [a, b, c] = world_coords;
    auto wcu = c - a;
    auto wcv = b - a;

    // normalized normal = cross product of u and v (u x v)
    auto face_normal = glm::normalize(glm::cross(wcu, wcv));

    const float_t light_intensity = glm::clamp(glm::dot(face_normal, light_dir) + ambient_light_contribution, 0.f, 1.f);

    if (light_intensity <= 0)
      return;

    std::transform(world_coords.begin(), world_coords.end(), screen_coords.begin(), [&](auto& x) { return world_to_screen(x, width, height); });

    raster_triangle_with_depth_buffer(screen_coords, face_texcoords, z_buffer, image, texture, light_intensity);
  });

   { // dump z-buffer (debugging purposes only)
        TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
        for (int i=0; i<width; i++) {
            for (int j=0; j<height; j++) {
                zbimage.set(i, j, TGAColor(int((0.5 + z_buffer[i+j*width]) / 2 * 0xFF), 1));
            }
        }
        zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        zbimage.write_tga_file("zbuffer.tga");
    }
}