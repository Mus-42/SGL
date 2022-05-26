#include <SGL/tokenizer.hpp>
#include <set>
#include <array>
#include <cctype>//std::isalpha, std::isalnum ...


namespace SGL {
    //TODO move it in tokenizer.hpp
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
            }
            case ',': case '.': {
                details::token t(details::token::t_punct, priotity);
                t.punct_v = ';';
                m_tokens.back().push_back(t);
                m_tokens.emplace_back();
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


            
            default: {
                if(std::isalnum(str[cur])) {
                    size_t beg = cur;
                    while(cur < str.size() && std::isalnum(str[cur]) || str[cur] == '_') cur++;
                    size_t len = cur - beg;
                    cur--;
                    details::token t(details::token::t_identifier, priotity);
                    t.identifier_v = str.substr(beg, len);
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