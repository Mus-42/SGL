cmake_minimum_required(VERSION 3.8)
project(SGL)

add_library(SGL SGL.cpp)
set_target_properties(SGL PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_executable(test main.cpp)
set_target_properties(test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(test PUBLIC SGL)