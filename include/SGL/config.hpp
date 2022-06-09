#pragma once
#ifndef SGL_CONFIG_HPP_INCLUDE_
#define SGL_CONFIG_HPP_INCLUDE_

//check types casts correctness in SGL 
#define SGL_OPTION_TYPE_CHECKS              1
#define SGL_OPTION_ENABLE_TYPE_NAME         1
#define SGL_OPTION_DISABLE_REALESE_ASSERT   1

//TODO set options from CMake?

//assertion macro
#ifndef SGL_ASSERT
#include <iostream>//std::cerr
#if defined(_DEBUG) || !SGL_OPTION_DISABLE_REALESE_ASSERT
#define SGL_ASSERT(v, msg) {if(!(v)) { std::cerr << "SGL assertion failed (l:" << __LINE__ << " f:" << __FUNCTION__  << ") " << (msg) << std::endl; }}
#else
#define SGL_ASSERT(v, msg) {}
#endif
#endif//SGL_ASSERT

//TODO add error levels?
#ifndef SGL_TOKENIZE_ERROR 
#include <iostream>//std::cerr
namespace SGL::details {
    inline void sgl_tokenize_error_impl(std::string_view desc, size_t line, size_t collumn) {
        std::cerr << "SGL tokenize error (l: " << line << ", c:" << collumn << "): " << desc << std::endl;
    }
}//SGL::details
#define SGL_TOKENIZE_ERROR(desc, line, collumn) {SGL::details::sgl_tokenize_error_impl(desc, line, collumn); }
#endif//SGL_TOKENIZE_ERROR


#endif