#pragma once
#ifndef SGL_EVALUATOR_HPP_INCLUDE_
#define SGL_EVALUATOR_HPP_INCLUDE_

#include "config.hpp"
#include "state.hpp"
#include "tokenizer.hpp"

namespace SGL {
    class evaluator {
    public:
        value evaluate(tokenizer&& tk);

        void print_tokens(const tokenizer::token_list& list) {
            for (auto& v : list) {
                switch (v.type) {
                case details::token::t_identifier: std::cout << "[identifier '" << v.identifier_v << "'] "; break;
                case details::token::t_none: std::cout << "[none] "; break;
                case details::token::t_operator: std::cout << "[operator '" << v.operator_str << "'] "; break;
                case details::token::t_punct: std::cout << "[punct '" << v.punct_v << "'] "; break;
                case details::token::t_value: std::cout << "[value `" << v.value_v.to_string() << "`] "; break;
                default: std::cout << "[invalid token] "; break;
                }
            }
            std::cout << std::endl;
        }

    protected:
        friend class state;

        //[[nodiscard]] explicit evaluator() = default;//TODO evaluator(state&) instead of default constructor?
        [[nodiscard]] explicit evaluator(const state& state) : m_state(state) {}

        const state& m_state;
    };
}//namespace SGL

#endif//SGL_EVALUATOR_HPP_INCLUDE_