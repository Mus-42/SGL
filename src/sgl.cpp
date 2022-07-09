#include <SGL/SGL.hpp>

//large functions linked static to increase compilation speed
namespace SGL {
    void state::init() {
        using namespace builtin_types;
        //builtin types:
        register_type<void>("void");
        //integer
        register_type<sgl_int8_t >("int8" );
        register_type<sgl_int16_t>("int16");
        auto t_int32 = register_type<sgl_int32_t>("int32");
        register_type<sgl_int64_t>("int64");

        register_type<sgl_uint8_t >("uint8" );
        register_type<sgl_uint16_t>("uint16");
        auto t_uint32 = register_type<sgl_uint32_t>("uint32");
        register_type<sgl_uint64_t>("uint64");

        register_type("int" , t_int32);
        register_type("uint", t_uint32);
        
        //floating point
        auto t_float32 = register_type<sgl_float32_t>("float32");
        auto t_float64 = register_type<sgl_float64_t>("float64");

        //TODO other way to add type alias

        register_type("float" , t_float32);
        register_type("double", t_float64);

        //other
        register_type<sgl_bool_t>("bool");
        register_type<sgl_char_t>("char");
        register_type<sgl_string_t>("string");

        //TODO type for type (result of typeof)?

        //builtin functions:
        add_function("addressof", {{{static_cast<value(*)(std::initializer_list<std::reference_wrapper<value>>)>([](std::initializer_list<std::reference_wrapper<value>> v)->value{
            if(v.size() != 1) throw std::runtime_error("addressof args count != 1");
            auto& q = v.begin()->get();
            if(q.is_const()) return { const_val<void*>(q.m_data) };
            else return { const_val<void*>(q.m_data) };
        }), function::function_overload::all_types_t{}, 1} }});
        add_function("sizeof", {{{static_cast<value(*)(std::initializer_list<std::reference_wrapper<value>>)>([](std::initializer_list<std::reference_wrapper<value>> v)->value{
            if(v.size() != 1) throw std::runtime_error("sizeof args count != 1");
            auto& q = v.begin()->get();
            return { const_val<sgl_uint64_t>(q.m_type->size()) };
        }), function::function_overload::all_types_t{}, 1} }});

        //TODO register operators

        //TODO for builtin types add all possible operator permutation
        //such as 1. + 1.f and 1.f + 1.

        //TODO how fix "to large obj file" 
        
        //add_binary_operator_permutations<sgl_float32_t, sgl_float64_t,
        //    sgl_int8_t, sgl_int16_t, sgl_int32_t, sgl_int64_t,
        //    sgl_uint8_t, sgl_uint16_t, sgl_uint32_t, sgl_uint64_t>();
    }
   
