#pragma once
#ifndef SGL_UTILS_HPP_INCLUDE_
#define SGL_UTILS_HPP_INCLUDE_

#include "config.hpp"
#include <cctype>//std::isalpha, std::isalnum ...
#include <algorithm>
#include <vector>

#if defined(SGL_OPTION_ENABLE_TYPE_NAME) && SGL_OPTION_ENABLE_TYPE_NAME
#include "type_name.hpp"
#endif//SGL_OPTION_ENABLE_TYPE_NAME

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


        template<typename... T>
        struct sgl_type_identity {};
        template<typename T> 
        struct sgl_type_identity<T> {
            using type = T;
        };

        template<typename... T>
        sgl_type_identity<T...> make_type_identity() { return {}; } 

        template<typename... T1, typename... T2>
        sgl_type_identity<T1..., T2...> concat_type_identity(sgl_type_identity<T1...>, sgl_type_identity<T2...>) { return {}; }

        constexpr bool is_alpha(unsigned char ch) {//constexpr isalpha fucntion
            return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
        }
        constexpr bool is_num(unsigned char ch) {//constexpr isnum fucntion
            return '0' <= ch && ch <= '9';
        }
        constexpr bool is_alnum(unsigned char ch) {//constexpr isalnum fucntion
            return is_alpha(ch) || is_num(ch);
        }

        constexpr bool is_correct_identifier(std::string_view s) {
            if(!s.size() || (!is_alpha(s[0]) && s[0] != '_')) return false;
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


        //TODO add a<=>b, a?b:c, a->b
        //unary 
        template<typename T> constexpr bool has_op_unary_plus   = requires(T v) { + v; };//+a
        template<typename T> constexpr bool has_op_unary_minus  = requires(T v) { - v; };//-a
        template<typename T> constexpr bool has_op_bitwise_not  = requires(T v) { ~ v; };//~a
        template<typename T> constexpr bool has_op_not          = requires(T v) { ! v; };//!a
        template<typename T> constexpr bool has_op_deref        = requires(T v) { * v; };//*a
        template<typename T> constexpr bool has_op_adress_of    = requires(T v) { & v; };//&a
        template<typename T> constexpr bool has_op_prefix_incr  = requires(T v) { ++v; };//++a
        template<typename T> constexpr bool has_op_prefix_decr  = requires(T v) { --v; };//--a
        template<typename T> constexpr bool has_op_postfix_incr = requires(T v) { v++; };//a++
        template<typename T> constexpr bool has_op_postfix_decr = requires(T v) { v--; };//a--
        //binary
        template<typename A, typename B = A> constexpr bool has_op_sum                = requires(A a, B b) { a +   b; };//a+b
        template<typename A, typename B = A> constexpr bool has_op_sub                = requires(A a, B b) { a -   b; };//a-b
        template<typename A, typename B = A> constexpr bool has_op_mul                = requires(A a, B b) { a *   b; };//a*b
        template<typename A, typename B = A> constexpr bool has_op_div                = requires(A a, B b) { a /   b; };//a/b
        template<typename A, typename B = A> constexpr bool has_op_mod                = requires(A a, B b) { a %   b; };//a%b
        template<typename A, typename B = A> constexpr bool has_op_sum_assign         = requires(A a, B b) { a +=  b; };//a+=b
        template<typename A, typename B = A> constexpr bool has_op_sub_assign         = requires(A a, B b) { a -=  b; };//a-=b
        template<typename A, typename B = A> constexpr bool has_op_mul_assign         = requires(A a, B b) { a *=  b; };//a*=b
        template<typename A, typename B = A> constexpr bool has_op_div_assign         = requires(A a, B b) { a /=  b; };//a/=b
        template<typename A, typename B = A> constexpr bool has_op_mod_assign         = requires(A a, B b) { a %=  b; };//a%=b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_or         = requires(A a, B b) { a |   b; };//a|b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_and        = requires(A a, B b) { a &   b; };//a&b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_xor        = requires(A a, B b) { a ^   b; };//a^b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_lsh        = requires(A a, B b) { a <<  b; };//a<<b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_rsh        = requires(A a, B b) { a >>  b; };//a>>b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_or_assign  = requires(A a, B b) { a |=  b; };//a|=b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_and_assign = requires(A a, B b) { a &=  b; };//a&=b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_xor_assign = requires(A a, B b) { a ^=  b; };//a^=b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_lsh_assign = requires(A a, B b) { a <<= b; };//a<<=b
        template<typename A, typename B = A> constexpr bool has_op_bitwise_rsh_assign = requires(A a, B b) { a >>= b; };//a>>=b
        template<typename A, typename B = A> constexpr bool has_op_equal              = requires(A a, B b) { a ==  b; };//a==b
        template<typename A, typename B = A> constexpr bool has_op_not_equal          = requires(A a, B b) { a !=  b; };//a!=b
        template<typename A, typename B = A> constexpr bool has_op_less               = requires(A a, B b) { a >   b; };//a>b
        template<typename A, typename B = A> constexpr bool has_op_greater            = requires(A a, B b) { a <   b; };//a<b
        template<typename A, typename B = A> constexpr bool has_op_not_less           = requires(A a, B b) { a >=  b; };//a>=b
        template<typename A, typename B = A> constexpr bool has_op_not_greater        = requires(A a, B b) { a <=  b; };//a<=b
        template<typename A, typename B = A> constexpr bool has_op_or                 = requires(A a, B b) { a ||  b; };//a||b
        template<typename A, typename B = A> constexpr bool has_op_and                = requires(A a, B b) { a &&  b; };//a&&b
        
        template<typename T, typename index = size_t> constexpr bool has_op_subscript = requires(T v, index i) { v[i]; };//a[i]
    }//namespace details

    template<typename T> using arr = details::array_value<T>;
}//namespace SGL

#endif//SGL_UTILS_HPP_INCLUDE_