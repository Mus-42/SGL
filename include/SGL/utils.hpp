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

        template<typename... T>
        struct sgl_type_identity {};
        template<typename T> 
        struct sgl_type_identity<T> {
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


        namespace OperatorsExistCheck {
            struct NotExits {};
            //overloads which compiler choose if operator not exist (and... it generate compile time error if function marked as deleted.)
            
            //TODO FIX IT IN GCC (compile time error in each use case when operator not exist)
            #define SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR
            #define SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(name, pref, post)\
            template<typename A> auto has_##name##_operator_impl(int) -> decltype(pref std::declval<A>() post);\
            template<typename> auto has_##name##_operator_impl(...) -> NotExits;\
            template<typename A> constexpr static bool op_##name = !std::is_same_v<decltype(has_##name##_operator_impl<A>(0)), NotExits>;

            #define SGL_CREATE_HAS_OPERATOR_IMPL(name, operator)\
            template<typename A, typename B> auto has_##name##_operator_impl(int) -> decltype(std::declval<A>() operator std::declval<B>());\
            template<typename, typename> auto has_##name##_operator_impl(...) -> NotExits;\
            template<typename A, typename B = A> constexpr static bool op_##name = !std::is_same_v<decltype(has_##name##_operator_impl<A, B>(0)), NotExits>;

            /*
                in this way can't check following operators:

                a=b a(b...) a[b] a->b (cannot be overload as non-member)

                a<=>b (library writen in C++17)
                a.b a?b:c  (non overloadable)
            */

            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(unary_plus , +, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//+a
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(unary_minus, -, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//-a
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(bitwise_not, ~, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//~a
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(not        , !, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//!a
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(deref      , *, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//*a
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(adress_of  , &, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//&a

            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(prefix_incr, ++, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//++a
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(prefix_decr, --, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR)//--a
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(postfix_incr, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR, ++)//a++
            SGL_CREATE_HAS_UNARY_OPERATOR_IMPL(postfix_decr, SGL_HAS_UNARY_OPERATOR_EMPTY_OPERATOR, --)//a--

            //binary
            SGL_CREATE_HAS_OPERATOR_IMPL(sum, +)//a+b
            SGL_CREATE_HAS_OPERATOR_IMPL(sub, -)//a-b
            SGL_CREATE_HAS_OPERATOR_IMPL(mul, *)//a*b
            SGL_CREATE_HAS_OPERATOR_IMPL(div, /)//a/b
            SGL_CREATE_HAS_OPERATOR_IMPL(mod, %)//a%b

            SGL_CREATE_HAS_OPERATOR_IMPL(sum_assign, +=)//a+=b
            SGL_CREATE_HAS_OPERATOR_IMPL(sub_assign, -=)//a-=b
            SGL_CREATE_HAS_OPERATOR_IMPL(mul_assign, *=)//a*=b
            SGL_CREATE_HAS_OPERATOR_IMPL(div_assign, /=)//a/=b
            SGL_CREATE_HAS_OPERATOR_IMPL(mod_assign, %=)//a%=b

            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_or , | )//a|b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_and, & )//a&b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_xor, ^ )//a^b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_lsh, <<)//a<<b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_rsh, >>)//a>>b

            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_or_assign , |= )//a|=b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_and_assign, &= )//a&=b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_xor_assign, ^= )//a^=b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_lsh_assign, <<=)//a<<=b
            SGL_CREATE_HAS_OPERATOR_IMPL(bitwise_rsh_assign, >>=)//a>>=b

            SGL_CREATE_HAS_OPERATOR_IMPL(equal      , ==)//a==b
            SGL_CREATE_HAS_OPERATOR_IMPL(not_equal  , !=)//a!=b
            SGL_CREATE_HAS_OPERATOR_IMPL(less       , > )//a>b
            SGL_CREATE_HAS_OPERATOR_IMPL(greater    , < )//a<b
            SGL_CREATE_HAS_OPERATOR_IMPL(not_less   , >=)//a>=b
            SGL_CREATE_HAS_OPERATOR_IMPL(not_greater, <=)//a<=b
            
            SGL_CREATE_HAS_OPERATOR_IMPL(or , ||)//a||b
            SGL_CREATE_HAS_OPERATOR_IMPL(and, &&)//a&&b
            
            template<typename T, typename index> auto has_subscript_operator_impl(int) -> decltype(std::declval<T>()[std::declval<index>()]);
            template<typename, typename> auto has_subscript_operator_impl(...) -> NotExits;
    
            template<typename T, typename index = size_t> constexpr static bool op_subscript = !std::is_same_v<decltype(has_subscript_operator_impl<T, index>(0)), NotExits>;//a[i]
        }//namespace OperatorsExistCheck
    }//namespace details

    template<typename T> using arr = details::array_value<T>;
}//namespace SGL

#endif//SGL_UTILS_HPP_INCLUDE_