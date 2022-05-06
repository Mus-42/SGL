#pragma once
#ifndef SGL_VALUE_HPP_INCLUDE_
#define SGL_VALUE_HPP_INCLUDE_

#include <cstdint>
#include <string>
#include <vector>

#include <type_traits>

namespace SGL {
    /*
    enum primitive_type : uint8_t {
        t_void,

        t_int8, t_int16, t_int32, t_int64,
        t_uint8, t_uint16, t_uint32, t_uint64,

        t_bool,

        t_float32, t_float64,

        t_char,
        t_string,

        _primitive_types_count,
        //aliases
        t_float = t_float32,
        t_double = t_float64,
        t_int = t_int32,
        t_uint = t_uint32,
    };

    class custom_type {//passed from C++ or from SGL
    public:
        
    };

    class type {
    public:
        constexpr type(primitive_type t = t_void) : prim(t), cust(nullptr) {}
        constexpr type(const std::string& type_name) : prim(t_void), cust(nullptr) {}
        constexpr type(const custom_type* type) : prim(t_void), cust(type) {}
    protected:
        const primitive_type prim;
        const custom_type* cust;
    };*/
    
    class type_impl_base {
    public:
        virtual void default_construct(void* data) const {}
        virtual void copy_construct(void* data, const void* from) const {}
        virtual void move_construct(void* data, void* from) const {}
        //TODO add custom constructors?
        virtual void destruct(void* data) const {}

        virtual ~type_impl_base() {}
        struct {
            bool is_default_constructible = false;
            bool is_copy_constructible = false;
            bool is_move_constructible = false;

            bool is_copy_assignable = false;
            bool is_move_assignable = false;

            //TODO add is_ref | is_pointer?
        } traits;
    };
    template<typename T>
    class type_impl : public type_impl_base {
    public:
        constexpr type_impl() {
            traits = {
                std::is_default_constructible_v<T>,
                std::is_copy_constructible_v<T>,
                std::is_move_constructible_v<T>,

                std::is_copy_assignable_v<T>,
                std::is_move_assignable_v<T>,

            };
        }

        virtual void default_construct(void* data) const override { new (data) T(); }
        virtual void copy_construct(void* data, const void* from) const override { new (data) T(*static_cast<const T*>(from)); }
        virtual void move_construct(void* data, void* from) const override { new (data) T(std::move(*static_cast<T*>(from))); }
        //TODO add custom constructors?
        virtual void destruct(void* data) const override {
            static_cast<T*>(data)->~T();
        }


        virtual ~type_impl() {}
    };

    template<typename T> struct sgl_type_identity {};

    class type {
    public:
        template<typename T>
        explicit type(sgl_type_identity<T> t) : m_impl(new type_impl<T>) {}

        
        //no copy
        type(const type&) = delete;
        type& operator=(const type&) = delete;
        //but move
        type(type&&) = default;
        type& operator=(type&&) = default;

        type_impl_base* m_impl;
    };
} // namespace SGL

#endif// SGL_VALUE_HPP_INCLUDE_