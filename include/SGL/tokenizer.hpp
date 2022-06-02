#pragma once
#ifndef SGL_TOKENIZER_HPP_INCLUDE_
#define SGL_TOKENIZER_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"
#include "value.hpp"

#include <cstdint>
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
            explicit token(token&& tk) : type(tk.type), priority(tk.priority) {
                switch (type) {
                case t_none:       break;   
                case t_value:      new (&value_v) value(std::move(tk.value_v)); break;   
                case t_punct:      punct_v = tk.punct_v; break;   
                case t_operator:   operator_v = tk.operator_v; break;   
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
    }//namespace details
    class tokenizer : public details::no_copy {
    public:
        tokenizer(tokenizer&& o) : m_tokens(std::move(o.m_tokens)) {

        }
        //TODO add settings such as "ignore trailing comma"?
        [[nodiscard]] explicit tokenizer(std::string_view str);//TODO add char type as template arg?
        ~tokenizer() = default;
    //protected:
        friend class evaluator;

        using token_list = std::list<details::token>;

        std::vector<token_list> m_tokens;
    };
    
    tokenizer::tokenizer(std::string_view str) : m_tokens(1) {
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

        int priotity = 0;

        while(skip_spaces_and_comments(), cur < str.size()) {
            switch(str[cur]) {
            case ';': {
                //TODO check priority == 0
                details::token t(details::token::t_punct, priotity);
                t.punct_v = ';';
                m_tokens.back().emplace_back(std::move(t));
                m_tokens.emplace_back();
            } break;
            case ',': {
                details::token t(details::token::t_punct, priotity);
                t.punct_v = str[cur];
                m_tokens.back().emplace_back(std::move(t));
            } break;
            case '(': case '{': case '[': {
                priotity++;
                details::token t(details::token::t_punct, priotity);
                t.punct_v = str[cur];
                m_tokens.back().emplace_back(std::move(t));
            } break;
            case ')': case '}': case ']': {
                //TODO check priority > 0
                details::token t(details::token::t_punct, priotity);
                priotity--;
                t.punct_v = str[cur];
                m_tokens.back().emplace_back(std::move(t));
            } break;

            case '.': {//puntc ot value (a.b or .1426)
                if(cur + 1 >= str.size() || !std::isdigit(str[cur+1])) {
                    details::token t(details::token::t_punct, priotity);
                    t.punct_v = str[cur];
                    m_tokens.back().emplace_back(std::move(t));
                    break;
                } 
            } 
            case '0':
                if(cur + 1 < str.size() && str[cur] == '0' && (str[cur+1] == 'x' || str[cur+1] == 'X' || str[cur+1] == 'b' || str[cur+1] == 'B')) {       
                    cur++;

                    uint64_t num = 0;

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
                            cur++;
                            //TODO overflow check
                            //TODO str[cur] <= '1'
                        }

                    }

                    size_t lit_beg = cur;
                    while (cur < str.size() && (std::isalnum(str[cur]) || str[cur] == '_')) cur++;
                    auto type_literal = str.substr(lit_beg, cur - lit_beg);
                    cur--;

                    details::token t(details::token::t_value, priotity);//TODO use lit_beg to choose type
                    t.value_v = value(const_val<uint64_t>(num));
                    m_tokens.back().emplace_back(std::move(t));

                    break;
                }
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                //[int].[fract]e[+|-][exp]
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
                    //TODO check here exp_size > 0 && (!has_fract || fract_size > 0)
                }

                size_t lit_beg = cur;
                while (cur < str.size() && (std::isalnum(str[cur]) || str[cur] == '_')) cur++;
                auto type_literal = str.substr(lit_beg, cur - lit_beg);
                
                cur--;

                static std::array<double, 308*2+1> pow10_table = ([](){
                    static std::array<double, 308*2+1> ret;
                    for(size_t i = 0; i <= 308*2; i++) ret[i] = pow(10, int(i)-308);
                    return ret;
                })();

                details::token t(details::token::t_value, priotity);
                if(has_fract || has_fract) {//TODO choose correct type using type_literal
                    double val = double(int_part);
                    //division by pow10_table[fract_size+308] same as multiplication by pow10_table[308-fract_size]
                    if(has_fract) val += double(fract_part) * pow10_table[308-fract_size];
                    if(has_exp) val *= pow10_table[308 + (is_exp_positive ? 1 : -1) * int(exp_part)];//
                    t.value_v = value(const_val<double>(val));
                }
                else {
                    t.value_v = value(const_val<uint64_t>(int_part));
                }
                m_tokens.back().emplace_back(std::move(t));
            } break;
            
            default: {
                if(std::isalnum(str[cur])) {//identifier
                    size_t beg = cur;
                    while(cur < str.size() && (std::isalnum(str[cur]) || str[cur] == '_')) cur++;
                    auto identifier_str = str.substr(beg, cur - beg);
                    cur--;
                    details::token t(details::token::t_identifier, priotity);
                    t.identifier_v = identifier_str;
                    m_tokens.back().emplace_back(std::move(t));
                } else {//operator
                    details::token t(details::token::t_operator, priotity);
                    if(cur + 2 < str.size()) {
                        static constexpr std::array<std::string_view, 3> op_3_char_wide = { "<<=", ">>=", "<=>" };
                        if(auto f = std::find(op_3_char_wide.begin(), op_3_char_wide.end(), str.substr(cur, 3)); f != op_3_char_wide.end()) {
                            t.operator_v = *f;
                            m_tokens.back().emplace_back(std::move(t));
                            cur += 2;
                            break;
                        }
                    }
                    if(cur + 1 < str.size()) {
                        static constexpr std::array<std::string_view, 18> op_2_char_wide = { 
                            "++", "--", 
                            "<<", ">>", 
                            "+=", "-=", "*=", "/=", "%=", "|=", "^=",
                            "<=", ">=", "!=", "==", "&&", "||", "->"
                        };
                        if(auto f = std::find(op_2_char_wide.begin(), op_2_char_wide.end(), str.substr(cur, 2)); f != op_2_char_wide.end()) {
                            t.operator_v = *f;
                            m_tokens.back().emplace_back(std::move(t));
                            cur += 1;
                            break;
                        }
                    }
                    {
                        static constexpr std::array<std::string_view, 14> op_1_char_wide = { 
                            "+", "-", "*", "/", "%", 
                            "|", "&", "~", "^",
                            "=",
                            "!", "<", ">", "."
                        };
                        if(auto f = std::find(op_1_char_wide.begin(), op_1_char_wide.end(), str.substr(cur, 1)); f != op_1_char_wide.end()) {
                            t.operator_v = *f;
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