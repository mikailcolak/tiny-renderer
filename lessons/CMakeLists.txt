
set(lessons "")
list(APPEND lessons
    perspective_projection.hpp
    perspective_projection.cpp
)
add_library(lessons ${lessons})