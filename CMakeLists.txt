cmake_minimum_required(VERSION 3.19)
project(SGL)

add_library(SGL INTERFACE)
target_include_directories(SGL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
set_target_properties(SGL PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

option(sgl_build_examples "build examples" NO)

if(sgl_build_examples) 
    add_subdirectory(examples)
endif()