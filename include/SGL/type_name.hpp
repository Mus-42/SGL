#pragma once
#ifndef SGL_TYPE_NAME_HPP_INCLUDE_
#define SGL_TYPE_NAME_HPP_INCLUDE_
#include <string_view>
#include "config.hpp"
namespace SGL {
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
        constexpr auto find_res_sv = type_name_helper<int>();
        constexpr auto find_sv = std::string_view{"int"};
        constexpr size_t beg_offset = find_res_sv.find(find_sv);
        constexpr bool is_compatible_compiler = beg_offset < find_res_sv.size();
        constexpr size_t beg_offset_2 = find_res_sv.find(find_sv, beg_offset+find_sv.size());
        constexpr auto find_substr = find_res_sv.substr(beg_offset + find_sv.size(), beg_offset_2-find_sv.size()-beg_offset);
    }//namespace details
    template<typename T>
    constexpr std::string_view get_type_name() {//Not same in all C++ implementations
        if constexpr(details::is_compatible_compiler) {
            auto raw = details::type_name_helper<T>();
            return raw.substr(details::beg_offset, raw.find(details::find_substr) - details::beg_offset);
        } 
        else return details::type_name_helper<T>();
    } 
}//namespace SGL
#endif//SGL_TYPE_NAME_HPP_INCLUDE_