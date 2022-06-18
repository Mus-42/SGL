cmake_minimum_required(VERSION 3.16)
project(SGL)

add_library(SGL INTERFACE)
target_include_directories(SGL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

option(sgl_build_examples "build examples" ON)

add_compile_options(-Wall -Wextra)

if(sgl_build_examples) 
    add_subdirectory(examples)
endif()