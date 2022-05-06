cmake_minimum_required(VERSION 3.8)
project(Examples)


file(GLOB examples_sources "*.cpp")

#all examples - single *.cpp file
foreach(example_source_file IN LISTS examples_sources)
    #convert filename to example name
    string(REPLACE "." "_" example_name ${example_source_file})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" example_name ${example_name})
    string(TOUPPER ${example_name} example_name)
    set(example_name "SGL_${example_name}_EXAMPLE")
    #add example executable
    add_executable(${example_name} ${example_source_file})
    target_link_libraries(${example_name} PUBLIC SGL)
    set_target_properties(${example_name} PROPERTIES 
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES      
        CXX_EXTENSIONS NO
    )
endforeach()

#copy data folder to binary folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${PROJECT_BINARY_DIR})