/*
    SGL - Simplified GenLang
*/

#pragma once
#ifndef SGL_INCLUDE_HPP_
#define SGL_INCLUDE_HPP_ 1

#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <type_traits>
#include <functional>

#include <cstdint>
#include <cmath>
#include <cassert>

#ifndef SGL_ASSERT
    #define SGL_ASSERT(v) { if(!(v)) { SGL::error("assertion failed in " + std::to_string(__LINE__)); } }
#endif

namespace SGL {
    enum primitive_type : uint8_t {
        t_void = 0,
        t_int8, t_int16, t_int32, t_int64,
        t_uint8, t_uint16, t_uint32, t_uint64,
        t_float32, t_float64,
        t_bool,
        t_string,//std::string
        t_cstring,//sgl_cstring struct. for compatibility with C 
        t_char,

        t_custom
    };
    struct cstring {
        const char* data = nullptr;
        size_t size = 0;
    };
    struct type {
        struct member {
            member() : type(t_void), name(), offset(0), m_type(nullptr) {}
            member(const std::string& name, primitive_type type, size_t offset, size_t array_size = 0) : type(type), name(name), offset(offset),
                m_type(nullptr), array_size(array_size) {}
            member(const std::string& name, const std::string& custom_type_name, size_t offset, size_t array_size = 0) : type(t_custom), name(name),
                custom_type_name(custom_type_name), offset(offset), m_type(nullptr), array_size(array_size) {}

            primitive_type type;
            size_t offset, array_size;
            const SGL::type* m_type;
            std::string name, custom_type_name;
        };
        primitive_type base_type;//t_custom for custom
        //for custom:
        std::vector<member> members;
        size_t size;
        //function pointers
        void* m_construct;
        void* m_destruct;
        void* m_copy;
    };

    struct value {
        const type* m_type = nullptr;
        size_t array_size = 0;
        void* data = nullptr;
    };

    struct parse_result;
    struct state;

    //using error_callback_t = void(*)(const std::string&);//string -> error description
    using error_callback_t = std::function<void(const std::string&)>;//string -> error description
    void set_error_callback(error_callback_t f);
    void error(const std::string& description);

    struct function {
        struct function_overload {
            void* ptr;
            primitive_type ret_type;
            std::vector<primitive_type> args_types;
            //only for primitive types args & ret value
            function_overload(void* f_ptr, primitive_type ret_type, std::vector<primitive_type>&& args_types) : ptr(f_ptr), ret_type(ret_type), args_types(std::move(args_types)) {
                if(args_types.size() > 3) SGL::error("SGL: SGL functions cant take more than 3 args");//replase with SGL_ASSERT?
            }
        };
        function() = default;
        function(function&&) = default;
        function(const function&) = default;
        function& operator=(function&&) = default;
        function& operator=(const function&) = default;
        function(std::vector<function_overload>&& overloads) : m_overloads(std::move(overloads)) {
            if(m_overloads.empty()) SGL::error("SGL: no function overloads given");
        }
        std::vector<function_overload> m_overloads;//TODO sort it or store overloads by args count??
    };

    namespace details {
        template<typename T> using t_construct = void(*)(T*);//this
        template<typename T> using t_destruct = void(*)(T*);//this
        template<typename T> using t_copy = void(*)(T*, T*);//this, other

        void construct_val(const type* t, size_t arr_size, void* v);
        void destruct_val(const type* t, size_t arr_size, void* v);
        void copy_val(const type* t, size_t arr_size, void* v, void* from);

        value* get_local_value(parse_result& p, const std::string& name);
        type& register_struct(state& s, const std::string& name, size_t size, std::vector<type::member>&& members, void*, void*, void*);

        void set_global_variable(state& s, const std::string& variable_name, primitive_type t, void* data, size_t array_size);
        void set_global_variable(state& s, const std::string& variable_name, const std::string& type_name, void* data, size_t array_size);

        //TODO type& get_type_ ...

        void pass_iternal_cxx_exceprion_in_error_callback(bool pass);//by default false
    };

    bool contains(parse_result& p, const std::string& name);
    bool is_array(parse_result& p, const std::string& name);

    bool is_primitive_type(parse_result& p, const std::string& name);
    bool is_custom_type(parse_result& p, const std::string& name);
    
    bool is_same_primitive_type(parse_result& p, const std::string& name, primitive_type t);
    bool is_same_custom_type(parse_result& p, const std::string& name, const std::string& type_name);

