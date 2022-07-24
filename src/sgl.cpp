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

        register_type<std::nullptr_t>("__nullptr_t");

        //TODO type for type (result of typeof)?
        add_variable("true", true);
        add_variable("false", false);
        add_variable("nan", std::numeric_limits<double>::quiet_NaN());
        add_variable("inf", std::numeric_limits<double>::infinity());
        add_variable("nullptr", nullptr);

        //builtin functions:
        add_function("addressof", {{{static_cast<value(*)(const std::vector<value>&)>([](const std::vector<value>& v)->value{
            if(v.size() != 1) throw std::runtime_error("addressof args count != 1");
            if(v.front().is_const()) return { const_val<const void*>(v.front().m_const_data) };//TODO fix const
            else return { const_val<void*>(v.front().m_data) };
        }), function::function_overload::all_types_t{}, 1} }});
        add_function("sizeof", {{{static_cast<value(*)(const std::vector<value>&)>([](const std::vector<value>& v)->value{
            if(v.size() != 1) throw std::runtime_error("sizeof args count != 1");
            return { const_val<sgl_uint64_t>(v.front().m_type->size()) };
        }), function::function_overload::all_types_t{}, 1} }});


        add_function("__type_name", {{{static_cast<value(*)(const std::vector<value>&)>([](const std::vector<value>& v)->value{
            if(v.size() != 1) throw std::runtime_error("__type_name args count != 1");
            return { const_val<sgl_string_t>(v.front().m_type->type_to_str()) };
        }), function::function_overload::all_types_t{}, 1} }});
        add_function("__print", {{{static_cast<value(*)(const std::vector<value>&)>([](const std::vector<value>& v)->value{
            if(v.size() != 1) throw std::runtime_error("__print args count != 1");
            std::cout << v[0].to_string() << std::endl;
            return v[0];
        }), function::function_overload::all_types_t{}, 1} }});

        //TODO for builtin types add all possible operator permutation
        //such as 1. + 1.f and 1.f + 1.
        
        auto add_default_constructors_for_t = [this]<typename T>(details::sgl_type_identity<T>, const std::string& type_name) {
            add_constructors_impl<T, 
                details::sgl_type_identity<sgl_int8_t>,
                details::sgl_type_identity<sgl_int16_t>,
                details::sgl_type_identity<sgl_int32_t>,
                details::sgl_type_identity<sgl_int64_t>,

                details::sgl_type_identity<sgl_uint8_t>,
                details::sgl_type_identity<sgl_uint16_t>,
                details::sgl_type_identity<sgl_uint32_t>,
                details::sgl_type_identity<sgl_uint64_t>,

                details::sgl_type_identity<sgl_float32_t>,
                details::sgl_type_identity<sgl_float64_t>,

                details::sgl_type_identity<sgl_bool_t>,
                details::sgl_type_identity<sgl_char_t>
                //details::sgl_type_identity<sgl_string_t>
            >(type_name);
        };

        add_default_constructors_for_t(details::sgl_type_identity<sgl_int_t>{}, "int");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_uint_t>{}, "uint");

        add_default_constructors_for_t(details::sgl_type_identity<sgl_int8_t>{},  "int8");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_int16_t>{}, "int16");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_int32_t>{}, "int32");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_int64_t>{}, "int64");

        add_default_constructors_for_t(details::sgl_type_identity<sgl_uint8_t>{},  "uint8");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_uint16_t>{}, "uint16");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_uint32_t>{}, "uint32");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_uint64_t>{}, "uint64");
        
        add_default_constructors_for_t(details::sgl_type_identity<sgl_float32_t>{}, "float32");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_float64_t>{}, "float64");

        add_default_constructors_for_t(details::sgl_type_identity<sgl_float_t>{}, "float");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_double_t>{}, "double");

        add_default_constructors_for_t(details::sgl_type_identity<sgl_char_t>{}, "char");
        add_default_constructors_for_t(details::sgl_type_identity<sgl_bool_t>{}, "bool");
        //add_default_constructors_for_t(details::sgl_type_identity<sgl_string_t>{}, "string");

        add_constructors_impl<sgl_string_t, 
            details::sgl_type_identity<const sgl_string_t&>,
            //details::sgl_type_identity<sgl_uint64_t>,//TODO Fix
            details::sgl_type_identity<sgl_uint64_t, sgl_char_t>
            //TODO add other string constructors
        >("string");

        //TODO add constants (nullptr, true, false)

    }

    [[noreturn]] static inline void tokenize_error(std::string_view str, size_t cur, std::string_view desc) {
        size_t line = 0, collumn = 0;
        for(size_t c = 0; c <= cur && c < str.size(); c++, collumn++) if(str[c] == '\n') [[unlikely]] line++, collumn = 0;
        SGL_TOKENIZE_ERROR(desc, line, collumn);
    }
    
    static inline value scan_number(std::string_view base_str, std::string_view str, const char** cur_end) {
        const char *str_beg = str.data(), *str_cur = str_beg, *str_end = str_beg + str.size();
        
        auto int_value_from_suffix = [base_str, str](uint64_t val, std::string_view literal_suffix) -> value {
            using namespace builtin_types;
            //TODO type overflow check
            //TODO fix sing? (0bFFFFi16 -> -1i16 or ..?)
            auto i_val = [&]<typename T>(details::sgl_type_identity<T>, uint64_t val){
                if(std::numeric_limits<T>::max() < val) [[unlikely]] tokenize_error(base_str, str.data()-base_str.data(), "specified integer type (by using literal suffix) overflowed");
                return value(const_val<T>(static_cast<T>(val)));
            };

            if(literal_suffix == "i")                               return i_val(details::sgl_type_identity<sgl_int_t>   {}, val);
            if(literal_suffix == "u" || literal_suffix == "ui")     return i_val(details::sgl_type_identity<sgl_uint_t>  {}, val);
            if(literal_suffix == "i8")                              return i_val(details::sgl_type_identity<sgl_int8_t>  {}, val);
            if(literal_suffix == "i16")                             return i_val(details::sgl_type_identity<sgl_int16_t> {}, val);
            if(literal_suffix == "i32")                             return i_val(details::sgl_type_identity<sgl_int32_t> {}, val);
            if(literal_suffix == "i64")                             return i_val(details::sgl_type_identity<sgl_int64_t> {}, val);
            if(literal_suffix == "u8"  || literal_suffix == "ui8" ) return i_val(details::sgl_type_identity<sgl_uint8_t> {}, val);
            if(literal_suffix == "u16" || literal_suffix == "ui16") return i_val(details::sgl_type_identity<sgl_uint16_t>{}, val);
            if(literal_suffix == "u32" || literal_suffix == "ui32") return i_val(details::sgl_type_identity<sgl_uint32_t>{}, val);
            if(literal_suffix == "u64" || literal_suffix == "ui64") return i_val(details::sgl_type_identity<sgl_uint64_t>{}, val);

            if(literal_suffix == "f" || literal_suffix == "f32") return value(const_val<sgl_float32_t>(static_cast<sgl_float32_t>(val)));
            if(literal_suffix == "f64") return value(const_val<sgl_float64_t>(static_cast<sgl_float64_t>(val)));

            if(!literal_suffix.empty()) tokenize_error(base_str, str.data()-base_str.data(), "invalid integer literal suffix");

            if(val <= std::numeric_limits<sgl_int32_t>::max()) return value(const_val<sgl_int32_t>(static_cast<sgl_int32_t>(val)));
            if(val <= std::numeric_limits<sgl_int64_t>::max()) return value(const_val<sgl_int64_t>(static_cast<sgl_int64_t>(val)));
            return value(const_val<sgl_uint64_t>(val));
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
        bool has_float = false;

        //TODO hex floats?
        if(std::isdigit(static_cast<unsigned char>(*str_cur))) {
            const char* int_part_begin = str_cur;
            while(str_cur < str_end && std::isdigit(static_cast<unsigned char>(*str_cur))) str_cur++;
            int_part = {int_part_begin, size_t(str_cur-int_part_begin)};
        }
        if(str_cur < str_end && *str_cur == '.') {
            has_float = true;
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
            if(exp_part.empty() || exp_part.size() == 1 && !std::isdigit(exp_part[0])) 
                [[unlikely]] tokenize_error(base_str, exp_part_begin-base_str.data(), "invalid exponent");
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

        if(exp_part.empty() && fract_part.empty() && int_part.empty()) [[unlikely]] 
            tokenize_error(base_str, str_cur-base_str.data(), "invalid number");

        if(!fract_part.empty() || !exp_part.empty() || has_float) {
            double int_v = 0.;
            double fract_v = 0.;
            int exp_v = 0;

            auto m_from_chars = [&]<typename T>(const char* beg, const char* end, T& val) {
                if constexpr(requires(const char* a, const char* b, T& v) { std::from_chars(a, b, v); }) {
                    auto r = std::from_chars(beg, end, val);
                    if(r.ec != std::errc()) [[unlikely]] tokenize_error(base_str, str_beg-base_str.data(), "invalid number");
                }
                else std::istringstream({beg, end}) >> val;//TODO add warn here?  
            };

            if(!int_part.empty())   m_from_chars(int_part.data(), int_part.data()+int_part.size(), int_v);
            if(!fract_part.empty()) m_from_chars(fract_part.data(), fract_part.data()+fract_part.size(), fract_v);
            if(!exp_part.empty())   m_from_chars(exp_part.data(), exp_part.data()+exp_part.size(), exp_v);
            

            double res = int_v + fract_v * pow10_table[308-fract_part.size()];
            if(exp_v > 308) [[unlikely]] res = std::abs(res) == 0. ? 0. : std::numeric_limits<double>::infinity();
            else if(exp_v < -308) [[unlikely]] res = 0.;
            else [[likely]] res *= pow10_table[308+exp_v];
            
            using namespace builtin_types;
            if(literal_suffix == "f" || literal_suffix == "f32") return cur_end ? *cur_end = str_cur : nullptr, value(const_val<sgl_float32_t>(static_cast<sgl_float32_t>(res)));
            if(literal_suffix == "f64") return cur_end ? *cur_end = str_cur : nullptr, value(const_val<sgl_float64_t>(static_cast<sgl_float64_t>(res)));

            if(!literal_suffix.empty()) tokenize_error(base_str, str.data()-base_str.data(), "invalid floating point literal suffix");
            return cur_end ? *cur_end = str_cur : nullptr, value(const_val<double>(res));
        }

        uint64_t val = 0;
        auto r = std::from_chars(int_part.data(), int_part.data()+int_part.size(), val);

        if(r.ec == std::errc::result_out_of_range) [[unlikely]] tokenize_error(base_str, str_beg-base_str.data(), "uint64 overflow");
        if(r.ec != std::errc()) [[unlikely]] tokenize_error(base_str, str_beg-base_str.data(), "invalid number");

        return cur_end ? *cur_end = str_cur : nullptr, int_value_from_suffix(val, literal_suffix);
    }

    value details::eval_expr_rec_impl(const state& m_state, std::string_view base_str, std::string_view str, details::eval_impl_args args) {
        const char *str_beg = str.data(), *str_cur = str_beg, *str_end = str_beg + str.size();
        auto skip_comments_and_spaces = [&str_cur, &str_end, str, str_beg](){
            while(str_cur < str_end && (std::isspace(static_cast<unsigned char>(*str_cur)) || *str_cur == '/')) {
                if(*str_cur  == '/') {
                    if(str_cur + 1 < str_end) [[likely]] {
                        if(*(str_cur+1) == '/') str_cur = str.find('\n', str_cur-str_beg) + 1 + str_beg;
                        else if(*(str_cur+1) == '*') str_cur = str.find("*/", str_cur-str_beg) + 2 + str_beg;
                        else return;
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
            ret = details::eval_expr_rec_impl(m_state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, static_cast<uint8_t>(operator_precedence_step), false, false, true});
            if(str_cur == str_end || *str_cur != ')') [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "unclosed bracket");
            str_cur++;
        } break;
        case ')': {
            tokenize_error(base_str, str_cur-base_str.data(), "missing open bracket");
        } break;
        //unary opeators
        case '+': case '-': case '!': case '~': case '*': case '&': {
            char m_op_char = *str_cur;
            static constexpr std::array<char, 6> ops_chars = {
                '+', '-', '!', '~', '*', '&'
            }; 
            static constexpr std::array<operator_type, 6> ops_types = {
                operator_type::op_unary_plus, operator_type::op_unary_minus, operator_type::op_not, 
                operator_type::op_bit_not, operator_type::op_deref, operator_type::op_adress_of
            }; 
            auto m_op = ops_types[std::find(ops_chars.begin(), ops_chars.end(), m_op_char)-ops_chars.begin()];
            auto m_op_prior = operator_precedence[static_cast<size_t>(m_op)];
            auto v = details::eval_expr_rec_impl(m_state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, m_op_prior, args.is_in_function, args.is_in_ternary, args.is_in_brackets});
            
            ret = m_state.m_operator_list.call_unary_operator(m_op, v);
        } break;

        case '.':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': {
            ret = scan_number(base_str, {str_cur, size_t(str_end-str_cur)}, &str_cur);
        } break;

        //TODO scan value (string, char, number) literals here

        default:
            if(std::isalnum(static_cast<unsigned char>(*str_cur)) || *str_cur == '_') [[likely]] {
                const char* id_beg = str_cur;
                while(str_cur < str_end && (std::isalnum(static_cast<unsigned char>(*str_cur)) || *str_cur == '_')) str_cur++;
                std::string_view id = {id_beg, size_t(str_cur - id_beg)};
                std::string id_str = std::string{id};
                skip_comments_and_spaces();
                if(str_cur < str_end && *str_cur == '(') {//func call
                    std::vector<value> func_args;
                    str_cur+=1;
                    while(str_cur < str_end) {
                        skip_comments_and_spaces();
                        if(str_cur == str_end) [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "unexpected eof");
                        if(*str_cur == ')') break;
                        func_args.emplace_back(details::eval_expr_rec_impl(m_state, base_str, {str_cur, size_t(str_end-(str_cur))}, {&str_cur, static_cast<uint8_t>(operator_precedence_step), true, false, false}));
                        if(*str_cur == ')') break;
                        if(*str_cur != ',') [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "invalid character");
                    }
                    if(auto f = m_state.m_functions.find(id_str); f != m_state.m_functions.end()) 
                        ret = f->second.call(func_args);
                    else if(auto f = m_state.m_constructors.find(id_str); f != m_state.m_constructors.end()) [[likely]] 
                        ret = f->second.call(func_args);//TODO add constructors for types such as `const arr<const int**const>`
                    else [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "invalid function name `" + id_str + '`');
                    str_cur++;//skip ')'
                    break;
                }

                if(auto f = m_state.m_variables.find(id_str); f != m_state.m_variables.end()) [[likely]] {
                    ret = f->second;
                    //TODO member fields & functions here
                } 
                else [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "invalid variable name `" + id_str + '`');

            } else [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "invalid character");
            break;
        }

        while(str_cur < str_end) {
            skip_comments_and_spaces();
            if(str_cur == str_end) break;
            switch(*str_cur) {
            case '(': {
                tokenize_error(base_str, str_cur-base_str.data(), "unexpected '('");
            } break;
            case ')': {
                if(!args.is_in_brackets && !args.is_in_function) [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "missing open bracket");
                return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
            } break;

            
            case '?': {
                if(args.call_prior < static_cast<uint8_t>(operator_precedence_step)) return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
                auto v1 = details::eval_expr_rec_impl(m_state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, static_cast<uint8_t>(operator_precedence_step), false, true, args.is_in_brackets});
                if(str_cur == str_end || *str_cur != ':') [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "ternary condition: missing ':'");
                auto v2 = details::eval_expr_rec_impl(m_state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, static_cast<uint8_t>(operator_precedence_step), false, true, args.is_in_brackets});
                //TODO typecast to bool
                ret = ret.get<bool>() ? std::move(v1) : std::move(v2);
            } break;

            case ':': {
                if(!args.is_in_ternary) [[unlikely]] tokenize_error(base_str, str_cur-base_str.data(), "':' without ternary conditional");
                return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
            } break;

            case ',': {//binary a, b operator or comma in function f(a, b, ...)
                //TODO add function check
                if(args.is_in_function || args.call_prior < static_cast<uint8_t>(operator_precedence_step)) return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
                ret = details::eval_expr_rec_impl(m_state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, static_cast<uint8_t>(operator_precedence_step), false, args.is_in_ternary, args.is_in_brackets});  
            } break;

            case '+': case '-': case '*': case '/': case '%': 
            case '|': case '&': case '^': case '<': case '>': 
            case '!': case '=': {
                static constexpr std::array<std::string_view, 8> op_2wide_str = {
                    "<<", ">>", "<=", ">=", "!=", "==", "&&", "||"
                };
                static constexpr std::array<operator_type, 8> op_2wide_type = {
                    operator_type::op_bit_lsh, operator_type::op_bit_rsh,
                    operator_type::op_not_greater, operator_type::op_not_less, 
                    operator_type::op_not_equal, operator_type::op_equal, 
                    operator_type::op_and, operator_type::op_or
                };
                if(str_cur + 1 < str_end) [[likely]] {
                    if(auto f = std::find(op_2wide_str.begin(), op_2wide_str.end(), std::string_view(str_cur, 2)); f != op_2wide_str.end()) {
                        auto m_op = op_2wide_type[f-op_2wide_str.begin()];
                        auto m_op_prior = operator_precedence[static_cast<size_t>(m_op)];
                        if(args.call_prior <= m_op_prior) return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
                        auto v = details::eval_expr_rec_impl(m_state, base_str, {str_cur+2, size_t(str_end-(str_cur+2))}, {&str_cur, m_op_prior, args.is_in_function, args.is_in_ternary, args.is_in_brackets});
                        ret = m_state.m_operator_list.call_binary_operator(m_op, ret, v);
                        break;
                    }
                }
                
                static constexpr std::array<char, 10> op_1wide_str = {
                    '+', '-', '*', '/', '%',
                    '|', '&', '^', '<', '>'
                };
                static constexpr std::array<operator_type, 10> op_1wide_type = {
                    operator_type::op_sum, operator_type::op_sub, operator_type::op_mul, operator_type::op_div, operator_type::op_mod,
                    operator_type::op_bit_or, operator_type::op_bit_and, operator_type::op_bit_xor, 
                    operator_type::op_less, operator_type::op_greater
                };

                if(auto f = std::find(op_1wide_str.begin(), op_1wide_str.end(), *str_cur); f != op_1wide_str.end()) [[likely]] {
                    auto m_op = op_1wide_type[f-op_1wide_str.begin()];
                    auto m_op_prior = operator_precedence[static_cast<size_t>(m_op)];
                    if(args.call_prior <= m_op_prior) return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
                    auto v = details::eval_expr_rec_impl(m_state, base_str, {str_cur+1, size_t(str_end-(str_cur+1))}, {&str_cur, m_op_prior, args.is_in_function, args.is_in_ternary, args.is_in_brackets});
                    ret = m_state.m_operator_list.call_binary_operator(m_op, ret, v);
                    break;
                }
            } 
            [[fallthrough]];//if not found
            default:
                if(std::isalnum(static_cast<unsigned char>(*str_cur)) || *str_cur == '_')
                    tokenize_error(base_str, str_cur-base_str.data(), "custom keywords not allowed");
                else tokenize_error(base_str, str_cur-base_str.data(), "invalid character");
                break;
            }
        }

        return args.cur_end ? *args.cur_end = str_cur : nullptr, ret;
    }
}//namespace SGL