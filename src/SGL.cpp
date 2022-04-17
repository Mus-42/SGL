#include <SGL/SGL.hpp>

#include <algorithm>
#include <iostream>
#include <functional>
#include <list>
#include <map>

#define SGL_ERROR(v) SGL_ERROR_impl__(v)

namespace SGL {
	inline void SGL_ERROR_impl__(const std::string& what) {
		throw std::runtime_error(what);
	}
	static const type buildin_types_v[t_custom]{
		{t_void},
		{t_int8}, {t_int16},{t_int32}, {t_int64},
		{t_uint8}, {t_uint16}, {t_uint32}, {t_uint64},
		{t_float32}, {t_float64},

		{t_bool},
		{
			t_string,
			(void*)(details::t_construct<std::string>)([](std::string* v) {
				new (v) std::string;
			}),
			(void*)(details::t_destruct<std::string>)([](std::string* v) {
				v->~basic_string();
			}),
			(void*)(details::t_copy<std::string>)([](std::string* v, std::string* from) {
				new (v) std::string(*from);
			})
		},
		{t_char}
	};
	static const std::unordered_map<std::string, const type*> buildin_types = {
		{"void", &buildin_types_v[t_void]},
		{"int8", &buildin_types_v[t_int8]}, {"int16", &buildin_types_v[t_int16]}, {"int32", &buildin_types_v[t_int32]}, {"int64", &buildin_types_v[t_int64]},
		{"uint8", &buildin_types_v[t_uint8]}, {"uint16", &buildin_types_v[t_uint16]}, {"uint32", &buildin_types_v[t_uint32]}, {"uint64", &buildin_types_v[t_uint64]},
		{"float32", &buildin_types_v[t_float32]}, {"float64", &buildin_types_v[t_float64]},

		{"bool", &buildin_types_v[t_bool]},
		{"string", &buildin_types_v[t_string]},
		{"char", &buildin_types_v[t_char]},

		{"int", &buildin_types_v[t_int32]}, {"uint", &buildin_types_v[t_uint32]},
		{"float", &buildin_types_v[t_float32]}, {"double", &buildin_types_v[t_float64]},
	};

	state::~state() {
		while (!m_results.empty()) (*m_results.begin())->~parse_result();//TODO fix array size
		for (auto& [name, var] : global_constants) details::destruct_val(var.m_type, 0, var.data);
	}
	parse_result::~parse_result() {
		if (m_state) m_state->m_results.erase(this);
		for (auto& [name, var] : local_variables) details::destruct_val(var.m_type, 0, var.data);
	}

