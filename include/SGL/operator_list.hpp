#pragma once
#ifndef SGL_OPERATOR_LIST_HPP_INCLUDE_
#define SGL_OPERATOR_LIST_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"

#include <array>
#include <map>
//#include <functional>
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
        //compare: a==b, a!=b, a<b, a>b, a>=b, a<=b //TODO three-way compare?
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
    constexpr std::array<std::string_view, operators_count> operator_str = {
        "[none]",
        "+", "-",
        "+", "-", "*", "/", "%",
        "|", "&", "^", "~", "<<", ">>",
        "==", "!=", "<", ">", ">=", "<=",
        "||", "&&", "!",
        "&", "*"
    };


    //TODO: use something more efficient than SGL::function to choose overload

    class operator_list {
    public:

        value call_unary_operator(operator_type op, value& arg) const {
            //TODO fix it
            size_t op_idex = static_cast<size_t>(op);
            using it = typename std::remove_cvref_t<decltype(m_unary_operators[op_idex])>::const_iterator;
            it res;
            auto& m_ops = m_unary_operators[op_idex]; 
            {
                auto t_v = *arg.m_type;
                res = m_ops.find(t_v);//T
                if(res == m_ops.end()) {         
                    auto t_ref_v = t_v;
                    t_ref_v.add_reference();
                    res = m_ops.find(t_ref_v);//T&
                    if(res == m_ops.end()) {
                        auto t_const_v = t_v;
                        t_const_v.add_const();
                        res = m_ops.find(t_const_v);//const T
                        if(res == m_ops.end()) 
                            res = m_ops.find(t_ref_v.add_const());//const T&
                    }
                }
            }

            if(res == m_ops.end()) [[unlikely]] throw std::runtime_error("can't find unary operator `" + std::string(operator_str[op_idex]) + "` for type T=" + arg.m_type->type_to_str());
            auto [f, impl] = res->second;
            return impl(f, arg);
        }
        value call_binary_operator(operator_type op, value& arg1, value& arg2) const {
            size_t op_idex = static_cast<size_t>(op);
            using it1 = typename std::remove_cvref_t<decltype(m_binary_operators[op_idex])>::const_iterator;
            using it2 = typename std::remove_cvref_t<decltype(m_binary_operators[op_idex].at(std::declval<type>()))>::const_iterator;
            it1 res1;
            it2 res2;
            
            auto& op1 = m_binary_operators[op_idex];

            {
                auto t_v = *arg1.m_type;
                res1 = op1.find(t_v);//T
                if(res1 == op1.end()) {         
                    auto t_ref_v = t_v;
                    t_ref_v.add_reference();
                    res1 = op1.find(t_ref_v);//T&
                    if(res1 == op1.end()) {
                        auto t_const_v = t_v;
                        t_const_v.add_const();
                        res1 = op1.find(t_const_v);//const T
                        if(res1 == op1.end()) 
                            res1 = op1.find(t_ref_v.add_const());//const T&
                    }
                }
            }

            if(res1 == op1.end()) [[unlikely]] 
                throw std::runtime_error("can't find binary operator `" + std::string(operator_str[op_idex]) + "` for types Arg1=" + arg1.m_type->type_to_str() + " Arg2="+ arg2.m_type->type_to_str());
            auto& op2 = res1->second;
            {
                auto t_v = *arg2.m_type;
                res2 = op2.find(t_v);//T
                if(res2 == op2.end()) {         
                    auto t_ref_v = t_v;
                    t_ref_v.add_reference();
                    res2 = op2.find(t_ref_v);//T&
                    if(res2 == op2.end()) {
                        auto t_const_v = t_v;
                        t_const_v.add_const();
                        res2 = op2.find(t_const_v);//const T
                        if(res2 == op2.end()) 
                            res2 = op2.find(t_ref_v.add_const());//const T&
                    }
                }
            }

            if(res2 == op2.end()) [[unlikely]] 
                throw std::runtime_error("can't find binary operator `" + std::string(operator_str[op_idex]) + "` for types Arg1=" + arg1.m_type->type_to_str() + " Arg2="+ arg2.m_type->type_to_str());
            auto [f, impl] = res2->second;
            return impl(f, arg1, arg2);
        }
        //TODO replace function pointer with std::function?
        template<typename Ret, typename Arg>
        void add_unary_operator(operator_type op, Ret(*f)(Arg)) {
            m_unary_operators[static_cast<size_t>(op)][*type::construct_type<Arg>()] = {static_cast<void*>(f), static_cast<value(*)(void*, value&)>([](void* f, value& arg1)->value{
                return value(val<Ret>(static_cast<Ret(*)(Arg)>(f)(arg1.get<Arg>())));
            })};
        }
        template<typename Ret, typename Arg1, typename Arg2>
        void add_binary_operator(operator_type op, Ret(*f)(Arg1, Arg2)) {
            m_binary_operators[static_cast<size_t>(op)][*type::construct_type<Arg1>()][*type::construct_type<Arg2>()] = {static_cast<void*>(f), static_cast<value(*)(void*, value&, value&)>([](void* f, value& arg1, value& arg2)->value{
                return value(val<Ret>(static_cast<Ret(*)(Arg1, Arg2)>(f)(arg1.get<Arg1>(), arg2.get<Arg2>())));
            })};
        }

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
#if defined(SGL_COMPILER_CLANG) || defined(SGL_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbool-operation"
#endif//CLANG | GCC

        template<typename T>
        constexpr void add_default_unary_operators_for_type() requires details::req_base_type<T> {
            if constexpr(!std::is_same_v<T, void>) {
                using T_t = std::add_lvalue_reference_t<std::add_const_t<T>>;
                
                if constexpr(details::has_op_unary_plus <T_t>) add_unary_operator(operator_type::op_unary_plus , static_cast<decltype(+std::declval<T_t>())(*)(T_t)>([](T_t v){ return +v;  }));
                if constexpr(details::has_op_unary_minus<T_t>) add_unary_operator(operator_type::op_unary_minus, static_cast<decltype(-std::declval<T_t>())(*)(T_t)>([](T_t v){ return -v;  }));
                if constexpr(details::has_op_bit_not    <T_t>) add_unary_operator(operator_type::op_bit_not    , static_cast<decltype(~std::declval<T_t>())(*)(T_t)>([](T_t v){ return ~v;  }));
                if constexpr(details::has_op_not        <T_t>) add_unary_operator(operator_type::op_not        , static_cast<decltype(!std::declval<T_t>())(*)(T_t)>([](T_t v){ return !v;  }));
                if constexpr(details::has_op_deref      <T_t>) add_unary_operator(operator_type::op_deref      , static_cast<decltype(*std::declval<T_t>())(*)(T_t)>([](T_t v){ return *v;  }));
                if constexpr(details::has_op_adress_of  <T_t>) add_unary_operator(operator_type::op_adress_of  , static_cast<decltype(&std::declval<T_t>())(*)(T_t)>([](T_t v){ return &v;  }));
            }
        }

        template<typename A, typename B = A>
        constexpr void add_default_binary_operators_between_types() requires details::req_base_type<A> && details::req_base_type<B> {
            if constexpr(!std::is_same_v<A, void> && !std::is_same_v<B, void>) {
                using A_t = std::add_lvalue_reference_t<std::add_const_t<A>>;
                using B_t = std::add_lvalue_reference_t<std::add_const_t<B>>;

                if constexpr(details::has_op_sum        <A_t, B_t>) add_binary_operator(operator_type::op_sum        , static_cast<decltype(std::declval<A_t>() +  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a +  b; }));
                if constexpr(details::has_op_sub        <A_t, B_t>) add_binary_operator(operator_type::op_sub        , static_cast<decltype(std::declval<A_t>() -  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a -  b; }));
                if constexpr(details::has_op_mul        <A_t, B_t>) add_binary_operator(operator_type::op_mul        , static_cast<decltype(std::declval<A_t>() *  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a *  b; }));
                if constexpr(details::has_op_div        <A_t, B_t>) add_binary_operator(operator_type::op_div        , static_cast<decltype(std::declval<A_t>() /  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a /  b; }));
                if constexpr(details::has_op_mod        <A_t, B_t>) add_binary_operator(operator_type::op_mod        , static_cast<decltype(std::declval<A_t>() %  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a %  b; }));
                if constexpr(details::has_op_bit_or     <A_t, B_t>) add_binary_operator(operator_type::op_bit_or     , static_cast<decltype(std::declval<A_t>() |  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a |  b; }));
                if constexpr(details::has_op_bit_and    <A_t, B_t>) add_binary_operator(operator_type::op_bit_and    , static_cast<decltype(std::declval<A_t>() &  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a &  b; }));
                if constexpr(details::has_op_bit_xor    <A_t, B_t>) add_binary_operator(operator_type::op_bit_xor    , static_cast<decltype(std::declval<A_t>() ^  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a ^  b; }));
                if constexpr(details::has_op_bit_lsh    <A_t, B_t>) add_binary_operator(operator_type::op_bit_lsh    , static_cast<decltype(std::declval<A_t>() << std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a << b; }));
                if constexpr(details::has_op_bit_rsh    <A_t, B_t>) add_binary_operator(operator_type::op_bit_rsh    , static_cast<decltype(std::declval<A_t>() >> std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a >> b; }));
                if constexpr(details::has_op_equal      <A_t, B_t>) add_binary_operator(operator_type::op_equal      , static_cast<decltype(std::declval<A_t>() == std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a == b; }));
                if constexpr(details::has_op_not_equal  <A_t, B_t>) add_binary_operator(operator_type::op_not_equal  , static_cast<decltype(std::declval<A_t>() != std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a != b; }));
                if constexpr(details::has_op_less       <A_t, B_t>) add_binary_operator(operator_type::op_less       , static_cast<decltype(std::declval<A_t>() <  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a <  b; }));
                if constexpr(details::has_op_greater    <A_t, B_t>) add_binary_operator(operator_type::op_greater    , static_cast<decltype(std::declval<A_t>() >  std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a >  b; }));
                if constexpr(details::has_op_not_less   <A_t, B_t>) add_binary_operator(operator_type::op_not_less   , static_cast<decltype(std::declval<A_t>() >= std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a >= b; }));
                if constexpr(details::has_op_not_greater<A_t, B_t>) add_binary_operator(operator_type::op_not_greater, static_cast<decltype(std::declval<A_t>() <= std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a <= b; }));
                if constexpr(details::has_op_or         <A_t, B_t>) add_binary_operator(operator_type::op_or         , static_cast<decltype(std::declval<A_t>() || std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a || b; }));
                if constexpr(details::has_op_and        <A_t, B_t>) add_binary_operator(operator_type::op_and        , static_cast<decltype(std::declval<A_t>() && std::declval<B_t>())(*)(A_t, B_t)>([](A_t a, B_t b){ return a && b; }));
            }
        }

#ifdef SGL_COMPILER_MSVC
#pragma warning(pop)
#endif//SGL_COMPILER_MSVC
#if defined(SGL_COMPILER_CLANG) || defined(SGL_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif//CLANG | GCC

    protected:
        friend class state;

        std::array<std::map<type, std::pair<void*, value(*)(void*, value&)>>, operators_count> m_unary_operators;
        std::array<std::map<type, std::map<type, std::pair<void*, value(*)(void*, value&, value&)>>>, operators_count> m_binary_operators;
    };
}//namespace SGL

#endif//SGL_OPERATOR_LIST_HPP_INCLUDE_