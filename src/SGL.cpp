#include <SGL/SGL.hpp>

#include <algorithm>
#include <iostream>
#include <functional>
#include <list>
#include <cassert>
#include <numeric>
#include <map>
#include <tuple>

#define SGL_ASSERT(v) if(!(v)) { SGL::error("assertion failed in " + std::to_string(__LINE__)); }
#define SGL_ERROR(v) throw SGL::details::sgl_exception(v)

namespace SGL {
	namespace details {
		class sgl_exception : public std::exception {
		public:
			sgl_exception() = default;
			sgl_exception(const char* msg) : msg(msg) {}
			virtual ~sgl_exception() = default; 
			virtual const char* what() const noexcept override {
				return msg;
			};
		private:
			const char* msg = nullptr;
		};
    	static_assert(sizeof(float) == 4 && sizeof(double) == 8, "required float size 32 bit & 64 bit for double");
    	static constexpr uint8_t type_size[] {
    	    0,//void
    	    1, 2, 4, 8, 1, 2, 4, 8,//int types
    	    4, 8,//float
    	    1, (uint8_t)sizeof(std::string), 1//bool, string, char
    	};
		static inline type construct_type(primitive_type t = t_void, void* v1 = nullptr, void* v2 = nullptr, void* v3 = nullptr) {
			type ret;
			ret.base_type = t;
            ret.size = type_size[t];
			ret.m_construct = v1;
			ret.m_destruct = v2;
			ret.m_copy = v3;
			return ret;
		}
		static void defualt_sgl_error_function(const std::string& description) {
			std::cerr << description << std::endl;
		}

		static int8_t  	m_min(int8_t   a, int8_t   b) { return a < b ? a : b; }
		static int16_t 	m_min(int16_t  a, int16_t  b) { return a < b ? a : b; }
		static int32_t 	m_min(int32_t  a, int32_t  b) { return a < b ? a : b; }
		static int64_t 	m_min(int64_t  a, int64_t  b) { return a < b ? a : b; }
		static uint8_t  m_min(uint8_t  a, uint8_t  b) { return a < b ? a : b; }
		static uint16_t m_min(uint16_t a, uint16_t b) { return a < b ? a : b; }
		static uint32_t m_min(uint32_t a, uint32_t b) { return a < b ? a : b; }
		static uint64_t m_min(uint64_t a, uint64_t b) { return a < b ? a : b; }
		static float 	m_min(float    a, float    b) { return a < b ? a : b; }
		static double 	m_min(double   a, double   b) { return a < b ? a : b; }

		static int8_t  	m_max(int8_t   a, int8_t   b) { return a < b ? a : b; }
		static int16_t 	m_max(int16_t  a, int16_t  b) { return a < b ? a : b; }
		static int32_t 	m_max(int32_t  a, int32_t  b) { return a < b ? a : b; }
		static int64_t 	m_max(int64_t  a, int64_t  b) { return a < b ? a : b; }
		static uint8_t  m_max(uint8_t  a, uint8_t  b) { return a < b ? a : b; }
		static uint16_t m_max(uint16_t a, uint16_t b) { return a < b ? a : b; }
		static uint32_t m_max(uint32_t a, uint32_t b) { return a < b ? a : b; }
		static uint64_t m_max(uint64_t a, uint64_t b) { return a < b ? a : b; }
		static float 	m_max(float    a, float    b) { return a < b ? a : b; }
		static double 	m_max(double   a, double   b) { return a < b ? a : b; }

