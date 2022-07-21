#pragma once
#ifndef SGL_EVALUATOR_HPP_INCLUDE_
#define SGL_EVALUATOR_HPP_INCLUDE_

#include "config.hpp"
#include "state.hpp"
#include "value.hpp"

namespace SGL {
    namespace details {
        struct eval_impl_args {
            const char** cur_end = nullptr;
            uint8_t call_prior = 16;//static_cast<uint8_t>(operator_precedence_step);
            bool is_in_function : 1 = false;
            bool is_in_ternary  : 1 = false;
            bool is_in_brackets : 1 = false;

        };
        value eval_expr_rec_impl(const state& m_state, std::string_view base_str, std::string_view str, eval_impl_args args);
    };
    class evaluator {
    public:
        value evaluate_expression(std::string_view str) {
            return details::eval_expr_rec_impl(m_state, str, str, {});
        }
    protected:
        friend class state;
        [[nodiscard]] explicit evaluator(const state& state) : m_state(state) {}

        const state& m_state;
    };
}//namespace SGL

#endif//SGL_EVALUATOR_HPP_INCLUDE_