    value evaluator::evaluate(tokenizer&& tk) {
        /*
            identifier can be reserved keyword, typename or user-defined name
            operator - use operator_list.hpp enum?
        */
        //std::cout << "before evaluation:\n";
        //for (auto& l : tk.m_tokens) print_tokens(l);


        //TODO choose operators, sort in evaluation order, evaluate

        for (auto& l : tk.m_tokens) {
            if (l.empty()) continue;
            using tk_iter = tokenizer::token_list::iterator;
            using details::token;

            bool is_variable = false;
            if(l.size() >= 3) if (auto third = std::next(l.begin(), 2); third->type == token::t_operator && third->operator_v.type == operator_type::op_assign) {//if T name = ...; -> variable decl
                //TODO create variable in parse result, remove first 3 tokens

                is_variable = true;
            }

            std::vector<std::pair<size_t, tk_iter>> operators;
            bool is_begin = true;
            //TODO add function calls
            for (auto tkn = l.begin(); tkn != l.end(); tkn++) {//choose operators
                if (tkn->type == token::t_operator && tkn->operator_v.type != operator_type::op_none) {
                    if (tkn->operator_v.type == operator_type::op_assign) [[unlikely]]  {
                        throw std::runtime_error("assign operator allowed only in variable definition");
                    }

                    //`+` `-` `&` `*` can be unary or binary 
                    
                    bool can_be_unary = is_begin || std::prev(tkn)->type != token::t_identifier && std::prev(tkn)->type != token::t_value;
                    
                    if (can_be_unary && is_operator_unary[static_cast<size_t>(tkn->operator_v.type)]) switch (tkn->operator_v.type) {
                    case operator_type::op_sum:     tkn->operator_v.type = operator_type::op_unary_plus;  break;
                    case operator_type::op_sub:     tkn->operator_v.type = operator_type::op_unary_minus; break;
                    case operator_type::op_mul:     tkn->operator_v.type = operator_type::op_deref;       break;
                    case operator_type::op_bit_and: tkn->operator_v.type = operator_type::op_adress_of;   break;
                    default: break;
                    }
                    operators.emplace_back(operator_precedence_step * (tkn->priority + 1) - operator_precedence[static_cast<size_t>(tkn->operator_v.type)], tkn);
                    is_begin = false;
                } else 
                //TODO add if(...is_func...) { ... } else
                if (tkn->type == token::t_punct && tkn->punct_v == '(') {//remove redundant brackets (12) -> 12
                    //TODO check if prev isnt func
                    if (auto nxt = std::next(tkn); nxt != l.end() && nxt->type == token::t_value)
                        if (auto br = std::next(nxt); br != l.end() && br->type == token::t_punct && br->punct_v == ')') {
                            l.erase(tkn);
                            tkn = l.erase(br);
                        }
                }
            }

            std::sort(operators.begin(), operators.end(), [](auto& a, auto& b) {
                return a.first > b.first;
                });

            for (auto& op : operators) {
                auto& tkn = op.second;
                auto t = tkn->operator_v.type;
                //TODO chek if operands is t_value
                token res(token::t_value, op.second->priority);

                bool is_beg = l.begin() == tkn;
                bool is_end = std::prev(l.end()) == tkn;

                //TODO fix parentheses: now a+b != (a) + (b)

                if (is_operator_unary[static_cast<size_t>(t)]) {
                    //TODO suffix operators support?
                    if (is_end) [[unlikely]] throw std::runtime_error(std::string("opeator `") + std::string(tkn->operator_v.str) + std::string("` can't get arg"));
                    auto next = std::next(tkn);
                    if (next->type != token::t_value) [[unlikely]] throw std::runtime_error(std::string("opeator `") + std::string(tkn->operator_v.str) + std::string("` can't get arg"));
                    

                    res.value_v = m_state.m_operator_list.call_operator(t, { next->value_v });
                    l.erase(next);
                }
                else {
                    if (is_beg || is_end) [[unlikely]] throw std::runtime_error(std::string("opeator `") + std::string(tkn->operator_v.str) + std::string("` can't get 2 args"));
                    auto prev = std::prev(tkn);
                    auto next = std::next(tkn);
                    if (prev->type != token::t_value || next->type != token::t_value) [[unlikely]] throw std::runtime_error(std::string("opeator `") + std::string(tkn->operator_v.str) + std::string("` can't get 2 args"));
                    
                    res.value_v = m_state.m_operator_list.call_operator(t, { prev->value_v, next->value_v });

                    l.erase(prev);
                    l.erase(next);
                }

                *tkn = std::move(res);

                //list changed -> update values
                is_beg = l.begin() == tkn;
                is_end = std::prev(l.end()) == tkn;

                if (!is_beg && !is_end) {
                    auto prev = std::prev(tkn);
                    auto next = std::next(tkn);

                    //TODO chek if isnt function?
                    if (prev->type == token::t_punct && prev->punct_v == '(' && next->type == token::t_punct && next->punct_v == ')') {
                        l.erase(prev);
                        l.erase(next);
                    }
                }
            }
        }

        //std::cout << "after evaluation:\n";
        //for (auto& l : tk.m_tokens) print_tokens(l);
        
        //TODO check legth?

        return tk.m_tokens.front().front().value_v;
    }
}//namespace SGL