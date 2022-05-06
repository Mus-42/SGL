#pragma once
#ifndef SGL_CONFIG_HPP_INCLUDE_
#define SGL_CONFIG_HPP_INCLUDE_

//assertion macro
#ifndef SGL_ASSERT
#include <iostream>//std::cerr
#define SGL_ASSERT(v) {if(!(v)) { std::cerr << "SGL assertion failed (l:" << __LINE__ << " f:" << __FUNCTION__  << ")"<< std::endl; }}
#endif//SGL_ASSERT

//check function types in SGL 
#define SGL_OPTION_TYPE_CHECKS 1

//TODO set options from CMake?

#endif