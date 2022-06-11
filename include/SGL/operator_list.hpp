#pragma once
#ifndef SGL_OPERATOR_LIST_HPP_INCLUDE_
#define SGL_OPERATOR_LIST_HPP_INCLUDE_

#include "config.hpp"
#include <cstdint>

namespace SGL {
    enum class operator_list : uint8_t {
        op_none = 0,
        //unary: +a, -a, ++a, --a, a++, a--
        op_unary_plus, op_unary_minus, op_prefix_incr, op_prefix_decr, op_postfix_incr, op_postfix_decr,
        //ariphmetic: a+b, a-b, a*b, a/b, a%b
        op_sum, op_sub, op_mul, op_div, op_mod,
        //ariphmetic assigment: a+=b, a-=b, a*=b, a/=b, a%=b
        op_sum_assign, op_sub_assign, op_mul_assign, op_div_assign, op_mod_assign,
        //bitwise: a|b, a&b, a^b, ~a, a<<b, a>>b
        op_bit_or, op_bit_and, op_bit_xor, op_bit_not, op_bit_lsh, op_bit_rsh,
        //bitwise assigment: a|=b, a&=b, a^=b, a<<=b, a>>=b
        op_bit_or_assign, op_bit_and_assign, op_bit_xor_assign, op_bit_lsh_assign, op_bit_rsh_assign, 
        //compare: a==b, a!=b, a<b, a>b, a<=b, a>=b //TODO three-way compare?
        op_equal, op_not_equal, op_less, op_greater, op_not_less, op_not_greater,
        //logic: a||b, a&&b, !a
        op_or, op_and, op_not,
        //pointer: &a, *a
        op_adress_of, op_deref,
        //TODO add other
        __op_count
    };
    constexpr size_t op_list_operator_count = static_cast<size_t>(operator_list::__op_count);
}//namespace SGL

#endif//SGL_OPERATOR_LIST_HPP_INCLUDE_