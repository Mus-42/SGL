#pragma once
#ifndef SGL_EVALUATOR_HPP_INCLUDE_
#define SGL_EVALUATOR_HPP_INCLUDE_

#include "config.hpp"
#include "state.hpp"
#include "tokenizer.hpp"

namespace SGL {
    class evaluator {
    public:
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
                    case details::token::t_value: std::cout << "[value `" << 
                        v.value_v.to_string()
//#if defined(SGL_OPTION_STORE_TYPE_NAME) && SGL_OPTION_STORE_TYPE_NAME
//                        v.value_v.m_type->m_base_type->m_type_name
//#else
//                        v.value_v.m_type->m_base_type->m_type.name() 
//#endif//SGL_OPTION_STORE_TYPE_NAME
                        << "`] "; break; 
                    default: std::cout << "[invalid token] "; break;
                    }
                }
                std::cout << std::endl;
            }
            //TODO choose operators, sort in evaluation order, evaluate


            return value();
        }

    protected:
        friend class state;

        //[[nodiscard]] explicit evaluator() = default;//TODO evaluator(state&) instead of default constructor?
        [[nodiscard]] explicit evaluator(const state& state) : m_state(state) {}

        const state& m_state;
    };
}//namespace SGL

#endif//SGL_EVALUATOR_HPP_INCLUDE_