	namespace details {
		void construct_val(const type* t, size_t arr_size, void* v) {
			if (t->m_construct) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++)
				static_cast<t_construct<void>>(t->m_construct)(static_cast<char*>(v) + i * t->size);
			else {
				if (t->base_type == t_custom) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++) for (auto m : t->members)
					construct_val(m.m_type, m.array_size, (static_cast<char*>(v) + i * t->size + m.offset));
				else memset(v, 0, (arr_size ? arr_size : 1) * t->size);
			}
		}
		void destruct_val(const type* t, size_t arr_size, void* v) {
			if (t->m_destruct) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++)
				static_cast<t_destruct<void>>(t->m_destruct)(static_cast<char*>(v));
			else {
				if (t->base_type == t_custom) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++) for (auto m : t->members)
					destruct_val(m.m_type, m.array_size, (static_cast<char*>(v) + i * t->size + m.offset));
			}
		}
		void copy_val(const type* t, size_t arr_size, void* v, void* from) {
			if (t->m_copy) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++)
				static_cast<t_copy<void>>(t->m_copy)(static_cast<char*>(v) + i * t->size, static_cast<char*>(from) + i * t->size);
			else {
				if (t->base_type == t_custom) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++) for (auto m : t->members)
					copy_val(m.m_type, m.array_size, (static_cast<char*>(v) + i * t->size + m.offset), (static_cast<char*>(from) + i * t->size + m.offset));
				else memcpy(v, from, (arr_size ? arr_size : 1) * t->size);
			}
		}


		void register_struct(state* s, const std::string& name, size_t size, std::vector<type::member>&& members, void* v1, void* v2, void* v3) {
			assert(([](const std::string& name)->bool {
				if (name.empty() || (name.front() != '_' && !std::isalpha(static_cast<unsigned char>(name.front())))) return false;
				for (auto ch : name) if (ch != '_' && !std::isalnum(static_cast<unsigned char>(ch))) return false;
				return true;
					})(name) && "incorrect type name");
			auto& t = s->global_types[name];
			t.base_type = t_custom;
			t.members = members;
			std::sort(t.members.begin(), t.members.end(), [](const type::member& a, const type::member& b) {
				return a.offset < b.offset;
					  });
			for (auto& m : t.members)
				if (m.type == t_custom) {
					m.m_type = &s->global_types[m.custom_type_name];
					assert(m.m_type);
				}
				else m.m_type = &buildin_types_v[m.type];
			auto& b = t.members.back();
			size_t sz = b.offset + (b.array_size ? b.array_size : 1) * (b.type == t_custom ? b.m_type->size : type_size[b.type]);
			assert(size && sz <= size);
			t.size = size;

			t.m_construct = v1;
			t.m_destruct = v2;
			t.m_copy = v3;
		}
		value* get_local_value(parse_result* p, const std::string& name) {
			auto f = p->local_variables.find(name);
			if (f == p->local_variables.end()) return nullptr;
			return &(f->second);
		}
	}

	static bool skip_comments_and_spaces(std::istream& in) {
		in.unget();
		int ch = in.get();
		if (!in.good() || (!std::isspace(ch) && ch != '/')) {
			in.unget();
			return false;
		}
		while (in.good() && (std::isspace(ch) || ch == '/')) {
			if (ch == '/') {
				ch = in.get();
				if (ch == '/') //single line
					while (in.good() && ch != '\n') ch = in.get();//skip line to end of line			
				else if (ch == '*') {
					int ch2 = in.get();
					ch = in.get();
					while (in.good() && !(ch2 == '*' && ch == '/')) ch2 = ch, ch = in.get();
					if (!in.good()) SGL_ERROR("SGL: unclosed multiline comment");
				}
				else {
					in.unget();
					break;
				}
				//else SGL_ERROR("SGL: invalid character");
			}
			ch = in.get();
		}
		in.unget();
		return true;
	};

	enum m_tok_t : uint8_t {
		none_v = 0,
		//numbers
		int_value_v,
		float_value_v,
		//string, char, bool
		string_value_v,
		char_value_v,
		bool_value_v,

		punct_v,//{} () [] . ,
		operator_v,//binary + - * / % ^ | & << >> && || == != > < <= >=, unary + - ! ~
		//TODO add primitive typename for typecasts?
	};
	struct m_token {
		m_token(m_tok_t t, int p) : type(t), prior(p) {
			if (type == string_value_v) new (&str_v) std::string;
		}
		m_token(const m_token& v) : type(v.type), prior(v.prior) {
			if (type == string_value_v) new (&str_v) std::string(v.str_v);
			else memcpy(this, &v, sizeof(m_token));
		}
		m_token& operator=(const m_token& v) {
			if (type == string_value_v) str_v.~basic_string();
			type = v.type;
			prior = v.prior;
			if (type == string_value_v) new (&str_v) std::string(v.str_v);
			else memcpy(this, &v, sizeof(m_token));
			return *this;
		}
		~m_token() { if (type == string_value_v) str_v.~basic_string(); }
		m_tok_t type = none_v;
		SGL_type tk_type;
		int prior = -1;
		union {
			//values
			struct {
				union {
					int8_t	 i8;
					int16_t	 i16;
					int32_t	 i32;
					int64_t	 i64;
					uint8_t  ui8;
					uint16_t ui16;
					uint32_t ui32;
					uint64_t ui64;
				};
			} int_v;
			double float_v; std::string str_v; char char_v; bool bool_v;
			//operators & punct
			char punct_v;
			struct {
				std::pair<char, char> op_v;
				bool is_unary;
			};
		};

	};

	static void unary_operator_plus(m_token& value) {
		if (value.type != int_value_v && value.type != float_value_v) SGL_ERROR("SGL: type must be integer or boolean for unary ~ operator");
	}
	static void unary_operator_minus(m_token& value) {
		if (value.type == float_value_v)value.float_v = -value.float_v;
		else if (value.type == int_value_v) {
			switch (value.tk_type) {
			case t_int8:   value.int_v.i8 =  -value.int_v.i8;  break;
			case t_int16:  value.int_v.i16 = -value.int_v.i16; break;
			case t_int32:  value.int_v.i32 = -value.int_v.i32; break;
			case t_int64:  value.int_v.i64 = -value.int_v.i64; break;
			case t_uint8:  value.tk_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui8);  break;
			case t_uint16: value.tk_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui16); break;
			case t_uint32: value.tk_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui32); break;
			case t_uint64: value.tk_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui64); break;
			}
		}
		else SGL_ERROR("SGL: invalid type for unary - operator");
	}
	static void unary_operator_not(m_token& value) {
		if (value.type == int_value_v) {
			switch (value.tk_type) {
			case t_int8:   value.bool_v= !value.int_v.i8;   break;
			case t_int16:  value.bool_v= !value.int_v.i16;  break;
			case t_int32:  value.bool_v= !value.int_v.i32;  break;
			case t_int64:  value.bool_v= !value.int_v.i64;  break;
			case t_uint8:  value.bool_v= !value.int_v.ui8;  break;
			case t_uint16: value.bool_v= !value.int_v.ui16; break;
			case t_uint32: value.bool_v= !value.int_v.ui32; break;
			case t_uint64: value.bool_v= !value.int_v.ui64; break;
			}
			value.type = bool_value_v;
		}
		else if (value.type == bool_value_v) value.bool_v = !value.bool_v;
		else SGL_ERROR("SGL: type must be integer or boolean for unary ~ operator");

	}
	static void unary_operator_bitwise_not(m_token& value) {
		if (value.type == int_value_v)
			switch (value.tk_type) {
			case t_int8:   value.int_v.i8   = ~value.int_v.i8;   break;
			case t_int16:  value.int_v.i16  = ~value.int_v.i16;  break;
			case t_int32:  value.int_v.i32  = ~value.int_v.i32;  break;
			case t_int64:  value.int_v.i64  = ~value.int_v.i64;  break;
			case t_uint8:  value.int_v.ui8  = ~value.int_v.ui8;  break;
			case t_uint16: value.int_v.ui16 = ~value.int_v.ui16; break;
			case t_uint32: value.int_v.ui32 = ~value.int_v.ui32; break;
			case t_uint64: value.int_v.ui64 = ~value.int_v.ui64; break;
			}
		else SGL_ERROR("SGL: type must be integer for unary ~ operator");
	}

	static constexpr std::pair<m_tok_t, SGL_type> result_of_value(std::pair<m_tok_t, SGL_type> a, std::pair<m_tok_t, SGL_type> b) {
		if(a == b) return a;
		switch (a.first) {
		case int_value_v: {
			if(b.first == int_value_v) {//integer result only if type is int
				bool au = t_uint8 <= a.second && a.second <= t_uint64;
				bool bu = t_uint8 <= b.second && b.second <= t_uint64;
				if(au && bu) return {int_value_v, std::max(a.second, b.second)};
				uint8_t da = au ? a.second - t_uint8 : a.second - t_int8;
				uint8_t db = au ? b.second - t_uint8 : b.second - t_int8;
				return {int_value_v, SGL_type(t_int8 + std::max(da, db))};
			} else if(b.first == float_value_v) return {float_value_v, t_void};
		} break;
		case float_value_v: if(b.first == int_value_v) return {float_value_v, t_void}; break;
		case string_value_v: if(b.first == char_value_v) return {string_value_v, t_void}; break;
		case char_value_v: if(b.first == string_value_v) return {string_value_v, t_void}; break;
		}
		SGL_ERROR("SGL: invalid type for binary operator");
		return {none_v, t_void};
	}
	static void cast_to_type(m_token& val, std::pair<m_tok_t, SGL_type> t) {
		if(val.type == t.first && t.first != int_value_v) return;
		switch (t.first) {
		case int_value_v: {
			switch (t.second) {		
			#define CAST_TO_INT_T(int_t, intv)\
			case t_##int_t: {\
				if(val.type == float_value_v) {\
					int_t##_t i = (int_t##_t)val.float_v;\
					val = m_token(int_value_v, val.prior);\
					val.tk_type = t_##int_t;\
					val.int_v.intv = i;\
					return;\
				}\
				else if(val.type == char_value_v) {\
					int_t##_t i = (int_t##_t)val.char_v;\
					val = m_token(int_value_v, val.prior);\
					val.tk_type = t_##int_t;\
					val.int_v.intv = i;\
					return;\
				}\
				else if(val.type == bool_value_v) {\
					int_t##_t i = (int_t##_t)val.bool_v;\
					val = m_token(int_value_v, val.prior);\
					val.tk_type = t_##int_t;\
					val.int_v.intv = i;\
					return;\
				}\
				else if(val.type == int_value_v) {\
					int_t##_t i = (int_t##_t)0;\
					switch (val.tk_type) {\
					case t_int8:   i = (int_t##_t)val.int_v.i8;   break;\
					case t_int16:  i = (int_t##_t)val.int_v.i16;  break;\
					case t_int32:  i = (int_t##_t)val.int_v.i32;  break;\
					case t_int64:  i = (int_t##_t)val.int_v.i64;  break;\
					case t_uint8:  i = (int_t##_t)val.int_v.ui8;  break;\
					case t_uint16: i = (int_t##_t)val.int_v.ui16; break;\
					case t_uint32: i = (int_t##_t)val.int_v.ui32; break;\
					case t_uint64: i = (int_t##_t)val.int_v.ui64; break;\
					}\
					val = m_token(int_value_v, val.prior);\
					val.tk_type = t_##int_t;\
					val.int_v.intv = i;\
					return;\
				}\
			}
			CAST_TO_INT_T(int8,  i8);
			CAST_TO_INT_T(int16, i16);
			CAST_TO_INT_T(int32, i32);
			CAST_TO_INT_T(int64, i64);
			CAST_TO_INT_T(uint8,  ui8);
			CAST_TO_INT_T(uint16, ui16);
			CAST_TO_INT_T(uint32, ui32);
			CAST_TO_INT_T(uint64, ui64);
			#undef CAST_TO_INT_T
			}
		
		} break;
		case float_value_v: {
			if(val.type == int_value_v) {
				double f = 0;
				switch (val.tk_type) {
				case t_int8:   f = (double)val.int_v.i8;   break;
				case t_int16:  f = (double)val.int_v.i16;  break;
				case t_int32:  f = (double)val.int_v.i32;  break;
				case t_int64:  f = (double)val.int_v.i64;  break;
				case t_uint8:  f = (double)val.int_v.ui8;  break;
				case t_uint16: f = (double)val.int_v.ui16; break;
				case t_uint32: f = (double)val.int_v.ui32; break;
				case t_uint64: f = (double)val.int_v.ui64; break;
				}
				val = m_token(float_value_v, val.prior);
				val.float_v = f;
				return;			
			}
			else if(val.type == char_value_v) {
				double i = (double)val.char_v;
				val = m_token(float_value_v, val.prior);
				val.float_v = i;
				return;	
			}
			else if(val.type == bool_value_v) {
				double i = (double)val.bool_v;
				val = m_token(float_value_v, val.prior);
				val.float_v = i;
				return;	
			}
		} break;
		case string_value_v: {
			if(val.type == char_value_v) {
				char ch = val.char_v;
				val = m_token(string_value_v, val.prior);
				val.str_v += ch;
				return;
			}
		} break;
		case char_value_v: {
			if(val.type == int_value_v) {
				char ch = 0;
				switch (val.tk_type) {
				case t_int8:   ch = (char)val.int_v.i8;   break;
				case t_int16:  ch = (char)val.int_v.i16;  break;
				case t_int32:  ch = (char)val.int_v.i32;  break;
				case t_int64:  ch = (char)val.int_v.i64;  break;
				case t_uint8:  ch = (char)val.int_v.ui8;  break;
				case t_uint16: ch = (char)val.int_v.ui16; break;
				case t_uint32: ch = (char)val.int_v.ui32; break;
				case t_uint64: ch = (char)val.int_v.ui64; break;
				}
				val = m_token(char_value_v, val.prior);
				val.char_v = ch;
				return;
			} if(val.type == float_value_v) {
				char ch = (char)val.float_v;
				val = m_token(char_value_v, val.prior);
				val.char_v = ch;
				return;	
			} if(val.type == bool_value_v) {
				char ch = (char)val.bool_v;
				val = m_token(char_value_v, val.prior);
				val.char_v = ch;
				return;	
			}
		} break;
		}
		SGL_ERROR("SGL: invalid type cast");
	}

	using binary_operator_template_func_t = void(*)(m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t);
	template<binary_operator_template_func_t func> 
	static void binary_operator_template(m_token& a, m_token& b) {
		std::pair<m_tok_t, SGL_type> a_type = {a.type, t_void};
		std::pair<m_tok_t, SGL_type> b_type = {b.type, t_void};
		if(a.type == int_value_v) a_type.second = a.tk_type;
		if(b.type == int_value_v) b_type.second = b.tk_type;
		auto result_type = result_of_value(a_type, b_type);
		cast_to_type(a, result_type);
		cast_to_type(b, result_type);
		func(a, b, result_type);
	}
	#define binary_operator_def_i0(func)
	#define binary_operator_def_i1(func)\
		case int_value_v:\
			switch (t.second) {\
			case t_int8:   a.int_v.i8   func b.int_v.i8;   return;\
			case t_int16:  a.int_v.i16  func b.int_v.i16;  return;\
			case t_int32:  a.int_v.i32  func b.int_v.i32;  return;\
			case t_int64:  a.int_v.i64  func b.int_v.i64;  return;\
			case t_uint8:  a.int_v.ui8  func b.int_v.ui8;  return;\
			case t_uint16: a.int_v.ui16 func b.int_v.ui16; return;\
			case t_uint32: a.int_v.ui32 func b.int_v.ui32; return;\
			case t_uint64: a.int_v.ui64 func b.int_v.ui64; return;\
			}
	#define binary_operator_def_f0(func)
	#define binary_operator_def_f1(func)\
		case float_value_v: a.float_v func b.float_v; return;
	#define binary_operator_def_s0(func)	
	#define binary_operator_def_s1(func)\
		case string_value_v: a.str_v func b.str_v; return;
	#define binary_operator_def_c0(func)	
	#define binary_operator_def_c1(func)\
		case char_value_v: a.char_v func b.char_v; return;
	#define binary_operator_def_b0(func)	
	#define binary_operator_def_b1(func)\
		case bool_value_v: a.bool_v func b.bool_v; return;

	#define binary_operator_def(name, func, en_int, en_float, en_string, en_char, en_bool)\
	static void binary_operator_##name(m_token& val, m_token& other) {\
		auto v = [](m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t){\
			switch (t.first) {\
				binary_operator_def_i##en_int(func)\
				binary_operator_def_f##en_float(func)\
				binary_operator_def_s##en_string(func)\
				binary_operator_def_c##en_char(func)\
				binary_operator_def_b##en_bool(func)\
			}\
			SGL_ERROR("SGL: invalid operation type");\
			return;\
		};\
		binary_operator_template<v>(val, other);\
	}
	//arithmetic + - * / % 
	binary_operator_def(sum, +=, 1, 1, 1, 0, 0);
	binary_operator_def(sub, +=, 1, 1, 0, 0, 0);
	binary_operator_def(mul, *=, 1, 1, 0, 0, 0);
	binary_operator_def(div, /=, 1, 1, 0, 0, 0);
	binary_operator_def(mod, %=, 1, 0, 0, 0, 0);
	//bitwise ^ | & << >>
	binary_operator_def(xor, ^=, 1, 0, 0, 0, 0);
	binary_operator_def(bitwise_or,  |=, 1, 0, 0, 0, 0);
	binary_operator_def(bitwise_and, &=, 1, 0, 0, 0, 0);
	binary_operator_def(lsh, <<=, 1, 0, 0, 0, 0);
	binary_operator_def(rsh, >>=, 1, 0, 0, 0, 0);
	//logic && || == != > < >= <=
	static void binary_operator_and(m_token& val, m_token& other) {
		auto v = [](m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t){
			bool v = false;
			switch (t.first) {
			case int_value_v:
				switch (t.second) {
				case t_int8:   v = a.int_v.i8   && b.int_v.i8;   break;
				case t_int16:  v = a.int_v.i16  && b.int_v.i16;  break;
				case t_int32:  v = a.int_v.i32  && b.int_v.i32;  break;
				case t_int64:  v = a.int_v.i64  && b.int_v.i64;  break;
				case t_uint8:  v = a.int_v.ui8  && b.int_v.ui8;  break;
				case t_uint16: v = a.int_v.ui16 && b.int_v.ui16; break;
				case t_uint32: v = a.int_v.ui32 && b.int_v.ui32; break;
				case t_uint64: v = a.int_v.ui64 && b.int_v.ui64; break;
				}
			case char_value_v:   v = a.char_v && b.char_v;   break;
			case bool_value_v:   v = a.bool_v && b.bool_v;   break;
			}
			cast_to_type(a, {bool_value_v, t_void});
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_or(m_token& val, m_token& other) {
		auto v = [](m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t){
			bool v = false;
			switch (t.first) {
			case int_value_v:
				switch (t.second) {
				case t_int8:   v = a.int_v.i8   || b.int_v.i8;   break;
				case t_int16:  v = a.int_v.i16  || b.int_v.i16;  break;
				case t_int32:  v = a.int_v.i32  || b.int_v.i32;  break;
				case t_int64:  v = a.int_v.i64  || b.int_v.i64;  break;
				case t_uint8:  v = a.int_v.ui8  || b.int_v.ui8;  break;
				case t_uint16: v = a.int_v.ui16 || b.int_v.ui16; break;
				case t_uint32: v = a.int_v.ui32 || b.int_v.ui32; break;
				case t_uint64: v = a.int_v.ui64 || b.int_v.ui64; break;
				}
			case char_value_v:   v = a.char_v || b.char_v;   break;
			case bool_value_v:   v = a.bool_v || b.bool_v;   break;
			}
			cast_to_type(a, {bool_value_v, t_void});
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_eqal(m_token& val, m_token& other) {	
		auto v = [](m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t){
			bool v = false;
			switch (t.first) {
			case int_value_v:
				switch (t.second) {
				case t_int8:   v = a.int_v.i8   == b.int_v.i8;   break;
				case t_int16:  v = a.int_v.i16  == b.int_v.i16;  break;
				case t_int32:  v = a.int_v.i32  == b.int_v.i32;  break;
				case t_int64:  v = a.int_v.i64  == b.int_v.i64;  break;
				case t_uint8:  v = a.int_v.ui8  == b.int_v.ui8;  break;
				case t_uint16: v = a.int_v.ui16 == b.int_v.ui16; break;
				case t_uint32: v = a.int_v.ui32 == b.int_v.ui32; break;
				case t_uint64: v = a.int_v.ui64 == b.int_v.ui64; break;
				}
			case float_value_v:  v = a.float_v == b.float_v; break;
			case string_value_v: v = a.str_v == b.str_v;     break;
			case char_value_v:   v = a.char_v == b.char_v;   break;
			case bool_value_v:   v = a.bool_v == b.bool_v;   break;
			}
			cast_to_type(a, {bool_value_v, t_void});
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_not_eqal(m_token& val, m_token& other) {
		auto v = [](m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t){
			bool v = false;
			switch (t.first) {
			case int_value_v:
				switch (t.second) {
				case t_int8:   v = a.int_v.i8   != b.int_v.i8;   break;
				case t_int16:  v = a.int_v.i16  != b.int_v.i16;  break;
				case t_int32:  v = a.int_v.i32  != b.int_v.i32;  break;
				case t_int64:  v = a.int_v.i64  != b.int_v.i64;  break;
				case t_uint8:  v = a.int_v.ui8  != b.int_v.ui8;  break;
				case t_uint16: v = a.int_v.ui16 != b.int_v.ui16; break;
				case t_uint32: v = a.int_v.ui32 != b.int_v.ui32; break;
				case t_uint64: v = a.int_v.ui64 != b.int_v.ui64; break;
				}
			case float_value_v:  v = a.float_v != b.float_v; break;
			case string_value_v: v = a.str_v != b.str_v;     break;
			case char_value_v:   v = a.char_v != b.char_v;   break;
			case bool_value_v:   v = a.bool_v != b.bool_v;   break;
			}
			cast_to_type(a, {bool_value_v, t_void});
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_greater(m_token& val, m_token& other) {	
		auto v = [](m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t){
			bool v = false;
			switch (t.first) {
			case int_value_v:
				switch (t.second) {
				case t_int8:   v = a.int_v.i8   > b.int_v.i8;   break;
				case t_int16:  v = a.int_v.i16  > b.int_v.i16;  break;
				case t_int32:  v = a.int_v.i32  > b.int_v.i32;  break;
				case t_int64:  v = a.int_v.i64  > b.int_v.i64;  break;
				case t_uint8:  v = a.int_v.ui8  > b.int_v.ui8;  break;
				case t_uint16: v = a.int_v.ui16 > b.int_v.ui16; break;
				case t_uint32: v = a.int_v.ui32 > b.int_v.ui32; break;
				case t_uint64: v = a.int_v.ui64 > b.int_v.ui64; break;
				}
			case float_value_v:  v = a.float_v > b.float_v; break;
			case string_value_v: v = a.str_v > b.str_v;     break;
			case char_value_v:   v = a.char_v > b.char_v;   break;
			case bool_value_v:   v = a.bool_v > b.bool_v;   break;
			}
			cast_to_type(a, {bool_value_v, t_void});
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_less(m_token& val, m_token& other) {
		auto v = [](m_token& a, m_token& b, std::pair<m_tok_t, SGL_type> t){
			bool v = false;
			switch (t.first) {
			case int_value_v:
				switch (t.second) {
				case t_int8:   v = a.int_v.i8   < b.int_v.i8;   break;
				case t_int16:  v = a.int_v.i16  < b.int_v.i16;  break;
				case t_int32:  v = a.int_v.i32  < b.int_v.i32;  break;
				case t_int64:  v = a.int_v.i64  < b.int_v.i64;  break;
				case t_uint8:  v = a.int_v.ui8  < b.int_v.ui8;  break;
				case t_uint16: v = a.int_v.ui16 < b.int_v.ui16; break;
				case t_uint32: v = a.int_v.ui32 < b.int_v.ui32; break;
				case t_uint64: v = a.int_v.ui64 < b.int_v.ui64; break;
				}
			case float_value_v:  v = a.float_v < b.float_v; break;
			case string_value_v: v = a.str_v < b.str_v;     break;
			case char_value_v:   v = a.char_v < b.char_v;   break;
			case bool_value_v:   v = a.bool_v < b.bool_v;   break;
			}
			cast_to_type(a, {bool_value_v, t_void});
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_not_less(m_token& val, m_token& other) {
		binary_operator_less(val, other);
		unary_operator_not(val);
	}
	static void binary_operator_not_greater(m_token& val, m_token& other) {
		binary_operator_greater(val, other);
		unary_operator_not(val);
	}

	static void parse_expession(const state* cur_state, const parse_result* res, const type* t, size_t array_size, char* data, std::istream& in) {
		//{1, 2, 3, int(23), g.v, 12+13, 12.4, "qq all"}
		skip_comments_and_spaces(in);
		int ch = in.get();
		std::list<m_token> tokens;
		using iter = typename decltype(tokens)::iterator;
		std::string s;
		int cur_prior = 0;
		size_t ops_count = 0;
		iter last_op = tokens.begin();
		iter eval_beg = tokens.begin();

		auto eval_expr = [&](){		
			if (!ops_count) return; 
			if(eval_beg == tokens.end()) 
				eval_beg = tokens.begin();
			auto it = eval_beg, prev_it = it, next_it = it;
			next_it++;
			using ops_val_t = std::pair<m_token*, std::pair<int, iter>>;
			std::vector<ops_val_t> operators(ops_count, { nullptr, {0, it} });
			ops_count = 0;
			for (size_t i = 0; i < tokens.size() && it != tokens.end(); i++, prev_it = it, it = next_it, (next_it != tokens.end()) ? next_it++ : next_it) {
				auto& tok = *it;
				const auto& prev = (it != prev_it) ? *prev_it : m_token(none_v, -1);
				const auto& next = (next_it != tokens.end()) ? *next_it : m_token(none_v, -1);
				if (tok.type != operator_v) continue;
				tok.is_unary = false;
				if (tok.op_v.second == '\0' && ((i == 0 || prev.type == operator_v || prev.type == punct_v) && (tok.op_v.first == '+' || tok.op_v.first == '-') ||
					tok.op_v.first == '!' || tok.op_v.first == '~' || tok.op_v.first == 't')) {	
					tok.is_unary = true;
					//else SGL_ERROR("SGL: less than 2 args given to binary operator");
				}
				if (i + 1 == tokens.size() || (next.type == punct_v && (
					next.punct_v == '}' || next.punct_v == ','
					))) SGL_ERROR("SGL: less than 2 args given to binary operator");
				operators[ops_count++] = { &tok, {(int)i, it} };
			}

			std::sort(operators.begin(), operators.end(), [](const ops_val_t& a, const ops_val_t& b) {
				if (a.first->prior != b.first->prior) return a.first->prior > b.first->prior;//brackets level
				if (a.first->is_unary != b.first->is_unary) return int(a.first->is_unary) > int(b.first->is_unary);//unary priority greater then binary
				if (a.first->is_unary) return a.second.first > b.second.first;//a && b is unary -> priority from right	
				//by precedence: 1: {  * / %  } 2: {  + -  } 3: {  << >>  } 4: {  > < <= >=  } 5: {  == !=  } 6: {  ^  } 7: {  &  } 8: {  |  } 9: {  &&  } 10: { || }
				static const std::map<std::pair<char, char>, size_t> op_precedence {
					{{'*', '\0'}, 1}, {{'/', '\0'}, 1}, {{'%', '\0'}, 1},
					{{'+', '\0'}, 2}, {{'-', '\0'}, 2},
					{{'<', '<'}, 3}, {{'>', '>'}, 2},
					{{'>', '\0'}, 4}, {{'<', '\0'}, 4}, {{'>', '='}, 4}, {{'<', '='}, 4},
					{{'=', '='}, 5}, {{'!', '='}, 5},
					{{'^', '\0'}, 6},
					{{'&', '\0'}, 7},
					{{'|', '\0'}, 8},
					{{'&', '&'}, 9},
					{{'|', '|'}, 10},
				};
				auto p1 = op_precedence.find(a.first->op_v);
				auto p2 = op_precedence.find(b.first->op_v);
				if (p1 == op_precedence.end() || p2 == op_precedence.end()) SGL_ERROR("SGL: invalid binary operator");
				if (p1->second == p2->second) return a.second.first < b.second.first;//same priority -> from left
				return p1->second < p2->second;
			});

			//process
			for (auto& v : operators) {
				auto cur = v.second.second, next = cur, prev = cur;
				next++;
				if (v.first->is_unary) {
					switch (v.first->op_v.first) {
					case '-': unary_operator_minus(*next); break;
					case '~': unary_operator_bitwise_not(*next); break;
					case '!': unary_operator_not(*next); break;
					case '+': unary_operator_plus(*next); break;
					case 't': cast_to_type(*next, t_int8 <= v.first->tk_type && v.first->tk_type <= t_uint64 ? 
					std::pair<m_tok_t, SGL_type>{int_value_v, v.first->tk_type} : std::pair<m_tok_t, SGL_type>{
						v.first->tk_type == t_float32 || v.first->tk_type == t_float64 ? float_value_v : 
						v.first->tk_type == t_string ? string_value_v :
						v.first->tk_type == t_char ? char_value_v :
						v.first->tk_type == t_bool ? bool_value_v :
						none_v, t_void}); break;
					}
					tokens.erase(v.second.second);//erase operator
				}
				else {
					prev--;
					switch ((int(v.first->op_v.first) << 8) | int(v.first->op_v.second)) {
					//arithmetic + - * / % 
					case (int('+') << 8) | int('\0'): binary_operator_sum(*prev, *next); break;
					case (int('-') << 8) | int('\0'): binary_operator_sub(*prev, *next); break;
					case (int('*') << 8) | int('\0'): binary_operator_mul(*prev, *next); break;
					case (int('/') << 8) | int('\0'): binary_operator_div(*prev, *next); break;
					case (int('%') << 8) | int('\0'): binary_operator_mod(*prev, *next); break;
					//bitwise ^ | & << >>
					case (int('^') << 8) | int('\0'): binary_operator_xor(*prev, *next); 		break;
					case (int('|') << 8) | int('\0'): binary_operator_bitwise_or(*prev, *next); break;
					case (int('&') << 8) | int('\0'): binary_operator_bitwise_and(*prev, *next);break;	
					case (int('<') << 8) | int('<'):  binary_operator_lsh(*prev, *next); 		break;
					case (int('>') << 8) | int('>'):  binary_operator_rsh(*prev, *next); 		break;
					//logic && || == != > < >= <=
					case (int('&') << 8) | int('&'):  binary_operator_and(*prev, *next);		break;
					case (int('|') << 8) | int('|'):  binary_operator_or(*prev, *next);			break;
					case (int('=') << 8) | int('='):  binary_operator_eqal(*prev, *next);		break;
					case (int('!') << 8) | int('='):  binary_operator_not_eqal(*prev, *next);	break;
					case (int('>') << 8) | int('\0'): binary_operator_greater(*prev, *next);	break;
					case (int('<') << 8) | int('\0'): binary_operator_less(*prev, *next);		break;
					case (int('>') << 8) | int('='):  binary_operator_not_less(*prev, *next);	break;
					case (int('<') << 8) | int('='):  binary_operator_not_greater(*prev, *next);break;
					default: SGL_ERROR("SGL: invalid operator");
					}					
					tokens.erase(v.second.second);
					tokens.erase(next);
				}
			}
			ops_count = 0;
			eval_beg = tokens.end();
			eval_beg--;
		};

		while (in.good() && ch != ';') {
			skip_comments_and_spaces(in);
			ch = in.get();
			if (!in.good()) break;
			switch (ch) {
			case '+': case '-': case '*': case '/': case '%':
			case '^': case '~': { m_token t{ operator_v, cur_prior }; t.op_v = { ch, '\0' }; tokens.push_back(t); ops_count++; } break;
			case '&': case '|': {//&& & || |
				m_token t{ operator_v, cur_prior };
				t.op_v = { ch, '\0' };
				if (ch == in.peek()) t.op_v.second = ch, ch = in.get();
				tokens.push_back(t);
				ops_count++;
			} break;
			case '=': {//== 
				m_token t{ operator_v, cur_prior };
				t.op_v = { ch, '\0' };
				if (ch == in.peek()) t.op_v.second = ch, ch = in.get();
				else SGL_ERROR("SGL: invalid operator");
				tokens.push_back(t);
				ops_count++;
			} break;
			case '!': {// != !
				m_token t{ operator_v, cur_prior };
				t.op_v = { ch, '\0' };
				if (in.peek() == '=') t.op_v.second = '=', ch = in.get();
				tokens.push_back(t);
				ops_count++;
			} break;
			case '<': case '>': {// < << <= > >> >=
				m_token t{ operator_v, cur_prior };
				t.op_v = { ch, '\0' };
				if (in.peek() == '=') t.op_v.second = '=', ch = in.get();
				else if (in.peek() == ch) t.op_v.second = ch, ch = in.get();
				tokens.push_back(t);
				ops_count++;
			} break;
			case '(': { cur_prior++; } break;
			case ')': { cur_prior--; if(cur_prior < 0) SGL_ERROR("SGL: invalid brackets sequence"); } break;
			case '}': case ',': { if (cur_prior != 0) SGL_ERROR("SGL: invalid brackets sequence"); eval_expr(); }
			case '{': { m_token t{ punct_v, cur_prior }; t.punct_v = ch; tokens.push_back(t); } break;
			case '.':
			if (!std::isdigit(in.peek())) { m_token t{ punct_v, cur_prior };t.punct_v = ch;tokens.push_back(t);break; }
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': {
				bool is_float = ch == '.';
				bool exp_positive = true;
				uint64_t int_part = 0, int_part_l = 0, v0 = 0, fract_part = 0, fract_part_div = 1, exp_part = 0;
				while (std::isdigit(ch)) {
					int_part = int_part * 10 + ch - '0', ch = in.get(), int_part_l++;
					if (int_part < v0) SGL_ERROR("SGL: to large number");
					v0 = int_part;
				}
				if (ch == '.') {
					is_float = true;
					ch = in.get();
					v0 = 0;
					while (std::isdigit(ch)) {
						fract_part = fract_part * 10 + ch - '0', ch = in.get(), fract_part_div *= 10;
						if (fract_part < v0) SGL_ERROR("SGL: to large fract part");
						v0 = fract_part;
					}
				}
				if (ch == 'e') {
					if (fract_part_div == 1 && int_part_l == 0) SGL_ERROR("SGL: number must contain digits before exponent");
					is_float = true;
					bool correct_exp = false;//has digits
					ch = in.get();
					if (ch == '-') exp_positive = false;
					if (ch == '+' || ch == '-') ch = in.get();
					while (std::isdigit(ch)) {
						exp_part = exp_part * 10 + ch - '0', ch = in.get();
						correct_exp = true;
						if (exp_part > 308) SGL_ERROR("SGL: to large exponent");
					}
					if (!correct_exp) SGL_ERROR("SGL: invalid exponent");
				}
				if (is_float) {
					m_token t{ float_value_v, cur_prior };
					double n = double(int_part) + double(fract_part) / double(fract_part_div);
					n *= pow(10., (exp_positive ? 1. : -1) * exp_part);
					t.float_v = n;
					tokens.push_back(t);
				}
				else {
					m_token t{ int_value_v, cur_prior };

					if (int_part <= (uint64_t)std::numeric_limits<int64_t>::max())
						t.int_v.i64 = int_part, t.tk_type = t_int64;
					else t.int_v.ui64 = int_part, t.tk_type = t_uint64;

					t.tk_type = t_uint64;
					tokens.push_back(t);
				}
				in.unget();
			} break;

			case '\'': case '"': {
				int del = ch;
				ch = in.get();
				std::string s;
				while (in.good() && ch != del) {
					if(ch == '\\') {
						switch (in.get()) {
						case '"': s += '"';  break;
						case '\'': s += '\''; break;
						case '\\': s += '\\'; break;
						case 'a': s += '\a'; break;
						case 'b': s += '\b'; break;
						case 'f': s += '\f'; break;
						case 'n': s += '\b'; break;
						case 'r': s += '\r'; break;
						case 't': s += '\t'; break;
						case 'v': s += '\v'; break;
						case '0': s += '\0'; break;
						default: SGL_ERROR("SGL: invalid escape sequence"); break;
						}
					}
					else s += ch;
					ch = in.get();
				}
				if(!in.good()) SGL_ERROR("SGL: unexpected eof");
				if(del == '"') {//string
					m_token t{ string_value_v, cur_prior };
					t.str_v = std::move(s);
					tokens.push_back(t);
				} else {//char
					m_token t{ char_value_v, cur_prior };
					if(s.size() != 1) SGL_ERROR("SGL: invalid character literal");
					t.char_v = s[0];
					tokens.push_back(t);
				}
			} break;

			default: {
				std::string s;
				auto scan_string = [&](){
					s.clear();
					while(in.good() && std::isalnum(ch)) s += ch, ch = in.get();
				};
				scan_string();
				if(s == "true") {
					m_token t{ bool_value_v, cur_prior };
					t.bool_v = true;
					tokens.push_back(t);
				} else if(s == "false") {
					m_token t{ bool_value_v, cur_prior };
					t.bool_v = false;
					tokens.push_back(t);
				} else if(auto f = buildin_types.find(s); f != buildin_types.end()) {
					skip_comments_and_spaces(in);
					if(in.peek() != '(') SGL_ERROR("SGL: missing open '(' in type cast");
					m_token t{ operator_v, cur_prior };
					t.op_v = { 't', '\0' };
					t.tk_type = f->second->base_type;
					tokens.push_back(t);
					ops_count++;
				}
				else {//constant or error
					const type* result_type = nullptr;
					void* data = nullptr;
					size_t max_array_size = 0;
					auto scan_v = [&](){};
					if(auto f1 = res->local_variables.find(s); f1 != res->local_variables.end()) {//local var
						data = f1->second.data;
						result_type = f1->second.m_type;
						max_array_size = f1->second.array_size;
					} 
					else if(auto f2 = cur_state->global_constants.find(s); f2 != cur_state->global_constants.end()) {
						data = f2->second.data;
						result_type = f2->second.m_type;
						max_array_size = f2->second.array_size;
					} 
					else SGL_ERROR("SGL: undeclared identifier");
					if(result_type) {
						skip_comments_and_spaces(in);
						while(in.peek() == '.' || in.peek() == '[') {
							if(in.peek() == '.') {//begin member
								ch = in.get();//get '.'
								ch = in.get();//get next
								skip_comments_and_spaces(in);
								ch = in.get();
								if(result_type->base_type != t_custom) SGL_ERROR("SGL: can't acces to member in primitive type");
								scan_string();
								auto it = result_type->members.begin();
								for(; it != result_type->members.end(); it++) {
									if(it->name == s) break;
								} 
								if(it == result_type->members.end()) SGL_ERROR("SGL: can't acces to member in primitive type");
								data = static_cast<char*>(data) + it->offset;
								result_type = it->m_type;
								max_array_size = it->array_size;
							} else if(in.peek() == '[') {
								ch = in.get();//get '['
								//TODO add array
							}
							skip_comments_and_spaces(in);
						}
						//in.unget();
						if(result_type->base_type == t_custom) {//add custom type

						}
						else {
							switch (result_type->base_type) {
							case t_int8:  	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.i8 = *reinterpret_cast<int8_t*>(data);
								tokens.push_back(t); 
							} break;
							case t_int16: 	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.i32 = *reinterpret_cast<int16_t*>(data);
								tokens.push_back(t);
							} break;
							case t_int32: 	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.i32 = *reinterpret_cast<int32_t*>(data);
								tokens.push_back(t);
							} break;
							case t_int64: 	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.i64 = *reinterpret_cast<int64_t*>(data);
								tokens.push_back(t);
							} break;
							case t_uint8:  	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.ui8 = *reinterpret_cast<uint8_t*>(data);
								tokens.push_back(t);
							} break;
							case t_uint16: 	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.ui16 = *reinterpret_cast<uint16_t*>(data);
								tokens.push_back(t);
							} break;
							case t_uint32: 	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.ui32 = *reinterpret_cast<uint32_t*>(data);
								tokens.push_back(t);
							} break;
							case t_uint64: 	{
								m_token t{ int_value_v, cur_prior };
								t.tk_type = result_type->base_type;
								t.int_v.ui64 = *reinterpret_cast<uint64_t*>(data);
								tokens.push_back(t);
							} break;	
							case t_float32: { 
								m_token t{ float_value_v, cur_prior };
								t.float_v = (double)*reinterpret_cast<float*>(data);
								tokens.push_back(t);
							} break;
							case t_float64: {
								m_token t{ float_value_v, cur_prior };
								t.float_v = *reinterpret_cast<double*>(data);
								tokens.push_back(t);
							} break;
							case t_string: 	{ 
								m_token t{ string_value_v, cur_prior };
								t.str_v = *reinterpret_cast<std::string*>(data);
								tokens.push_back(t);
							} break;		
							case t_char: 	{ 
								m_token t{ char_value_v, cur_prior };
								t.char_v = *reinterpret_cast<char*>(data);
								tokens.push_back(t);
							} break;		
							case t_bool: 	{ 
								m_token t{ bool_value_v, cur_prior };
								t.bool_v = *reinterpret_cast<bool*>(data);
								tokens.push_back(t);
							} break;	
							default: SGL_ERROR("SGL: invalid type"); break;
							}
						}
					}
				}
			} break;		
			}

			ch = in.get();
		}
		if (!in.good()) SGL_ERROR("SGL: unexpected eof");
		eval_expr();

		//state* cur_state, const type* t, size_t array_size, char* data
		std::function<iter(iter, const type*, size_t, char*)> get_result;
		get_result = [&](iter it, const type* t, size_t array_size, char* data) -> iter {	
			if(array_size) 
				if(it->type == punct_v && it->punct_v == '{') it++;
				else SGL_ERROR("SGL: excepted '{'");
			for(size_t s = array_size ? array_size : 1, i = s-1, l = 0; i < s; i--, l++) {
				char* cur_data = data + l * t->size;
				if(t->base_type == t_custom) {
					if(it->type == punct_v && it->punct_v == '{') it++;
					else SGL_ERROR("SGL: excepted '{'");
					size_t mi = t->members.size();
					for(auto& m : t->members) {
						it = get_result(it, m.m_type, m.array_size, cur_data + m.offset);
						if(--mi) 
							if(it->type == punct_v && it->punct_v == ',') it++;
							else SGL_ERROR("SGL: excepted ','");
					}
					if(it->type == punct_v && it->punct_v == '}') it++;
					else SGL_ERROR("SGL: excepted '}'");
				} else {
					switch (t->base_type) {
					case t_int8:  cast_to_type(*it, {int_value_v, t_int8});  *reinterpret_cast<int8_t*>(cur_data)  = it->int_v.i8;  break;
					case t_int16: cast_to_type(*it, {int_value_v, t_int16}); *reinterpret_cast<int16_t*>(cur_data) = it->int_v.i16; break;
					case t_int32: cast_to_type(*it, {int_value_v, t_int32}); *reinterpret_cast<int32_t*>(cur_data) = it->int_v.i32; break;
					case t_int64: cast_to_type(*it, {int_value_v, t_int64}); *reinterpret_cast<int64_t*>(cur_data) = it->int_v.i64; break;
					case t_uint8:  cast_to_type(*it, {int_value_v, t_uint8});  *reinterpret_cast<uint8_t*>(cur_data)  = it->int_v.ui8;  break;
					case t_uint16: cast_to_type(*it, {int_value_v, t_uint16}); *reinterpret_cast<uint16_t*>(cur_data) = it->int_v.ui16; break;
					case t_uint32: cast_to_type(*it, {int_value_v, t_uint32}); *reinterpret_cast<uint32_t*>(cur_data) = it->int_v.ui32; break;
					case t_uint64: cast_to_type(*it, {int_value_v, t_uint64}); *reinterpret_cast<uint64_t*>(cur_data) = it->int_v.ui64; break;	

					case t_float32: cast_to_type(*it, {float_value_v, t_void}); *reinterpret_cast<float*>(cur_data) = (float)it->float_v; break;
					case t_float64: cast_to_type(*it, {float_value_v, t_void}); *reinterpret_cast<double*>(cur_data) = it->float_v; break;

					case t_string: cast_to_type(*it, {string_value_v, t_void}); *reinterpret_cast<std::string*>(cur_data) = it->str_v; break;		
					case t_char: cast_to_type(*it, {char_value_v, t_void}); *reinterpret_cast<char*>(cur_data) = it->char_v; break;		
					case t_bool: cast_to_type(*it, {bool_value_v, t_void}); *reinterpret_cast<bool*>(cur_data) = it->bool_v; break;	

					default: SGL_ERROR("SGL: invalid type"); break;
					}
					it++;
				}
				if(i) 
					if(it->type == punct_v && it->punct_v == ',') it++;
					else SGL_ERROR("SGL: excepted ','");
			}
			if(array_size) 
				if(it->type == punct_v && it->punct_v == '}') it++;
				else SGL_ERROR("SGL: excepted '}'");
			return it;
		};
		auto it = get_result(tokens.begin(), t, array_size, data);
		if(it != tokens.end()) SGL_ERROR("SGL: excepted ';'");
	}

	parse_result* parse_stream(state* cur_state, std::istream& in) {
		parse_result* res = new parse_result;
		cur_state->m_results.insert(res);
		res->m_state = cur_state;
		int ch = in.get();
		std::string s;
		while (in.good()) {
			skip_comments_and_spaces(in);
			ch = in.get();

			if (!in.good()) break;
			//scan type name
			if (!std::isalpha(ch) && ch != '_') SGL_ERROR("SGL: type name must begin from alphabet character or from _");
			std::string type_name;
			while (in.good() && (std::isalnum(ch) || ch == '_')) type_name += ch, ch = in.get();

			if (!in.good()) SGL_ERROR("SGL: unexpected end of file");

			skip_comments_and_spaces(in);
			ch = in.get();

			if (!std::isalpha(ch) && ch != '_') SGL_ERROR("SGL: variable name must begin from alphabet character or from _");
			std::string name;
			while (in.good() && (std::isalnum(ch) || ch == '_')) name += ch, ch = in.get();

			skip_comments_and_spaces(in);
			ch = in.get();

			if (!in.good()) SGL_ERROR("SGL: unexpected end of file");
			if (ch != '=') SGL_ERROR("SGL: expected '=' caracter");
			ch = in.get();

			auto tf = cur_state->global_types.find(type_name);
			auto btf = buildin_types.find(type_name);
			if (tf == cur_state->global_types.end() && btf == buildin_types.end()) SGL_ERROR("SGL: invalid type name");
			const type* t = (tf != cur_state->global_types.end()) ? &(tf->second) : btf->second;

			auto& cur_val = res->local_variables[name];
			cur_val.m_type = t;
			char* data = new char[t->size];
			cur_val.data = data;

			details::construct_val(t, 0, data);

			parse_expession(cur_state, res, t, 0, data, in);
			ch = in.get();
		}
		return res;
	}
}