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
        op_none = 0,//TODO remove?
        //unary: +a, -a, ++a, --a, a++, a--
        op_unary_plus, op_unary_minus, //op_prefix_incr, op_prefix_decr, op_postfix_incr, op_postfix_decr,
        //ariphmetic: a+b, a-b, a*b, a/b, a%b
        op_sum, op_sub, op_mul, op_div, op_mod,
        //ariphmetic assigment: a+=b, a-=b, a*=b, a/=b, a%=b
        //op_sum_assign, op_sub_assign, op_mul_assign, op_div_assign, op_mod_assign,
        //bitwise: a|b, a&b, a^b, ~a, a<<b, a>>b
        op_bit_or, op_bit_and, op_bit_xor, op_bit_not, op_bit_lsh, op_bit_rsh,
        //bitwise assigment: a|=b, a&=b, a^=b, a<<=b, a>>=b
        //op_bit_or_assign, op_bit_and_assign, op_bit_xor_assign, op_bit_lsh_assign, op_bit_rsh_assign, 
        //compare: a==b, a!=b, a<b, a>b, a<=b, a>=b //TODO three-way compare?
        op_equal, op_not_equal, op_less, op_greater, op_not_less, op_not_greater,
        //logic: a||b, a&&b, !a
        op_or, op_and, op_not,
        //pointer: &a, *a
        op_adress_of, op_deref,
        //TODO add other (subscript, ...)

        __op_count,

        op_assign, // a=b (non overloadable operator)
    };
    constexpr size_t operators_count = static_cast<size_t>(operator_type::__op_count);

    constexpr std::array<uint8_t, operators_count> operator_precedence = {//TODO add associativity for operators (unary + - ! ~ ...)? (l to r (->) or r to l (<-)) (now all operators is l to r)
        //op_none
        0, 
        //op_unary_plus, op_unary_minus, //op_prefix_incr, op_prefix_decr, op_postfix_incr, op_postfix_decr,
        3, 3,//3, 3, 2, 2,
        //op_sum, op_sub, op_mul, op_div, op_mod,
        6, 6, 5, 5, 5,
        //op_sum_assign, op_sub_assign, op_mul_assign, op_div_assign, op_mod_assign,
        //16, 16, 16, 16, 16,
        //op_bit_or, op_bit_and, op_bit_xor, op_bit_not, op_bit_lsh, op_bit_rsh,
        13, 11, 12, 3, 7, 7,
        //op_bit_or_assign, op_bit_and_assign, op_bit_xor_assign, op_bit_lsh_assign, op_bit_rsh_assign, 
        //...
        //op_equal, op_not_equal, op_less, op_greater, op_not_less, op_not_greater,
        10, 10, 9, 9, 9, 9,
        //op_or, op_and, op_not,
        15, 14, 3,
        //op_adress_of, op_deref,
        3, 3,
    };
    constexpr size_t operator_precedence_step = 16;// > max(operator_precedence)

    //TODO: use something more efficient than SGL::function to choose overload

    class operator_list {
    public:
        value call_operator(operator_type op, std::initializer_list<std::reference_wrapper<value>> args) const {
            return m_operators[static_cast<size_t>(op)].call(args);
        };
        void add_operator(operator_type op, auto op_func) {
            m_operators[static_cast<size_t>(op)].add_overload(op_func);
        }

        //TODO replace with add operators between <A, B=A>
        template<typename T> 
        void add_default_operators_for_type() requires details::req_base_type<T> {
            add_default_unary_operators_for_type<T>();
            add_default_binary_operators_between_types<T>();
        }

//disable warnings for add_operator lambdas:
#ifdef SGL_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:4018 4146 4389 4804)
#endif//SGL_COMPILER_MSVC
#ifdef SGL_COMPILER_CLANG 
#pragma clang diagonstic push
#pragma clang diagnostic ignored "-Wbool-operation"
#endif//SGL_COMPILER_CLANG

//TODO disable warnings for GCC?

        template<typename T>
        constexpr void add_default_unary_operators_for_type() requires details::req_base_type<T> {
            if constexpr(!std::is_same_v<T, void>) {
                using T_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

                if constexpr(details::has_op_unary_plus  <T_t>) add_operator(operator_type::op_unary_plus  , static_cast<decltype(+std::declval<T_t>()) (*)(T_t)>([](T_t v) { return +v;  }));
                if constexpr(details::has_op_unary_minus <T_t>) add_operator(operator_type::op_unary_minus , static_cast<decltype(-std::declval<T_t>()) (*)(T_t)>([](T_t v) { return -v;  }));
                if constexpr(details::has_op_bit_not     <T_t>) add_operator(operator_type::op_bit_not     , static_cast<decltype(~std::declval<T_t>()) (*)(T_t)>([](T_t v) { return ~v;  }));
                if constexpr(details::has_op_not         <T_t>) add_operator(operator_type::op_not         , static_cast<decltype(!std::declval<T_t>()) (*)(T_t)>([](T_t v) { return !v;  }));
                if constexpr(details::has_op_deref       <T_t>) add_operator(operator_type::op_deref       , static_cast<decltype(*std::declval<T_t>()) (*)(T_t)>([](T_t v) { return *v;  }));
                if constexpr(details::has_op_adress_of   <T_t>) add_operator(operator_type::op_adress_of   , static_cast<decltype(&std::declval<T_t>()) (*)(T_t)>([](T_t v) { return &v;  }));
                //if constexpr(details::has_op_prefix_incr <T_t>) add_operator(operator_type::op_prefix_incr , static_cast<decltype(++std::declval<T_t>())(*)(T_t)>([](T_t v) { return ++v; }));
                //if constexpr(details::has_op_prefix_decr <T_t>) add_operator(operator_type::op_prefix_decr , static_cast<decltype(--std::declval<T_t>())(*)(T_t)>([](T_t v) { return --v; }));
                //if constexpr(details::has_op_postfix_incr<T_t>) add_operator(operator_type::op_postfix_incr, static_cast<decltype(std::declval<T_t>()++)(*)(T_t)>([](T_t v) { return v++; }));
                //if constexpr(details::has_op_postfix_decr<T_t>) add_operator(operator_type::op_postfix_decr, static_cast<decltype(std::declval<T_t>()--)(*)(T_t)>([](T_t v) { return v--; }));
            }
        }

        template<typename A, typename B = A>
        constexpr void add_default_binary_operators_between_types() requires details::req_base_type<A> && details::req_base_type<B> {
            if constexpr(!std::is_same_v<A, void> && !std::is_same_v<B, void>) {
                using A_t = std::add_lvalue_reference_t<std::add_const_t<A>>;
                using B_t = std::add_lvalue_reference_t<std::add_const_t<B>>;

                if constexpr(details::has_op_sum           <A_t, B_t>) add_operator(operator_type::op_sum           , static_cast<decltype(std::declval<A_t>() +   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a +   b; }));
                if constexpr(details::has_op_sub           <A_t, B_t>) add_operator(operator_type::op_sub           , static_cast<decltype(std::declval<A_t>() -   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a -   b; }));
                if constexpr(details::has_op_mul           <A_t, B_t>) add_operator(operator_type::op_mul           , static_cast<decltype(std::declval<A_t>() *   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a *   b; }));
                if constexpr(details::has_op_div           <A_t, B_t>) add_operator(operator_type::op_div           , static_cast<decltype(std::declval<A_t>() /   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a /   b; }));
                if constexpr(details::has_op_mod           <A_t, B_t>) add_operator(operator_type::op_mod           , static_cast<decltype(std::declval<A_t>() %   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a %   b; }));
                //if constexpr(details::has_op_sum_assign    <A_t, B_t>) add_operator(operator_type::op_sum_assign    , static_cast<decltype(std::declval<A_t>() +=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a +=  b; }));
                //if constexpr(details::has_op_sub_assign    <A_t, B_t>) add_operator(operator_type::op_sub_assign    , static_cast<decltype(std::declval<A_t>() -=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a -=  b; }));
                //if constexpr(details::has_op_mul_assign    <A_t, B_t>) add_operator(operator_type::op_mul_assign    , static_cast<decltype(std::declval<A_t>() *=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a *=  b; }));
                //if constexpr(details::has_op_div_assign    <A_t, B_t>) add_operator(operator_type::op_div_assign    , static_cast<decltype(std::declval<A_t>() /=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a /=  b; }));
                //if constexpr(details::has_op_mod_assign    <A_t, B_t>) add_operator(operator_type::op_mod_assign    , static_cast<decltype(std::declval<A_t>() %=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a %=  b; }));
                if constexpr(details::has_op_bit_or        <A_t, B_t>) add_operator(operator_type::op_bit_or        , static_cast<decltype(std::declval<A_t>() |   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a |   b; }));
                if constexpr(details::has_op_bit_and       <A_t, B_t>) add_operator(operator_type::op_bit_and       , static_cast<decltype(std::declval<A_t>() &   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a &   b; }));
                if constexpr(details::has_op_bit_xor       <A_t, B_t>) add_operator(operator_type::op_bit_xor       , static_cast<decltype(std::declval<A_t>() ^   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a ^   b; }));
                if constexpr(details::has_op_bit_lsh       <A_t, B_t>) add_operator(operator_type::op_bit_lsh       , static_cast<decltype(std::declval<A_t>() <<  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a <<  b; }));
                if constexpr(details::has_op_bit_rsh       <A_t, B_t>) add_operator(operator_type::op_bit_rsh       , static_cast<decltype(std::declval<A_t>() >>  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a >>  b; }));
                //if constexpr(details::has_op_bit_or_assign <A_t, B_t>) add_operator(operator_type::op_bit_or_assign , static_cast<decltype(std::declval<A_t>() |=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a |=  b; }));
                //if constexpr(details::has_op_bit_and_assign<A_t, B_t>) add_operator(operator_type::op_bit_and_assign, static_cast<decltype(std::declval<A_t>() &=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a &=  b; }));
                //if constexpr(details::has_op_bit_xor_assign<A_t, B_t>) add_operator(operator_type::op_bit_xor_assign, static_cast<decltype(std::declval<A_t>() ^=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a ^=  b; }));
                //if constexpr(details::has_op_bit_lsh_assign<A_t, B_t>) add_operator(operator_type::op_bit_lsh_assign, static_cast<decltype(std::declval<A_t>() <<= std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a <<= b; }));
                //if constexpr(details::has_op_bit_rsh_assign<A_t, B_t>) add_operator(operator_type::op_bit_rsh_assign, static_cast<decltype(std::declval<A_t>() >>= std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a >>= b; }));
                if constexpr(details::has_op_equal         <A_t, B_t>) add_operator(operator_type::op_equal         , static_cast<decltype(std::declval<A_t>() ==  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a ==  b; }));
                if constexpr(details::has_op_not_equal     <A_t, B_t>) add_operator(operator_type::op_not_equal     , static_cast<decltype(std::declval<A_t>() !=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a !=  b; }));
                if constexpr(details::has_op_less          <A_t, B_t>) add_operator(operator_type::op_less          , static_cast<decltype(std::declval<A_t>() >   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a >   b; }));
                if constexpr(details::has_op_greater       <A_t, B_t>) add_operator(operator_type::op_greater       , static_cast<decltype(std::declval<A_t>() <   std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a <   b; }));
                if constexpr(details::has_op_not_less      <A_t, B_t>) add_operator(operator_type::op_not_less      , static_cast<decltype(std::declval<A_t>() >=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a >=  b; }));
                if constexpr(details::has_op_not_greater   <A_t, B_t>) add_operator(operator_type::op_not_greater   , static_cast<decltype(std::declval<A_t>() <=  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a <=  b; }));
                if constexpr(details::has_op_or            <A_t, B_t>) add_operator(operator_type::op_or            , static_cast<decltype(std::declval<A_t>() ||  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a ||  b; }));
                if constexpr(details::has_op_and           <A_t, B_t>) add_operator(operator_type::op_and           , static_cast<decltype(std::declval<A_t>() &&  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a &&  b; }));
            }
        }

#ifdef SGL_COMPILER_MSVC
#pragma warning(pop)
#endif//SGL_COMPILER_MSVC
#ifdef SGL_COMPILER_CLANG 
#pragma clang diagonstic pop
#endif//SGL_COMPILER_CLANG

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