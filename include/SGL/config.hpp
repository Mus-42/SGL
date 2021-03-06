#pragma once
#ifndef SGL_CONFIG_HPP_INCLUDE_
#define SGL_CONFIG_HPP_INCLUDE_

//check types casts correctness in SGL 
#define SGL_OPTION_TYPE_CHECKS              1
#define SGL_OPTION_ENABLE_ASSERT            1

//TODO set options from CMake?

//assertion macro
//TODO add error levels?
#ifndef SGL_ASSERT
#include <iostream>//std::cerr
#if SGL_OPTION_ENABLE_ASSERT//TODO throw exception when assertion failed (after log?)?
#define SGL_ASSERT(v, msg) {if(!(v)) { std::cerr << "SGL assertion failed (l:" << __LINE__ << " f:" << __FUNCTION__  << ") " << (msg) << std::endl; }}
#else
#define SGL_ASSERT(v, msg) {}
#endif
#endif//SGL_ASSERT

#ifndef SGL_TOKENIZE_ERROR 
#include <iostream>//std::cerr
#include <sstream>//std::stringstream
namespace SGL::details {
    [[noreturn]] inline void sgl_tokenize_error_impl(std::string_view desc, size_t line, size_t collumn) {
        throw std::runtime_error((std::stringstream() << "SGL tokenize error (l: " << line << ", c:" << collumn << "): " << desc).str());
    }
}//SGL::details
#define SGL_TOKENIZE_ERROR(desc, line, collumn) {SGL::details::sgl_tokenize_error_impl(desc, line, collumn); }//TODO replace with exceptions
#endif//SGL_TOKENIZE_ERROR



#if defined(_MSC_VER)//MSVC
#define SGL_COMPILER_MSVC    1
#elif defined(__GNUC__)//GCC or Clang
#if defined(__clang__)
#define SGL_COMPILER_CLANG   1
#else
#define SGL_COMPILER_GCC     1
#endif//__clang__
#else
#define SGL_COMPILER_UNKNOWN 1
#endif

#endif