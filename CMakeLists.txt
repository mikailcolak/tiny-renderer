cmake_minimum_required(VERSION 3.0.0)
project(tiny-renderer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(external)
include_directories(external/glm)

include(CTest)
enable_testing()

add_executable(tiny-renderer
    main.cpp
    tgaimage.hpp
    tgaimage.cpp
    model.hpp
    model.cpp
    tiny_obj_loader.hpp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
