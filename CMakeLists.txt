cmake_minimum_required(VERSION 3.16)
project(SGL)

#TODO remove it
#add_compile_options(-Wall -Wextra)
add_compile_options(/W4)# 

add_library(SGL STATIC src/sgl.cpp)
target_include_directories(SGL PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

set_target_properties(SGL PROPERTIES 
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES      
    CXX_EXTENSIONS NO
)

option(sgl_build_examples "build examples" ON)

if(sgl_build_examples) 
    add_subdirectory(examples)
endif()