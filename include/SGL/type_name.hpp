#pragma once
#ifndef SGL_TYPE_NAME_HPP_INCLUDE_
#define SGL_TYPE_NAME_HPP_INCLUDE_
#include <string_view>
#include "config.hpp"
namespace SGL {
#if defined(SGL_OPTION_ENABLE_TYPE_NAME) && SGL_OPTION_ENABLE_TYPE_NAME
    namespace details {     
        template<typename T>
        constexpr std::string_view type_name_helper() {
#if defined(SGL_COMPILER_MSVC)//MSVC
            return __FUNCSIG__;
#elif defined(SGL_COMPILER_GCC)
#if __GNUC__ < 8
            return "T [to old GCC compiler]";//can't be constexpr
#else
            return __PRETTY_FUNCTION__;
#endif//GCC_VERSION
#elif defined(SGL_COMPILER_CLANG)
            return __PRETTY_FUNCTION__;
#else//TODO add other compilers?
            return "T [uncompatible compiler]";
#endif//comiler switch
        }
        constexpr auto void_sv = type_name_helper<void>();
        constexpr bool is_compatible_compiler = void_sv.find("void") < void_sv.size();
        constexpr size_t beg_offset = void_sv.find("void");
        constexpr size_t beg_offset_2 = void_sv.find("void", beg_offset+4);
        constexpr auto find_substr = void_sv.substr(beg_offset + 4, beg_offset_2-4-beg_offset);
    }//namespace details
#endif//SGL_OPTION_ENABLE_TYPE_NAME
    template<typename T>
    constexpr std::string_view get_type_name() {//Not same in all C++ implementations
#if defined(SGL_OPTION_ENABLE_TYPE_NAME) && SGL_OPTION_ENABLE_TYPE_NAME
        if constexpr(details::is_compatible_compiler) {
            auto raw = details::type_name_helper<T>();
            return raw.substr(details::beg_offset, raw.find(details::find_substr) - details::beg_offset);
        } else return details::type_name_helper<T>();
#else
        return "T [type name option disabled. check config.hpp for details]";
#endif//SGL_OPTION_ENABLE_TYPE_NAME
    } 
}//namespace SGL
#endif//SGL_TYPE_NAME_HPP_INCLUDE_