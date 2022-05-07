#pragma once
#ifndef SGL_TOKENIZER_HPP_INCLUDE_
#define SGL_TOKENIZER_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"
#include "value.hpp"

#include <cstdint>

namespace SGL {
    class token {
    public:
        enum token_type {
            t_none,
            t_value,//1, 14.34, "str" ...
            t_punct,// {} () , and other
            t_operator,// + ^ -> etc.
        };
        const int priority;
        const token_type type;
        union {
            struct { } none_v;//OK...
            value value_v;
            char punct_v;
            //operator_v
        };
    };
    class tokenizer : public no_copy {
    public:
        //TODO add settings such as ignore trailing comma?
        tokenizer(std::string_view str) {}

        

        ~tokenizer() {}
    };
}

#endif//SGL_TOKENIZER_HPP_INCLUDE_