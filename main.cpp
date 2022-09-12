#include "model.hpp"
#include "tgaimage.hpp"

#include <glm/glm.hpp>

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

// lessons
#include "lessons/model_rendering.hpp"
#include "lessons/triangle_rendering.hpp"
#include "lessons/depth_buffer.hpp"
#include "lessons/perspective_projection.hpp"

int main(int, char**) {

  TGAImage image(800, 800, TGAImage::RGB);
  
  // triangle_rendering(image);
  // model_rendering(image);
  // depth_buffer_1(image);
  // depth_buffer_2(image);
  perspective_projection_study_1(image);

  image.flip_vertically(); // i want to have the origin at the left bottom
  image.write_tga_file("result.tga");

  return EXIT_SUCCESS;
}