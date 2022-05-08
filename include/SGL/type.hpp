#pragma once
#ifndef SGL_TYPE_HPP_INCLUDE_
#define SGL_TYPE_HPP_INCLUDE_

#include "config.hpp"
#include "utils.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

#include <type_traits>

namespace SGL {
    class type_impl_base {
    public:
        virtual void default_construct(void* data) const {}
        virtual void copy_construct(void* data, const void* from) const {}
        virtual void move_construct(void* data, void* from) const {}
        
        virtual void copy_assign(void* data, const void* from) const {}
        virtual void move_assign(void* data, void* from) const {}

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
        size_t size;
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
                std::is_move_assignable_v<T>
            };
            size = sizeof(T);
        }

        virtual void default_construct(void* data) const override { 
            if constexpr (std::is_default_constructible_v<T>) new (data) T(); 
        }
        virtual void copy_construct(void* data, const void* from) const override { 
            if constexpr (std::is_copy_constructible_v<T>) new (data) T(*static_cast<const T*>(from)); 
        }
        virtual void move_construct(void* data, void* from) const override { 
            if constexpr (std::is_move_constructible_v<T>) new (data) T(std::move(*static_cast<T*>(from))); 
        }

        virtual void copy_assign(void* data, const void* from) const override {
            if constexpr (std::is_copy_assignable_v<T>) *static_cast<T*>(data) = *static_cast<const T*>(from); 
        }
        virtual void move_assign(void* data, void* from) const override {
            if constexpr (std::is_move_assignable_v<T>) *static_cast<T*>(data) = std::move(*static_cast<T*>(from)); 
        }

        //TODO add custom constructors?

        virtual void destruct(void* data) const override {
            static_cast<T*>(data)->~T();
        }


        virtual ~type_impl() {}
    };

    class state;

    class type : public no_copy {
    public:
        template<typename T>
        explicit type(sgl_type_identity<T> t, std::string_view type_name, const state* state) : m_impl(new type_impl<T>), 
            m_type_name(type_name), m_type(typeid(T)), m_state(state) {
            SGL_ASSERT(is_correct_identifier(type_name), "type name is incorrect");
        }

        ~type() {
            delete m_impl;
        }

        template<typename T> void default_construct(T& data) const { check_type<T>(); m_impl->default_construct(&data); }
        template<typename T> void copy_construct(T& data, const T& from) const { check_type<T>(); m_impl->copy_construct(&data, &from); }
        template<typename T> void move_construct(T& data, T&& from) const { check_type<T>(); m_impl->move_construct(&data, &from); }
        //TODO add custom constructors?
        template<typename T> void destruct(T& data) const { check_type<T>(); m_impl->destruct(&data); }

        
        template<typename T> void copy_assign(T& data, const T& from) const { check_type<T>(); m_impl->copy_assign(&data, &from); }
        template<typename T> void move_assign(T& data, T&& from) const { check_type<T>(); m_impl->move_assign(&data, &from); }

        template<typename T, typename U> type& add_member(const std::string& member_name, U T::*member_ptr) {
            check_type<T>();
            const auto member_t = &m_state->get_type<U>();

            add_member(member_name, member_t, m_offsetof(member_ptr));

            return *this;
        }

        template<typename U, typename T> U& get_member(T& val, const std::string& member_name) const {
            check_type<T>();
            auto [member_t, offset] = m_members.at(member_name);
            member_t->check_type<U>();
            return *reinterpret_cast<U*>(reinterpret_cast<char*>(&val) + offset);
        } 
        
    protected:
        friend class state;
        friend class value;
        friend class value_type;
        
        const type_impl_base* m_impl;
        const type_info& m_type;//used in state and in check_type
        const std::string m_type_name;
        const state* const m_state;

        std::unordered_map<std::string, std::pair<const type*, size_t>> m_members;//name, type, offset of member

        template<typename T>
        void check_type() const {
#if defined(SGL_OPTION_TYPE_CHECKS) && SGL_OPTION_TYPE_CHECKS
            SGL_ASSERT(m_type == typeid(T), "type specified in constructor call must me same with T");
#endif//SGL_OPTION_TYPE_CHECKS
        }

        //TODO not-template constructor. (for *.sgl deifned types). cannot cast value to C++ type. but can cast members?

        void add_member(const std::string& member_name, const type* member_t, size_t offset) {
            SGL_ASSERT(m_members.find(member_name) == m_members.end(), "type contains member with same name");
            m_members[member_name] = { member_t,  offset };
        }

        template<typename U, typename T> 
        static constexpr size_t m_offsetof(U T::*member_ptr) {
            return reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*member_ptr));
        }
    };

    class value_type {
    public:
        struct {
            const bool is_array      = false;
            const bool is_const      = false;
            const bool is_pointer    = false;
            const bool is_reference  = false;
            const bool is_final_type = false;
        } traits;

        template<typename T>//, std::enable_if_t<is_base_type<T>, bool> = true
        explicit value_type(sgl_type_identity<T> t, const state* state) : m_type(typeid(T)) {
            //value_type(sgl_type_identity<T>{}, state);
            std::cout << m_type.name() << std::endl;
        }

        //pointer
        template<typename T>
        explicit value_type(sgl_type_identity<T*> t, const state* state) : m_type(typeid(T*)) {
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }
        template<typename T>
        explicit value_type(sgl_type_identity<T*const> t, const state* state) : m_type(typeid(T*const)) {
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }
        template<typename T>
        explicit value_type(sgl_type_identity<T*volatile> t, const state* state) : m_type(typeid(T*volatile)) {
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }
        template<typename T>
        explicit value_type(sgl_type_identity<T*const volatile> t, const state* state) : m_type(typeid(T*const volatile)) {
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }

        //link
        template<typename T>
        explicit value_type(sgl_type_identity<T&> t, const state* state) : m_type(typeid(T&)) {
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }
        template<typename T>
        explicit value_type(sgl_type_identity<T&&> t, const state* state) : m_type(typeid(T&&)) {
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }

        //array
        template<typename T, size_t N>
        explicit value_type(sgl_type_identity<T[N]> t, const state* state) : m_type(typeid(T[N])) {
            //function can't return value as T[N]. value get<T[N]> return T*? or pair <T*, size>?
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }
        template<typename T>
        explicit value_type(sgl_type_identity<T[]> t, const state* state) : m_type(typeid(T[])) {
            //actually T[] same as T*
            //array size from value. return value as T*?
            std::cout << m_type.name() << std::endl;
            value_type(sgl_type_identity<T>{}, state);
        }
    
    protected:
        friend class state;
        friend class value;
        friend class type;
        
        union {
            const type* type_v = nullptr;       
            const value_type* value_type_v;
        };
        const type_info& m_type;
    };


} // namespace SGL

#endif// SGL_TYPE_HPP_INCLUDE_