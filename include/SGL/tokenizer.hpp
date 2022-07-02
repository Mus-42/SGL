#pragma once
#ifndef SGL_TOKENIZER_HPP_INCLUDE_
#define SGL_TOKENIZER_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"
#include "value.hpp"
#include "operator_list.hpp"

#include <cstdint>
#include <cmath>//std::pow
#include <vector>
#include <array>
#include <list>

namespace SGL {
    namespace details {
    class token {
        public:
            enum token_type {
                t_none,
                t_value,//1, 14.34, "str" (constant in tokenizer)
                t_punct,// {} () , and other
                t_operator,// + ^ -> etc.
                t_identifier,//type name or constant or variable... (evaluator chose what it mean)
            };

            explicit token(token_type t, size_t p) : type(t), priority(p) {         
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
                case t_operator:   operator_v.type = tk.operator_v.type; operator_v.str = tk.operator_v.str; break;
                case t_identifier: new (&identifier_v) std::string(tk.identifier_v); break;   
                default: break;
                }
            }
            explicit token(token&& tk) : type(tk.type), priority(tk.priority) {
                switch (type) {
                case t_none:       break;   
                case t_value:      new (&value_v) value(std::move(tk.value_v)); break;   
                case t_punct:      punct_v = tk.punct_v; break;   
                case t_operator:   operator_v.type = tk.operator_v.type; operator_v.str = tk.operator_v.str;  break;
                case t_identifier: new (&identifier_v) std::string(std::move(tk.identifier_v)); break;   
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
            token& operator=(const token& tk) {
                this->~token();
                new (this) token(tk);
                return *this;
            }
            token& operator=(token&& tk) {
                this->~token();
                new (this) token(tk);
                return *this;
            }

            const token_type type;
            const size_t priority;
            union {
                value value_v;
                char punct_v;
                struct op_v_t  {
                    std::string_view str;//TODO remove?
                    operator_type type;
                } operator_v;
                std::string identifier_v;
            };
        };
    }//namespace details
    class tokenizer : public details::no_copy {
    public:
        tokenizer(tokenizer&& o) : m_tokens(std::move(o.m_tokens)) {}
        //TODO add settings such as "ignore trailing comma"?
        [[nodiscard]] explicit tokenizer(std::string_view str);//TODO add char type as template arg?
        ~tokenizer() = default;
    //protected:
        friend class evaluator;

        using token_list = std::list<details::token>;

        std::vector<token_list> m_tokens;
    };
    
