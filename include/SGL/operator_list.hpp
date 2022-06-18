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
        void add_default_operators_for_type() requires(details::req_base_type<T>) {
            add_default_unary_operators_for_type<T>();
            add_default_binary_operators_between_types<T>();
        }
        template<typename T>
        void add_default_unary_operators_for_type() requires(details::req_base_type<T>) {
            if constexpr(!std::is_same_v<T, void>) {
                using T_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

                if constexpr(details::has_op_unary_plus  <T_t>) add_operator<operator_type::op_unary_plus  > (std::function<decltype(+std::declval<T_t>()) (T_t)>([](T_t v) { return +v;  }));
                if constexpr(details::has_op_unary_minus <T_t>) add_operator<operator_type::op_unary_minus > (std::function<decltype(-std::declval<T_t>()) (T_t)>([](T_t v) { return -v;  }));
                if constexpr(details::has_op_bit_not     <T_t>) add_operator<operator_type::op_bit_not     > (std::function<decltype(~std::declval<T_t>()) (T_t)>([](T_t v) { return ~v;  }));
                if constexpr(details::has_op_not         <T_t>) add_operator<operator_type::op_not         > (std::function<decltype(!std::declval<T_t>()) (T_t)>([](T_t v) { return !v;  }));
                if constexpr(details::has_op_deref       <T_t>) add_operator<operator_type::op_deref       > (std::function<decltype(*std::declval<T_t>()) (T_t)>([](T_t v) { return *v;  }));
                if constexpr(details::has_op_adress_of   <T_t>) add_operator<operator_type::op_adress_of   > (std::function<decltype(&std::declval<T_t>()) (T_t)>([](T_t v) { return &v;  }));
                if constexpr(details::has_op_prefix_incr <T_t>) add_operator<operator_type::op_prefix_incr > (std::function<decltype(++std::declval<T_t>())(T_t)>([](T_t v) { return ++v; }));
                if constexpr(details::has_op_prefix_decr <T_t>) add_operator<operator_type::op_prefix_decr > (std::function<decltype(--std::declval<T_t>())(T_t)>([](T_t v) { return --v; }));
                if constexpr(details::has_op_postfix_incr<T_t>) add_operator<operator_type::op_postfix_incr> (std::function<decltype(std::declval<T_t>()++)(T_t)>([](T_t v) { return v++; }));
                if constexpr(details::has_op_postfix_decr<T_t>) add_operator<operator_type::op_postfix_decr> (std::function<decltype(std::declval<T_t>()--)(T_t)>([](T_t v) { return v--; }));
            }
        }

        template<typename A, typename B = A>
        void add_default_binary_operators_between_types() requires(details::req_base_type<A> && details::req_base_type<B>) {
            if constexpr(!std::is_same_v<A, void> && !std::is_same_v<B, void>) {
                using A_t = std::add_lvalue_reference_t<std::add_const_t<A>>;
                using B_t = std::add_lvalue_reference_t<std::add_const_t<B>>;

                if constexpr(details::has_op_sum           <A_t, B_t>) add_operator<operator_type::op_sum           >(std::function<decltype(std::declval<A_t>() +   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a +   b; }));
                if constexpr(details::has_op_sub           <A_t, B_t>) add_operator<operator_type::op_sub           >(std::function<decltype(std::declval<A_t>() -   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a -   b; }));
                if constexpr(details::has_op_mul           <A_t, B_t>) add_operator<operator_type::op_mul           >(std::function<decltype(std::declval<A_t>() *   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a *   b; }));
                if constexpr(details::has_op_div           <A_t, B_t>) add_operator<operator_type::op_div           >(std::function<decltype(std::declval<A_t>() /   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a /   b; }));
                if constexpr(details::has_op_mod           <A_t, B_t>) add_operator<operator_type::op_mod           >(std::function<decltype(std::declval<A_t>() %   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a %   b; }));
                if constexpr(details::has_op_sum_assign    <A_t, B_t>) add_operator<operator_type::op_sum_assign    >(std::function<decltype(std::declval<A_t>() +=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a +=  b; }));
                if constexpr(details::has_op_sub_assign    <A_t, B_t>) add_operator<operator_type::op_sub_assign    >(std::function<decltype(std::declval<A_t>() -=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a -=  b; }));
                if constexpr(details::has_op_mul_assign    <A_t, B_t>) add_operator<operator_type::op_mul_assign    >(std::function<decltype(std::declval<A_t>() *=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a *=  b; }));
                if constexpr(details::has_op_div_assign    <A_t, B_t>) add_operator<operator_type::op_div_assign    >(std::function<decltype(std::declval<A_t>() /=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a /=  b; }));
                if constexpr(details::has_op_mod_assign    <A_t, B_t>) add_operator<operator_type::op_mod_assign    >(std::function<decltype(std::declval<A_t>() %=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a %=  b; }));
                if constexpr(details::has_op_bit_or        <A_t, B_t>) add_operator<operator_type::op_bit_or        >(std::function<decltype(std::declval<A_t>() |   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a |   b; }));
                if constexpr(details::has_op_bit_and       <A_t, B_t>) add_operator<operator_type::op_bit_and       >(std::function<decltype(std::declval<A_t>() &   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a &   b; }));
                if constexpr(details::has_op_bit_xor       <A_t, B_t>) add_operator<operator_type::op_bit_xor       >(std::function<decltype(std::declval<A_t>() ^   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a ^   b; }));
                if constexpr(details::has_op_bit_lsh       <A_t, B_t>) add_operator<operator_type::op_bit_lsh       >(std::function<decltype(std::declval<A_t>() <<  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a <<  b; }));
                if constexpr(details::has_op_bit_rsh       <A_t, B_t>) add_operator<operator_type::op_bit_rsh       >(std::function<decltype(std::declval<A_t>() >>  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a >>  b; }));
                if constexpr(details::has_op_bit_or_assign <A_t, B_t>) add_operator<operator_type::op_bit_or_assign >(std::function<decltype(std::declval<A_t>() |=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a |=  b; }));
                if constexpr(details::has_op_bit_and_assign<A_t, B_t>) add_operator<operator_type::op_bit_and_assign>(std::function<decltype(std::declval<A_t>() &=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a &=  b; }));
                if constexpr(details::has_op_bit_xor_assign<A_t, B_t>) add_operator<operator_type::op_bit_xor_assign>(std::function<decltype(std::declval<A_t>() ^=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a ^=  b; }));
                if constexpr(details::has_op_bit_lsh_assign<A_t, B_t>) add_operator<operator_type::op_bit_lsh_assign>(std::function<decltype(std::declval<A_t>() <<= std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a <<= b; }));
                if constexpr(details::has_op_bit_rsh_assign<A_t, B_t>) add_operator<operator_type::op_bit_rsh_assign>(std::function<decltype(std::declval<A_t>() >>= std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a >>= b; }));
                if constexpr(details::has_op_equal         <A_t, B_t>) add_operator<operator_type::op_equal         >(std::function<decltype(std::declval<A_t>() ==  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a ==  b; }));
                if constexpr(details::has_op_not_equal     <A_t, B_t>) add_operator<operator_type::op_not_equal     >(std::function<decltype(std::declval<A_t>() !=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a !=  b; }));
                if constexpr(details::has_op_less          <A_t, B_t>) add_operator<operator_type::op_less          >(std::function<decltype(std::declval<A_t>() >   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a >   b; }));
                if constexpr(details::has_op_greater       <A_t, B_t>) add_operator<operator_type::op_greater       >(std::function<decltype(std::declval<A_t>() <   std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a <   b; }));
                if constexpr(details::has_op_not_less      <A_t, B_t>) add_operator<operator_type::op_not_less      >(std::function<decltype(std::declval<A_t>() >=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a >=  b; }));
                if constexpr(details::has_op_not_greater   <A_t, B_t>) add_operator<operator_type::op_not_greater   >(std::function<decltype(std::declval<A_t>() <=  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a <=  b; }));
                if constexpr(details::has_op_or            <A_t, B_t>) add_operator<operator_type::op_or            >(std::function<decltype(std::declval<A_t>() ||  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a ||  b; }));
                if constexpr(details::has_op_and           <A_t, B_t>) add_operator<operator_type::op_and           >(std::function<decltype(std::declval<A_t>() &&  std::declval<B_t>())(A_t, B_t)>([](A_t a, B_t b){ return a &&  b; }));
            }
        }
    protected:
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