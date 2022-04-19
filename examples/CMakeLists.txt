cmake_minimum_required(VERSION 3.8)
project(Examples)


file(GLOB examples_sources "*.cpp" "*.c")

#all examples - single *.c or *.cpp file
foreach(example_source_file IN LISTS examples_sources)
    #convert filename to example name
    string(REPLACE "." "_" example_name ${example_source_file})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" example_name ${example_name})
    string(TOUPPER ${example_name} example_name)
    set(example_name "SGL_${example_name}_EXAMPLE")
    #add example executable
    add_executable(${example_name} ${example_source_file})
    target_link_libraries(${example_name} PUBLIC SGL)
endforeach()
