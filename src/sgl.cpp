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

    [[noreturn]] static inline void tokenize_error(std::string_view str, size_t cur, std::string_view desc) {
        size_t line = 0, collumn = 0;
        for(size_t c = 0; c <= cur && c < str.size(); c++, collumn++) if(str[c] == '\n') [[unlikely]] line++, collumn = 0;
        SGL_TOKENIZE_ERROR(desc, line, collumn);
    }

    static inline value scan_number(std::string_view str, const char** cur_end) {
        if(str == "NaN") [[unlikely]] return value(const_val<double>(std::numeric_limits<double>::quiet_NaN()));
        if(str == "inf") [[unlikely]] return value(const_val<double>(std::numeric_limits<double>::infinity()));

        const char *str_beg = str.data(), *str_cur = str_beg, *str_end = str_beg + str.size();
        //[int][.][fract][e[+|-]exp][literal_suffix]
        std::string_view int_part, fract_part, exp_part;

    }

    value details::eval_rec_impl(const state& state, std::string_view base_str, std::string_view str, details::eval_rec_impl_args args) {
        const char *str_beg = str.data(), *str_cur = str_beg, *str_end = str_beg + str.size();

        auto skip_comments_and_spaces = [&str_cur, &str_end, str, str_beg](){
            while(str_cur < str_end && (std::isspace(static_cast<unsigned char>(*str_cur)) || *str_cur == '/')) {
                if(*str_cur  == '/') {
                    if(str_cur + 1 < str_end) {
                        if(*(str_cur+1) == '/') str_cur = str.find('\n', str_cur-str_beg) + 1 + str_beg;
                        else if(*(str_cur+1) == '*') str_cur = str.find("*/", str_cur-str_beg) + 2 + str_beg;
                        if(str_cur > str_end) [[unlikely]] str_cur = str_end;
                    } else return;
                }
                else str_cur++;
            }
        };

        if(str_cur == str_end) return args.cur_end ? *args.cur_end = str_cur : nullptr, value();
        value ret;
        //number|string literal or prefix unary operator
        switch (*str_cur) {
        //brackets
        case '(': {
            auto v = details::eval_rec_impl(state, base_str, {str_cur+1, str_end}, {&str_cur, 0, false, false, true});
            if(str_cur == str_end || *str_cur != ')') [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "unclosed bracket");
            return args.cur_end ? *args.cur_end = str_cur+1 : nullptr, v;
        } break;
        case ')': {
            tokenize_error(base_str, str_cur-base_str.data(), "missing open bracket");
            //if(!args.is_in_brackets && !args.is_in_function) [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "missing open bracket");
            //return args.cur_end ? *args.cur_end = str_cur : nullptr, value();
        } break;
        //unary opeators
        case '+': {
            auto v = details::eval_rec_impl(state, base_str, {str_cur+1, str_end}, {&str_cur, 0, args.is_in_function, args.is_in_ternary, args.is_in_brackets});
            auto v2 = state.m_operator_list.call_operator(SGL::operator_type::op_unary_plus, {v});
            return args.cur_end ? *args.cur_end = str_cur : nullptr, v2;
        } break;
        case '-': {
            auto v = details::eval_rec_impl(state, base_str, {str_cur+1, str_end}, {&str_cur, 0, args.is_in_function, args.is_in_ternary, args.is_in_brackets});
            auto v2 = state.m_operator_list.call_operator(SGL::operator_type::op_unary_minus, {v});
            return args.cur_end ? *args.cur_end = str_cur : nullptr, v2;
        } break;
        case '!': {
            
        } break;
        case '~': {
            
        } break;
        case '*': {
            
        } break;
        case '&': {
        } break;

        case '0': {
            //TODO hex | bin (| oct?)
        }
        [[fallthrough]];
        case '.':
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            auto v = scan_number({str_cur, str_end}, &str_cur);
        } break;

        //TODO scan value (string, char, number) literals here

        default:
            break;
        }

        return args.cur_end ? *args.cur_end = str_cur : nullptr, value();
    }
}//namespace SGL