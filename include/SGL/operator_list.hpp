#pragma once
#ifndef SGL_OPERATOR_LIST_HPP_INCLUDE_
#define SGL_OPERATOR_LIST_HPP_INCLUDE_

#include "config.hpp"
#include "function.hpp"
#include "utils.hpp"

#include <array>
#include <cstdint>

namespace SGL {
    enum class operator_type : uint8_t {
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
    constexpr size_t operators_count = static_cast<size_t>(operator_type::__op_count);

    //TODO add operator priority list

    class operator_list {
    public:
        value call_operator(operator_type op, std::initializer_list<std::reference_wrapper<value>> args) const {
            return m_operators[static_cast<size_t>(op)].call(args);
        };
        template<operator_type op, typename Ret, typename... Args> 
        void add_operator(std::function<Ret(Args...)> op_func) {
            constexpr size_t op_index = static_cast<size_t>(op);
            static_assert(op_index < operators_count, "invalid operator index");
            m_operators[op_index].add_overload(op_func);
        }

        //TODO replace with add operators between <A, B=A>
        template<typename T>
        void add_default_operators_for_type() {
            //if constexpr(details::has_op_unary_plus<T>) add_operator<operator_type::op_unary_plus>(get_unary_operator_func<T, operator_type::op_unary_plus>());
            //TODO: in C++20 branch if constexpr(details::)
        }
    protected:
        template<typename T, operator_type op>
        auto get_unary_operator_func() {
   
            if constexpr(operator_type::op_unary_plus  == op) return std::function<decltype(+std::declval<const T&>())(const T&)>([](const T& v) { return +v; });
            if constexpr(operator_type::op_unary_minus == op) return std::function<decltype(-std::declval<const T&>())(const T&)>([](const T& v) { return -v; });
            
        }



        template<operator_type op> 
        void add_operator(const function::function_overload& op_func) {
            constexpr size_t op_index = static_cast<size_t>(op);
            static_assert(op_index < operators_count, "invalid operator index");
            m_operators[op_index].add_overload(op_func);
        }
        friend class state;
        std::array<function, operators_count> m_operators;
    };
}//namespace SGL

#endif//SGL_OPERATOR_LIST_HPP_INCLUDE_