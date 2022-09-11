#pragma once
#include "glm/common.hpp"
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc

#include "../tgaimage.hpp"
#include "../model.hpp"
#include "../rasterization.hpp"
#include "../tiny_obj_loader.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <tuple>
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

inline glm::vec3 world2screen(glm::vec3 v, size_t width, size_t height) {
  const size_t width_half = width / 2;
  const size_t height_half = height / 2;
  const float_t aspect_ratio = width < height ? height / float(width) : width / float(height);

  return glm::vec3(int((v.x * aspect_ratio +1.)*width_half+.5), int((v.y+1.)*width_half+.5), v.z);
}

inline glm::vec3 screen2world(glm::vec2 v, size_t width, size_t height) {
  const size_t width_half = width / 2;
  const size_t height_half = height / 2;
  const float_t aspect_ratio = width < height ? height / float(width) : width / float(height);

  return glm::vec3(v.x  / width_half, v.y / width_half, 0);
}

inline void depth_buffer_2(TGAImage& image) {
  const size_t width = image.get_width();
  const size_t height = image.get_height();

  TGAImage texture{};
  texture.read_tga_file("./assets/african_head_diffuse.tga");
  texture.flip_vertically();

  float_t ambient_light_contribution = 0.4;

  std::string inputfile = "./assets/african_head.obj";
  tinyobj::ObjReaderConfig reader_config{};
  tinyobj::ObjReader reader{};

  if (!reader.ParseFromFile(inputfile, reader_config)) {
    if (!reader.Error().empty()) {
        std::cerr << "TinyObjReader: " << reader.Error();
    }
    exit(1);
  }

  if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
  }

  glm::vec3 light_dir{ 0.3, 0, -1 };
  std::vector<float_t> z_buffer(width * height, -std::numeric_limits<float_t>::max());

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  auto& materials = reader.GetMaterials();

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      std::array<glm::vec3, 3> face_vertices{};
      std::array<glm::vec3, 3> face_normals{};
      std::array<glm::vec2, 3> face_texcoord{};

      // TODO: Instead of discarding quad face split them into 2 triangular face!!!
      // if (fv > 3) continue;

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
        tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
        tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
        face_vertices[v] = glm::vec3{vx, vy, vz};

        // Check if `normal_index` is zero or positive. negative = no normal data
        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
          tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
          tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
          face_normals[v] = glm::vec3{nx, ny, nz};
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
          tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
          face_texcoord[v] = glm::vec2{tx, ty};
        }

        // Optional: vertex colors
        // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
        // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
        // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
      }

      index_offset += fv;

      // per-face material
      shapes[s].mesh.material_ids[f];
      // end of tiny obj loader routine

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
        continue;

      std::transform(world_coords.begin(), world_coords.end(), screen_coords.begin(), [&](auto& x) {
        return world2screen(x, width, height);
      });

      // std::transform(face_texcoord.begin(), face_texcoord.end(), face_texcoord.begin(), [&](auto& x) {
      //   return screen2world(x, width, height);
      // });

      raster_triangle_with_depth_buffer(screen_coords, face_texcoord, z_buffer, image, texture, light_intensity);
    }
  }
}