    template<typename T>
    type& register_struct(state& s, const std::string& name, std::vector<type::member>&& members,
        details::t_construct<T> v1 = nullptr, details::t_destruct<T> v2 = nullptr, details::t_copy<T> v3 = nullptr
    ) {
        return details::register_struct(s, name, sizeof(T), std::move(members), (void*)v1, (void*)v2, (void*)v3);
    }

    parse_result& parse_stream(state& s, std::istream& in);

    template<typename T>
    inline void get_local_value(parse_result& p, const std::string& name, T& val) {
        value* v = details::get_local_value(p, name);
        SGL_ASSERT(v && v->data && sizeof(T) == v->m_type->size);
        details::copy_val(v->m_type, 0, &val, v->data);
    }
    template<typename T>
    inline T get_local_value(parse_result& p, const std::string& name) {  
        T val;
        get_local_value<T>(p, name, val);
        return val;
    }

    
    template<typename T>//pair array pointer & size_t size
    inline std::pair<T*, size_t> get_local_value_array(parse_result& p, const std::string& name) {
        value* v = details::get_local_value(p, name);
        SGL_ASSERT(v && v->data && sizeof(T) == v->m_type->size && );
        return { static_cast<T*>(v->data), v->array_size };
    }
    template<typename T>//pair array pointer & size_t size
    inline void get_local_value_array(parse_result& p, const std::string& name, T* array, size_t array_size) {
        value* v = details::get_local_value(p, name);
        SGL_ASSERT(v && v->data && sizeof(T) == v->m_type->size && array_size <= v->array_size);
        details::copy_val(v->m_type, array_size, array, v->data);
    }

    struct state {
        state() = default;
        ~state();

        std::unordered_map<std::string, value> global_constants;
        std::unordered_map<std::string, type> global_types;
        std::unordered_map<std::string, function> global_functions;
        std::set<parse_result*> m_results;

        template<typename T>
        type& register_struct(const std::string& name, std::vector<type::member>&& members,
            details::t_construct<T> v1 = nullptr, details::t_destruct<T> v2 = nullptr, details::t_copy<T> v3 = nullptr
        ) {
            return details::register_struct(*this, name, sizeof(T), std::move(members), (void*)v1, (void*)v2, (void*)v3);
        }

        parse_result& parse_stream(std::istream& in) {
            return SGL::parse_stream(*this, in);
        }

        void set_global_variable(const std::string& variable_name, primitive_type t, void* data, size_t array_size = 0) {
            details::set_global_variable(*this, variable_name, t, data, array_size);
        }
        void set_global_variable(const std::string& variable_name, const std::string& type_name, void* data, size_t array_size = 0) {
            details::set_global_variable(*this, variable_name, type_name, data, array_size);
        }

        void add_function(const std::string& name, const function& f) {
            global_functions[name] = f;
        }
    };

    struct parse_result {
        parse_result() = default;
        ~parse_result();

        state* m_state = nullptr;
        std::unordered_map<std::string, value> local_variables;

        template<typename T>
        T get_local_value(const std::string& name) {
            return SGL::get_local_value<T>(*this, name);
        }
        template<typename T>
        void get_local_value(const std::string& name, T& val) {
            SGL::get_local_value<T>(*this, name, val);
        }


        template<typename T>//pair array pointer & size_t size
        inline std::pair<T*, size_t> get_local_value_array(const std::string& name) {
            return get_local_value_array<T>(*this, name);
        }
        template<typename T>
        inline void get_local_value_array(const std::string& name, T* array, size_t array_size) {
            get_local_value_array<T>(*this, name, array, array_size);
        }

        
        bool contains(const std::string& name) {
            return SGL::contains(*this, name);
        }
        bool is_array(const std::string& name) {
            return SGL::is_array(*this, name);
        }

        bool is_primitive_type(const std::string& name) {
            return SGL::is_primitive_type(*this, name);
        }
        bool is_custom_type(const std::string& name) {
            return SGL::is_custom_type(*this, name);
        }
        bool is_same_primitive_type(const std::string& name, primitive_type t) {
            return SGL::is_same_primitive_type(*this, name, t);
        }
        bool is_same_custom_type(const std::string& name, const std::string& type_name) {
            return SGL::is_same_custom_type(*this, name, type_name);
        }
    };
};

#endif//SGL_INCLUDE_HPP_