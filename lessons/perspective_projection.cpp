#include "perspective_projection.hpp"
#include "../obj_loader_helper.hpp"
#include "../rasterization.hpp"
#include "glm/trigonometric.hpp"
#include "tga_color.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <ostream>

#define MAT3x3_ROW_MAJOR_INIT_LIST(_00, _10, _20, _01, _11, _21, _02, _12, _22, _)                                    _00, _01, _02, _10, _11, _12, _20, _21, _22
#define MAT4x4_ROW_MAJOR_INIT_LIST(_00, _10, _20, _30, _01, _11, _21, _31, _02, _12, _22, _32, _03, _13, _23, _33, _) _00, _01, _02, _03, _10, _11, _12, _13, _20, _21, _22, _23, _30, _31, _32, _33

void perspective_projection_study_1(TGAImage& image) {
  const glm::vec2 scale{ 0.25f, 0.25f };
  const glm::vec2 shear{ 0.0f, -0.0f };
  const glm::vec2 translation{ 0.0f, 0.0f };
  const glm::vec2 rotation{ glm::radians(0.0f), glm::radians(0.0f) };

  // clang-format off
  glm::mat3 model_shear_matrix{MAT3x3_ROW_MAJOR_INIT_LIST(
       1.0f , shear.x ,  0.0f ,
    shear.y ,    1.0f ,  0.0f ,
       0.0f ,    0.0f ,  1.0f ,
  )};

  glm::mat3 model_scale_matrix{MAT3x3_ROW_MAJOR_INIT_LIST(
    scale.x ,    0.0f , translation.x ,
       0.0f , scale.y , translation.y ,
       0.0f ,    0.0f ,          1.0f ,
  )};

  glm::mat3 model_rotation_matrix{MAT3x3_ROW_MAJOR_INIT_LIST(
    glm::cos(rotation.x) , -glm::sin(rotation.y) , 0.0f ,
    glm::sin(rotation.x) ,  glm::cos(rotation.y) , 0.0f ,
    0                    ,                     0 , 1.0f ,
  )};

  glm::mat3 model_translation_matrix{MAT3x3_ROW_MAJOR_INIT_LIST(
    1.0f , 0.0f , -translation.x ,
    0.0f , 1.0f , -translation.y ,
    0.0f , 0.0f , 1.0f           ,
  )};
  
  glm::mat3 model_transformation_matrix = model_scale_matrix * model_shear_matrix * model_rotation_matrix * model_translation_matrix;
  // clang-format on

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

    glm::vec2 x_axis{ 1, 0 };
    glm::vec2 y_axis{ 0, 1 };
    glm::vec2 screen_origin{ image.get_width() / 2, image.get_height() / 2 };

    image.line(screen_origin, screen_origin + x_axis * screen_origin / 2.0f, RED);
    image.line(screen_origin, screen_origin + y_axis * screen_origin / 2.0f, GREEN);
  });
}

void perspective_projection_study_2(TGAImage& image) {
  // clang-format off
  const glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
  const glm::vec3 translation{ 0.0f, 0.0f, 1.0f };
  const glm::vec3 rotation{ glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f)};

  glm::mat4 model_scale_matrix{MAT4x4_ROW_MAJOR_INIT_LIST(
    scale.x ,    0.0f ,     0.0f , translation.x ,
       0.0f , scale.y ,     0.0f , translation.y ,
       0.0f ,    0.0f ,  scale.z , translation.y ,
       0.0f ,    0.0f ,     0.0f ,          1.0f ,
  )};

  glm::quat rotation_quat(rotation);
  glm::mat4 model_rotation_matrix{glm::rotate(rotation_quat, 00.0f, glm::vec3(0.0f, 0.0f, 1.0f))};

  glm::mat4 model_translation_matrix{MAT4x4_ROW_MAJOR_INIT_LIST(
    1.0f , 0.0f , 0.0f, -translation.x ,
    0.0f , 1.0f , 0.0f, -translation.y ,
    0.0f , 0.0f , 1.0f, -translation.z ,
    0.0f , 0.0f , 0.0f ,          1.0f ,
  )};
  
  glm::mat4 model_transformation_matrix = model_scale_matrix * model_rotation_matrix * model_translation_matrix;


  const float_t width = image.get_width();
  const float_t height = image.get_height();
  constexpr glm::vec3 light_dir{ 0.0f, 0.0f, -1.0f };
  constexpr float_t ambient_light_contribution = 0.4;
  constexpr float_t z_near = 0.5f;
  constexpr float_t z_far = 10000.0f;
  std::vector<float_t> z_buffer(width * height, -std::numeric_limits<float_t>::max());
  TGAImage texture{};
  texture.read_tga_file("./assets/african_head_diffuse.tga");
  texture.flip_vertically();
  
  constexpr float_t fov = 180.0f;
  const float_t $fv = 1.0f / glm::tan(fov / 2.0f);

  const float_t aspect_ratio = (float_t)image.get_height() / (float_t)image.get_width();
  const float_t $ar = (float_t)image.get_height() / (float_t)image.get_width();
  
  glm::mat4 perspective_projection_matrix{MAT4x4_ROW_MAJOR_INIT_LIST(
    $ar * $fv , 0.0f , 0.0f                    ,                                 0.0f ,
    0.0f      ,  $fv , 0.0f                    ,                                 0.0f ,
    0.0f      , 0.0f , z_far / (z_far - z_near), (-z_far * z_near) / (z_far - z_near) ,
    0.0f      , 0.0f , 1.0f                    ,                                 0.0f ,
  )};
  // clang-format on

  load_model_and_for_each_face("assets/african_head.obj", [&](auto face_vertices, auto face_normals, auto face_texcoords) -> void {
    std::array<glm::vec3, 3> screen_coords;

    std::transform(face_vertices.begin(), face_vertices.end(), screen_coords.begin(), [&](auto& p) {
      auto perspective_pos = perspective_projection_matrix * model_transformation_matrix * glm::vec4(p, 1.0f);
      if (perspective_pos.z != 0.0f) {
        perspective_pos.x /= perspective_pos.z;
        perspective_pos.y /= perspective_pos.z;
        perspective_pos.z /= perspective_pos.z;
      }
      return world_to_screen(
          perspective_pos,
          image.get_width(),
          image.get_height()
      );
    });

    auto& [a, b, c] = face_vertices;
    auto wcu = c - a;
    auto wcv = b - a;
    auto face_normal = glm::normalize(glm::cross(wcu, wcv));
    std::cout << wcu.x << " - " << wcu.y << std::endl;
    const float_t nl_dot = glm::dot(face_normal, light_dir);
    const float_t light_intensity = glm::clamp((nl_dot + 0.1f / 2.0f) + ambient_light_contribution, 0.0f, 1.0f);

    if (light_intensity <= std::numeric_limits<float_t>::epsilon())
      return;

    raster_triangle_with_depth_buffer(screen_coords, face_texcoords, z_buffer, image, texture, light_intensity);

    glm::vec2 x_axis{ 1, 0 };
    glm::vec2 y_axis{ 0, 1 };
    glm::vec2 screen_origin{ image.get_width() / 2, image.get_height() / 2 };

    image.line(screen_origin, screen_origin + x_axis * screen_origin / 2.0f, RED);
    image.line(screen_origin, screen_origin + y_axis * screen_origin / 2.0f, GREEN);
  });
}
