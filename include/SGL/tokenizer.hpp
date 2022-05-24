#pragma once
#ifndef SGL_TOKENIZER_HPP_INCLUDE_
#define SGL_TOKENIZER_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"
#include "value.hpp"

#include <cstdint>
#include <vector>
#include <list>

namespace SGL {
    class token {
    public:
        enum token_type {
            t_none,
            t_value,//1, 14.34, "str" (constant in tokenizer)
            t_punct,// {} () , and other
            t_operator,// + ^ -> etc.
            t_identifier,//type name or constant or variable... (evaluator chose what it mean)
        };

        explicit token(token_type t, int p) : type(t), priority(p) {         
            switch (type) {
            case t_none:       break;   
            case t_value:      new (&value_v) value; break;   
            case t_punct:      break;   
            case t_operator:   break;   
            case t_identifier: new (&identifier_v) std::string; break;   
            default: break;
            }
        }
        explicit token(const token& tk) : type(tk.type), priority(tk.priority) {
            switch (type) {
            case t_none:       break;   
            case t_value:      new (&value_v) value(tk.value_v); break;   
            case t_punct:      punct_v = tk.punct_v; break;   
            case t_operator:   operator_v = tk.operator_v; break;   
            case t_identifier: new (&identifier_v) std::string(tk.identifier_v); break;   
            default: break;
            }
        }
        ~token() {
            switch (type) {
            case t_none:       break;   
            case t_value:      value_v.~value(); break;   
            case t_punct:      break;   
            case t_operator:   break;   
            case t_identifier: identifier_v.~basic_string(); break;   
            default: break;
            }
        }

        const token_type type;
        const int priority;
        union {
            struct { } none_v;//OK...
            value value_v;
            char punct_v;
            std::string_view operator_v;
            std::string identifier_v;
        };
    };
    class tokenizer : public no_copy {
    public:
        //TODO add settings such as "ignore trailing comma"?
        tokenizer(std::string_view str);//TODO add char type as template arg?
        ~tokenizer() = default;
    protected:
        friend class evaluator;

        std::vector<std::list<token>> m_tokens;
    };
}//namespace SGL

#endif//SGL_TOKENIZER_HPP_INCLUDE_