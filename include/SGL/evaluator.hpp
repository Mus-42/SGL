#pragma once
#ifndef SGL_EVALUATOR_HPP_INCLUDE_
#define SGL_EVALUATOR_HPP_INCLUDE_

#include "config.hpp"
#include "state.hpp"
#include "tokenizer.hpp"

namespace SGL {
    class evaluator {
    public:
        [[nodiscard]] explicit evaluator() = default;//TODO evaluator(state&) instead of default constructor?

    //protected:
        value evaluate(tokenizer&& tk) {
            /*
                identifier can be reserved keyword, typename or user-defined name
                operator - use operator_list.hpp enum?
            */
            for(auto& l : tk.m_tokens) {
                for(auto& v : l) {
                    switch (v.type) {
                    case details::token::t_identifier: std::cout << "[identifier '" << v.identifier_v << "'] "; break;
                    case details::token::t_none: std::cout << "[none] "; break;
                    case details::token::t_operator: std::cout << "[operator '" << v.operator_v << "'] "; break;
                    case details::token::t_punct: std::cout << "[punct '" << v.punct_v << "'] "; break;
                    case details::token::t_value: std::cout << "[value " << v.value_v.get<uint64_t>() << "] "; break;//TODO replace this get with to_string or something like this   
                    default: std::cout << "[invalid token] "; break;
                    }
                }
                std::cout << std::endl;
            }


            return value();
        }
    };
}//namespace SGL

#endif//SGL_EVALUATOR_HPP_INCLUDE_