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

        template<typename T> 
        struct sgl_type_identity {
            using type = T;
        };
        template<typename... T>
        struct sgl_type_identity<T...> {//variadic args overload

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
            
            //unary
            template<typename A> NotExits operator+(A);
            template<typename A> NotExits operator-(A);
            template<typename A> NotExits operator~(A);
            template<typename A> NotExits operator!(A);
            template<typename A> NotExits operator*(A);
            template<typename A> NotExits operator&(A);

            template<typename A> NotExits operator++(A);//prefix
            template<typename A> NotExits operator--(A);//prefix

            template<typename A> NotExits operator++(A, int);//postfix
            template<typename A> NotExits operator--(A, int);//postfix
            
            //binary
            template<typename A, typename B> NotExits operator+(A, B);
            template<typename A, typename B> NotExits operator-(A, B);
            template<typename A, typename B> NotExits operator*(A, B);
            template<typename A, typename B> NotExits operator/(A, B);
            template<typename A, typename B> NotExits operator%(A, B);

            template<typename A, typename B> NotExits operator+=(A, B);
            template<typename A, typename B> NotExits operator-=(A, B);
            template<typename A, typename B> NotExits operator*=(A, B);
            template<typename A, typename B> NotExits operator/=(A, B);
            template<typename A, typename B> NotExits operator%=(A, B);

            template<typename A, typename B> NotExits operator|(A, B);
            template<typename A, typename B> NotExits operator&(A, B);
            template<typename A, typename B> NotExits operator^(A, B);
            template<typename A, typename B> NotExits operator<<(A, B);
            template<typename A, typename B> NotExits operator>>(A, B);

            template<typename A, typename B> NotExits operator|=(A, B);
            template<typename A, typename B> NotExits operator&=(A, B);
            template<typename A, typename B> NotExits operator^=(A, B);
            template<typename A, typename B> NotExits operator<<=(A, B);
            template<typename A, typename B> NotExits operator>>=(A, B);

            template<typename A, typename B> NotExits operator==(A, B);
            template<typename A, typename B> NotExits operator!=(A, B);
            template<typename A, typename B> NotExits operator>(A, B);
            template<typename A, typename B> NotExits operator<(A, B);
            template<typename A, typename B> NotExits operator>=(A, B);
            template<typename A, typename B> NotExits operator<=(A, B);

            template<typename A, typename B> NotExits operator||(A, B);
            template<typename A, typename B> NotExits operator&&(A, B);
            /*
                can't check following operators:

                a=b a(b...) a[b] a->b (cannot be overload as non-member)

                a<=>b (library writen in C++17)
                a.b a?b:c  (non overloadable)
            */
            
            template<typename A, typename B = A> constexpr bool op_unary_plus  = !std::is_same_v<decltype(+std::declval<A>()), NotExits>;//+a
            template<typename A, typename B = A> constexpr bool op_unary_minus = !std::is_same_v<decltype(-std::declval<A>()), NotExits>;//-a
            template<typename A, typename B = A> constexpr bool op_bitwise_not = !std::is_same_v<decltype(~std::declval<A>()), NotExits>;//~a
            template<typename A, typename B = A> constexpr bool op_not         = !std::is_same_v<decltype(!std::declval<A>()), NotExits>;//!a
            template<typename A, typename B = A> constexpr bool op_deref       = !std::is_same_v<decltype(*std::declval<A>()), NotExits>;//*a
            template<typename A, typename B = A> constexpr bool op_adress_of   = !std::is_same_v<decltype(&std::declval<A>()), NotExits>;//&a

            template<typename A, typename B = A> constexpr bool op_sum = !std::is_same_v<decltype(std::declval<A>() + std::declval<B>()), NotExits>;//a+b
            template<typename A, typename B = A> constexpr bool op_dif = !std::is_same_v<decltype(std::declval<A>() - std::declval<B>()), NotExits>;//a-b
            template<typename A, typename B = A> constexpr bool op_mul = !std::is_same_v<decltype(std::declval<A>() * std::declval<B>()), NotExits>;//a*b
            template<typename A, typename B = A> constexpr bool op_div = !std::is_same_v<decltype(std::declval<A>() / std::declval<B>()), NotExits>;//a/b
            template<typename A, typename B = A> constexpr bool op_mod = !std::is_same_v<decltype(std::declval<A>() % std::declval<B>()), NotExits>;//a%b

            template<typename A, typename B = A> constexpr bool op_sum_assign = !std::is_same_v<decltype(std::declval<A>() += std::declval<B>()), NotExits>;//a+=b
            template<typename A, typename B = A> constexpr bool op_dif_assign = !std::is_same_v<decltype(std::declval<A>() -= std::declval<B>()), NotExits>;//a-=b
            template<typename A, typename B = A> constexpr bool op_mul_assign = !std::is_same_v<decltype(std::declval<A>() *= std::declval<B>()), NotExits>;//a*=b
            template<typename A, typename B = A> constexpr bool op_div_assign = !std::is_same_v<decltype(std::declval<A>() /= std::declval<B>()), NotExits>;//a/=b
            template<typename A, typename B = A> constexpr bool op_mod_assign = !std::is_same_v<decltype(std::declval<A>() %= std::declval<B>()), NotExits>;//a%=b

            template<typename A, typename B = A> constexpr bool op_bitwise_or  = !std::is_same_v<decltype(std::declval<A>() | std::declval<B>()), NotExits>;//a|b
            template<typename A, typename B = A> constexpr bool op_bitwise_and = !std::is_same_v<decltype(std::declval<A>() & std::declval<B>()), NotExits>;//a&b
            template<typename A, typename B = A> constexpr bool op_bitwise_xor = !std::is_same_v<decltype(std::declval<A>() ^ std::declval<B>()), NotExits>;//a^b
            template<typename A, typename B = A> constexpr bool op_bitwise_lsh = !std::is_same_v<decltype(std::declval<A>() << std::declval<B>()), NotExits>;//a<<b
            template<typename A, typename B = A> constexpr bool op_bitwise_rsh = !std::is_same_v<decltype(std::declval<A>() >> std::declval<B>()), NotExits>;//a>>b

            template<typename A, typename B = A> constexpr bool op_bitwise_or_assing  = !std::is_same_v<decltype(std::declval<A>() |= std::declval<B>()), NotExits>;//a|=b
            template<typename A, typename B = A> constexpr bool op_bitwise_and_assing = !std::is_same_v<decltype(std::declval<A>() &= std::declval<B>()), NotExits>;//a&=b
            template<typename A, typename B = A> constexpr bool op_bitwise_xor_assing = !std::is_same_v<decltype(std::declval<A>() ^= std::declval<B>()), NotExits>;//a^=b
            template<typename A, typename B = A> constexpr bool op_bitwise_lsh_assing = !std::is_same_v<decltype(std::declval<A>() <<= std::declval<B>()), NotExits>;//a<<=b
            template<typename A, typename B = A> constexpr bool op_bitwise_rsh_assing = !std::is_same_v<decltype(std::declval<A>() >>= std::declval<B>()), NotExits>;//a>>=b

            template<typename A, typename B = A> constexpr bool op_equal =        !std::is_same_v<decltype(std::declval<A>() == std::declval<B>()), NotExits>;//a==b
            template<typename A, typename B = A> constexpr bool op_not_equal =    !std::is_same_v<decltype(std::declval<A>() != std::declval<B>()), NotExits>;//a!=b
            template<typename A, typename B = A> constexpr bool op_less =         !std::is_same_v<decltype(std::declval<A>() < std::declval<B>()), NotExits>; //a<b
            template<typename A, typename B = A> constexpr bool op_greater =      !std::is_same_v<decltype(std::declval<A>() > std::declval<B>()), NotExits>; //a>b
            template<typename A, typename B = A> constexpr bool op_not_less =     !std::is_same_v<decltype(std::declval<A>() >= std::declval<B>()), NotExits>;//a>=b
            template<typename A, typename B = A> constexpr bool op_not_greater =  !std::is_same_v<decltype(std::declval<A>() <= std::declval<B>()), NotExits>;//a<=b

            template<typename A, typename B = A> constexpr bool op_or  = !std::is_same_v<decltype(std::declval<A>() || std::declval<B>()), NotExits>;//a||b
            template<typename A, typename B = A> constexpr bool op_and = !std::is_same_v<decltype(std::declval<A>() && std::declval<B>()), NotExits>;//a&&b

        }//namespace OperatorsExistCheck
    }//namespace details

    template<typename T> using arr = details::array_value<T>;
}//namespace SGL

#endif//SGL_UTILS_HPP_INCLUDE_