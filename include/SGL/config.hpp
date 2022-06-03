#pragma once
#ifndef SGL_CONFIG_HPP_INCLUDE_
#define SGL_CONFIG_HPP_INCLUDE_

//assertion macro
#ifndef SGL_ASSERT
#include <iostream>//std::cerr
#define SGL_ASSERT(v, msg) {if(!(v)) { std::cerr << "SGL assertion failed (l:" << __LINE__ << " f:" << __FUNCTION__  << ") " << (msg) << std::endl; }}
#endif//SGL_ASSERT

//TODO add error levels?
#ifndef SGL_TOKENIZE_ERROR 
#include <iostream>//std::cerr
namespace SGL::details {
    inline void sgl_tokenize_error_impl(std::string_view desc, size_t line, size_t collumn) {
        std::cerr << "SGL tokenize error (l: " << line << ", c:" << collumn << "): " << desc << std::endl;
    }
}//SGL::details
#define SGL_TOKENIZE_ERROR(desc, line, collumn) {sgl_tokenize_error_impl(desc, line, collumn); }
#endif//SGL_TOKENIZE_ERROR

//check function types in SGL 
#define SGL_OPTION_TYPE_CHECKS 1

//TODO set options from CMake?

#endif