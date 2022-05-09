#pragma once
#ifndef SGL_OPERATOR_LIST_HPP_INCLUDE_
#define SGL_OPERATOR_LIST_HPP_INCLUDE_

#include "config.hpp"
#include <cstdint>

namespace SGL {
    enum operator_list : uint8_t {
        //ariphmetic: +a, a+b, -a, a-b, a*b, a/b, a%b, a++, ++a, a--, --a
        op_sum, op_dif, op_mul, op_div, op_mod, op_incr, op_decr,
        //bitwise: a|b, a&b, a^b, ~a, a<<b, a>>b
        op_bit_or, op_bit_and, op_bit_xor, op_bit_not, op_bit_lsh, op_bit_rsh,
        //assigment:
        op_assign, op_

    };
}//namespace SGL

#endif//SGL_OPERATOR_LIST_HPP_INCLUDE_