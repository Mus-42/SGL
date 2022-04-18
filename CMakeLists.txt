cmake_minimum_required(VERSION 3.8)
project(SGL)

add_library(SGL src/SGL.cpp src/CSGL.cpp)
target_include_directories(SGL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set_target_properties(SGL PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

add_executable(example example.cpp)
set_target_properties(example PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
target_link_libraries(example PUBLIC SGL)

add_executable(cexample example.c)
target_link_libraries(cexample PUBLIC SGL)