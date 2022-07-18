#include <SGL/SGL.hpp>
#include <charconv>
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
    
    static inline value scan_number(std::string_view base_str, std::string_view str, const char** cur_end) {
        if(str.empty() || str == "NaN") [[unlikely]] return cur_end ? *cur_end = str.data()+3 : nullptr, value(const_val<double>(std::numeric_limits<double>::quiet_NaN()));
        if(str == "inf") [[unlikely]] return cur_end ? *cur_end = str.data()+3 : nullptr,value(const_val<double>(std::numeric_limits<double>::infinity()));

        const char *str_beg = str.data(), *str_cur = str_beg, *str_end = str_beg + str.size();
        
        auto int_value_from_suffix = [base_str, str](uint64_t val, std::string_view literal_suffix) -> value {
            //TODO type overflow check
            //TODO fix sing? (0bFFFFi16 -> -1i16 or ..?)
            if(literal_suffix == "i") return value(const_val<builtin_types::sgl_int_t>(static_cast<builtin_types::sgl_int_t>(val)));
            if(literal_suffix == "u" || literal_suffix == "ui") return value(const_val<builtin_types::sgl_uint_t>(static_cast<builtin_types::sgl_uint_t>(val)));
            if(literal_suffix == "i8")  return value(const_val<builtin_types::sgl_int8_t> (static_cast<builtin_types::sgl_int8_t> (val)));
            if(literal_suffix == "i16") return value(const_val<builtin_types::sgl_int16_t>(static_cast<builtin_types::sgl_int16_t>(val)));
            if(literal_suffix == "i32") return value(const_val<builtin_types::sgl_int32_t>(static_cast<builtin_types::sgl_int32_t>(val)));
            if(literal_suffix == "i64") return value(const_val<builtin_types::sgl_int64_t>(static_cast<builtin_types::sgl_int64_t>(val)));

            if(literal_suffix == "u8"  || literal_suffix == "ui8" ) return value(const_val<builtin_types::sgl_uint8_t> (static_cast<builtin_types::sgl_uint8_t> (val)));
            if(literal_suffix == "u16" || literal_suffix == "ui16") return value(const_val<builtin_types::sgl_uint16_t>(static_cast<builtin_types::sgl_uint16_t>(val)));
            if(literal_suffix == "u32" || literal_suffix == "ui32") return value(const_val<builtin_types::sgl_uint32_t>(static_cast<builtin_types::sgl_uint32_t>(val)));
            if(literal_suffix == "u64" || literal_suffix == "ui64") return value(const_val<builtin_types::sgl_uint64_t>(static_cast<builtin_types::sgl_uint64_t>(val)));

            if(!literal_suffix.empty()) tokenize_error(base_str, str.data()-base_str.data(), "invalid integer literal suffix");

            return value(const_val<uint64_t>(val));
        };

        if(str.size() > 2 && str[0] == '0') [[likely]] {
            uint64_t num = 0;
            bool has_num = false;
            if(str[1] == 'x' || str[1] == 'X') {//hex
                str_cur+=2;
                has_num = true;
                while(str_cur < str_end && std::isxdigit(static_cast<unsigned char>(*str_cur))) {
                    num<<=4;
                    if('0' <= *str_cur && *str_cur <= '9') num |= *str_cur - '0';
                    else if('a' <= *str_cur && *str_cur <= 'f') num |= *str_cur - 'a' + 10;
                    else num |= *str_cur - 'A' + 10;
                    str_cur++;
                }
                if(str_beg + 2 + (64/4) < str_cur) [[unlikely]] tokenize_error(base_str, str_beg+2-base_str.data(), "uint64 overflow");
                if(str_beg + 2 == str_cur) [[unlikely]] tokenize_error(base_str, str_beg+2-base_str.data(), "invalid integer constant");
            }
            else if(str[1] == 'b' || str[1] == 'B') {//bin
                str_cur+=2;
                has_num = true;
                while(str_cur < str_end && (*str_cur == '0' || *str_cur == '1')) num=num<<1|*str_cur++-'0';
                if(str_beg + 2 + 64 < str_cur) [[unlikely]] tokenize_error(base_str, str_beg+2-base_str.data(), "uint64 overflow");
                if(str_beg + 2 == str_cur) [[unlikely]] tokenize_error(base_str, str_beg+2-base_str.data(), "invalid integer constant");
            }
            //TODO octal?
            if(has_num) {
                std::string_view literal_suffix;
                if(str_cur < str_end) {
                    const char* suffix_beg = str_cur;
                    while(str_cur < str_end && std::isalnum(static_cast<unsigned char>(*str_cur))) str_cur++;
                    literal_suffix = {suffix_beg, size_t(str_cur-suffix_beg)};
                }

                return cur_end ? *cur_end = str_cur : nullptr, int_value_from_suffix(num, literal_suffix);
            }
        }

        //[int][.][fract][e[+|-]exp][literal_suffix]
        std::string_view int_part, fract_part, exp_part, literal_suffix;

        //TODO hex floats?
        if(std::isdigit(static_cast<unsigned char>(*str_cur))) {
            const char* int_part_begin = str_cur;
            while(str_cur < str_end && std::isdigit(static_cast<unsigned char>(*str_cur))) str_cur++;
            int_part = {int_part_begin, size_t(str_cur-int_part_begin)};
        }
        if(str_cur < str_end && *str_cur == '.') {
            str_cur++;
            const char* fract_part_begin = str_cur;
            while(str_cur < str_end && std::isdigit(static_cast<unsigned char>(*str_cur))) str_cur++;
            fract_part = {fract_part_begin, size_t(std::min(fract_part_begin+308, str_cur)-fract_part_begin)};
        }
        if(str_cur < str_end && *str_cur == 'e') {
            str_cur++;
            const char* exp_part_begin = str_cur;
            if(str_cur < str_end && (*str_cur == '+' || *str_cur == '-')) {
                if(*str_cur == '+') exp_part_begin++;
                else str_cur++;
            }
            while(str_cur < str_end && std::isdigit(static_cast<unsigned char>(*str_cur))) str_cur++;
            exp_part = {exp_part_begin, size_t(str_cur-exp_part_begin)};
        }
        if(str_cur < str_end) {
            const char* suffix_beg = str_cur;
            while(str_cur < str_end && std::isalnum(static_cast<unsigned char>(*str_cur))) str_cur++;
            literal_suffix = {suffix_beg, size_t(str_cur-suffix_beg)};
        }

        static std::array<double, 308*2+1> pow10_table = ([](){
            static std::array<double, 308*2+1> ret;
            for(size_t i = 0; i <= 308*2; i++) ret[i] = std::pow(10, int(i)-308);
            return ret;
        })();

        if(!fract_part.empty() || !exp_part.empty()) {
            double int_v = 0.;
            double fract_v = 0.;
            int exp_v = 0;

            auto m_from_chars = []<typename T>(const char* beg, const char* end, T& val) {
                if constexpr(requires(const char* a, const char* b, T& v) { std::from_chars(a, b, v); }) std::from_chars(beg, end, val);
                else std::istringstream({beg, end}) >> val;//TODO add warn here?  
            };

            if(!int_part.empty())   m_from_chars(int_part.data(), int_part.data()+int_part.size(), int_v);
            if(!fract_part.empty()) m_from_chars(fract_part.data(), fract_part.data()+fract_part.size(), fract_v);
            if(!exp_part.empty())   m_from_chars(exp_part.data(), exp_part.data()+exp_part.size(), exp_v);
            

            double res = int_v + fract_v * pow10_table[308-fract_part.size()];
            if(exp_v > 308) [[unlikely]] res = std::abs(res) == 0. ? 0. : std::numeric_limits<double>::infinity();
            else if(exp_v < -308) [[unlikely]] res = 0.;
            else [[likely]] res *= pow10_table[308+exp_v];
            
            if(literal_suffix == "f" || literal_suffix == "f32") return cur_end ? *cur_end = str_cur : nullptr, value(const_val<builtin_types::sgl_float32_t>(static_cast<builtin_types::sgl_float32_t>(res)));
            if(literal_suffix == "f64") return cur_end ? *cur_end = str_cur : nullptr, value(const_val<builtin_types::sgl_float64_t>(static_cast<builtin_types::sgl_float64_t>(res)));

            if(!literal_suffix.empty()) tokenize_error(base_str, str.data()-base_str.data(), "invalid floating point literal suffix");
            return cur_end ? *cur_end = str_cur : nullptr, value(const_val<double>(res));
        }
        uint64_t val = 0;
        auto r = std::from_chars(int_part.data(), int_part.data()+int_part.size(), val);

        if(r.ec == std::errc::result_out_of_range) [[unlikely]] tokenize_error(base_str, str_beg+2-base_str.data(), "uint64 overflow");

        return cur_end ? *cur_end = str_cur : nullptr, int_value_from_suffix(val, literal_suffix);
    }

    value details::eval_rec_impl(const state& state, std::string_view base_str, std::string_view str, details::eval_rec_impl_args args) {
        const char *str_beg = str.data(), *str_cur = str_beg, *str_end = str_beg + str.size();
        auto skip_comments_and_spaces = [&str_cur, &str_end, str, str_beg](){
            while(str_cur < str_end && (std::isspace(static_cast<unsigned char>(*str_cur)) || *str_cur == '/')) {
                if(*str_cur  == '/') {
                    if(str_cur + 1 < str_end) [[likely]] {
                        if(*(str_cur+1) == '/') str_cur = str.find('\n', str_cur-str_beg) + 1 + str_beg;
                        else if(*(str_cur+1) == '*') str_cur = str.find("*/", str_cur-str_beg) + 2 + str_beg;
                        if(str_cur > str_end) [[unlikely]] str_cur = str_end;
                    } else return;
                }
                else str_cur++;
            }
        };
        skip_comments_and_spaces();
        if(str_cur == str_end) return args.cur_end ? *args.cur_end = str_cur : nullptr, value();//TODO throw error?
        value ret;
        //number|string literal or prefix unary operator
        switch(*str_cur) {
        //brackets
        case '(': {
            auto v = details::eval_rec_impl(state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, 0, false, false, true});
            if(str_cur == str_end || *str_cur != ')') [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "unclosed bracket");
            return args.cur_end ? *args.cur_end = str_cur+1 : nullptr, v;
        } break;
        case ')': {
            tokenize_error(base_str, str_cur-base_str.data(), "missing open bracket");
        } break;
        //unary opeators
        case '+': case '-': case '!': case '~': case '*': case '&': {
            char m_op_char = *str_cur;
            auto v = details::eval_rec_impl(state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, 0, args.is_in_function, args.is_in_ternary, args.is_in_brackets});
            static constexpr std::array<char, 6> ops_chars = {
                '+', '-', '!', '~', '*', '&'
            }; 
            static constexpr std::array<operator_type, 6> ops_types = {
                operator_type::op_unary_plus, operator_type::op_unary_minus, operator_type::op_not, 
                operator_type::op_bit_not, operator_type::op_deref, operator_type::op_adress_of
            }; 
            ret = state.m_operator_list.call_operator(ops_types[std::find(ops_chars.begin(), ops_chars.end(), m_op_char)-ops_chars.begin()], {v});
        } break;

        case '.':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            ret = scan_number(base_str, {str_cur, size_t(str_end-str_cur)}, &str_cur);
        } break;

        //TODO scan value (string, char, number) literals here

        default:
            //TODO identifiers (functions, variables)
            break;
        }

        while(str_cur < str_end) {
            switch(*str_cur) {
            case '(': {
                tokenize_error(base_str, str_cur-base_str.data(), "unexpected '('");
            } break;
            case ')': {
                if(!args.is_in_brackets && !args.is_in_function) [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "missing open bracket");
                return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
            } break;

            case '+': {

            };
            
            default:
                break;
            }
        }

        return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
    }
}//namespace SGL