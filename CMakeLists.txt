cmake_minimum_required(VERSION 3.0.0)
project(tiny-renderer VERSION 0.1.0)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

include(CTest)
enable_testing()

add_executable(tiny-renderer main.cpp tgaimage.hpp tgaimage.cpp)
target_link_libraries(tiny-renderer ${CURSES_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
