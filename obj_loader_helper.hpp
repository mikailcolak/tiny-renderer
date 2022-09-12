#pragma once

#include "tiny_obj_loader.hpp"
#include <glm/glm.hpp>

#include <functional>
#include <iostream>

using for_each_fn = std::function<void(std::array<glm::vec3, 3> face_vertices, std::array<glm::vec3, 3> face_normals, std::array<glm::vec2, 3> face_texcoords)>;

inline void load_model_and_for_each_face(std::string inputfile, for_each_fn fn) {
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

  auto& attrib = reader.GetAttrib();
  auto& shapes = reader.GetShapes();
  auto& materials = reader.GetMaterials();

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      std::array<glm::vec3, 3> face_vertices{};
      std::array<glm::vec3, 3> face_normals{};
      std::array<glm::vec2, 3> face_texcoords{};

      // TODO: Instead of discarding quad face split them into 2 triangular face!!!
      // if (fv > 3) continue;

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        face_vertices[v] = glm::vec3{ vx, vy, vz };

        // Check if `normal_index` is zero or positive. negative = no normal data
        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
          face_normals[v] = glm::vec3{ nx, ny, nz };
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord data
        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
          face_texcoords[v] = glm::vec2{ tx, ty };
        }

        // Optional: vertex colors
        // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
        // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
        // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
        // face_texcoord[v] = glm::vec2{ tx, ty };

      }

      index_offset += fv;

      // per-face material
      shapes[s].mesh.material_ids[f];
      // end of tiny obj loader routine

      fn(face_vertices, face_normals, face_texcoords);
    }
  }
}