		static int8_t  	m_clamp(int8_t   v, int8_t   v_min, int8_t   v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static int16_t 	m_clamp(int16_t  v, int16_t  v_min, int16_t  v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static int32_t 	m_clamp(int32_t  v, int32_t  v_min, int32_t  v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static int64_t 	m_clamp(int64_t  v, int64_t  v_min, int64_t  v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static uint8_t  m_clamp(uint8_t  v, uint8_t  v_min, uint8_t  v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static uint16_t m_clamp(uint16_t v, uint16_t v_min, uint16_t v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static uint32_t m_clamp(uint32_t v, uint32_t v_min, uint32_t v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static uint64_t m_clamp(uint64_t v, uint64_t v_min, uint64_t v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static float 	m_clamp(float    v, float    v_min, float    v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }
		static double 	m_clamp(double   v, double   v_min, double   v_max) { return v < v_min ? v_min : v > v_max ? v_max : v; }

		//TODO add clamp function
	}
	static const type buildin_types_v[t_custom]{
		details::construct_type(t_void),
		details::construct_type(t_int8), 	details::construct_type(t_int16),	details::construct_type(t_int32), 	details::construct_type(t_int64),
		details::construct_type(t_uint8), 	details::construct_type(t_uint16), 	details::construct_type(t_uint32), 	details::construct_type(t_uint64),
		details::construct_type(t_float32), details::construct_type(t_float64),
		details::construct_type(t_bool),
		details::construct_type(
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
		),
		details::construct_type(
			t_cstring,
			(void*)(details::t_construct<SGL::cstring>)([](SGL::cstring* v) {
				v->data = nullptr;
				v->size = 0;
			}),
			(void*)(details::t_destruct<SGL::cstring>)([](SGL::cstring* v) {
				if(v->data) delete v->data;
			}),
			(void*)(details::t_copy<SGL::cstring>)([](SGL::cstring* v, SGL::cstring* from) {
				v->data = new char[from->size+1];
				v->size = from->size;
				memcpy(v->data, from->data, from->size+1);
			})
		),
		details::construct_type(t_char)
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
	static const std::unordered_map<std::string, const function> buildin_functions = {
		{"abs", {{
			{(void*)(int32_t(*)(int32_t))(std::abs), t_int32, {t_int32}},
			{(void*)(int64_t(*)(int64_t))(std::abs), t_int64, {t_int64}},
			{(void*)(float(*)(float))(std::abs), t_float32, {t_float32}},
			{(void*)(double(*)(double))(std::abs), t_float64, {t_float64}}
		}}},
 
		{"fmod", {{
			{(void*)(float(*)(float, float))(std::fmod), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(std::fmod), t_float64, {t_float64, t_float64}}
		}}},

		{"remainder", {{
			{(void*)(float(*)(float, float))(std::remainder), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(std::remainder), t_float64, {t_float64, t_float64}}
		}}},

		{"max", {{
			{(void*)(int8_t (*)(int8_t,    int8_t  ))(details::m_max), t_int8 , {t_int8 , t_int8 }},
			{(void*)(int16_t(*)(int16_t,   int16_t ))(details::m_max), t_int16, {t_int16, t_int16}},
			{(void*)(int32_t(*)(int32_t,   int32_t ))(details::m_max), t_int32, {t_int32, t_int32}},
			{(void*)(int64_t(*)(int64_t,   int64_t ))(details::m_max), t_int64, {t_int64, t_int64}},
			{(void*)(uint8_t (*)(uint8_t , uint8_t ))(details::m_max), t_uint8 , {t_uint8 , t_uint8 }},
			{(void*)(uint16_t(*)(uint16_t, uint16_t))(details::m_max), t_uint16, {t_uint16, t_uint16}},
			{(void*)(uint32_t(*)(uint32_t, uint32_t))(details::m_max), t_uint32, {t_uint32, t_uint32}},
			{(void*)(uint64_t(*)(uint64_t, uint64_t))(details::m_max), t_uint64, {t_uint64, t_uint64}},
			{(void*)(float(*)(float, float))(details::m_max), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(details::m_max), t_float64, {t_float64, t_float64}}
		}}},
		
		{"min",{{
			{(void*)(int8_t (*)(int8_t,    int8_t  ))(details::m_min), t_int8 , {t_int8 , t_int8 }},
			{(void*)(int16_t(*)(int16_t,   int16_t ))(details::m_min), t_int16, {t_int16, t_int16}},
			{(void*)(int32_t(*)(int32_t,   int32_t ))(details::m_min), t_int32, {t_int32, t_int32}},
			{(void*)(int64_t(*)(int64_t,   int64_t ))(details::m_min), t_int64, {t_int64, t_int64}},
			{(void*)(uint8_t (*)(uint8_t,  uint8_t ))(details::m_min), t_uint8 , {t_uint8 , t_uint8 }},
			{(void*)(uint16_t(*)(uint16_t, uint16_t))(details::m_min), t_uint16, {t_uint16, t_uint16}},
			{(void*)(uint32_t(*)(uint32_t, uint32_t))(details::m_min), t_uint32, {t_uint32, t_uint32}},
			{(void*)(uint64_t(*)(uint64_t, uint64_t))(details::m_min), t_uint64, {t_uint64, t_uint64}},
			{(void*)(float(*)(float, float))(details::m_min), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(details::m_min), t_float64, {t_float64, t_float64}}
		}}},

		{"clamp",{{
			{(void*)(int8_t  (*)(int8_t,   int8_t,   int8_t  ))(details::m_clamp), t_int8 , {t_int8 , t_int8 , t_int8 }},
			{(void*)(int16_t (*)(int16_t,  int16_t,  int16_t ))(details::m_clamp), t_int16, {t_int16, t_int16, t_int16}},
			{(void*)(int32_t (*)(int32_t,  int32_t,  int32_t ))(details::m_clamp), t_int32, {t_int32, t_int32, t_int32}},
			{(void*)(int64_t (*)(int64_t,  int64_t,  int64_t ))(details::m_clamp), t_int64, {t_int64, t_int64, t_int64}},
			{(void*)(uint8_t (*)(uint8_t,  uint8_t,  uint8_t ))(details::m_clamp), t_uint8 , {t_uint8 , t_uint8 , t_uint8 }},
			{(void*)(uint16_t(*)(uint16_t, uint16_t, uint16_t))(details::m_clamp), t_uint16, {t_uint16, t_uint16, t_uint16}},
			{(void*)(uint32_t(*)(uint32_t, uint32_t, uint32_t))(details::m_clamp), t_uint32, {t_uint32, t_uint32, t_uint32}},
			{(void*)(uint64_t(*)(uint64_t, uint64_t, uint64_t))(details::m_clamp), t_uint64, {t_uint64, t_uint64, t_uint64}},
			{(void*)(float(*)(float, float, float))(details::m_clamp), t_float32, {t_float32, t_float32, t_float32}},
			{(void*)(double(*)(double, double, double))(details::m_clamp), t_float64, {t_float64, t_float64, t_float64}}
		}}},
		
		{"nan", {{
			{(void*)(double(*)())(std::nan), t_float64, {}}
		}}},
		{"nanf", {{
			{(void*)(float(*)())(std::nan), t_float32, {}},
		}}},

		#define SGL_DECLARE_FLOAT_FUNC_1(name)\
		{#name, {{\
			{(void*)(float(*)(float))(std::name), t_float32, {t_float32}},\
			{(void*)(double(*)(double))(std::name), t_float64, {t_float64}}\
		}}},

		SGL_DECLARE_FLOAT_FUNC_1(exp)
		SGL_DECLARE_FLOAT_FUNC_1(exp2)

		SGL_DECLARE_FLOAT_FUNC_1(log)
		SGL_DECLARE_FLOAT_FUNC_1(log2)
		SGL_DECLARE_FLOAT_FUNC_1(log10)
	
		{"pow", {{
			{(void*)(float(*)(float, float))(std::pow), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(std::pow), t_float64, {t_float64, t_float64}}
		}}},
		SGL_DECLARE_FLOAT_FUNC_1(sqrt)
		SGL_DECLARE_FLOAT_FUNC_1(cbrt)

		{"hypot", {{
			{(void*)(float(*)(float, float))(std::hypot), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(std::hypot), t_float64, {t_float64, t_float64}},
			{(void*)(float(*)(float, float, float))(std::hypot), t_float32, {t_float32, t_float32, t_float32}},
			{(void*)(double(*)(double, double, double))(std::hypot), t_float64, {t_float64, t_float64, t_float64}}
		}}},


		SGL_DECLARE_FLOAT_FUNC_1(sin)
		SGL_DECLARE_FLOAT_FUNC_1(cos)
		SGL_DECLARE_FLOAT_FUNC_1(tan)
		SGL_DECLARE_FLOAT_FUNC_1(asin)
		SGL_DECLARE_FLOAT_FUNC_1(acos)
		SGL_DECLARE_FLOAT_FUNC_1(atan)

		{"atan2", {{
			{(void*)(float(*)(float, float))(std::atan2), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(std::atan2), t_float64, {t_float64, t_float64}}
		}}},
		
		SGL_DECLARE_FLOAT_FUNC_1(sinh)
		SGL_DECLARE_FLOAT_FUNC_1(cosh)
		SGL_DECLARE_FLOAT_FUNC_1(tanh)
		SGL_DECLARE_FLOAT_FUNC_1(asinh)
		SGL_DECLARE_FLOAT_FUNC_1(acosh)
		SGL_DECLARE_FLOAT_FUNC_1(acosh)
		SGL_DECLARE_FLOAT_FUNC_1(atanh)

		SGL_DECLARE_FLOAT_FUNC_1(tgamma)
		SGL_DECLARE_FLOAT_FUNC_1(lgamma)

		SGL_DECLARE_FLOAT_FUNC_1(ceil)
		SGL_DECLARE_FLOAT_FUNC_1(floor)
		SGL_DECLARE_FLOAT_FUNC_1(round)
		SGL_DECLARE_FLOAT_FUNC_1(nearbyint)
		SGL_DECLARE_FLOAT_FUNC_1(rint)

		{"copysign", {{
			{(void*)(float(*)(float, float))(std::copysign), t_float32, {t_float32, t_float32}},
			{(void*)(double(*)(double, double))(std::copysign), t_float64, {t_float64, t_float64}}
		}}},

		
		{"isfinite", {{
			{(void*)(bool(*)(float))(std::isfinite), t_bool, {t_float32}},
			{(void*)(bool(*)(double))(std::isfinite), t_bool, {t_float64}}
		}}},
		{"isinf", {{
			{(void*)(bool(*)(float))(std::isinf), t_bool, {t_float32}},
			{(void*)(bool(*)(double))(std::isinf), t_bool, {t_float64}}
		}}},
		{"isnan", {{
			{(void*)(bool(*)(float))(std::isnan), t_bool, {t_float32}},
			{(void*)(bool(*)(double))(std::isnan), t_bool, {t_float64}}
		}}},
		{"isnormal", {{
			{(void*)(bool(*)(float))(std::isnormal), t_bool, {t_float32}},
			{(void*)(bool(*)(double))(std::isnormal), t_bool, {t_float64}}
		}}},

		{"gcd", {{
			{(void*)(int8_t (*)(int8_t,  int8_t ))(std::gcd<int8_t , int8_t >), t_int8 , {t_int8 , t_int8 }},
			{(void*)(int16_t(*)(int16_t, int16_t))(std::gcd<int16_t, int16_t>), t_int16, {t_int16, t_int16}},
			{(void*)(int32_t(*)(int32_t, int32_t))(std::gcd<int32_t, int32_t>), t_int32, {t_int32, t_int32}},
			{(void*)(int64_t(*)(int64_t, int64_t))(std::gcd<int64_t, int64_t>), t_int64, {t_int64, t_int64}},
			{(void*)(uint8_t (*)(uint8_t,  uint8_t ))(std::gcd<uint8_t , uint8_t >), t_uint8 , {t_uint8 , t_uint8 }},
			{(void*)(uint16_t(*)(uint16_t, uint16_t))(std::gcd<uint16_t, uint16_t>), t_uint16, {t_uint16, t_uint16}},
			{(void*)(uint32_t(*)(uint32_t, uint32_t))(std::gcd<uint32_t, uint32_t>), t_uint32, {t_uint32, t_uint32}},
			{(void*)(uint64_t(*)(uint64_t, uint64_t))(std::gcd<uint64_t, uint64_t>), t_uint64, {t_uint64, t_uint64}},
		}}},
		{"lcm", {{
			{(void*)(int8_t (*)(int8_t,  int8_t ))(std::lcm<int8_t , int8_t >), t_int8 , {t_int8 , t_int8 }},
			{(void*)(int16_t(*)(int16_t, int16_t))(std::lcm<int16_t, int16_t>), t_int16, {t_int16, t_int16}},
			{(void*)(int32_t(*)(int32_t, int32_t))(std::lcm<int32_t, int32_t>), t_int32, {t_int32, t_int32}},
			{(void*)(int64_t(*)(int64_t, int64_t))(std::lcm<int64_t, int64_t>), t_int64, {t_int64, t_int64}},
			{(void*)(uint8_t (*)(uint8_t,  uint8_t ))(std::lcm<uint8_t , uint8_t >), t_uint8 , {t_uint8 , t_uint8 }},
			{(void*)(uint16_t(*)(uint16_t, uint16_t))(std::lcm<uint16_t, uint16_t>), t_uint16, {t_uint16, t_uint16}},
			{(void*)(uint32_t(*)(uint32_t, uint32_t))(std::lcm<uint32_t, uint32_t>), t_uint32, {t_uint32, t_uint32}},
			{(void*)(uint64_t(*)(uint64_t, uint64_t))(std::lcm<uint64_t, uint64_t>), t_uint64, {t_uint64, t_uint64}},
		}}},
	};

	state::~state() {
		while (!m_results.empty()) (*m_results.begin())->~parse_result();
		for (auto& [name, var] : global_constants) details::destruct_val(var.m_type, var.array_size, var.data);
	}
	parse_result::~parse_result() {
		if (m_state) m_state->m_results.erase(this);
		for (auto& [name, var] : local_variables) details::destruct_val(var.m_type, var.array_size, var.data);
	}

	static error_callback_t sgl_error_callback__ = details::defualt_sgl_error_function;
	static bool sgl_pass_iternal_cxx_exceprion_in_error_callback__ = false;

    void set_error_callback(error_callback_t f) {
		sgl_error_callback__ = f;
	}
    void error(const std::string& description) {
		if(sgl_error_callback__) sgl_error_callback__(description);
	}

	namespace details {
		void construct_val(const type* t, size_t arr_size, void* v) {
			if (t->m_construct) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++)
				reinterpret_cast<t_construct<void>>(t->m_construct)(static_cast<char*>(v) + i * t->size);
			else {
				if (t->base_type == t_custom) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++) for (auto m : t->members)
					construct_val(m.m_type, m.array_size, (static_cast<char*>(v) + i * t->size + m.offset));
				else memset(v, 0, (arr_size ? arr_size : 1) * t->size);
			}
		}
		void destruct_val(const type* t, size_t arr_size, void* v) {
			if (t->m_destruct) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++)
				reinterpret_cast<t_destruct<void>>(t->m_destruct)(static_cast<char*>(v));
			else {
				if (t->base_type == t_custom) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++) for (auto m : t->members)
					destruct_val(m.m_type, m.array_size, (static_cast<char*>(v) + i * t->size + m.offset));
			}
		}
		void copy_val(const type* t, size_t arr_size, void* v, void* from) {
			if (t->m_copy) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++)
				reinterpret_cast<t_copy<void>>(t->m_copy)(static_cast<char*>(v) + i * t->size, static_cast<char*>(from) + i * t->size);
			else {
				if (t->base_type == t_custom) for (size_t i = 0, s = arr_size ? arr_size : 1; i < s; i++) for (auto m : t->members)
					copy_val(m.m_type, m.array_size, (static_cast<char*>(v) + i * t->size + m.offset), (static_cast<char*>(from) + i * t->size + m.offset));
				else memcpy(v, from, (arr_size ? arr_size : 1) * t->size);
			}
		}


		type& register_struct(state& s, const std::string& name, size_t size, std::vector<type::member>&& members, void* v1, void* v2, void* v3) {
			SGL_ASSERT(([](const std::string& name)->bool {
				if (name.empty() || (name.front() != '_' && !std::isalpha(static_cast<unsigned char>(name.front())))) return false;
				for (auto ch : name) if (ch != '_' && !std::isalnum(static_cast<unsigned char>(ch))) return false;
				return true;
					})(name));//correct type name
			auto& t = s.global_types[name];
			t.base_type = t_custom;
			t.members = members;
			//std::sort(t.members.begin(), t.members.end(), [](const type::member& a, const type::member& b) {
			//	return a.offset < b.offset;
			//});
			for (auto& m : t.members)
				if (m.type == t_custom) {
					m.m_type = &s.global_types[m.custom_type_name];
					SGL_ASSERT(m.m_type);
				}
				else m.m_type = &buildin_types_v[m.type];
			auto& b = t.members.back();
			size_t sz = b.offset + (b.array_size ? b.array_size : 1) * (b.type == t_custom ? b.m_type->size : type_size[b.type]);
			SGL_ASSERT(size && sz <= size);

			t.size = size;

			t.m_construct = v1;
			t.m_destruct = v2;
			t.m_copy = v3;
			return t;
		}
		value* get_local_value(parse_result& p, const std::string& name) {
			auto f = p.local_variables.find(name);
			if (f == p.local_variables.end()) return nullptr;
			return &(f->second);
		}

        void set_global_variable(state& s, const std::string& variable_name, primitive_type t, void* data, size_t array_size) {
			auto& v = s.global_constants[variable_name];
			v.array_size = array_size;
			v.m_type = &buildin_types_v[t];
			v.data = new char[(array_size ? array_size : 1) * v.m_type->size];
			details::copy_val(v.m_type, array_size, v.data, data);
		}
        void set_global_variable(state& s, const std::string& variable_name, const std::string& type_name, void* data, size_t array_size) {
			auto& v = s.global_constants[variable_name];
			v.array_size = array_size;
			v.m_type = &s.global_types.find(type_name)->second;
			v.data = new char[(array_size ? array_size : 1) * v.m_type->size];
			details::copy_val(v.m_type, array_size, v.data, data);
		}

		void pass_iternal_cxx_exceprion_in_error_callback(bool pass) {
			sgl_pass_iternal_cxx_exceprion_in_error_callback__ = pass;
		}
	}
		
    bool contains(parse_result& p, const std::string& name) {
		return p.local_variables.find(name) != p.local_variables.end();
	}
    bool is_array(parse_result& p, const std::string& name) {
		auto f = p.local_variables.find(name);
		if (f == p.local_variables.end()) return false;
		return f->second.array_size != 0;
	}
    bool is_primitive_type(parse_result& p, const std::string& name) {
		auto f = p.local_variables.find(name);
		if (f == p.local_variables.end()) return false;
		return f->second.m_type->base_type != t_custom;
	}
    bool is_custom_type(parse_result& p, const std::string& name) {
		return !is_primitive_type(p, name);
	}
    bool is_same_primitive_type(parse_result& p, const std::string& name, primitive_type t) {
		auto f = p.local_variables.find(name);
		if (f == p.local_variables.end()) return false;
		return f->second.m_type->base_type == t;
	}
    bool is_same_custom_type(parse_result& p, const std::string& name, const std::string& type_name) {
		auto f = p.local_variables.find(name);
		if (f == p.local_variables.end() || f->second.m_type->base_type != t_custom || !p.m_state) return false;
		auto f2 = p.m_state->global_types.find(type_name);
		if (f2 == p.m_state->global_types.end()) return false;
		return &f2->second == f->second.m_type;
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

	enum token_type : uint8_t {
		none_v = 0,

		value_v,//int, float, ... 

		punct_v,//{} () [] . ,
		operator_v,//binary + - * / % ^ | & << >> && || == != > < <= >=, unary + - ! ~
		function_v,
	};
	struct m_token {
		m_token(token_type t, int p, primitive_type vt = t_void) : type(t), prior(p), value_type(vt) {
			if (type == value_v && value_type == t_string) new (&str_v) std::string;
		}
		m_token(const m_token& v) : type(v.type), prior(v.prior), value_type(v.value_type) {
			if (type == value_v && value_type == t_string) new (&str_v) std::string(v.str_v);
			else memcpy(this, &v, sizeof(m_token));
		}
		m_token& operator=(const m_token& v) {
			if (type == value_v && value_type == t_string) str_v.~basic_string();
			type = v.type;
			prior = v.prior;
			value_type = v.value_type;
			if (type == value_v && value_type == t_string) new (&str_v) std::string(v.str_v);
			else memcpy(this, &v, sizeof(m_token));
			return *this;
		}
		~m_token() { 
			if (type == value_v && value_type == t_string) str_v.~basic_string(); 
		}
		token_type type = none_v;
		primitive_type value_type = t_void;//value type or cast_to type
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
			value object_v;
			const function* function_v;
		};
	};

	static void cast_to_type(m_token& val, primitive_type t) {
		if(val.type != value_v) SGL_ERROR("SGL: invalid type cast");
		if(val.value_type == t) return;
		if(val.value_type == t_custom) {
			if(val.object_v.array_size != 0) SGL_ERROR("SGL: cannot cast array to scalar value");
			switch (val.object_v.m_type->base_type) {
			#define SGL_CAST_TO_T(name, value, var)\
			case name: {\
				m_token t{value_v, val.prior, name};\
				t.var = *static_cast<value*>(val.object_v.data);\
				val = t;\
			} break;
			SGL_CAST_TO_T(t_int8,  int8_t,  int_v.i8)
			SGL_CAST_TO_T(t_int16, int16_t, int_v.i16)
			SGL_CAST_TO_T(t_int32, int32_t, int_v.i32)
			SGL_CAST_TO_T(t_int64, int64_t, int_v.i64)
			SGL_CAST_TO_T(t_uint8,  uint8_t,  int_v.ui8)
			SGL_CAST_TO_T(t_uint16, uint16_t, int_v.ui16)
			SGL_CAST_TO_T(t_uint32, uint32_t, int_v.ui32)
			SGL_CAST_TO_T(t_uint64, uint64_t, int_v.ui64)
			SGL_CAST_TO_T(t_float32, float, float_v)
			SGL_CAST_TO_T(t_float64, double, float_v)
			SGL_CAST_TO_T(t_bool, bool, bool_v)
			SGL_CAST_TO_T(t_char, char, char_v)
			SGL_CAST_TO_T(t_string, std::string, str_v)
			#undef SGL_CAST_TO_T
			default: SGL_ERROR("SGL: invalid type cast"); break;
			}
			if(val.value_type == t) return;
		}
		switch (t) {	
		#define SGL_CAST_TO_INT_T(int_t, intv)\
		case t_##int_t: {\
			if(val.value_type == t_float64) {\
				int_t##_t i = (int_t##_t)val.float_v;\
				val = m_token(value_v, val.prior, t_##int_t);\
				val.int_v.intv = i;\
				return;\
			}\
			else if(val.value_type == t_char) {\
				int_t##_t i = (int_t##_t)val.char_v;\
				val = m_token(value_v, val.prior, t_##int_t);\
				val.int_v.intv = i;\
				return;\
			}\
			else if(val.value_type == t_bool) {\
				int_t##_t i = (int_t##_t)val.bool_v;\
				val = m_token(value_v, val.prior, t_##int_t);\
				val.int_v.intv = i;\
				return;\
			}\
			else if(t_int8 <= val.value_type && val.value_type <= t_uint64) {\
				int_t##_t i = (int_t##_t)0;\
				switch (val.value_type) {\
				case t_int8:   i = (int_t##_t)val.int_v.i8;   break;\
				case t_int16:  i = (int_t##_t)val.int_v.i16;  break;\
				case t_int32:  i = (int_t##_t)val.int_v.i32;  break;\
				case t_int64:  i = (int_t##_t)val.int_v.i64;  break;\
				case t_uint8:  i = (int_t##_t)val.int_v.ui8;  break;\
				case t_uint16: i = (int_t##_t)val.int_v.ui16; break;\
				case t_uint32: i = (int_t##_t)val.int_v.ui32; break;\
				case t_uint64: i = (int_t##_t)val.int_v.ui64; break;\
				default: break;\
				}\
				val = m_token(value_v, val.prior, t_##int_t);\
				val.int_v.intv = i;\
				return;\
			}\
		} break;
		SGL_CAST_TO_INT_T(int8,  i8);
		SGL_CAST_TO_INT_T(int16, i16);
		SGL_CAST_TO_INT_T(int32, i32);
		SGL_CAST_TO_INT_T(int64, i64);
		SGL_CAST_TO_INT_T(uint8,  ui8);
		SGL_CAST_TO_INT_T(uint16, ui16);
		SGL_CAST_TO_INT_T(uint32, ui32);
		SGL_CAST_TO_INT_T(uint64, ui64);
		#undef SGL_CAST_TO_INT_T
		
		case t_float64: {
			if(t_int8 <= val.value_type && val.value_type <= t_uint64) {
				double f = 0;
				switch (val.value_type) {
				case t_int8:   f = (double)val.int_v.i8;   break;
				case t_int16:  f = (double)val.int_v.i16;  break;
				case t_int32:  f = (double)val.int_v.i32;  break;
				case t_int64:  f = (double)val.int_v.i64;  break;
				case t_uint8:  f = (double)val.int_v.ui8;  break;
				case t_uint16: f = (double)val.int_v.ui16; break;
				case t_uint32: f = (double)val.int_v.ui32; break;
				case t_uint64: f = (double)val.int_v.ui64; break;
				default: break;
				}
				val = m_token(value_v, val.prior, t_float64);
				val.float_v = f;
				return;			
			}
			else if(val.value_type == t_char) {
				double i = (double)val.char_v;
				val = m_token(value_v, val.prior, t_float64);
				val.float_v = i;
				return;	
			}
			else if(val.value_type == t_bool) {
				double i = (double)val.bool_v;
				val = m_token(value_v, val.prior, t_float64);
				val.float_v = i;
				return;	
			}
		} break;
		case t_string: {
			if(val.value_type == t_char) {
				char ch = val.char_v;
				val = m_token(value_v, val.prior, t_string);
				val.str_v += ch;
				return;
			}
		} break;
		case t_char: {
			if(t_int8 <= val.value_type && val.value_type <= t_uint64) {
				char ch = 0;
				switch (val.value_type) {
				case t_int8:   ch = (char)val.int_v.i8;   break;
				case t_int16:  ch = (char)val.int_v.i16;  break;
				case t_int32:  ch = (char)val.int_v.i32;  break;
				case t_int64:  ch = (char)val.int_v.i64;  break;
				case t_uint8:  ch = (char)val.int_v.ui8;  break;
				case t_uint16: ch = (char)val.int_v.ui16; break;
				case t_uint32: ch = (char)val.int_v.ui32; break;
				case t_uint64: ch = (char)val.int_v.ui64; break;
				default: break;
				}
				val = m_token(value_v, val.prior, t_char);
				val.char_v = ch;
				return;
			} if(val.value_type == t_float64) {
				char ch = (char)val.float_v;
				val = m_token(value_v, val.prior, t_char);
				val.char_v = ch;
				return;	
			} if(val.value_type == t_float64) {
				char ch = (char)val.bool_v;
				val = m_token(value_v, val.prior, t_char);
				val.char_v = ch;
				return;	
			}
		} break;	
		default: break;
		}
		SGL_ERROR("SGL: invalid type cast");
	}
	template<size_t depth, size_t max_depth, typename... Args>
	static void call_function_impl(const function::function_overload* func, m_token* ans, m_token** args_t, Args... args) {
		if constexpr(depth < max_depth) {
			switch(args_t[depth]->value_type) {
			case t_int8 : 	call_function_impl<depth+1, max_depth, Args..., int8_t >(func, ans, args_t, args..., args_t[depth]->int_v.i8 ); break;
			case t_int16: 	call_function_impl<depth+1, max_depth, Args..., int16_t>(func, ans, args_t, args..., args_t[depth]->int_v.i16); break;
			case t_int32: 	call_function_impl<depth+1, max_depth, Args..., int32_t>(func, ans, args_t, args..., args_t[depth]->int_v.i32); break;
			case t_int64: 	call_function_impl<depth+1, max_depth, Args..., int64_t>(func, ans, args_t, args..., args_t[depth]->int_v.i64); break;
			case t_uint8 : 	call_function_impl<depth+1, max_depth, Args..., uint8_t >(func, ans, args_t, args..., args_t[depth]->int_v.ui8 ); break;
			case t_uint16: 	call_function_impl<depth+1, max_depth, Args..., uint16_t>(func, ans, args_t, args..., args_t[depth]->int_v.ui16); break;
			case t_uint32: 	call_function_impl<depth+1, max_depth, Args..., uint32_t>(func, ans, args_t, args..., args_t[depth]->int_v.ui32); break;
			case t_uint64: 	call_function_impl<depth+1, max_depth, Args..., uint64_t>(func, ans, args_t, args..., args_t[depth]->int_v.ui64); break;

			case t_float32: call_function_impl<depth+1, max_depth, Args..., float>(func, ans, args_t, args..., (double)args_t[depth]->float_v); break;
			case t_float64: call_function_impl<depth+1, max_depth, Args..., double>(func, ans, args_t, args..., args_t[depth]->float_v); break;

			case t_bool: 	call_function_impl<depth+1, max_depth, Args..., bool>(func, ans, args_t, args..., (double)args_t[depth]->bool_v); break;
			case t_string: 	call_function_impl<depth+1, max_depth, Args..., std::string>(func, ans, args_t, args..., args_t[depth]->str_v); break;
			case t_char: 	call_function_impl<depth+1, max_depth, Args..., char>(func, ans, args_t, args..., args_t[depth]->char_v); break;

			default: SGL_ERROR("SGL: invalid function args type");
			}
		} else {//do function call
			switch(func->ret_type) {
			case t_int8 : {
				*ans = m_token(value_v, ans->prior, t_int8);
				ans->int_v.i8 = reinterpret_cast<int8_t(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_int16: {
				*ans = m_token(value_v, ans->prior, t_int16);
				ans->int_v.i16 = reinterpret_cast<int16_t(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_int32: {
				*ans = m_token(value_v, ans->prior, t_int32);
				ans->int_v.i32 = reinterpret_cast<int32_t(*)(Args...)>(func->ptr)(args...);
			} break;	
			case t_int64: {
				*ans = m_token(value_v, ans->prior, t_int64);
				ans->int_v.i64 = reinterpret_cast<int64_t(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_uint8 : {
				*ans = m_token(value_v, ans->prior, t_uint8);
				ans->int_v.ui8 = reinterpret_cast<uint8_t(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_uint16: {
				*ans = m_token(value_v, ans->prior, t_uint16);
				ans->int_v.ui16 = reinterpret_cast<uint16_t(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_uint32: {
				*ans = m_token(value_v, ans->prior, t_uint32);
				ans->int_v.ui32 = reinterpret_cast<uint32_t(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_uint64: {
				*ans = m_token(value_v, ans->prior, t_uint64);
				ans->int_v.ui64 = reinterpret_cast<uint64_t(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_float32: {
				*ans = m_token(value_v, ans->prior, t_float64);
				ans->float_v = (double)reinterpret_cast<float(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_float64: {
				*ans = m_token(value_v, ans->prior, t_float64);
				ans->float_v = reinterpret_cast<double(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_bool: {
				*ans = m_token(value_v, ans->prior, t_bool);
				ans->bool_v = reinterpret_cast<bool(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_string: {
				*ans = m_token(value_v, ans->prior, t_string);
				ans->str_v = reinterpret_cast<std::string(*)(Args...)>(func->ptr)(args...);
			} break;
			case t_cstring: {
				*ans = m_token(value_v, ans->prior, t_string);
				auto cstr = reinterpret_cast<cstring(*)(Args...)>(func->ptr)(args...);
				ans->str_v = std::string(cstr.data, cstr.data+cstr.size);
			} break;
			case t_char: {
				*ans = m_token(value_v, ans->prior, t_char);
				ans->char_v = reinterpret_cast<char(*)(Args...)>(func->ptr)(args...);
			} break;
			default: SGL_ERROR("SGL: invalid function args type");
			}
		}
	}

	static void function_call(const function* f, m_token* ans, m_token** args, size_t args_count) {
		const function::function_overload* valid_overload = nullptr;
		{
			size_t min_diff = (size_t)1e9, count = 0;
			for(size_t i = 0, s = f->m_overloads.size(); i < s; i++)
				if(f->m_overloads[i].args_types.size() == args_count) {
					size_t diff = 0;
					for(size_t j = 0; j < args_count; j++)
						diff += args[j]->value_type == f->m_overloads[i].args_types[j];
					if(diff == min_diff) count++;
					else if(diff < min_diff) {
						valid_overload = &f->m_overloads[i];
						count = 1;
					}
				}
			if(!valid_overload) SGL_ERROR("SGL: cannot choose function overload. invalid arguments count");
			if(count > 1) SGL_ERROR("SGL: cannot choose function overload");
			//TODO if min_diff != 0 add warinig about typecast?
		}
		for(size_t i = 0; i < args_count; i++)
			cast_to_type(*args[i], valid_overload->args_types[i]);
		switch (valid_overload->args_types.size()) {
		#define SGL_CALL_FUNCTION_WITH_ARGS_COUNT(n)\
		case n: call_function_impl<0, n>(valid_overload, ans, args); break;
		SGL_CALL_FUNCTION_WITH_ARGS_COUNT(0);
		SGL_CALL_FUNCTION_WITH_ARGS_COUNT(1);
		SGL_CALL_FUNCTION_WITH_ARGS_COUNT(2);
		SGL_CALL_FUNCTION_WITH_ARGS_COUNT(3);//more than 3 args compile to long and increase binary size
		//SGL_CALL_FUNCTION_WITH_ARGS_COUNT(4);
		#undef SGL_CALL_FUNCTION_WITH_ARGS_COUNT
		default: SGL_ERROR("SGL: invalid arguments count"); break;
		}
	}

	static void unary_operator_plus(m_token& value) {
		if(!(value.type == value_v && t_int8 <= value.value_type && value.value_type <= t_float64)) SGL_ERROR("SGL: type must be integer or boolean for unary + operator");
	}
	static void unary_operator_minus(m_token& value) {
		if(value.type != value_v) SGL_ERROR("SGL: invalid type for unary - operator");
		else switch (value.value_type) {
		case t_int8:   value.int_v.i8 =  -value.int_v.i8;  break;
		case t_int16:  value.int_v.i16 = -value.int_v.i16; break;
		case t_int32:  value.int_v.i32 = -value.int_v.i32; break;
		case t_int64:  value.int_v.i64 = -value.int_v.i64; break;
		case t_uint8:  value.value_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui8);  break;
		case t_uint16: value.value_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui16); break;
		case t_uint32: value.value_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui32); break;
		case t_uint64: value.value_type = t_int64; value.int_v.i64 = -int64_t(value.int_v.ui64); break;
		case t_float32: value.float_v = -value.float_v;
		case t_float64: value.float_v = -value.float_v;
		default: SGL_ERROR("SGL: invalid type for unary - operator"); break;
		}
		
	}
	static void unary_operator_not(m_token& value) {
		if(value.type != value_v) SGL_ERROR("SGL: invalid type for unary ! operator");
		else switch (value.value_type) {
		case t_int8:   value.bool_v = !value.int_v.i8;   break;
		case t_int16:  value.bool_v = !value.int_v.i16;  break;
		case t_int32:  value.bool_v = !value.int_v.i32;  break;
		case t_int64:  value.bool_v = !value.int_v.i64;  break;
		case t_uint8:  value.bool_v = !value.int_v.ui8;  break;
		case t_uint16: value.bool_v = !value.int_v.ui16; break;
		case t_uint32: value.bool_v = !value.int_v.ui32; break;
		case t_uint64: value.bool_v = !value.int_v.ui64; break;
		case t_bool:   value.bool_v = !value.bool_v; 	 break;
		default: SGL_ERROR("SGL: type must be integer or boolean for unary ! operator"); break;
		}

	}
	static void unary_operator_bitwise_not(m_token& value) {
		if(value.type != value_v) SGL_ERROR("SGL: invalid type for unary ~ operator");
		else switch (value.value_type) {
		case t_int8:   value.int_v.i8   = ~value.int_v.i8;   break;
		case t_int16:  value.int_v.i16  = ~value.int_v.i16;  break;
		case t_int32:  value.int_v.i32  = ~value.int_v.i32;  break;
		case t_int64:  value.int_v.i64  = ~value.int_v.i64;  break;
		case t_uint8:  value.int_v.ui8  = ~value.int_v.ui8;  break;
		case t_uint16: value.int_v.ui16 = ~value.int_v.ui16; break;
		case t_uint32: value.int_v.ui32 = ~value.int_v.ui32; break;
		case t_uint64: value.int_v.ui64 = ~value.int_v.ui64; break;	
		default: SGL_ERROR("SGL: type must be integer for unary ~ operator"); break;
		}
	}

	static constexpr primitive_type result_of_value(primitive_type a, primitive_type b) {
		if(a == b) return a;
		if(t_int8 <= a && a <= t_uint64) {
			if(t_int8 <= b && b <= t_uint64) {
				if(t_uint8 <= a && a <= t_uint64) a = primitive_type(a - 4);
				if(t_uint8 <= b && b <= t_uint64) b = primitive_type(b - 4);
				return std::max(a, b);
			}
			else if(t_float32 == b || b == t_float64) return t_float64;	
		} else if(t_float32 == a || a == t_float64) {
			if(t_float32 == b || b == t_float64 || t_int8 <= b && b <= t_uint64) return t_float64;
		} else if((a == t_string || a == t_char) && (b == t_string || b == t_char)) return t_string;//a != b
		SGL_ERROR("SGL: invalid type for binary operator");
		return t_void;
	}

	using binary_operator_template_func_t = void(*)(m_token& a, m_token& b, primitive_type t);
	template<binary_operator_template_func_t func> 
	static void binary_operator_template(m_token& a, m_token& b) {
		primitive_type at = a.value_type == t_custom ? a.object_v.m_type->base_type : a.value_type;
		primitive_type bt = b.value_type == t_custom ? b.object_v.m_type->base_type : b.value_type;
		auto result_type = result_of_value(at, bt);
		cast_to_type(a, result_type);
		cast_to_type(b, result_type);
		func(a, b, result_type);
	}
	#define binary_operator_def_i0(func)
	#define binary_operator_def_i1(func)\
		case t_int8:   a.int_v.i8   func b.int_v.i8;   return;\
		case t_int16:  a.int_v.i16  func b.int_v.i16;  return;\
		case t_int32:  a.int_v.i32  func b.int_v.i32;  return;\
		case t_int64:  a.int_v.i64  func b.int_v.i64;  return;\
		case t_uint8:  a.int_v.ui8  func b.int_v.ui8;  return;\
		case t_uint16: a.int_v.ui16 func b.int_v.ui16; return;\
		case t_uint32: a.int_v.ui32 func b.int_v.ui32; return;\
		case t_uint64: a.int_v.ui64 func b.int_v.ui64; return;
			
	#define binary_operator_def_f0(func)
	#define binary_operator_def_f1(func)\
		case t_float64: a.float_v func b.float_v; return;
	#define binary_operator_def_s0(func)	
	#define binary_operator_def_s1(func)\
		case t_string: a.str_v func b.str_v; return;
	#define binary_operator_def_c0(func)	
	#define binary_operator_def_c1(func)\
		case t_char: a.char_v func b.char_v; return;
	#define binary_operator_def_b0(func)	
	#define binary_operator_def_b1(func)\
		case t_bool: a.bool_v func b.bool_v; return;

	#define binary_operator_def(name, func, en_int, en_float, en_string, en_char, en_bool)\
	static void binary_operator_##name(m_token& val, m_token& other) {\
		auto v = [](m_token& a, m_token& b, primitive_type t){\
			switch (t) {\
				binary_operator_def_i##en_int(func)\
				binary_operator_def_f##en_float(func)\
				binary_operator_def_s##en_string(func)\
				binary_operator_def_c##en_char(func)\
				binary_operator_def_b##en_bool(func)\
				default: break;\
			}\
			SGL_ERROR("SGL: invalid operation type");\
			return;\
		};\
		binary_operator_template<v>(val, other);\
	}
	//arithmetic + - * / % 
	binary_operator_def(sum, +=, 1, 1, 1, 0, 0);
	binary_operator_def(sub, -=, 1, 1, 0, 0, 0);
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
		auto v = [](m_token& a, m_token& b, primitive_type t){
			bool v = false;
			switch (t) {
			case t_int8:   v = a.int_v.i8   && b.int_v.i8;   break;
			case t_int16:  v = a.int_v.i16  && b.int_v.i16;  break;
			case t_int32:  v = a.int_v.i32  && b.int_v.i32;  break;
			case t_int64:  v = a.int_v.i64  && b.int_v.i64;  break;
			case t_uint8:  v = a.int_v.ui8  && b.int_v.ui8;  break;
			case t_uint16: v = a.int_v.ui16 && b.int_v.ui16; break;
			case t_uint32: v = a.int_v.ui32 && b.int_v.ui32; break;
			case t_uint64: v = a.int_v.ui64 && b.int_v.ui64; break;			
			case t_char:   v = a.char_v && b.char_v;   break;
			case t_bool:   v = a.bool_v && b.bool_v;   break;			
			default: break;
			}
			cast_to_type(a, t_bool);
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_or(m_token& val, m_token& other) {
		auto v = [](m_token& a, m_token& b, primitive_type t){
			bool v = false;
			switch (t) {
			case t_int8:   v = a.int_v.i8   || b.int_v.i8;   break;
			case t_int16:  v = a.int_v.i16  || b.int_v.i16;  break;
			case t_int32:  v = a.int_v.i32  || b.int_v.i32;  break;
			case t_int64:  v = a.int_v.i64  || b.int_v.i64;  break;
			case t_uint8:  v = a.int_v.ui8  || b.int_v.ui8;  break;
			case t_uint16: v = a.int_v.ui16 || b.int_v.ui16; break;
			case t_uint32: v = a.int_v.ui32 || b.int_v.ui32; break;
			case t_uint64: v = a.int_v.ui64 || b.int_v.ui64; break;
			case t_char:   v = a.char_v || b.char_v;   break;
			case t_bool:   v = a.bool_v || b.bool_v;   break;
			default: break;
			}
			cast_to_type(a, t_bool);
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_eqal(m_token& val, m_token& other) {	
		auto v = [](m_token& a, m_token& b, primitive_type t){
			bool v = false;
			switch (t) {
			case t_int8:   v = a.int_v.i8   == b.int_v.i8;   break;
			case t_int16:  v = a.int_v.i16  == b.int_v.i16;  break;
			case t_int32:  v = a.int_v.i32  == b.int_v.i32;  break;
			case t_int64:  v = a.int_v.i64  == b.int_v.i64;  break;
			case t_uint8:  v = a.int_v.ui8  == b.int_v.ui8;  break;
			case t_uint16: v = a.int_v.ui16 == b.int_v.ui16; break;
			case t_uint32: v = a.int_v.ui32 == b.int_v.ui32; break;
			case t_uint64: v = a.int_v.ui64 == b.int_v.ui64; break;			
			case t_float64:  v = a.float_v == b.float_v; break;
			case t_string: v = a.str_v == b.str_v;     break;
			case t_char:   v = a.char_v == b.char_v;   break;
			case t_bool:   v = a.bool_v == b.bool_v;   break;		
			default: break;
			}
			cast_to_type(a, t_bool);
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_not_eqal(m_token& val, m_token& other) {
		auto v = [](m_token& a, m_token& b, primitive_type t){
			bool v = false;
			switch (t) {
			case t_int8:   v = a.int_v.i8   != b.int_v.i8;   break;
			case t_int16:  v = a.int_v.i16  != b.int_v.i16;  break;
			case t_int32:  v = a.int_v.i32  != b.int_v.i32;  break;
			case t_int64:  v = a.int_v.i64  != b.int_v.i64;  break;
			case t_uint8:  v = a.int_v.ui8  != b.int_v.ui8;  break;
			case t_uint16: v = a.int_v.ui16 != b.int_v.ui16; break;
			case t_uint32: v = a.int_v.ui32 != b.int_v.ui32; break;
			case t_uint64: v = a.int_v.ui64 != b.int_v.ui64; break;
			case t_float64:  v = a.float_v != b.float_v; break;
			case t_string: v = a.str_v != b.str_v;     break;
			case t_char:   v = a.char_v != b.char_v;   break;
			case t_bool:   v = a.bool_v != b.bool_v;   break;
			default: break;
			}
			cast_to_type(a, t_bool);
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_greater(m_token& val, m_token& other) {	
		auto v = [](m_token& a, m_token& b, primitive_type t){
			bool v = false;
			switch (t) {
			case t_int8:   v = a.int_v.i8   > b.int_v.i8;   break;
			case t_int16:  v = a.int_v.i16  > b.int_v.i16;  break;
			case t_int32:  v = a.int_v.i32  > b.int_v.i32;  break;
			case t_int64:  v = a.int_v.i64  > b.int_v.i64;  break;
			case t_uint8:  v = a.int_v.ui8  > b.int_v.ui8;  break;
			case t_uint16: v = a.int_v.ui16 > b.int_v.ui16; break;
			case t_uint32: v = a.int_v.ui32 > b.int_v.ui32; break;
			case t_uint64: v = a.int_v.ui64 > b.int_v.ui64; break;
			case t_float64:  v = a.float_v > b.float_v; break;
			case t_string: v = a.str_v > b.str_v;     break;
			case t_char:   v = a.char_v > b.char_v;   break;
			case t_bool:   v = a.bool_v > b.bool_v;   break;
			default: break;
			}
			cast_to_type(a, t_bool);
			a.bool_v = v;
			return;
		};
		binary_operator_template<v>(val, other);
	}
	static void binary_operator_less(m_token& val, m_token& other) {
		auto v = [](m_token& a, m_token& b, primitive_type t){
			bool v = false;
			switch (t) {
			case t_int8:   v = a.int_v.i8   < b.int_v.i8;   break;
			case t_int16:  v = a.int_v.i16  < b.int_v.i16;  break;
			case t_int32:  v = a.int_v.i32  < b.int_v.i32;  break;
			case t_int64:  v = a.int_v.i64  < b.int_v.i64;  break;
			case t_uint8:  v = a.int_v.ui8  < b.int_v.ui8;  break;
			case t_uint16: v = a.int_v.ui16 < b.int_v.ui16; break;
			case t_uint32: v = a.int_v.ui32 < b.int_v.ui32; break;
			case t_uint64: v = a.int_v.ui64 < b.int_v.ui64; break;
			case t_float64:  v = a.float_v < b.float_v; break;
			case t_string: v = a.str_v < b.str_v;     break;
			case t_char:   v = a.char_v < b.char_v;   break;
			case t_bool:   v = a.bool_v < b.bool_v;   break;
			default: break;
			}
			cast_to_type(a, t_bool);
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
		size_t ops_count = 0;//TODO remove it?
		iter last_op = tokens.begin();
		iter eval_beg = tokens.begin();

		auto eval_expr = [&](size_t with_priority = 0){		
			if(eval_beg == tokens.end()) 
				eval_beg = tokens.begin();
			auto it = eval_beg, prev_it = it, next_it = it;
			next_it++;
			using ops_val_t = std::pair<m_token*, std::pair<int, iter>>;
			std::vector<ops_val_t> operators;
			for (size_t i = it == tokens.begin() ? 0 : 1; i < tokens.size() && it != tokens.end(); i++, prev_it = it, it = next_it, (next_it != tokens.end()) ? next_it++ : next_it) {
				auto& tok = *it;
				const auto& prev = (it != prev_it) ? *prev_it : m_token(none_v, -1);
				const auto& next = (next_it != tokens.end()) ? *next_it : m_token(none_v, -1);
				if ((tok.type != operator_v && tok.type != function_v) || tok.prior < with_priority) continue;
				if(tok.type == operator_v ) {
					tok.is_unary = false;
					if (tok.op_v.second == '\0' && ((i == 0 || prev.type == operator_v || prev.type == punct_v) && (tok.op_v.first == '+' || tok.op_v.first == '-') ||
						tok.op_v.first == '!' || tok.op_v.first == '~' || tok.op_v.first == 't')) {	
						tok.is_unary = true;
						//else SGL_ERROR("SGL: less than 2 args given to binary operator");
					}
					if (i + 1 == tokens.size() || (next.type == punct_v && (
						next.punct_v == '}' || next.punct_v == ','
						))) SGL_ERROR("SGL: less than 2 args given to binary operator");
				}
				operators.push_back({ &tok, {(int)i, it} });
			}

			if(operators.empty()) return;

			std::sort(operators.begin(), operators.end(), [](const ops_val_t& a, const ops_val_t& b) {
				if (a.first->prior != b.first->prior) return a.first->prior > b.first->prior;//brackets level
				if (a.first->type != b.first->type) return a.first->type > b.first->type;
				if (a.first->type == function_v) return false;
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
				if(cur->type == function_v) {
					std::vector<m_token*> args;
					size_t args_count = 0;
					for(auto it = next; it != tokens.end(); it++) {
						if(it->prior <= cur->prior) break;
						if((args_count%2 == 0 && it->type != value_v) || (args_count%2 == 1 && (it->type != punct_v || it->punct_v != ','))) 
							SGL_ERROR("SGL: invalid function args operator");
						if(args_count%2 == 0) args.push_back(&*it);
					}
					function_call(cur->function_v, &*cur, args.data(), args.size());
					auto it = cur;
					it++;
					if(args.size()) for(size_t i = 0, s = args.size() * 2 - 1; i < s; i++) 	
						it = tokens.erase(it);
				} else {
					if (v.first->is_unary) {
						switch (v.first->op_v.first) {
						case '-': unary_operator_minus(*next); break;
						case '~': unary_operator_bitwise_not(*next); break;
						case '!': unary_operator_not(*next); break;
						case '+': unary_operator_plus(*next); break;
						case 't': cast_to_type(*next, v.first->value_type); break;
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
			}

			ops_count = 0;
			if(with_priority == 0) eval_beg = tokens.end(), eval_beg--;
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
			case '[': { 
				cur_prior+=2;
				m_token t{ punct_v, cur_prior };
				t.punct_v = ch;
				tokens.push_back(t);
			} break;
			case ']': { 
				cur_prior-=2;
				eval_expr(cur_prior+1);
				if(tokens.size() < 3) SGL_ERROR("SGL: invalid array element access");
				cast_to_type(tokens.back(), t_uint64);
				uint64_t index = tokens.back().int_v.ui64;
				tokens.pop_back();
				if(tokens.back().type != punct_v || tokens.back().punct_v != '[') SGL_ERROR("SGL: invalid array element access");
				tokens.pop_back();
				if(tokens.back().type != value_v || tokens.back().value_type != t_custom) SGL_ERROR("SGL: invalid array element access");
				auto& v = tokens.back().object_v; 
				if(v.array_size <= index) SGL_ERROR("SGL: invalid array element index");
				v.array_size = 0;
				v.data = static_cast<char*>(v.data) + (v.m_type->size * index);
			} break;
			case '}': case ',': { if (cur_prior != 0) SGL_ERROR("SGL: invalid brackets sequence"); eval_expr(); }
			case '{': { m_token t{ punct_v, cur_prior }; t.punct_v = ch; tokens.push_back(t); } break;
			case '.': {
				if (!std::isdigit(in.peek())) { m_token t{ punct_v, cur_prior };t.punct_v = ch;tokens.push_back(t);break; }
			}
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
					m_token t{ value_v, cur_prior, t_float64 };
					double n = double(int_part) + double(fract_part) / double(fract_part_div);
					n *= pow(10., (exp_positive ? 1. : -1) * exp_part);
					t.float_v = n;
					tokens.push_back(t);
				}
				else {
					m_token t{ value_v, cur_prior, t_uint64 };
					t.int_v.ui64 = int_part;
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
					m_token t{ value_v, cur_prior, t_string };
					t.str_v = std::move(s);
					tokens.push_back(t);
				} else {//char
					m_token t{ value_v, cur_prior, t_char };
					if(s.size() != 1) SGL_ERROR("SGL: invalid character literal");
					t.char_v = s[0];
					tokens.push_back(t);
				}
			} break;

			default: {
				std::string s;
				while(in.good() && std::isalnum(ch)) s += ch, ch = in.get();
				if(s == "true") {
					m_token t{ value_v, cur_prior, t_bool };
					t.bool_v = true;
					tokens.push_back(t);
				} else if(s == "false") {
					m_token t{ value_v, cur_prior };
					t.bool_v = false;
					tokens.push_back(t);
				} else if(auto f = buildin_types.find(s); f != buildin_types.end()) {//typecast
					skip_comments_and_spaces(in);
					if(in.get() != '(') SGL_ERROR("SGL: missing open '(' in type cast");
					m_token t{ operator_v, cur_prior, f->second->base_type };
					t.op_v = { 't', '\0' };
					tokens.push_back(t);
					ops_count++;
				} else if(auto f = buildin_functions.find(s); f != buildin_functions.end()) {//buildin function
					skip_comments_and_spaces(in);
					if(in.get() != '(') SGL_ERROR("SGL: missing open '(' in type cast");
					m_token t{ function_v, cur_prior };
					t.function_v = &f->second;	
					tokens.push_back(t);
					ops_count++;
				} else if(auto f = cur_state->global_functions.find(s); f != cur_state->global_functions.end()) {//custom function
					skip_comments_and_spaces(in);
					if(in.get() != '(') SGL_ERROR("SGL: missing open '(' in type cast");
					m_token t{ function_v, cur_prior };
					t.function_v = &f->second;	
					tokens.push_back(t);
					ops_count++;
				}
				else {//constant or error
					if(!tokens.empty() && tokens.back().type == punct_v && tokens.back().punct_v == '.') {//object member
						if(tokens.size() < 2) SGL_ERROR("SGL: invalid '.' operator");
						tokens.pop_back();
						if(tokens.back().type != value_v || tokens.back().value_type != t_custom) SGL_ERROR("SGL: invalid '.' operator");
						auto& v = tokens.back().object_v;
						auto it = v.m_type->members.begin();
						for(; it != tokens.back().object_v.m_type->members.end(); it++) {
							if(it->name == s) break;
						}
						if(it == v.m_type->members.end()) SGL_ERROR("SGL: invalid type member");
						v.data = static_cast<char*>(v.data) + it->offset;
						v.m_type = it->m_type;
						v.array_size = it->array_size;
					}
					else {
						if(auto f1 = res->local_variables.find(s); f1 != res->local_variables.end()) {//local var
							m_token t { value_v, cur_prior, t_custom };
							t.object_v = f1->second;
							tokens.push_back(t);
						} 
						else if(auto f2 = cur_state->global_constants.find(s); f2 != cur_state->global_constants.end()) {
							m_token t { value_v, cur_prior, t_custom };
							t.object_v = f2->second;
							tokens.push_back(t);
						} else SGL_ERROR("SGL: invalid value");
					}
				}
				in.unget();
			} break;		
			}

			ch = in.get();
		}
		if (!in.good()) SGL_ERROR("SGL: unexpected eof");
		eval_expr();

		//state* cur_state, const type* t, size_t array_size, char* data
		std::function<iter(iter, const type*, size_t, char*)> get_result;
		get_result = [&](iter it, const type* t, size_t array_size, char* data) -> iter {	
			if(it->type == value_v && it->value_type == t_custom && t->base_type == t_custom) {
				if(it->object_v.m_type == t) {
					if(it->object_v.array_size != array_size) {
						SGL_ERROR("SGL: invalid array size");		
					}
					details::copy_val(t, array_size, data, static_cast<char*>(it->object_v.data));
					it++;
				} else SGL_ERROR("SGL: invalid object type");
			} 
			else {
				if(array_size) {
					if(it->type == punct_v && it->punct_v == '{') it++;
					else SGL_ERROR("SGL: excepted '{'");
				}
			for(size_t s = array_size ? array_size : 1, i = s-1, l = 0; i < s; i--, l++) {
				char* cur_data = data + l * t->size;
				if(t->base_type == t_custom) { 
					if(it->type == punct_v && it->punct_v == '{') it++;
					else SGL_ERROR("SGL: excepted '{'");
					size_t mi = t->members.size();
					for(auto& m : t->members) {
						it = get_result(it, m.m_type, m.array_size, cur_data + m.offset);
						if(--mi) {
							if(it->type == punct_v && it->punct_v == ',') it++;
							else SGL_ERROR("SGL: excepted ','");
						}
					}
					if(it->type == punct_v && it->punct_v == '}') it++;
					else SGL_ERROR("SGL: excepted '}'");	
				} else {
						switch (t->base_type) {
						case t_int8:  cast_to_type(*it,	 t_int8);  *reinterpret_cast<int8_t*>(cur_data)  = it->int_v.i8;  break;
						case t_int16: cast_to_type(*it,	 t_int16); *reinterpret_cast<int16_t*>(cur_data) = it->int_v.i16; break;
						case t_int32: cast_to_type(*it,	 t_int32); *reinterpret_cast<int32_t*>(cur_data) = it->int_v.i32; break;
						case t_int64: cast_to_type(*it,	 t_int64); *reinterpret_cast<int64_t*>(cur_data) = it->int_v.i64; break;
						case t_uint8:  cast_to_type(*it, t_uint8); *reinterpret_cast<uint8_t*>(cur_data)  = it->int_v.ui8;  break;
						case t_uint16: cast_to_type(*it, t_uint16); *reinterpret_cast<uint16_t*>(cur_data) = it->int_v.ui16; break;
						case t_uint32: cast_to_type(*it, t_uint32); *reinterpret_cast<uint32_t*>(cur_data) = it->int_v.ui32; break;
						case t_uint64: cast_to_type(*it, t_uint64); *reinterpret_cast<uint64_t*>(cur_data) = it->int_v.ui64; break;	

						case t_float32: cast_to_type(*it, t_float64); *reinterpret_cast<float*>(cur_data) = (float)it->float_v; break;
						case t_float64: cast_to_type(*it, t_float64); *reinterpret_cast<double*>(cur_data) = it->float_v; break;

						case t_string: cast_to_type(*it, t_string); *reinterpret_cast<std::string*>(cur_data) = it->str_v; break;	
						case t_cstring: {
							cast_to_type(*it, t_string);
							auto& str = *reinterpret_cast<SGL::cstring*>(cur_data);
							str.data = new char[it->str_v.size()+1];
							str.size = it->str_v.size();
							memcpy(str.data, it->str_v.data(), str.size+1);
						} break;		
						case t_char: cast_to_type(*it, t_char); *reinterpret_cast<char*>(cur_data) = it->char_v; break;		
						case t_bool: cast_to_type(*it, t_bool); *reinterpret_cast<bool*>(cur_data) = it->bool_v; break;	

						default: SGL_ERROR("SGL: invalid type"); break;
						}
						it++;
					}
					if(i) {
						if(it->type == punct_v && it->punct_v == ',') it++;
						else SGL_ERROR("SGL: excepted ','");
					}
				}
			
				if(array_size) {
					if(it->type == punct_v && it->punct_v == '}') it++;
					else SGL_ERROR("SGL: excepted '}'");
				}
			}
			return it;
		};
		auto it = get_result(tokens.begin(), t, array_size, data);
		if(it != tokens.end()) SGL_ERROR("SGL: excepted ';'");
	}

	parse_result& parse_stream(state& cur_state, std::istream& in) {
		parse_result* res = new parse_result;
		try {
			cur_state.m_results.insert(res);
			res->m_state = &cur_state;
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

				auto tf = cur_state.global_types.find(type_name);
				auto btf = buildin_types.find(type_name);
				if (tf == cur_state.global_types.end() && btf == buildin_types.end()) SGL_ERROR("SGL: invalid type name");
				const type* t = (tf != cur_state.global_types.end()) ? &(tf->second) : btf->second;

				auto& cur_val = res->local_variables[name];
				cur_val.m_type = t;
				char* data = new char[t->size];
				cur_val.data = data;

				details::construct_val(t, 0, data);

				parse_expession(&cur_state, res, t, 0, data, in);
				ch = in.get();
			}
		}
		catch(details::sgl_exception& ex) {
			SGL::error(ex.what());
		}
		catch(std::exception& ex) {
			if(sgl_pass_iternal_cxx_exceprion_in_error_callback__) SGL::error(ex.what());
			else throw ex;//rethrow std::exception
		}
		return *res;
	}
}