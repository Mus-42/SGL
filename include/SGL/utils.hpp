#pragma once
#ifndef SGL_UTILS_HPP_INCLUDE_
#define SGL_UTILS_HPP_INCLUDE_

#include "config.hpp"
#include <cctype>//std::isalpha, std::isalnum ...
#include <algorithm>

namespace SGL {

    class no_copy {
    public:
        no_copy() = default;
        ~no_copy() = default;

        no_copy(const no_copy&) = delete;
        no_copy& operator=(const no_copy&) = delete;

        no_copy(no_copy&&) = default;
        no_copy& operator=(no_copy&&) = default;
    };

    template<typename T> struct sgl_type_identity {};

    constexpr bool is_correct_identifier(std::string_view s) {
        if(!s.size() || !std::isalpha(s[0]) && s[0] != '_') return false;
        return std::all_of(s.begin(), s.end(), [](unsigned char ch) -> bool { return std::isalnum(ch) || ch == '_'; });
    }
}//namespace SGL

#endif//SGL_UTILS_HPP_INCLUDE_