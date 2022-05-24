#pragma once
#ifndef SGL_UTILS_HPP_INCLUDE_
#define SGL_UTILS_HPP_INCLUDE_

#include "config.hpp"
#include <cctype>//std::isalpha, std::isalnum ...
#include <algorithm>
#include <vector>

namespace SGL {
    namespace details {   
        template<typename T>
        struct array_value {
            using type = T;
            static_assert(!std::is_reference_v<T>, "arr<T&>: cannot create array of references. did you mean pointers instead of references?");
        };

        class no_copy {
        public:
            no_copy() = default;
            ~no_copy() = default;

            no_copy(const no_copy&) = delete;
            no_copy& operator=(const no_copy&) = delete;

            no_copy(no_copy&&) = default;
            no_copy& operator=(no_copy&&) = default;
        };

        template<typename T> struct sgl_type_identity {
            using type = T;
        };

        constexpr bool is_alpha(unsigned char ch) {//constexpr isalpha fucntion
            return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z';
        }
        constexpr bool is_num(unsigned char ch) {//constexpr isnum fucntion
            return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z';
        }
        constexpr bool is_alnum(unsigned char ch) {//constexpr isalnum fucntion
            return is_alpha(ch) || is_num(ch);
        }

        constexpr bool is_correct_identifier(std::string_view s) {
            if(!s.size() || !is_alpha(s[0]) && s[0] != '_') return false;
            bool ret = true;
            for(unsigned char ch : s) ret &= is_alnum(ch) || ch == '_';
            return ret;
        }

        template<typename T>
        struct make_base_type {
            using type = std::remove_cv_t<T>;
        };
        //pointer
        template<typename T>
        struct make_base_type<T*> {
            using type = typename make_base_type<std::remove_pointer_t<T>>::type;
        };
        template<typename T>
        struct make_base_type<T* const> {
            using type = typename make_base_type<std::remove_pointer_t<T>>::type;
        };
        template<typename T>
        struct make_base_type<T* volatile> {
            using type = typename make_base_type<std::remove_pointer_t<T>>::type;
        };
        template<typename T>
        struct make_base_type<T* const volatile> {
            using type = typename make_base_type<std::remove_pointer_t<T>>::type;
        };
        //reference
        template<typename T>
        struct make_base_type<T&> {
            using type = typename make_base_type<std::remove_reference_t<T>>::type;
        };
        template<typename T>
        struct make_base_type<T&&> {
            using type = typename make_base_type<std::remove_reference_t<T>>::type;
        };
        template<typename T>
        struct make_base_type<details::array_value<T>> {
            using type = typename make_base_type<T>::type;
        };

        template<typename T>
        using make_base_type_t = typename make_base_type<T>::type;

        template<typename T>
        constexpr bool is_base_type = std::is_same_v<T, make_base_type_t<T>>;

        template<typename T>
        struct get_vector_from_arr {
            using type = T;
            using base_type = T;
        };
        template<typename T>
        struct get_vector_from_arr<details::array_value<T>> {
            using type = std::vector<typename get_vector_from_arr<T>::type>;
            using base_type = typename get_vector_from_arr<T>::base_type;
        };
        template<typename T> using get_vector_from_arr_t = typename get_vector_from_arr<T>::type;
        template<typename T> using get_vector_from_arr_b = typename get_vector_from_arr<T>::base_type;


        template<typename T>
        struct is_sgl_array {
            constexpr static bool value = false;
        };
        template<typename T>
        struct is_sgl_array<details::array_value<T>> {
            constexpr static bool value = true;
        };
        template<typename T>
        constexpr bool is_sgl_array_v = is_sgl_array<T>::value;

    }//namespace details

    template<typename T> using arr = details::array_value<T>;
}//namespace SGL

#endif//SGL_UTILS_HPP_INCLUDE_