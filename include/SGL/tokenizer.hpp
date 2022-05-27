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
                m_tokens.back().push_back(t);
                m_tokens.emplace_back();
            } break;
            case ',': {
                details::token t(details::token::t_punct, priotity);
                t.punct_v = str[cur];
                m_tokens.back().push_back(t);
            } break;
            case '(': case '{': case '[': {
                priotity++;
                details::token t(details::token::t_punct, priotity);
                t.punct_v = str[cur];
                m_tokens.back().push_back(t);
            } break;
            case ')': case '}': case ']': {
                //TODO check priority > 0
                details::token t(details::token::t_punct, priotity);
                priotity--;
                t.punct_v = str[cur];
                m_tokens.back().push_back(t);
            } break;

            case '.': {//puntc ot value (a.b or .1426)
                if(cur + 1 >= str.size() || !std::isdigit(str[cur+1])) {
                    details::token t(details::token::t_punct, priotity);
                    t.punct_v = str[cur];
                    m_tokens.back().push_back(t);
                    break;
                } 
            } 
            case '0': {
                if(str[cur] == '0' && false) {//can be '.'           
                    //TODO add binary & hex numbers parising here
                    break;
                }
            }
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                //[int].[frag]e[exp]
                uint64_t int_part = 0;
                //TODO add float nubers parse
                while(cur < str.size() && std::isdigit(str[cur])) int_part = int_part*10 + str[cur]-'0', cur++;//TODO int_part overflow fix?
                cur--;
                
                
                details::token t(details::token::t_value, priotity);
                t.value_v = value(const_val<uint64_t>(int_part));
                m_tokens.back().push_back(t);
            } break;
            
            default: {
                if(std::isalnum(str[cur])) {//identifier
                    size_t beg = cur;
                    while(cur < str.size() && std::isalnum(str[cur]) || str[cur] == '_') cur++;
                    size_t len = cur - beg;
                    cur--;
                    auto identifier_str = str.substr(beg, len);
                    details::token t(details::token::t_identifier, priotity);
                    t.identifier_v = identifier_str;
                    m_tokens.back().push_back(t);
                } else {//operator
                    details::token t(details::token::t_operator, priotity);
                    if(cur + 2 < str.size()) {
                        static constexpr std::array<std::string_view, 3> op_3_char_wide = { "<<=", ">>=", "<=>" };
                        if(auto f = std::find(op_3_char_wide.begin(), op_3_char_wide.end(), str.substr(cur, 3)); f != op_3_char_wide.end()) {
                            t.operator_v = *f;
                            m_tokens.back().push_back(t);
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
                            m_tokens.back().push_back(t);
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
                            m_tokens.back().push_back(t);
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