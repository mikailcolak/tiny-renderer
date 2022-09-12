#include "perspective_projection.hpp"
#include "../obj_loader_helper.hpp"
#include "../rasterization.hpp"
#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"
#include "tga_color.hpp"
#include <algorithm>
#include <array>

void perspective_projection_study_1(TGAImage& image) {
  const glm::vec2 scale{ 0.25f, 0.25f };
  const glm::vec2 translation{ 0.7f, 0.7f };
  const glm::vec2 rotation{ glm::radians(10.0f), glm::radians(10.0f)};

  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);

  glm::mat3 model_transformation_matrix(
    {
      scale.x * c1             ,
      scale.x * -s1            ,
      0.0f                     ,
    },
    {
      scale.y * s2,
      scale.y * c2             ,
      0.0f                     ,
    },
    {
      translation.x            ,
      translation.y            ,
      1.0f                     ,
    }
  );

  load_model_and_for_each_face("assets/plane.obj", [&](auto face_vertices, auto face_normals, auto face_texcoords) -> void {
    std::array<glm::vec2, 3> screen_coords;
    std::transform(face_vertices.begin(), face_vertices.end(), screen_coords.begin(), [&](auto& p) {
      return glm::vec2{
        world_to_screen(
          model_transformation_matrix * glm::vec3(p.x, p.y, 1.0f),
          image.get_width(),
          image.get_height()
        )
      };
    });

    triangle(screen_coords, image, TGAColor(face_texcoords[0].x * 255, face_texcoords[1].x * 255, face_texcoords[2].x * 255, 255));
  });
}
