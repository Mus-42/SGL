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

#include <cstdint>
#include <cmath>
#include <cassert>

namespace SGL {
    enum SGL_type : uint8_t {
        t_void = 0,
        t_int8, t_int16, t_int32, t_int64,
        t_uint8, t_uint16, t_uint32, t_uint64,
        t_float32, t_float64,
        t_bool,
        t_string,
        t_char,

        t_custom
    };
    static_assert(sizeof(float) == 4 && sizeof(double) == 8);//for t_float32 & t_float64

    constexpr uint8_t type_size[]{
        0,//void
        1, 2, 4, 8, 1, 2, 4, 8,//int types
        4, 8, 1, (uint8_t)sizeof(std::string), 1
    };

    struct type {
        type(SGL_type t = t_void) : base_type(t), size(type_size[t]) {}
        type(SGL_type t, void* v1, void* v2, void* v3) : base_type(t), size(type_size[t]), m_construct(v1), m_destruct(v2), m_copy(v3) {}
        ~type() = default;

        struct member {
            member() : type(t_void), name(), offset(0), m_type(nullptr) {}
            member(const std::string& name, SGL_type type, size_t offset, size_t array_size = 0) : type(type), name(name), offset(offset),
                m_type(nullptr), array_size(array_size) {}
            member(const std::string& name, const std::string& custom_type_name, size_t offset, size_t array_size = 0) : type(t_custom), name(name),
                custom_type_name(custom_type_name), offset(offset), m_type(nullptr), array_size(array_size) {}

            SGL_type type;
            size_t offset, array_size;
            const SGL::type* m_type;
            std::string name, custom_type_name;
        };
        SGL_type base_type;//t_custom for custom
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

    struct state {
        state() = default;
        ~state();
        std::unordered_map<std::string, value> global_constants;
        std::unordered_map<std::string, type> global_types;
        std::set<parse_result*> m_results;
    };

    struct parse_result {
        parse_result() = default;
        ~parse_result();
        state* m_state = nullptr;
        std::unordered_map<std::string, value> local_variables;
    };
    
    namespace details {
        template<typename T> using t_construct_noptr = void(T*);//this
        template<typename T> using t_construct = t_construct_noptr<T>*;

        template<typename T> using t_destruct_noptr = void(T*);//this
        template<typename T> using t_destruct = t_destruct_noptr<T>*;

        template<typename T> using t_copy_noptr = void(T*, T*);//this, other
        template<typename T> using t_copy = t_copy_noptr<T>*;

        void construct_val(const type* t, size_t arr_size, void* v);
        void destruct_val(const type* t, size_t arr_size, void* v);
        void copy_val(const type* t, size_t arr_size, void* v, void* from);

        value* get_local_value(parse_result* p, const std::string& name);
        void register_struct(state* s, const std::string& name, size_t size, std::vector<type::member>&& members, void*, void*, void*);
    };

    template<typename T>
    void register_struct(state* s, const std::string& name, std::vector<type::member>&& members,
        details::t_construct<T> v1 = nullptr, details::t_destruct<T> v2 = nullptr, details::t_copy<T> v3 = nullptr
    ) {
        details::register_struct(s, name, sizeof(T), std::move(members), (void*)v1, (void*)v2, (void*)v3);
    }


    parse_result* parse_stream(state* s, std::istream& in);


    template<typename T>
    inline T get_local_value(parse_result* p, const std::string& name) {
        value* v = details::get_local_value(p, name);
        assert(v && v->data && sizeof(T) == v->m_type->size);
        T val;
        details::copy_val(v->m_type, 0, &val, v->data);
        return val;
    }
};

#endif//SGL_INCLUDE_HPP_