    inline tokenizer::tokenizer(std::string_view str) : m_tokens(1) {
        size_t cur = 0;
        auto skip_spaces_and_comments = [&cur, str](){
            while(cur < str.size() && (std::isspace(str[cur]) || str[cur] == '/')) {
                if(str[cur] == '/') {
                    if(cur + 1 < str.size()) {
                        if(str[cur+1] == '/') cur = str.find('\n', cur) + 1;
                        else if(str[cur+1] == '*') cur = str.find("*/", cur) + 2;
                        else return;
                    } else return;
                }
                else cur++;
            }
        };
        auto tokenize_error = [&cur, str](std::string_view desc) {//calculate line & collumn
            size_t line = 0, collumn = 0;
            for(size_t c = 0; c <= cur && c < str.size(); c++, collumn++) if(str[c] == '\n') line++, collumn = 0;
            SGL_TOKENIZE_ERROR(desc, line, collumn);
        };

        /*
            uint64_t fract_part = 0, exp_part = 0, fract_size = 0, exp_size = 0;
                bool has_fract = false, has_exp = false;
                bool is_exp_positive = true;
        */

        //all numbers is unsigned in parse stage
        auto construct_number_using_type_literal = [tokenize_error](uint64_t int_part, uint64_t fract_part, uint64_t fract_size, 
            int exp, bool has_fract, bool has_exp, std::string_view type_literal) -> value {

            static std::array<double, 308*2+1> pow10_table = ([](){
                static std::array<double, 308*2+1> ret;
                for(size_t i = 0; i <= 308*2; i++) ret[i] = std::pow(10, int(i)-308);
                return ret;
            })();
            
            
            double float_val = 0.;
            uint64_t int_val = int_part;
            bool is_float = false;

            if(has_fract || has_exp) {
                is_float = true;
                float_val = double(int_part);
                //division by pow10_table[fract_size+308] same as multiplication by pow10_table[308-fract_size]
                if(has_fract) float_val += double(fract_part) * pow10_table[308-fract_size];
                if(has_exp) float_val *= pow10_table[308 + exp];
            }

            if(type_literal.empty()) {
                if(is_float) return value(const_val<builtin_types::sgl_float64_t>(static_cast<builtin_types::sgl_float64_t>(float_val)));
                else return value(const_val<uint64_t>(int_val));
            } else {//not empty
                //TODO use std::hash<std::string_view> for fast literals swith?
                if(is_float) {
                    if(type_literal == "f" || type_literal == "f32") return value(const_val<builtin_types::sgl_float32_t>(static_cast<builtin_types::sgl_float32_t>(float_val)));
                    if(type_literal == "f64") return value(const_val<builtin_types::sgl_float64_t>(static_cast<builtin_types::sgl_float64_t>(float_val)));
                } else {//integer
                    if(type_literal == "f" || type_literal == "f32") return value(const_val<builtin_types::sgl_float32_t>(static_cast<builtin_types::sgl_float32_t>(int_val)));
                    if(type_literal == "f64") return value(const_val<builtin_types::sgl_float64_t>(static_cast<builtin_types::sgl_float64_t>(int_val)));

                    if(type_literal == "i") return value(const_val<builtin_types::sgl_int_t>(static_cast<builtin_types::sgl_int_t>(int_val)));
                    if(type_literal == "u" || type_literal == "ui") return value(const_val<builtin_types::sgl_uint_t>(static_cast<builtin_types::sgl_uint_t>(int_val)));

                    if(type_literal == "i8")  return value(const_val<builtin_types::sgl_int8_t> (static_cast<builtin_types::sgl_int8_t> (int_val)));
                    if(type_literal == "i16") return value(const_val<builtin_types::sgl_int16_t>(static_cast<builtin_types::sgl_int16_t>(int_val)));
                    if(type_literal == "i32") return value(const_val<builtin_types::sgl_int32_t>(static_cast<builtin_types::sgl_int32_t>(int_val)));
                    if(type_literal == "i64") return value(const_val<builtin_types::sgl_int64_t>(static_cast<builtin_types::sgl_int64_t>(int_val)));

                    if(type_literal == "u8"  || type_literal == "ui8" ) return value(const_val<builtin_types::sgl_uint8_t> (static_cast<builtin_types::sgl_uint8_t> (int_val)));
                    if(type_literal == "u16" || type_literal == "ui16") return value(const_val<builtin_types::sgl_uint16_t>(static_cast<builtin_types::sgl_uint16_t>(int_val)));
                    if(type_literal == "u32" || type_literal == "ui32") return value(const_val<builtin_types::sgl_uint32_t>(static_cast<builtin_types::sgl_uint32_t>(int_val)));
                    if(type_literal == "u64" || type_literal == "ui64") return value(const_val<builtin_types::sgl_uint64_t>(static_cast<builtin_types::sgl_uint64_t>(int_val)));
                }
            }

            tokenize_error("unknown type_literal");

            return value();
        };

        size_t priority = 0;//brackets level

        while(skip_spaces_and_comments(), cur < str.size()) {
            switch(str[cur]) {
            case ';': {
                //TODO check priority == 0
                details::token t(details::token::t_punct, priority);
                t.punct_v = ';';
                m_tokens.back().emplace_back(std::move(t));
                m_tokens.emplace_back();
            } break;
            case ',': {
                details::token t(details::token::t_punct, priority);
                t.punct_v = str[cur];
                m_tokens.back().emplace_back(std::move(t));
            } break;
            case '(': case '{': case '[': {//TODO remove round brackets (let in function calls)? increase execution speed
                priority++;
                details::token t(details::token::t_punct, priority);
                t.punct_v = str[cur];
                m_tokens.back().emplace_back(std::move(t));
            } break;
            case ')': case '}': case ']': {
                //TODO check priority > 0
                details::token t(details::token::t_punct, priority);
                priority--;
                t.punct_v = str[cur];
                m_tokens.back().emplace_back(std::move(t));
            } break;

            case '.': {//puntc ot value (a.b or .1426)
                if(cur + 1 >= str.size() || !std::isdigit(str[cur+1])) {
                    details::token t(details::token::t_punct, priority);
                    t.punct_v = str[cur];
                    m_tokens.back().emplace_back(std::move(t));
                    break;
                } 
            }
            [[fallthrough]];
            case '0':
                if(cur + 1 < str.size() && str[cur] == '0' && (str[cur+1] == 'x' || str[cur+1] == 'X' || str[cur+1] == 'b' || str[cur+1] == 'B')) {       
                    cur++;

                    uint64_t num = 0;

                    //no octal
                    if(str[cur] == 'x' || str[cur] == 'X') {//hex
                        cur++;
                        while(cur < str.size() && std::isxdigit(str[cur])) {
                            num = num * 16 + (std::isdigit(str[cur]) ? str[cur] - '0' : 10 + (std::islower(str[cur]) ? str[cur] - 'a' : str[cur] - 'A'));
                            cur++;
                            //TODO overflow check
                        }
                        
                    } else {//bin    
                        cur++;
                        while(cur < str.size() && std::isdigit(str[cur])) {
                            num = num * 2 + str[cur] - '0';
                            if('0' != str[cur] && str[cur] != '1') tokenize_error("invalid binary number");
                            cur++;
                            //TODO overflow check
                        }

                    }

                    size_t lit_beg = cur;
                    while (cur < str.size() && (std::isalnum(str[cur]) || str[cur] == '_')) cur++;
                    auto type_literal = str.substr(lit_beg, cur - lit_beg);//suffix
                    cur--;

                    details::token t(details::token::t_value, priority);//TODO use type_literal to choose type
                    t.value_v = construct_number_using_type_literal(num, 0, 0, 0, false, false, type_literal);
                    m_tokens.back().emplace_back(std::move(t));

                    break;
                }
            [[fallthrough]];
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                //[int][.][fract][e[+|-]exp][literal]
                uint64_t int_part = 0, int_size = 0;
                while(cur < str.size() && std::isdigit(str[cur])) int_part = int_part*10 + str[cur]-'0', cur++, int_size++;//TODO int_part overflow fix?
                
                uint64_t fract_part = 0, exp_part = 0, fract_size = 0, exp_size = 0;
                bool has_fract = false, has_exp = false;
                bool is_exp_positive = true;

                if(cur < str.size() && str[cur] == '.') {
                    has_fract = true;
                    cur++;
                    while(cur < str.size() && std::isdigit(str[cur])) fract_part = fract_part*10 + str[cur]-'0', cur++, fract_size++;
                }
                if(cur < str.size() && (str[cur] == 'e' || str[cur] == 'E')) {
                    has_exp = true;
                    cur++;
                    if(cur < str.size() && (str[cur] == '+' || str[cur] == '-')) {
                        is_exp_positive = str[cur] != '-';
                        cur++;
                    }
                    while(cur < str.size() && std::isdigit(str[cur])) exp_part = exp_part*10 + str[cur]-'0', cur++, exp_size++;
                    if(exp_size == 0) tokenize_error("invalid number literal");//TODO  (has_fract && fract_size == 0) if .e (for example 1.e+1) not alloved?
                }

                //TODO check int_size + exp_part + fract_part > 0

                size_t lit_beg = cur;
                while (cur < str.size() && (std::isalnum(str[cur]) || str[cur] == '_')) cur++;
                auto type_literal = str.substr(lit_beg, cur - lit_beg);//suffix
                
                cur--;

                details::token t(details::token::t_value, priority);

                t.value_v = construct_number_using_type_literal(int_part, fract_part, fract_size, (is_exp_positive ? 1 : -1) * int(exp_part), has_fract, has_exp, type_literal);

                m_tokens.back().emplace_back(std::move(t));
            } break;
            
            case '"': case '\'': {
                bool is_char = str[cur] == '\'';
                char quote = str[cur];
                cur++;
                //TODO store unicode characters in utf-8?
                std::string s;
                while(cur < str.size() && (str[cur] != quote)) {
                    if(str[cur] == '\\') {
                        switch(str[cur + 1]) {
                        case '\\': s+='\\'; break;
                        case 'a': s+='\a'; break;
                        case 'b': s+='\b'; break;
                        case 'f': s+='\f'; break;
                        case 'n': s+='\n'; break;
                        case 'r': s+='\r'; break;
                        case 't': s+='\t'; break;
                        case 'v': s+='\v'; break;
                        //TODO add \u (unicode) and hex|octal chars
                        case '0': s+='\0'; break;
                        default: tokenize_error("invalid escape sequence"); break;
                        }     
                        cur+=2;
                    } else s += str[cur++];
                }
                if(is_char && s.size() != 1) tokenize_error("invalid character literal");
                
                details::token t(details::token::t_value, priority);

                if(is_char) t.value_v = value(const_val<char>(s[0]));//TODO use other char type? char8_t? or 16 (or 32?) for unicode?
                else t.value_v = value(const_val<std::string>(std::move(s)));

                m_tokens.back().emplace_back(std::move(t));
            } break;

            default: {
                if(std::isalnum(str[cur])) {//identifier
                    size_t beg = cur;
                    while(cur < str.size() && (std::isalnum(str[cur]) || str[cur] == '_')) cur++;
                    auto identifier_str = str.substr(beg, cur - beg);
                    cur--;
                    details::token t(details::token::t_identifier, priority);
                    t.identifier_v = identifier_str;
                    m_tokens.back().emplace_back(std::move(t));
                } else {//operator
                    details::token t(details::token::t_operator, priority);
                    if(cur + 2 < str.size()) {
                        static constexpr std::array<std::string_view, 3> op_str = { "<<=", ">>=", "<=>" };
                        static constexpr std::array<operator_type, 3> op_types = { operator_type::op_none, operator_type::op_none, operator_type::op_none };
                        if(auto f = std::find(op_str.begin(), op_str.end(), str.substr(cur, 3)); f != op_str.end()) {
                            size_t i = f - op_str.begin();
                            t.operator_v.str = op_str[i];
                            t.operator_v.type = op_types[i];
                            m_tokens.back().emplace_back(std::move(t));
                            cur += 2;
                            break;
                        }
                    }
                    if(cur + 1 < str.size()) {
                        static constexpr std::array<std::string_view, 18> op_str = {
                            "++", "--", 
                            "<<", ">>", 
                            "+=", "-=", "*=", "/=", "%=", "|=", "^=",
                            "<=", ">=", "!=", "==", "&&", "||", "->"
                        };
                        static constexpr std::array<operator_type, 18> op_types = {
                            operator_type::op_none, operator_type::op_none,
                            operator_type::op_bit_lsh, operator_type::op_bit_rsh,
                            operator_type::op_none, operator_type::op_none, operator_type::op_none, operator_type::op_none, operator_type::op_none, operator_type::op_none, operator_type::op_none,
                            operator_type::op_not_greater, operator_type::op_not_less, operator_type::op_not_equal, operator_type::op_equal, operator_type::op_and, operator_type::op_or, operator_type::op_none
                        };
                        if(auto f = std::find(op_str.begin(), op_str.end(), str.substr(cur, 2)); f != op_str.end()) {
                            size_t i = f - op_str.begin();
                            t.operator_v.str = op_str[i];
                            t.operator_v.type = op_types[i];
                            m_tokens.back().emplace_back(std::move(t));
                            cur += 1;
                            break;
                        }
                    }
                    {
                        static constexpr std::array<std::string_view, 14> op_str = {
                            "+", "-", "*", "/", "%", 
                            "|", "&", "~", "^",
                            "=",
                            "!", "<", ">", "."
                        };
                        static constexpr std::array<operator_type, 14> op_types = {
                            operator_type::op_sum, operator_type::op_sub, operator_type::op_mul, operator_type::op_div, operator_type::op_mod,
                            operator_type::op_bit_or, operator_type::op_bit_and, operator_type::op_bit_not, operator_type::op_bit_xor,
                            operator_type::op_none,
                            operator_type::op_not, operator_type::op_less, operator_type::op_greater, operator_type::op_none,
                        };
                        if(auto f = std::find(op_str.begin(), op_str.end(), str.substr(cur, 1)); f != op_str.end()) {
                            size_t i = f - op_str.begin();
                            t.operator_v.str = op_str[i];
                            t.operator_v.type = op_types[i];
                            m_tokens.back().emplace_back(std::move(t));
                            break;
                        }
                    }
                    //TODO error operator not found
                }
            } break;
            }//switch
            cur++;
        }
        if(m_tokens.back().empty()) m_tokens.pop_back();
    }
}//namespace SGL

#endif//SGL_TOKENIZER_HPP_INCLUDE_